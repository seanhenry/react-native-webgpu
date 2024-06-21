#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class AdapterHostObject : public HostObject {
public:
    explicit AdapterHostObject(WGPUAdapter value, WGPUContext *context): _value(value), _context(context) {
        _context->_adapter = value;
    }
    ~AdapterHostObject() { 
        _context->_adapter = nullptr;
        wgpuAdapterRelease(_value);
    }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUAdapter _value;
    WGPUContext *_context;
};

}
