#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class SamplerHostObject : public HostObject {
public:
    explicit SamplerHostObject(WGPUSampler value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~SamplerHostObject() { wgpuSamplerRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUSampler _value;
    WGPUContext *_context;
    std::string _label;
};

}
