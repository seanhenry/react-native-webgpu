#include "PipelineLayoutHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value PipelineLayoutHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    return Value::undefined();
}

std::vector<PropNameID> PipelineLayoutHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "");
}
