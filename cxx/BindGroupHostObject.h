#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class BindGroupHostObject : public HostObject {
public:
    explicit BindGroupHostObject(WGPUBindGroup value, std::shared_ptr<WGPUContext> context, std::string label): _value(value), _context(context), _label(label) {}
    ~BindGroupHostObject() { wgpuBindGroupRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    inline WGPUBindGroup getValue() { return _value; }
private:
    WGPUBindGroup _value;
    std::shared_ptr<WGPUContext> _context;
    std::string _label;
};

}
