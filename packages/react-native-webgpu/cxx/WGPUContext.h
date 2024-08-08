#pragma once

#include "JSIInstance.h"
#include "WGPUJsiUtils.h"

using namespace facebook::react;

namespace wgpu {

class WGPUContext {
 public:
  WGPUContext(std::shared_ptr<AdapterWrapper> adapter, std::shared_ptr<DeviceWrapper> device)
    : _adapter(adapter), _device(device) {}
  inline void runOnJsThread(std::function<void()>&& fn) { JSIInstance::instance->jsThread->run(std::move(fn)); }
  bool poll(bool wait);
  inline WGPUAdapter getAdapter() { return _adapter->_adapter; }
  inline WGPUDevice getDevice() { return _device->_device; }

 private:
  std::shared_ptr<AdapterWrapper> _adapter;
  std::shared_ptr<DeviceWrapper> _device;
};

}  // namespace wgpu
