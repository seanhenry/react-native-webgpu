#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class QueueHostObject : public HostObject {
public:
    explicit QueueHostObject(WGPUQueue value, WGPUContext *context): _value(value), _context(context) {}
    ~QueueHostObject() { wgpuQueueRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUQueue _value;
    WGPUContext *_context;
};

}
