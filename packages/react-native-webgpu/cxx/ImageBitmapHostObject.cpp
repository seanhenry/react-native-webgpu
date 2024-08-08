#include "ImageBitmapHostObject.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value ImageBitmapHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    WGPU_LOG_GET_PROP;

    if (name == "width") {
        return Value((int)_width);
    }

    if (name == "height") {
        return Value((int)_height);
    }

    if (name == "close") {
        return WGPU_FUNC_FROM_HOST_FUNC(close, 0, [this]) {
            WGPU_LOG_FUNC_ARGS(close);
            this->destroy();
            return Value::undefined();
        });
    }

    WGPU_LOG_UNIMPLEMENTED_GET_PROP;

    return Value::undefined();
}

std::vector<PropNameID> ImageBitmapHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "width", "height", "close");
}
