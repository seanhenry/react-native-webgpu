#include "ContextHostObject.h"

#include <iostream>

#include "AdapterHostObject.h"
#include "ConstantConversion.h"
#include "DeviceHostObject.h"
#include "TextureHostObject.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value ContextHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "getCurrentTexture") {
    return WGPU_FUNC_FROM_HOST_FUNC(getCurrentTexture, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(getCurrentTexture);
      if (_configuredContext == nullptr) {
        // TODO: warn
        return Value::null();
      }
      // [Surface texture note 1] If surface texture has already been requested, return it
      if (_texture != nullptr) {
        return Object::createFromHostObject(
          runtime, std::make_shared<TextureHostObject>(_texture, _configuredContext, "[WEBGPU] SurfaceTexture", true));
      }
      WGPUSurfaceTexture texture;
      wgpuSurfaceGetCurrentTexture(_surface->getWGPUSurface(), &texture);
      throwPendingJSIError();
      if (texture.status == WGPUSurfaceGetCurrentTextureStatus_Success) {
        _texture = texture.texture;
        // [Surface texture note 2] isSurfaceTexture = true so TextHostObject does not
        // release it. We will release it in presentSurface, or keep using it until this is
        // called.
        return Object::createFromHostObject(
          runtime,
          std::make_shared<TextureHostObject>(texture.texture, _configuredContext, "[WEBGPU] SurfaceTexture", true));
      } else if (texture.texture) {
        // TODO: check this
        // TODO: log
        wgpuTextureRelease(texture.texture);
      }
      return Value::null();
    });
  }

  if (name == "presentSurface") {
    return WGPU_FUNC_FROM_HOST_FUNC(present, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(present);
      // [Surface texture note 3] assuming we called getCurrentTexture() we have a surface
      // texture. Once the surface is presented, we release it, so we can get the next one.
      if (_texture != nullptr) {
        wgpuSurfacePresent(_surface->getWGPUSurface());
        wgpuTextureRelease(_texture);
        _texture = nullptr;
        throwPendingJSIError();
        return Value::undefined();
      }
      jsLog(runtime, "warn",
            {"Cannot present surface because surface texture doesn't exist. Call "
             "context.getCurrentTexture() to create one."});
      return Value::undefined();
    });
  }

  if (name == "configure") {
    return WGPU_FUNC_FROM_HOST_FUNC(configure, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(configure);
      auto options = arguments[0].asObject(runtime);
      auto device = WGPU_HOST_OBJ(options, device, DeviceHostObject);
      auto format = WGPU_UTF8(options, format);
      auto usage = WGPU_NUMBER_OPT(options, usage, WGPUTextureUsage, WGPUTextureUsage_RenderAttachment);
      auto alphaModeStr = WGPU_UTF8_OPT(options, alphaMode, "auto");
      auto alphaMode = StringToWGPUCompositeAlphaMode(alphaModeStr);

      WGPUSurfaceCapabilities capabilities = {
        .formatCount = 0,
      };
      wgpuSurfaceGetCapabilities(_surface->getWGPUSurface(), device->getAdapter(), &capabilities);

      if (capabilities.alphaModeCount == 0) {
        throw JSError(runtime, "Surface had 0 alpha modes");
      }

      auto isAlphaModeValid = alphaMode == WGPUCompositeAlphaMode_Auto ||
                              cArrayContains(capabilities.alphaModes, capabilities.alphaModeCount, alphaMode);
      if (!isAlphaModeValid) {
        alphaMode = capabilities.alphaModes[0];
        jsLog(runtime, "warn",
              {"Alpha mode", alphaModeStr, "is not valid. Using", WGPUCompositeAlphaModeToString(alphaMode)});
      }

      auto width = _surface->getPixelWidth();
      auto height = _surface->getPixelHeight();

      if (width == 0 || height == 0) {
        throw JSError(runtime, "[WebGPU] Metal layer size was 0");
      }

      WGPUSurfaceConfiguration config = {
        .device = device->getValue(),
        .format = StringToWGPUTextureFormat(format),
        .usage = usage,
        .viewFormatCount = 0,
        .viewFormats = nullptr,
        .alphaMode = alphaMode,
        .width = width,
        .height = height,
        .presentMode = WGPUPresentMode_Fifo,  // TODO:
      };
      wgpuSurfaceConfigure(_surface->getWGPUSurface(), &config);
      _configuredContext = device->getContext();
      throwPendingJSIError();
      _surface->createTimer();

      return Value::undefined();
    });
  }

  if (name == "unconfigure") {
    return WGPU_FUNC_FROM_HOST_FUNC(unconfigure, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(unconfigure);
      if (_texture != nullptr) {
        wgpuTextureRelease(_texture);
        _texture = nullptr;
      }
      _surface->invalidateTimer();
      wgpuSurfaceUnconfigure(_surface->getWGPUSurface());
      throwPendingJSIError();
      _configuredContext = nullptr;
      return Value::undefined();
    });
  }

  if (name == "surfaceCapabilities") {
    return WGPU_FUNC_FROM_HOST_FUNC(surfaceCapabilities, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(surfaceCapabilities);
      auto adapter = arguments[0].asObject(runtime).asHostObject<AdapterHostObject>(runtime)->_adapter->_adapter;
      auto surface = _surface->getWGPUSurface();

      WGPUSurfaceCapabilities capabilities;
      wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);

      auto formats = cArrayToJsi(runtime, capabilities.formats, capabilities.formatCount,
                                 [](Runtime &runtime, WGPUTextureFormat format) {
                                   return Value(String::createFromAscii(runtime, WGPUTextureFormatToString(format)));
                                 });

      auto alphaModes =
        cArrayToJsi(runtime, capabilities.alphaModes, capabilities.alphaModeCount,
                    [](Runtime &runtime, WGPUCompositeAlphaMode alphaMode) {
                      return Value(String::createFromAscii(runtime, WGPUCompositeAlphaModeToString(alphaMode)));
                    });

      auto result = Object(runtime);
      result.setProperty(runtime, PropNameID::forAscii(runtime, "formats"), std::move(formats));
      result.setProperty(runtime, PropNameID::forAscii(runtime, "alphaModes"), std::move(alphaModes));
      result.setProperty(runtime, PropNameID::forAscii(runtime, "usages"), Value((int)capabilities.usages));

      wgpuSurfaceCapabilitiesFreeMembers(capabilities);

      return std::move(result);
    });
  }

  if (name == "width") {
    return Value((int)_surface->getPixelWidth());
  }

  if (name == "height") {
    return Value((int)_surface->getPixelHeight());
  }

  if (name == "pointWidth") {
    return Value((int)_surface->getPointWidth());
  }

  if (name == "pointHeight") {
    return Value((int)_surface->getPointHeight());
  }

  if (name == "scale") {
    return Value(_surface->getScale());
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> ContextHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "presentSurface", "configure", "surfaceCapabilities", "getCurrentTexture", "width",
                           "height", "destroy", "scale", "pointWidth", "pointHeight");
}

void ContextHostObject::throwPendingJSIError() {
  if (_configuredContext != nullptr) {
    _configuredContext->getErrorHandler()->throwPendingJSIError();
  }
}
