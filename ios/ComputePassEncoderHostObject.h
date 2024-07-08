#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class ComputePassEncoderHostObject : public HostObject {
public:
    explicit ComputePassEncoderHostObject(WGPUComputePassEncoder value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~ComputePassEncoderHostObject() { wgpuComputePassEncoderRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUComputePassEncoder _value;
    WGPUContext *_context;
    std::string _label;
};

}
