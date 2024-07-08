#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class TextureHostObject : public HostObject {
public:
    explicit TextureHostObject(WGPUTexture value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~TextureHostObject() {
        // Note, must call delete() after each frame as JS garbage collection is too slow
        // Clean up anyway in case.
        destroy();
    }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    void destroy() {
        if (_value != nullptr) {
            wgpuTextureRelease(_value);
            _value = nullptr;
        }
    }
    WGPUTexture _value;
    WGPUContext *_context;
    std::string _label;
};

}
