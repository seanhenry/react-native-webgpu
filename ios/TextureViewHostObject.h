#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class TextureViewHostObject : public HostObject {
public:
    explicit TextureViewHostObject(WGPUTextureView value, WGPUContext *context): _value(value), _context(context) {}
    ~TextureViewHostObject() { wgpuTextureViewRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUTextureView _value;
    WGPUContext *_context;
};

}