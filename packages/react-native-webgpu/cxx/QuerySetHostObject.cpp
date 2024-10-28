#include "QuerySetHostObject.h"

#include "ConstantConversion.h"
#include "Mixins.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value QuerySetHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "destroy") {
    return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(destroy);
      wgpuQuerySetDestroy(_value);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "type") {
    return String::createFromUtf8(runtime, WGPUQuerySetToString(wgpuQuerySetGetType(_value)));
  }

  if (name == "count") {
    return Value((int)wgpuQuerySetGetCount(_value));
  }

  WGPU_GET_LABEL()

  WGPU_GET_BRAND(GPUQuerySet)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> QuerySetHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "destroy", "type", "count", "label", "__brand");
}
