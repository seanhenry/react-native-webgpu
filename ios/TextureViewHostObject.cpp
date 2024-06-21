#include "TextureViewHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value TextureViewHostObject::get(Runtime &runtime, const PropNameID &propName) {
    return Value::undefined();
}

std::vector<PropNameID> TextureViewHostObject::getPropertyNames(Runtime& runtime) {
    return {};
}
