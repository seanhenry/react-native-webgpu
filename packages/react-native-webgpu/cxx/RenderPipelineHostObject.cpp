#include "RenderPipelineHostObject.h"

#include "BindGroupLayoutHostObject.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderPipelineHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "getBindGroupLayout") {
    return WGPU_FUNC_FROM_HOST_FUNC(getBindGroupLayout, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(getBindGroupLayout);
      auto index = (uint32_t)arguments[0].asNumber();
      auto layout = wgpuRenderPipelineGetBindGroupLayout(_value, index);
      return Object::createFromHostObject(runtime, std::make_shared<BindGroupLayoutHostObject>(layout, _context, ""));
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> RenderPipelineHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "getBindGroupLayout", "label");
}