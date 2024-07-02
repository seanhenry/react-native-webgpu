#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class PipelineLayoutHostObject : public HostObject {
public:
    explicit PipelineLayoutHostObject(WGPUPipelineLayout value, WGPUContext *context): _value(value), _context(context) {}
    ~PipelineLayoutHostObject() { wgpuPipelineLayoutRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUPipelineLayout _value;
    WGPUContext *_context;
};

}
