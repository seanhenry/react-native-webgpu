#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#import "webgpu.h"
#import "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class DeviceHostObject : public HostObject {
 public:
  explicit DeviceHostObject(std::shared_ptr<DeviceWrapper> value, std::shared_ptr<WGPUContext> context)
    : _value(value), _context(context) {}
  ~DeviceHostObject() {}
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline WGPUAdapter getAdapter() { return _context->getAdapter(); }
  inline WGPUDevice getValue() { return _value->_device; }
  inline std::shared_ptr<WGPUContext> getContext() { return _context; }

 private:
  std::shared_ptr<DeviceWrapper> _value;
  std::shared_ptr<WGPUContext> _context;
};

}  // namespace wgpu
