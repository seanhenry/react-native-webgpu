#include "SamplerHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value SamplerHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    return Value::undefined();
}

std::vector<PropNameID> SamplerHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "");
}
