#include "BindGroupLayoutHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value BindGroupLayoutHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    return Value::undefined();
}

std::vector<PropNameID> BindGroupLayoutHostObject::getPropertyNames(Runtime& runtime) {
    return {};
}
