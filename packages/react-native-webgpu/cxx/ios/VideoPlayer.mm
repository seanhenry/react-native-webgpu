#include "VideoPlayer.h"
#import <AVFoundation/AVFoundation.h>
#include "ArrayBufferUtils.h"

using namespace wgpu;

namespace wgpu {

class PixelBuffer : public HostObject {
 public:
  PixelBuffer(CVPixelBufferRef pixelBuffer);
  ~PixelBuffer();
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;

 private:
  void release();
  CVPixelBufferRef _pixelBuffer;
};

}  // namespace wgpu

#pragma mark - WGPUVideoPlayer

@interface WGPUVideoPlayer : NSObject

@property(nonatomic) AVPlayer *player;
@property(nonatomic) AVPlayerItem *playerItem;
@property(nonatomic) AVPlayerItemVideoOutput *output;

@end

@implementation WGPUVideoPlayer

- (instancetype)initWithURL:(NSString *)urlString {
  self = [super init];
  if (self) {
    [self loadWithURL:urlString];
  }
  return self;
}

- (void)loadWithURL:(NSString *)urlString {
  AVURLAsset *asset = [AVURLAsset assetWithURL:[NSURL URLWithString:urlString]];
  self.playerItem = [AVPlayerItem playerItemWithAsset:asset];

  self.output = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:@{
    (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_420YpCbCr8Planar),
  }];
  [self.playerItem addOutput:self.output];
  self.player = [AVPlayer playerWithPlayerItem:self.playerItem];
  [self.player play];
}

- (CVPixelBufferRef)nextPixelBuffer {
  CMTime time = self.playerItem.currentTime;
  if ([self.output hasNewPixelBufferForItemTime:time]) {
    return [self.output copyPixelBufferForItemTime:time itemTimeForDisplay:nil];
  }
  return nil;
}

- (void)seek:(Float64)position {
  [self.player seekToTime:CMTimeMakeWithSeconds(position, 60)];
}

- (void)play {
  [self.player setRate:1.0];
}

@end

namespace wgpu {

#pragma mark - VideoPlayer

inline WGPUVideoPlayer *getVideoPlayer(void *videoPlayer) { return (__bridge WGPUVideoPlayer *)videoPlayer; }

VideoPlayer::VideoPlayer(const std::string &url) {
  auto player = [[WGPUVideoPlayer alloc] initWithURL:[NSString stringWithCString:url.data()
                                                                        encoding:NSUTF8StringEncoding]];
  this->videoPlayer = (void *)CFBridgingRetain(player);
}

VideoPlayer::~VideoPlayer() { release(); }

void VideoPlayer::release() {
  if (this->videoPlayer != nullptr) {
    CFBridgingRelease(this->videoPlayer);
    this->videoPlayer = nullptr;
  }
}

Value VideoPlayer::get(Runtime &runtime, const PropNameID &name) {
  auto propName = name.utf8(runtime);

  if (propName == "nextPixelBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(nextPixelBuffer, 0, [this]) {
      auto player = getVideoPlayer(videoPlayer);
      auto pixelBuffer = [player nextPixelBuffer];
      if (pixelBuffer == nullptr) {
        return Value::null();
      }
      auto host = std::make_shared<PixelBuffer>(pixelBuffer);
      return Object::createFromHostObject(runtime, host);
    });
  }

  if (propName == "seek") {
    return WGPU_FUNC_FROM_HOST_FUNC(seek, 1, [this]) {
      auto position = arguments[0].asNumber();
      auto player = getVideoPlayer(videoPlayer);
      [player seek:position];
      return Value::undefined();
    });
  }

  if (propName == "play") {
    return WGPU_FUNC_FROM_HOST_FUNC(play, 0, [this]) {
      auto player = getVideoPlayer(videoPlayer);
      [player play];
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

#pragma mark - PixelBuffer

PixelBuffer::PixelBuffer(CVPixelBufferRef pixelBuffer) : _pixelBuffer(pixelBuffer) {
  CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
}

PixelBuffer::~PixelBuffer() { release(); }

void PixelBuffer::release() {
  if (_pixelBuffer != nullptr) {
    CVPixelBufferUnlockBaseAddress(_pixelBuffer, kCVPixelBufferLock_ReadOnly);
    CVPixelBufferRelease(_pixelBuffer);
    _pixelBuffer = nullptr;
  }
}

Value PixelBuffer::get(Runtime &runtime, const PropNameID &name) {
  auto propName = name.utf8(runtime);

  if (propName == "getArrayBufferOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getArrayBufferOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      auto yPlane = CVPixelBufferGetBaseAddressOfPlane(_pixelBuffer, planeIndex);
      auto size = CVPixelBufferGetWidthOfPlane(_pixelBuffer, planeIndex) *
                  CVPixelBufferGetHeightOfPlane(_pixelBuffer, planeIndex);
      return createUnownedArrayBuffer(runtime, yPlane, size);
    });
  }

  if (propName == "getBytesPerRowOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getBytesPerRowOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      return (int)CVPixelBufferGetBytesPerRowOfPlane(_pixelBuffer, planeIndex);
    });
  }

  if (propName == "getWidthOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getWidthOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      return (int)CVPixelBufferGetWidthOfPlane(_pixelBuffer, planeIndex);
    });
  }

  if (propName == "getHeightOfPlane") {
    return WGPU_FUNC_FROM_HOST_FUNC(getHeightOfPlane, 1, [this]) {
      auto planeIndex = (size_t)arguments[0].asNumber();
      return (int)CVPixelBufferGetHeightOfPlane(_pixelBuffer, planeIndex);
    });
  }

  if (propName == "planeCount") {
    return (int)CVPixelBufferGetPlaneCount(_pixelBuffer);
  }

  if (propName == "width") {
    return (int)CVPixelBufferGetWidth(_pixelBuffer);
  }

  if (propName == "height") {
    return (int)CVPixelBufferGetHeight(_pixelBuffer);
  }

  if (propName == "isInterleaved") {
    return false;
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
