#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class ShaderModuleHostObject : public HostObject {
public:
    explicit ShaderModuleHostObject(WGPUShaderModule value, WGPUContext *context): _value(value), _context(context) {}
    ~ShaderModuleHostObject() { wgpuShaderModuleRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUShaderModule _value;
    WGPUContext *_context;
};

}
