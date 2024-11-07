#import "Surface.h"
#import <QuartzCore/CAMetalLayer.h>
#import <React-Core/React/RCTAssert.h>
#import <React-Core/React/RCTCxxUtils.h>
#include <jsi/jsi.h>
#include "JSIInstance.h"

using namespace wgpu;
using namespace facebook::jsi;

@interface WGPUTimer : NSObject {
 @public
  std::vector<Function> _animationCallbacks;
  std::vector<Function> _animationCallbacksForProcessing;
  CADisplayLink *_displayLink;
  std::shared_ptr<JSIInstance> _jsiInstance;
}

@end

@implementation WGPUTimer

- (instancetype)init {
  self = [super init];
  if (self) {
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onAnimationFrame)];
    _displayLink.paused = YES;
    [_displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSRunLoopCommonModes];
  }
  return self;
}

- (void)start {
  _displayLink.paused = NO;
}

- (void)stop {
  _animationCallbacks.clear();
  _displayLink.paused = YES;
}

- (void)onAnimationFrame {
  std::swap(_animationCallbacks, _animationCallbacksForProcessing);

  NSError *error = facebook::react::tryAndReturnError([self]() {
    auto &runtime = _jsiInstance->runtime;
    auto performance = runtime.global().getPropertyAsObject(runtime, "performance");
    auto now =
      performance.getPropertyAsFunction(runtime, "now").callWithThis(runtime, performance, nullptr, 0).asNumber();

    for (auto &callback : _animationCallbacksForProcessing) {
      @autoreleasepool {
        callback.call(runtime, now);
      }
    }
  });

  if (error != nil) {
    RCTFatal(error);
  }

  _animationCallbacksForProcessing.clear();
}

- (void)invalidate {
  [self stop];
  [_displayLink invalidate];
  _displayLink = nil;
}

@end

Surface::~Surface() {
  invalidateTimer();
  wgpuSurfaceRelease(_wgpuSurface);
  wgpuInstanceRelease(_wgpuInstance);
}

void Surface::createTimer(std::shared_ptr<JSIInstance> jsiInstance) {
  invalidateTimer();
  auto timer = [[WGPUTimer alloc] init];
  timer->_jsiInstance = jsiInstance;
  [timer start];
  _timer = (void *)CFBridgingRetain(timer);
}

void Surface::invalidateTimer() {
  if (_timer != nil) {
    auto timer = (WGPUTimer *)CFBridgingRelease(_timer);
    [timer invalidate];
    _timer = nil;
  }
}

void Surface::requestAnimationFrame(Function fn) {
  if (_timer != nil) {
    auto timer = (__bridge WGPUTimer *)_timer;
    timer->_animationCallbacks.emplace_back(std::move(fn));
  }
}
