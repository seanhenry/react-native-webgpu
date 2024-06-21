#include "TextureHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "TextureViewHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value TextureHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "createView") {
        return WGPU_FUNC_FROM_HOST_FUNC(createView, 1, [this]) {
            auto view = wgpuTextureCreateView(_value, NULL);
            return Object::createFromHostObject(runtime, std::make_shared<TextureViewHostObject>(view, _context));
        });
    }

    if (name == "destroy") {
        return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
            this->destroy();
            return Value::undefined();
        });
    }

    if (name == "width") {
        return Value((int)wgpuTextureGetWidth(_value));
    }

    if (name == "height") {
        return Value((int)wgpuTextureGetHeight(_value));
    }

    return Value::undefined();
}

std::vector<PropNameID> TextureHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createView", "destroy", "width", "height");
}
