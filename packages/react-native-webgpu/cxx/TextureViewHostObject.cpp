#include "TextureViewHostObject.h"

#include "Mixins.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value TextureViewHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  WGPU_GET_LABEL()

  WGPU_GET_BRAND(GPUTextureView)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> TextureViewHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "label", "__brand");
}
