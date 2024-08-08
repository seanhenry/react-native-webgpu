#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class CommandBufferHostObject : public HostObject {
 public:
  explicit CommandBufferHostObject(WGPUCommandBuffer value, std::shared_ptr<WGPUContext> context, std::string label)
    : _value(value), _context(context), _label(label) {}
  ~CommandBufferHostObject() { wgpuCommandBufferRelease(_value); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline WGPUCommandBuffer getValue() { return _value; }

 private:
  WGPUCommandBuffer _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
