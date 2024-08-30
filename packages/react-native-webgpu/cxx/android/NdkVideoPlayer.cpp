#include "NdkVideoPlayer.h"

#include <jsi/jsi.h>
#include <unistd.h>

#include <algorithm>
#include <thread>

#include "WGPULog.h"

// See for format codes
// https://developer.android.com/reference/android/media/MediaCodecInfo.CodecCapabilities#COLOR_FormatYUV420SemiPlanar
#define COLOR_FormatYUV420Planar 19
#define COLOR_FormatYUV420SemiPlanar 21

#define NDK_VIDEO_PLAYER_MESSAGE_QUIT 0
#define NDK_VIDEO_PLAYER_MESSAGE_DECODE 1
#define NDK_VIDEO_PLAYER_MESSAGE_SEEK 2
#define NDK_VIDEO_PLAYER_MESSAGE_RESUME 3

#define SECS_TO_USECS 1000000.0

using namespace facebook::jsi;

namespace wgpu {

template <typename T>
inline double clamp(T in, T min, T max) {
  return in > max ? max : in < min ? min : in;
}

inline int64_t systemnanotime() {
  timespec now{};
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000000000LL + now.tv_nsec;
}

NdkVideoPlayer::NdkVideoPlayer(const std::string &url) {
  sem_init(&_pixelBufferSemaphore, 0, 1);
  sem_init(&_queueSemaphore, 0, 1);
  _extractor = AMediaExtractor_new();
  auto status = AMediaExtractor_setDataSource(_extractor, url.data());
  if (status != AMEDIA_OK) {
    throw JSINativeException("Failed to load media from url");
  }
  auto numTracks = AMediaExtractor_getTrackCount(_extractor);
  for (size_t i = 0; i < numTracks; i++) {
    auto format = AMediaExtractor_getTrackFormat(_extractor, i);
    const char *mime;
    if (!AMediaFormat_getString(format, AMEDIAFORMAT_KEY_MIME, &mime)) {
      AMediaFormat_delete(format);
      throw JSINativeException("Mime not found in media");
    }
    if (strncmp(mime, "video/", 6) == 0) {
      status = AMediaExtractor_selectTrack(_extractor, i);
      if (status != AMEDIA_OK) {
        AMediaFormat_delete(format);
        throw JSINativeException("Could not select video track");
      }
      _codec = AMediaCodec_createDecoderByType(mime);
      status = AMediaCodec_configure(_codec, format, nullptr, nullptr, 0);
      if (status != AMEDIA_OK) {
        AMediaFormat_delete(format);
        AMediaCodec_delete(_codec);
        throw JSINativeException("Failed to configure media codec");
      }
      status = AMediaCodec_start(_codec);
      if (status != AMEDIA_OK) {
        throw JSINativeException("Media codec failed to start");
      }
    }
    AMediaFormat_delete(format);
  }
  if (_codec == nullptr) {
    throw JSINativeException("No video track was found");
  }
  appendMessage(NDK_VIDEO_PLAYER_MESSAGE_DECODE);
  loop();
}

NdkVideoPlayer::~NdkVideoPlayer() {
  appendMessage(NDK_VIDEO_PLAYER_MESSAGE_QUIT, true);
  sem_destroy(&_queueSemaphore);
  sem_wait(&_pixelBufferSemaphore);
  deletePixelBuffer();
  sem_destroy(&_pixelBufferSemaphore);
  AMediaCodec_stop(_codec);
  AMediaCodec_delete(_codec);
  AMediaExtractor_delete(_extractor);
}

void NdkVideoPlayer::loop() {
  std::thread([this] {
    while (true) {
      sem_wait(&_queueSemaphore);
      if (_messages.empty()) {
        sem_post(&_queueSemaphore);
        continue;
      }
      auto message = _messages.front();
      _messages.pop();
      sem_post(&_queueSemaphore);

      if (message == NDK_VIDEO_PLAYER_MESSAGE_DECODE) {
        decode();
      } else if (message == NDK_VIDEO_PLAYER_MESSAGE_SEEK) {
        AMediaExtractor_seekTo(_extractor, (int64_t)(_loopData.seekPosition * SECS_TO_USECS),
                               AMEDIAEXTRACTOR_SEEK_CLOSEST_SYNC);
        AMediaCodec_flush(_codec);
        _startTime = -1;
        if (_loopData.isOutputEOS) {
          _loopData.isInputEOS = false;
          _loopData.isOutputEOS = false;
          appendMessage(NDK_VIDEO_PLAYER_MESSAGE_DECODE);
        }
      } else if (message == NDK_VIDEO_PLAYER_MESSAGE_RESUME) {
        // TODO: video player is always playing
      } else if (message == NDK_VIDEO_PLAYER_MESSAGE_QUIT) {
        break;
      }
    }
  }).detach();
}

void NdkVideoPlayer::seek(double position) {
  _loopData.seekPosition = position;
  appendMessage(NDK_VIDEO_PLAYER_MESSAGE_SEEK);
}

void NdkVideoPlayer::resume() { appendMessage(NDK_VIDEO_PLAYER_MESSAGE_RESUME); }

void NdkVideoPlayer::appendMessage(int message, bool deleteAll) {
  sem_wait(&_queueSemaphore);
  if (deleteAll) {
    while (!_messages.empty()) _messages.pop();
  }
  _messages.push(message);
  sem_post(&_queueSemaphore);
}

void NdkVideoPlayer::decode() {
  if (!_loopData.isInputEOS) {
    auto inputIndex = AMediaCodec_dequeueInputBuffer(_codec, 2000);
    if (inputIndex >= 0) {
      size_t size = 0;
      auto buffer = AMediaCodec_getInputBuffer(_codec, inputIndex, &size);
      auto sampleSize = AMediaExtractor_readSampleData(_extractor, buffer, size);
      if (sampleSize < 0) {
        _loopData.isInputEOS = true;
        sampleSize = 0;
      }
      auto time = AMediaExtractor_getSampleTime(_extractor);
      auto status = AMediaCodec_queueInputBuffer(_codec, inputIndex, 0, sampleSize, time,
                                                 _loopData.isInputEOS ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);
      if (status != AMEDIA_OK) {
        WGPU_LOG_ERROR("Failed to queue input buffer");
      }
      if (!AMediaExtractor_advance(_extractor)) {
        WGPU_LOG_ERROR("Failed to advance media");
      }
    }
  }

  if (!_loopData.isOutputEOS) {
    AMediaCodecBufferInfo info;
    auto outputIndex = AMediaCodec_dequeueOutputBuffer(_codec, &info, 0);
    if (outputIndex >= 0) {
      if (info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) {
        _loopData.isOutputEOS = true;
      }
      int64_t presentationNano = info.presentationTimeUs * 1000;
      if (_startTime < 0) {
        _startTime = systemnanotime() - presentationNano;
      }
      int64_t delay = (_startTime + presentationNano) - systemnanotime();
      if (delay > 0) {
        usleep(delay / 1000);
      }
      size_t size = 0;
      auto buffer = AMediaCodec_getOutputBuffer(_codec, outputIndex, &size);

      sem_wait(&_pixelBufferSemaphore);
      replaceBuffer(buffer, outputIndex, size);
      sem_post(&_pixelBufferSemaphore);
    } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED) {
      WGPU_LOG_INFO("output buffers changed");
    } else if (outputIndex == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED) {
      auto format = AMediaCodec_getOutputFormat(_codec);
      sem_wait(&_pixelBufferSemaphore);
      setOutputFormatInfo(format);
      sem_post(&_pixelBufferSemaphore);
      WGPU_LOG_INFO("format changed to: %s", AMediaFormat_toString(format));
      AMediaFormat_delete(format);
    } else if (outputIndex == AMEDIACODEC_INFO_TRY_AGAIN_LATER) {
      WGPU_LOG_INFO("no output buffer right now");
    } else {
      WGPU_LOG_ERROR("unexpected info code: %zd", outputIndex);
    }
  }

  if (!_loopData.isInputEOS || !_loopData.isOutputEOS) {
    appendMessage(NDK_VIDEO_PLAYER_MESSAGE_DECODE);
  }
}

YUVPixelBuffer NdkVideoPlayer::copyPixelBuffer() {
  sem_wait(&_pixelBufferSemaphore);
  YUVPixelBuffer buffer;

  if (_pixelBufferData.colorFormat != COLOR_FormatYUV420SemiPlanar &&
      _pixelBufferData.colorFormat != COLOR_FormatYUV420Planar) {
    WGPU_LOG_ERROR("Unsupported color format: %i", _pixelBufferData.colorFormat);
  } else if (_pixelBufferData.buffer != nullptr) {
    buffer.buffer = (uint8_t *)malloc(_pixelBufferData.size);
    memcpy(buffer.buffer, _pixelBufferData.buffer, _pixelBufferData.size);
    buffer.size = _pixelBufferData.size;
    buffer.width = _pixelBufferData.width;
    buffer.height = _pixelBufferData.height;
    buffer.bytesPerRow = (uint32_t)_pixelBufferData.width;
    buffer.isInterleaved = _pixelBufferData.colorFormat == COLOR_FormatYUV420SemiPlanar;
  }
  deletePixelBuffer();
  sem_post(&_pixelBufferSemaphore);
  return buffer;
}
void NdkVideoPlayer::deletePixelBuffer() {
  if (_pixelBufferData.buffer != nullptr) {
    AMediaCodec_releaseOutputBuffer(_codec, _pixelBufferData.bufferIndex, false);
    _pixelBufferData.buffer = nullptr;
  }
}

void NdkVideoPlayer::replaceBuffer(uint8_t *buffer, size_t index, size_t size) {
  deletePixelBuffer();
  _pixelBufferData.buffer = buffer;
  _pixelBufferData.bufferIndex = index;
  _pixelBufferData.size = size;
}

void NdkVideoPlayer::setOutputFormatInfo(AMediaFormat *format) {
  AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_WIDTH, &_pixelBufferData.width);
  AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_HEIGHT, &_pixelBufferData.height);
  AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_COLOR_FORMAT, &_pixelBufferData.colorFormat);
  AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_MAX_WIDTH, &_pixelBufferData.maxWidth);
  AMediaFormat_getInt32(format, AMEDIAFORMAT_KEY_MAX_HEIGHT, &_pixelBufferData.maxHeight);
}

}  // namespace wgpu