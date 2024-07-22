#pragma once

#include "webgpu.h"
#include "WGPUJsiUtils.h"
#include <functional>
#include <jsi/jsi.h>

#ifdef ANDROID
#include <android/native_window_jni.h>
#include <android/choreographer.h>
#endif

using namespace facebook::jsi;

namespace wgpu {
    class Surface;
}

#ifdef ANDROID
typedef struct WGPUSurfaceCallbackData {
    // Keeps a strong reference to self whilst a frame has been requested
    std::shared_ptr<wgpu::Surface> surface;
    std::vector<Function> animationCallbacks;
    std::vector<Function> animationCallbacksForProcessing;
} WGPUSurfaceCallbackData;
#endif

namespace wgpu {

class Surface : public std::enable_shared_from_this<Surface> {
public:
#ifdef ANDROID
    explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, uint32_t width, uint32_t height, ANativeWindow *window): _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _width(width), _height(height), _window(window) {}
#else
    explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, uint32_t width, uint32_t height): _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _width(width), _height(height) {}
#endif
    ~Surface();
    inline WGPUSurface getWGPUSurface() { return _wgpuSurface; }
    inline WGPUInstance getWGPUInstance() { return _wgpuInstance; }
    inline std::weak_ptr<AdapterWrapper> getUnownedWGPUAdapter() { return _unownedWGPUAdapter; }
    inline void setAdapter(std::shared_ptr<AdapterWrapper> adapter) { _unownedWGPUAdapter = adapter; }
    inline void setWidth(uint32_t width) { _width = width; }
    inline void setHeight(uint32_t height) { _height = height; }
    inline uint32_t getWidth() { return _width; }
    inline uint32_t getHeight() { return _height; }
    void requestAnimationFrame(Function fn);
    void createTimer();
    void invalidateTimer();
private:
    WGPUInstance _wgpuInstance;
    WGPUSurface _wgpuSurface;
    std::weak_ptr<AdapterWrapper> _unownedWGPUAdapter;
    uint32_t _width;
    uint32_t _height;
#ifdef ANDROID
    ANativeWindow *_window;
    AChoreographer *_choreographer;
    WGPUSurfaceCallbackData _callbackData;
#else
    void *_timer;
#endif
};

}
