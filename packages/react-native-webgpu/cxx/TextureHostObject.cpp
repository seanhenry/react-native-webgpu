#include "TextureHostObject.h"

#include "ConstantConversion.h"
#include "Mixins.h"
#include "TextureViewHostObject.h"
#include "WGPUContext.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"

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
        if (WGPU_HAS_PROP(desc, format)) {
          descriptor.format = StringToWGPUTextureFormat(WGPU_UTF8(desc, format));
        }
        view = wgpuTextureCreateView(_value, &descriptor);
      } else {
        view = wgpuTextureCreateView(_value, NULL);
      }
      _context->getErrorHandler()->throwPendingJSIError();

      return Object::createFromHostObject(runtime,
                                          std::make_shared<TextureViewHostObject>(view, _context, std::move(label)));
    });
  }

  if (name == "destroy") {
    return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(destroy);
      wgpuTextureDestroy(_value);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "width") {
    return Value((int)wgpuTextureGetWidth(_value));
  }

  if (name == "height") {
    return Value((int)wgpuTextureGetHeight(_value));
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

  if (name == "depthOrArrayLayers") {
    return Value((int)wgpuTextureGetDepthOrArrayLayers(_value));
  }

  WGPU_GET_LABEL()

  WGPU_GET_BRAND(GPUTexture)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> TextureHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "createView", "destroy", "width", "height", "label", "format", "mipLevelCount",
                           "sampleCount", "usage", "depthOrArrayLayers", "__brand");
}
