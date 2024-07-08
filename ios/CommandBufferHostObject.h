#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class CommandBufferHostObject : public HostObject {
public:
    explicit CommandBufferHostObject(WGPUCommandBuffer value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~CommandBufferHostObject() { wgpuCommandBufferRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUCommandBuffer _value;
    WGPUContext *_context;
    std::string _label;
};

}
