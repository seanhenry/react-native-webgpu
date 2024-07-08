#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class ComputePipelineHostObject : public HostObject {
public:
    explicit ComputePipelineHostObject(WGPUComputePipeline value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~ComputePipelineHostObject() { wgpuComputePipelineRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUComputePipeline _value;
    WGPUContext *_context;
    std::string _label;
};

}
