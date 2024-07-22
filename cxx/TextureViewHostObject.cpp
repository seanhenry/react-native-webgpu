#include "TextureViewHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value TextureViewHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> TextureViewHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "label");
}
