#include "ComputePassEncoderHostObject.h"

#include "BindGroupHostObject.h"
#include "ComputePipelineHostObject.h"
#include "Mixins.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value ComputePassEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_PIPELINE(wgpuComputePassEncoderSetPipeline, ComputePipelineHostObject);

  WGPU_GPU_BINDING_COMMANDS_MIXIN_SET_BIND_GROUP(wgpuComputePassEncoderSetBindGroup)

  if (name == "dispatchWorkgroups") {
    return WGPU_FUNC_FROM_HOST_FUNC(dispatchWorkgroups, 3, [this]) {
      WGPU_LOG_FUNC_ARGS(dispatchWorkgroups);
      uint32_t x = (uint32_t)arguments[0].asNumber();
      uint32_t y = count > 1 ? (uint32_t)arguments[1].asNumber() : 1;
      uint32_t z = count > 2 ? (uint32_t)arguments[2].asNumber() : 1;
      wgpuComputePassEncoderDispatchWorkgroups(_value, x, y, z);
      return Value::undefined();
    });
  }

  if (name == "end") {
    return WGPU_FUNC_FROM_HOST_FUNC(end, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(end);
      wgpuComputePassEncoderEnd(_value);
      return Value::undefined();
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> ComputePassEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "setPipeline", "dispatchWorkgroups", "end", "label",
                           WGPU_GPU_BINDING_COMMANDS_MIXIN_PROP_NAMES);
}
