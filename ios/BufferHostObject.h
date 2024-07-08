#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class BufferHostObject : public HostObject {
public:
    explicit BufferHostObject(WGPUBuffer value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~BufferHostObject() { wgpuBufferRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUBuffer _value;
    WGPUContext *_context;
    std::string _label;
};

}
