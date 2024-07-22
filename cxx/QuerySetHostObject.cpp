#include "QuerySetHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"

using namespace facebook::jsi;
using namespace wgpu;

Value QuerySetHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "destroy") {
        return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
            wgpuQuerySetDestroy(_value);
            return Value::undefined();
        });
    }

    if (name == "type") {
        return String::createFromUtf8(runtime, WGPUQuerySetToString(wgpuQuerySetGetType(_value)));
    }

    if (name == "count") {
        return Value((int)wgpuQuerySetGetCount(_value));
    }

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> QuerySetHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "destroy", "type", "count", "label");
}
