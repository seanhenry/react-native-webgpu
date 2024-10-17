#pragma once

#include <semaphore.h>

#include <queue>
#include <string>

#include "media/NdkMediaCodec.h"
#include "media/NdkMediaExtractor.h"

namespace wgpu {
typedef struct PixelBufferData {
  uint8_t *buffer = nullptr;
  size_t bufferIndex = -1;
  size_t size = -1;
  int32_t width = -1;
  int32_t height = -1;
  int32_t maxWidth = -1;
  int32_t maxHeight = -1;
  int32_t colorFormat = -1;
} PixelBufferData;

typedef struct YUVPixelBuffer {
  uint8_t *buffer = nullptr;
  size_t size = 0;
  int32_t width = 0;
  int32_t height = 0;
  uint32_t bytesPerRow = 0;
  bool isInterleaved = false;
} YUVPixelBuffer;

typedef struct LoopData {
  bool isInputEOS = false;
  bool isOutputEOS = false;
  double seekPosition = 0.0;
} LoopData;

class NdkVideoPlayer {
 public:
  NdkVideoPlayer(const std::string &url);
  ~NdkVideoPlayer();
  YUVPixelBuffer copyPixelBuffer();
  void seek(double position);
  void resume();

 private:
  void deletePixelBuffer();
  void loop();
  void appendMessage(int message, bool deleteAll = false);
  void decode();
  void replaceBuffer(uint8_t *buffer, size_t index, size_t size);
  void setOutputFormatInfo(AMediaFormat *format);

  AMediaCodec *_codec = nullptr;
  AMediaExtractor *_extractor = nullptr;
  PixelBufferData _pixelBufferData;
  LoopData _loopData;
  int64_t _startTime = -1;
  sem_t _pixelBufferSemaphore{};
  sem_t _queueSemaphore{};
  std::queue<int> _messages;
};
}  // namespace wgpu