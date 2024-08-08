#import "Surface.h"

#include <jsi/jsi.h>

#include "JSIInstance.h"

using namespace wgpu;
using namespace facebook::jsi;

#if __ANDROID_API__ >= 29
static void wgpuChoreographerFrameCallback(int64_t frameTimeNanos, void *data);
#else
static void wgpuChoreographerFrameCallback(long frameTimeNanos, void *data);
#endif

Surface::~Surface() {
    invalidateTimer();
    wgpuSurfaceRelease(_wgpuSurface);
    wgpuInstanceRelease(_wgpuInstance);
    ANativeWindow_release(_window);
}

void Surface::createTimer() {
    invalidateTimer();
    _choreographer = AChoreographer_getInstance();
}

void Surface::invalidateTimer() {
    _callbackData.animationCallbacks.clear();
    _choreographer = nullptr;
}

void Surface::requestAnimationFrame(Function fn) {
    if (_choreographer == nullptr) {
        return;
    }
    _callbackData.animationCallbacks.emplace_back(std::move(fn));
    if (_callbackData.surface == nullptr) {
        // Make retain cycle in case `this` is deleted before the callback
        _callbackData.surface = shared_from_this();
#if __ANDROID_API__ >= 29
        AChoreographer_postFrameCallback64(_choreographer, wgpuChoreographerFrameCallback,
                                           &_callbackData);
#else
        AChoreographer_postFrameCallback(_choreographer, wgpuChoreographerFrameCallback,
                                         &_callbackData);
#endif
    }
}

#if __ANDROID_API__ >= 29
static void wgpuChoreographerFrameCallback(int64_t frameTimeNanos, void *data) {
#else
static void wgpuChoreographerFrameCallback(long frameTimeNanos, void *data) {
#endif
    // Note, in the event that the timer has been invalidated after this
    // callback was scheduled, `animationCallbacks` will be cleared so this
    // callback will not do anything except break the deliberate retain cycle
    // once `callbackData` has been consumed.
    auto callbackData = (WGPUSurfaceCallbackData *)data;
    auto surface = callbackData->surface;
    // Break retain cycle as we have a strong reference now
    callbackData->surface = nullptr;
    if (surface == nullptr) {
        return;
    }

    std::swap(callbackData->animationCallbacks, callbackData->animationCallbacksForProcessing);

    auto &runtime = JSIInstance::instance->runtime;
    auto performance = runtime.global().getPropertyAsObject(runtime, "performance");
    auto now = performance.getPropertyAsFunction(runtime, "now")
                   .callWithThis(runtime, performance, nullptr, 0)
                   .asNumber();

    for (auto &callback : callbackData->animationCallbacksForProcessing) {
        callback.call(runtime, now);
    }

    callbackData->animationCallbacksForProcessing.clear();
}
