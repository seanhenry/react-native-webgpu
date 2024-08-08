#include "SamplerHostObject.h"

#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value SamplerHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> SamplerHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "label");
}
