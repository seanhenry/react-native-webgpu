#include "ContextHostObject.h"
#include "DeviceHostObject.h"
#include "TextureHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include <iostream>

using namespace facebook::jsi;
using namespace wgpu;

Value ContextHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "presentSurface") {
        return WGPU_FUNC_FROM_HOST_FUNC(present, 0, [this]) {
            wgpuSurfacePresent(_context->_surface);
            return Value::undefined();
        });
    }

    if (name == "configure") {
        return WGPU_FUNC_FROM_HOST_FUNC(configure, 0, [this]) {
            auto options = arguments[0].asObject(runtime);
            auto device = WGPU_HOST_OBJ(options, device, DeviceHostObject);
            auto format = WGPU_UTF8(options, format);
            auto alphaMode = WGPU_UTF8_OPT(options, alphaMode, "auto");
            auto surface = _context->_surface;
            auto width = _context->_getWidth();
            auto height = _context->_getHeight();

            if (width == 0 || height == 0) {
                throw JSError(runtime, "[WebGPU] Metal layer size was 0");
            }

            WGPUSurfaceConfiguration config = {
              .width = width,
              .height = height,
              .device = device->_value,
              .format = StringToWGPUTextureFormat(format),
              .usage = WGPUTextureUsage_RenderAttachment, // TODO: this is the default
              .presentMode = WGPUPresentMode_Fifo, // TODO:
//              .viewFormats = {},
//              .viewFormatCount = 0,
              .alphaMode = StringToWGPUCompositeAlphaMode(alphaMode.data()),
            };
            wgpuSurfaceConfigure(surface, &config);

            return Value::undefined();
        });
    }

    if (name == "surfaceCapabilities") {
        auto surface = this->_context->_surface;
        auto adapter = this->_context->_adapter;

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
        return std::move(result);
    }

    if (name == "getCurrentTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(getCurrentTexture, 0, [this]) {
            WGPUSurfaceTexture texture;
            wgpuSurfaceGetCurrentTexture(_context->_surface, &texture);
            if (texture.status == WGPUSurfaceGetCurrentTextureStatus_Success) {
                return Object::createFromHostObject(runtime, std::make_shared<TextureHostObject>(texture.texture, _context));
            } else if (texture.texture) {
                // TODO: check this
                wgpuTextureRelease(texture.texture);
            }
            return Value::null();
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> ContextHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "configure", "surfaceCapabilities", "getCurrentTexture");
}
