#include "CommandBufferHostObject.h"

#include "Mixins.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value CommandBufferHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  WGPU_GET_LABEL()

  WGPU_GET_BRAND(GPUCommandBuffer)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> CommandBufferHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "label", "__brand");
}
