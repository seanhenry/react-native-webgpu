#include "ContextHostObject.h"
#include "DeviceHostObject.h"
#include "TextureHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include <iostream>
#include "AdapterHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value ContextHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "presentSurface") {
        return WGPU_FUNC_FROM_HOST_FUNC(present, 1, [this]) {
            if (!isSurfaceConfigured()) {
                jsLog(runtime, "warn", {"context.presentSurface() was called but the surface was not configured"});
                return Value::undefined();
            }
            auto texture = arguments[0].asObject(runtime).asHostObject<TextureHostObject>(runtime);
            wgpuSurfacePresent(_surface->getWGPUSurface());
            // We must release the texture before the next pass so that the surface can provide the next texture.
            texture->release();
            return Value::undefined();
        });
    }

    if (name == "configure") {
        return WGPU_FUNC_FROM_HOST_FUNC(configure, 0, [this]) {
            auto options = arguments[0].asObject(runtime);
            auto device = WGPU_HOST_OBJ(options, device, DeviceHostObject);
            auto format = WGPU_UTF8(options, format);
            auto alphaModeStr = WGPU_UTF8_OPT(options, alphaMode, "auto");
            auto alphaMode = StringToWGPUCompositeAlphaMode(alphaModeStr.data());

            WGPUSurfaceCapabilities capabilities = {0};
            wgpuSurfaceGetCapabilities(_surface->getWGPUSurface(), device->getAdapter(), &capabilities);

            if (capabilities.alphaModeCount == 0) {
                throw JSError(runtime, "Surface had 0 alpha modes");
            }

            auto isAlphaModeValid = cArrayContains(capabilities.alphaModes, capabilities.alphaModeCount, alphaMode);
            if (!isAlphaModeValid) {
                alphaMode = capabilities.alphaModes[0];
                jsLog(runtime, "warn", {"Alpha mode", alphaModeStr, "is not valid. Using", WGPUCompositeAlphaModeToString(alphaMode)});
            }

            auto width = _surface->getWidth();
            auto height = _surface->getHeight();

            if (width == 0 || height == 0) {
                throw JSError(runtime, "[WebGPU] Metal layer size was 0");
            }

            WGPUSurfaceConfiguration config = {
              .device = device->getValue(),
              .format = StringToWGPUTextureFormat(format),
              .usage = WGPU_NUMBER_OPT(options, usage, WGPUTextureUsage, WGPUTextureUsage_RenderAttachment),
              .viewFormatCount = 0,
              .viewFormats = nullptr,
              .alphaMode = alphaMode,
              .width = width,
              .height = height,
              .presentMode = WGPUPresentMode_Fifo, // TODO:
            };
            wgpuSurfaceConfigure(_surface->getWGPUSurface(), &config);
            _configuredContext = device->getContext();
            _surface->createTimer();

            return Value::undefined();
        });
    }

    if (name == "unconfigure") {
        return WGPU_FUNC_FROM_HOST_FUNC(unconfigure, 0, [this]) {
            _surface->invalidateTimer();
            wgpuSurfaceUnconfigure(_surface->getWGPUSurface());
            _configuredContext = nullptr;
            return Value::undefined();
        });
    }

    if (name == "surfaceCapabilities") {
        return WGPU_FUNC_FROM_HOST_FUNC(surfaceCapabilities, 1, [this]) {
            auto adapter = arguments[0].asObject(runtime).asHostObject<AdapterHostObject>(runtime)->_adapter->_adapter;
            auto surface = _surface->getWGPUSurface();

            WGPUSurfaceCapabilities capabilities;
            wgpuSurfaceGetCapabilities(surface, adapter, &capabilities);

            auto formats = cArrayToJsi(runtime, capabilities.formats, capabilities.formatCount, [](Runtime &runtime, WGPUTextureFormat format) {
                return Value(String::createFromAscii(runtime, WGPUTextureFormatToString(format)));
            });

            auto alphaModes = cArrayToJsi(runtime, capabilities.alphaModes, capabilities.alphaModeCount, [](Runtime &runtime, WGPUCompositeAlphaMode alphaMode) {
                return Value(String::createFromAscii(runtime, WGPUCompositeAlphaModeToString(alphaMode)));
            });

            auto result = Object(runtime);
            result.setProperty(runtime, PropNameID::forAscii(runtime, "formats"), std::move(formats));
            result.setProperty(runtime, PropNameID::forAscii(runtime, "alphaModes"), std::move(alphaModes));

            wgpuSurfaceCapabilitiesFreeMembers(capabilities);
            return std::move(result);
        });
    }

    if (name == "getCurrentTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(getCurrentTexture, 0, [this]) {
            if (_configuredContext == nullptr) {
                throw JSError(runtime, "Call context.configure() before context.getCurrentTexture()");
            }
            WGPUSurfaceTexture texture;
            wgpuSurfaceGetCurrentTexture(_surface->getWGPUSurface(), &texture);
            if (texture.status == WGPUSurfaceGetCurrentTextureStatus_Success) {
                return Object::createFromHostObject(runtime, std::make_shared<TextureHostObject>(texture.texture, _configuredContext, ""));
            } else if (texture.texture) {
                // TODO: check this
                wgpuTextureRelease(texture.texture);
            }
            return Value::null();
        });
    }

    if (name == "width") {
        return Value((int)_surface->getWidth());
    }

    if (name == "height") {
        return Value((int)_surface->getHeight());
    }

    return Value::undefined();
}

std::vector<PropNameID> ContextHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "presentSurface", "configure", "surfaceCapabilities", "getCurrentTexture", "width", "height", "destroy");
}
