#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class BindGroupHostObject : public HostObject {
public:
    explicit BindGroupHostObject(WGPUBindGroup value, WGPUContext *context): _value(value), _context(context) {}
    ~BindGroupHostObject() { wgpuBindGroupRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUBindGroup _value;
    WGPUContext *_context;
};

}