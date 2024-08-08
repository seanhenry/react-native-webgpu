#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class ShaderModuleHostObject : public HostObject {
 public:
  explicit ShaderModuleHostObject(WGPUShaderModule value, std::shared_ptr<WGPUContext> context, std::string label)
    : _value(value), _context(context), _label(label) {}
  ~ShaderModuleHostObject() { wgpuShaderModuleRelease(_value); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline WGPUShaderModule getValue() { return _value; }

 private:
  WGPUShaderModule _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
