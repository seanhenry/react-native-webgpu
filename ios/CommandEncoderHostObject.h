#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class CommandEncoderHostObject : public HostObject {
public:
    explicit CommandEncoderHostObject(WGPUCommandEncoder value, WGPUContext *context): _value(value), _context(context) {}
    ~CommandEncoderHostObject() { wgpuCommandEncoderRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUCommandEncoder _value;
    WGPUContext *_context;
};

}
