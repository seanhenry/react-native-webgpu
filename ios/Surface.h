#pragma once

#include "webgpu.h"
#include "WGPUJsiUtils.h"
#include <functional>
#include <jsi/jsi.h>

using namespace facebook::jsi;

namespace wgpu {

class Surface {
public:
    explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, uint32_t width, uint32_t height): _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _width(width), _height(height) {}
    ~Surface();
    inline WGPUSurface getWGPUSurface() { return _wgpuSurface; }
    inline WGPUInstance getWGPUInstance() { return _wgpuInstance; }
    inline std::weak_ptr<AdapterWrapper> getUnownedWGPUAdapter() { return _unownedWGPUAdapter; }
    inline void setAdapter(std::shared_ptr<AdapterWrapper> adapter) { _unownedWGPUAdapter = adapter; }
    inline void setWidth(uint32_t width) { _width = width; }
    inline void setHeight(uint32_t height) { _height = height; }
    inline uint32_t getWidth() { return _width; }
    inline uint32_t getHeight() { return _height; }
    void createTimer();
    void invalidateTimer();
    void requestAnimationFrame(Function fn);
private:
    void *_timer;
    WGPUInstance _wgpuInstance;
    WGPUSurface _wgpuSurface;
    std::weak_ptr<AdapterWrapper> _unownedWGPUAdapter;
    uint32_t _width;
    uint32_t _height;
};

}
