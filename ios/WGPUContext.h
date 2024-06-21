#pragma once

#include "wgpu.h"
#include <functional>

namespace wgpu {

class WGPUContext {
public:
    WGPUContext(WGPUInstance instance, WGPUSurface surface, std::function<uint32_t()> getWidth, std::function<uint32_t()> getHeight): _instance(instance), _surface(surface), _getWidth(getWidth), _getHeight(getHeight) {}
    ~WGPUContext() {
        wgpuInstanceRelease(_instance);
        wgpuSurfaceRelease(_surface);
        // Adapter lifetime managed by AdapterHostObject
    }
    WGPUInstance _instance;
    WGPUSurface _surface;
    WGPUAdapter _adapter;
    std::function<uint32_t()> _getWidth;
    std::function<uint32_t()> _getHeight;
};

}
