#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class TextureViewHostObject : public HostObject {
public:
    explicit TextureViewHostObject(WGPUTextureView value, std::shared_ptr<WGPUContext> context, std::string label): _value(value), _context(context), _label(label) {}
    ~TextureViewHostObject() { wgpuTextureViewRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    inline WGPUTextureView getValue() { return _value; }
private:
    WGPUTextureView _value;
    std::shared_ptr<WGPUContext> _context;
    std::string _label;
};

}
