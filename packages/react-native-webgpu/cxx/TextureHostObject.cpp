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

    WGPU_LOG_GET_PROP;

    if (name == "createView") {
        return WGPU_FUNC_FROM_HOST_FUNC(createView, 1, [this]) {
            WGPU_LOG_FUNC_ARGS(createView);
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

            return Object::createFromHostObject(runtime, std::make_shared<TextureViewHostObject>(view, _context, std::move(label)));
        });
    }

    if (name == "destroy") {
        return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
            WGPU_LOG_FUNC_ARGS(destroy);
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

    if (name == "format") {
        auto format = wgpuTextureGetFormat(_value);
        return String::createFromUtf8(runtime, WGPUTextureFormatToString(format));
    }

    if (name == "mipLevelCount") {
        return Value((int)wgpuTextureGetMipLevelCount(_value));
    }

    if (name == "sampleCount") {
        return Value((int)wgpuTextureGetSampleCount(_value));
    }

    if (name == "usage") {
        return Value((int)wgpuTextureGetUsage(_value));
    }

    if (name == "dimension") {
        auto dim = wgpuTextureGetDimension(_value);
        return String::createFromUtf8(runtime, WGPUTextureDimensionToString(dim));
    }

    WGPU_LOG_UNIMPLEMENTED_GET_PROP;

    return Value::undefined();
}

std::vector<PropNameID> TextureHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createView", "destroy", "width", "height", "label", "format", "mipLevelCount", "sampleCount", "usage");
}
