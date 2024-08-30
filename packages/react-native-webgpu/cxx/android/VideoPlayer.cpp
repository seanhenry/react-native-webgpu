#include "VideoPlayer.h"

#include <memory>

#include "ArrayBufferUtils.h"
#include "NdkVideoPlayer.h"

using namespace wgpu;

namespace wgpu {

class PixelBuffer : public HostObject {
 public:
  PixelBuffer(YUVPixelBuffer pixelBuffer) : _pixelBuffer(pixelBuffer) {}
  ~PixelBuffer() override { release(); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;

 private:
  void release();
  YUVPixelBuffer _pixelBuffer;
};

// region VideoPlayer

inline NdkVideoPlayer *getVideoPlayer(void *videoPlayer) { return (NdkVideoPlayer *)videoPlayer; }

VideoPlayer::VideoPlayer(const std::string &url) {
  auto player = new NdkVideoPlayer(url);
  this->videoPlayer = player;
}

VideoPlayer::~VideoPlayer() { release(); }

void VideoPlayer::release() {
  if (this->videoPlayer != nullptr) {
    delete (NdkVideoPlayer *)this->videoPlayer;
    this->videoPlayer = nullptr;
  }
}

Value VideoPlayer::get(Runtime &runtime, const PropNameID &name) {
  auto propName = name.utf8(runtime);

  if (propName == "nextPixelBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(nextPixelBuffer, 0, [this]) {
      auto player = getVideoPlayer(videoPlayer);
      size_t size;
      uint32_t width, height;
      auto pixelBuffer = player->copyPixelBuffer();
      if (pixelBuffer.buffer == nullptr) {
        return Value::null();
      }
      auto host = std::make_shared<PixelBuffer>(pixelBuffer);
      return Object::createFromHostObject(runtime, host);
    });
  }

  if (propName == "seek") {
    return WGPU_FUNC_FROM_HOST_FUNC(seek, 1, [this]) {
      auto position = arguments[0].asNumber();
      getVideoPlayer(videoPlayer)->seek(position);
      return Value::undefined();
    });
  }

  if (propName == "play") {
    return WGPU_FUNC_FROM_HOST_FUNC(play, 0, [this]) {
      getVideoPlayer(videoPlayer)->resume();
      return Value::undefined();
    });
  }

  if (propName == "release") {
    return WGPU_FUNC_FROM_HOST_FUNC(release, 0, [this]) {
      release();
      return Value::undefined();
    });
  }

  return Value::undefined();
}

std::vector<PropNameID> VideoPlayer::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "nextPixelBuffer", "seek", "play", "release");
}

// endregion
// region PixelBuffer

void PixelBuffer::release() {
  if (_pixelBuffer.buffer != nullptr) {
    free(_pixelBuffer.buffer);
    _pixelBuffer.buffer = nullptr;
  }
}

Value PixelBuffer::get(Runtime &runtime, const PropNameID &name) {
  auto propName = name.utf8(runtime);

  if (propName == "arrayBuffer") {
    return createUnownedArrayBuffer(runtime, _pixelBuffer.buffer, _pixelBuffer.size);
  }

  if (propName == "bytesPerRow") {
    return (int)_pixelBuffer.bytesPerRow;
  }

  if (propName == "getArrayBufferOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getArrayBufferOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      auto offset = 0;
      auto size = _pixelBuffer.width * _pixelBuffer.height;
      if (planeIndex == 1) {
        offset = size;
        size /= _pixelBuffer.isInterleaved ? 2 : 4;
      } else if (planeIndex == 2) {
        offset = size + size / 4;
        size /= 4;
      }
      return createUnownedArrayBuffer(runtime, _pixelBuffer.buffer + offset, size);
    });
  }

  if (propName == "getBytesPerRowOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getBytesPerRowOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      if (planeIndex == 0) {
        return _pixelBuffer.width;
      }
      return _pixelBuffer.isInterleaved ? _pixelBuffer.width : _pixelBuffer.width / 2;
    });
  }

  if (propName == "getWidthOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getWidthOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      if (planeIndex == 0) {
        return _pixelBuffer.width;
      }
      return _pixelBuffer.isInterleaved ? _pixelBuffer.width : _pixelBuffer.width / 2;
    });
  }

  if (propName == "getHeightOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getHeightOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      if (planeIndex == 0) {
        return _pixelBuffer.height;
      }
      return _pixelBuffer.height / 2;
    });
  }

  if (propName == "planeCount") {
    return _pixelBuffer.isInterleaved ? 2 : 3;
  }

  if (propName == "width") {
    return (int)_pixelBuffer.width;
  }

  if (propName == "height") {
    return (int)_pixelBuffer.height;
  }

  if (propName == "isInterleaved") {
    return _pixelBuffer.isInterleaved;
  }

  if (propName == "release") {
    return WGPU_FUNC_FROM_HOST_FUNC(release, 0, [this]) {
      release();
      return Value::undefined();
    });
  }
  return Value::undefined();
}

std::vector<PropNameID> PixelBuffer::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "getArrayBufferOfPlane", "getBytesPerRowOfPlane", "getWidthOfPlane",
                           "getHeightOfPlane", "planeCount", "width", "height", "isInterleaved", "release");
}

}  // namespace wgpu

// endregion
