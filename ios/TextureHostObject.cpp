#include "TextureHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "TextureViewHostObject.h"
#include "ConstantConversion.h"
#include "WGPUDefaults.h"

using namespace facebook::jsi;
using namespace wgpu;

Value TextureHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "createView") {
        return WGPU_FUNC_FROM_HOST_FUNC(createView, 1, [this]) {
            WGPUTextureView view = NULL;
            std::string label;
            if (count > 0) {
                auto desc = arguments[0].asObject(runtime);
                label = WGPU_UTF8_OPT(desc, label, "");
                auto descriptor = makeDefaultWGPUTextureViewDescriptor(runtime, desc, _value);
                descriptor.label = label.data();
                view = wgpuTextureCreateView(_value, &descriptor);
            } else {
                view = wgpuTextureCreateView(_value, NULL);
            }

            return Object::createFromHostObject(runtime, std::make_shared<TextureViewHostObject>(view, _context, label));
        });
    }

    if (name == "destroy") {
        return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
            wgpuTextureDestroy(_value);
            return Value::undefined();
        });
    }

    if (name == "width") {
        return Value((int)wgpuTextureGetWidth(_value));
    }

    if (name == "height") {
        return Value((int)wgpuTextureGetHeight(_value));
    }

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> TextureHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createView", "destroy", "width", "height", "label");
}
