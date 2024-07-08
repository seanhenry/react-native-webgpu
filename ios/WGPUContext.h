#pragma once

#include "wgpu.h"
#include <functional>

namespace wgpu {

class WGPUContext {
public:
    WGPUContext(WGPUInstance instance, WGPUSurface surface, std::function<uint32_t()> getWidth, std::function<uint32_t()> getHeight): _instance(instance), _surface(surface), _getWidth(getWidth), _getHeight(getHeight) {}
    ~WGPUContext() {
        wgpuSurfaceRelease(_surface);
        wgpuInstanceRelease(_instance);
        // Adapter lifetime managed by AdapterHostObject
        // Device lifetime managed by DeviceHostObject
    }
    WGPUInstance _instance;
    WGPUSurface _surface;
    WGPUAdapter _adapter;
    WGPUDevice _device;
    std::function<uint32_t()> _getWidth;
    std::function<uint32_t()> _getHeight;
};

}
