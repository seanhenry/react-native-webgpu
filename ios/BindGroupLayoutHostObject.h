#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class BindGroupLayoutHostObject : public HostObject {
public:
    explicit BindGroupLayoutHostObject(WGPUBindGroupLayout value, WGPUContext *context): _value(value), _context(context) {}
    ~BindGroupLayoutHostObject() { wgpuBindGroupLayoutRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUBindGroupLayout _value;
    WGPUContext *_context;
};

}
