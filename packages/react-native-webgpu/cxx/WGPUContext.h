#pragma once

#include "ErrorHandler.h"
#include "JSIInstance.h"
#include "WGPUWrappers.h"

using namespace facebook::react;

namespace wgpu {

class WGPUContext {
 public:
  WGPUContext(std::shared_ptr<AdapterWrapper> adapter, std::shared_ptr<DeviceWrapper> device,
              std::shared_ptr<ErrorHandler> errorHandler, std::shared_ptr<JSIInstance> jsiInstance)
    : _adapter(adapter), _device(device), _errorHandler(errorHandler), _jsiInstance(jsiInstance) {}
  inline void runOnJsThread(std::function<void()>&& fn) { _jsiInstance->jsThread->run(std::move(fn)); }
  bool poll(bool wait);
  inline WGPUAdapter getAdapter() { return _adapter->_adapter; }
  inline WGPUDevice getDevice() { return _device->_device; }
  inline std::shared_ptr<ErrorHandler> getErrorHandler() { return _errorHandler; }
  inline std::shared_ptr<JSIInstance> getJSIInstance() { return _jsiInstance; }

 private:
  std::shared_ptr<AdapterWrapper> _adapter;
  std::shared_ptr<DeviceWrapper> _device;
  std::shared_ptr<ErrorHandler> _errorHandler;
  std::shared_ptr<JSIInstance> _jsiInstance;
};

}  // namespace wgpu
