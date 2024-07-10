#include "TimerHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#import <QuartzCore/QuartzCore.h>

using namespace facebook::jsi;
using namespace wgpu;

@interface WGPUTimer : NSObject {
    @public std::vector<Function> _animationCallbacks;
    std::vector<Function> _animationCallbacksForProcessing;
    Runtime *_runtime;
    CADisplayLink *_displayLink;
}

@end

@implementation WGPUTimer

- (instancetype)initWithRuntime:(Runtime*)runtime {
    self = [super init];
    if (self) {
        _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(onAnimationFrame)];
        _displayLink.paused = YES;
        [_displayLink addToRunLoop:NSRunLoop.currentRunLoop forMode:NSRunLoopCommonModes];
        _runtime = runtime;
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

    auto &runtime = *_runtime;
    auto performance = runtime.global().getPropertyAsObject(runtime, "performance");
    auto now = performance.getPropertyAsFunction(runtime, "now").callWithThis(runtime, performance, nullptr, 0).asNumber();

    for (auto &callback : _animationCallbacksForProcessing) {
        @autoreleasepool {
            callback.call(runtime, now, 1);
        }
    }

    _animationCallbacksForProcessing.clear();
}

- (void)invalidate {
    [self stop];
    [_displayLink invalidate];
    _displayLink = nil;
}

@end

TimerHostObject::TimerHostObject(Runtime *runtime) {
    _timer = [[WGPUTimer alloc] initWithRuntime:runtime];
}

TimerHostObject::~TimerHostObject() {
    [_timer invalidate];
}

Value TimerHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "requestAnimationFrame") {
        return WGPU_FUNC_FROM_HOST_FUNC(requestAnimationFrame, 1, [this]) {
            auto callback = arguments[0].asObject(runtime).asFunction(runtime);
            _timer->_animationCallbacks.emplace_back(std::move(callback));
            return Value::undefined();
        });
    }

    if (name == "start") {
        return WGPU_FUNC_FROM_HOST_FUNC(start, 0, [this]) {
            [_timer start];
            return Value::undefined();
        });
    }

    if (name == "stop") {
        return WGPU_FUNC_FROM_HOST_FUNC(stop, 0, [this]) {
            [_timer stop];
            return Value::undefined();
        });
    }

    if (name == "invalidate") {
        return WGPU_FUNC_FROM_HOST_FUNC(invalidate, 0, [this]) {
            [_timer invalidate];
            return Value::undefined();
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> TimerHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "requestAnimationFrame", "start", "stop", "invalidate");
}
