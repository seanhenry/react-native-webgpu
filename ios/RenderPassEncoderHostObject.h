#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class RenderPassEncoderHostObject : public HostObject {
public:
    explicit RenderPassEncoderHostObject(WGPURenderPassEncoder value, WGPUContext *context): _value(value), _context(context) { }
    ~RenderPassEncoderHostObject() { wgpuRenderPassEncoderRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPURenderPassEncoder _value;
    WGPUContext *_context;
};

}
