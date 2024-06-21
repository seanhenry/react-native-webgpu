#include "RenderPipelineHostObject.h"
#include "BindGroupLayoutHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderPipelineHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "getBindGroupLayout") {
        return WGPU_FUNC_FROM_HOST_FUNC(getBindGroupLayout, 1, [this]) {
            auto index = (uint32_t)arguments[0].asNumber();
            auto layout = wgpuRenderPipelineGetBindGroupLayout(_value, index);
            return Object::createFromHostObject(runtime, std::make_shared<BindGroupLayoutHostObject>(layout, _context));
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> RenderPipelineHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "getBindGroupLayout");
}
