#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class BindGroupLayoutHostObject : public HostObject {
public:
    explicit BindGroupLayoutHostObject(WGPUBindGroupLayout value, std::shared_ptr<WGPUContext> context, std::string label): _value(value), _context(context), _label(label) {}
    ~BindGroupLayoutHostObject() { wgpuBindGroupLayoutRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    inline WGPUBindGroupLayout getValue() { return _value; }
private:
    WGPUBindGroupLayout _value;
    std::shared_ptr<WGPUContext> _context;
    std::string _label;
};

}
