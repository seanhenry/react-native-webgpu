#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class QuerySetHostObject : public HostObject {
public:
    explicit QuerySetHostObject(WGPUQuerySet value, WGPUContext *context, std::string label): _value(value), _context(context), _label(label) {}
    ~QuerySetHostObject() { wgpuQuerySetRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUQuerySet _value;
    WGPUContext *_context;
    std::string _label;
};

}