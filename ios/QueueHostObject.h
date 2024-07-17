#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class QueueHostObject : public HostObject {
public:
    explicit QueueHostObject(WGPUQueue value, std::shared_ptr<WGPUContext> context): _value(value), _context(context) {}
    ~QueueHostObject() { wgpuQueueRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
private:
    WGPUQueue _value;
    std::shared_ptr<WGPUContext> _context;
};

}
