#pragma once

#include "wgpu.h"
#include <functional>

namespace facebook {
namespace react {

class RCTMessageThread;

}
}

using namespace facebook::react;

namespace wgpu {

class WGPUContext {
public:
    WGPUContext(WGPUInstance instance, WGPUSurface surface, std::function<uint32_t()> getWidth, std::function<uint32_t()> getHeight, std::shared_ptr<RCTMessageThread> jsThread): _instance(instance), _surface(surface), _getWidth(getWidth), _getHeight(getHeight), _jsThread(jsThread) {
        _adapter = nullptr;
        _device = nullptr;
    }
    ~WGPUContext() {
        wgpuSurfaceRelease(_surface);
        wgpuInstanceRelease(_instance);
        // Adapter lifetime managed by AdapterHostObject
        // Device lifetime managed by DeviceHostObject
    }
    void runOnJsThread(std::function<void()>&& fn);
    WGPUInstance _instance;
    WGPUSurface _surface;
    WGPUAdapter _adapter;
    WGPUDevice _device;
    std::function<uint32_t()> _getWidth;
    std::function<uint32_t()> _getHeight;
private:
    std::shared_ptr<RCTMessageThread> _jsThread;
};

}
