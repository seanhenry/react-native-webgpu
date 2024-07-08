#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"

using namespace facebook::jsi;

namespace wgpu {

class ContextHostObject : public HostObject {
public:
    explicit ContextHostObject(WGPUContext *context): _context(context) {}
    ~ContextHostObject() {
        destroy();
    }
    void destroy() {
        if (_context != nullptr) {
            delete _context;
            _context = nullptr;
        }
    }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUContext *_context;
};

}
