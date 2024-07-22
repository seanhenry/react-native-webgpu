#include "InstallRootJSI.h"
#include "WGPUJsiUtils.h"
#include "Surface.h"
#include "ContextHostObject.h"
#include "AdapterHostObject.h"
#include "CreateImageBitmap.h"
#include "ConstantConversion.h"
#include <sstream>

using namespace facebook::jsi;

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata);
typedef struct HandleRequestAdapterData {
    std::shared_ptr<Surface> optionalSurface;
} HandleRequestAdapterData;

void wgpu::installRootJSI(Runtime &runtime, const std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Surface>>>& newSurfaces) {
    auto getSurfaceBackedWebGPU = WGPU_FUNC_FROM_HOST_FUNC(getWebGPUForSurface, 1, [newSurfaces]) {
        auto uuid = arguments[0].asString(runtime).utf8(runtime);
        auto surfaceHandle = newSurfaces->extract(uuid);
        if (surfaceHandle.empty()) {
            throw JSError(runtime, "getWebGPUForSurface failed to find surface");
        }
        auto surface = std::move(surfaceHandle.mapped());
        (*JSIInstance::instance->weakSurfaces)[uuid] = surface;

        auto result = Object(runtime);

        result.setProperty(runtime, "context", Object::createFromHostObject(runtime, std::make_shared<ContextHostObject>(surface)));
        result.setProperty(runtime, "requestAnimationFrame", WGPU_FUNC_FROM_HOST_FUNC(requestAnimationFrame, 1, [surface]) {
            surface->requestAnimationFrame(arguments[0].asObject(runtime).asFunction(runtime));
            return Value::undefined();
        }));

        auto gpu = Object(runtime);
        gpu.setProperty(runtime, "requestAdapter", WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, [surface]) {
            auto promise = new Promise<HandleRequestAdapterData>(runtime);
            return promise->jsPromise([promise, surface]() {
                promise->data = {
                    .optionalSurface = surface,
                };
                WGPURequestAdapterOptions adapterOptions = {
                    .nextInChain = nullptr,
                    .compatibleSurface = surface->getWGPUSurface(),
                    .powerPreference = WGPUPowerPreference_Undefined,
                    .backendType = WGPUBackendType_Undefined,
                    .forceFallbackAdapter = false,
                };

                wgpuInstanceRequestAdapter(surface->getWGPUInstance(), &adapterOptions, wgpuHandleRequestAdapter, promise);
            });
        }));
        gpu.setProperty(runtime, "getPreferredCanvasFormat", WGPU_FUNC_FROM_HOST_FUNC(getPreferredCanvasFormat, 0, [surface]) {
            auto adapter = surface->getUnownedWGPUAdapter().lock();
            if (adapter == nullptr) {
                jsLog(runtime, "warn", {"Adapter not found. Call navigator.gpu.requestAdapter before navigator.gpu.getPreferredCanvasFormat"});
                return String::createFromUtf8(runtime, "bgra8unorm-srgb");
            }
            auto format = wgpuSurfaceGetPreferredFormat(surface->getWGPUSurface(), adapter->_adapter);
            return String::createFromUtf8(runtime, WGPUTextureFormatToString(format));
        }));

        auto navigator = Object(runtime);
        navigator.setProperty(runtime, "gpu", std::move(gpu));

        result.setProperty(runtime, "navigator", std::move(navigator));

        return std::move(result);
    });

    auto getHeadlessWebGPU = WGPU_FUNC_FROM_HOST_FUNC(getWebGPUForHeadless, 0, []) {
        // context, requestAnimationFrame, getPreferredCanvasFormat don't make sense without a surface
        auto result = Object(runtime);

        auto gpu = Object(runtime);
        gpu.setProperty(runtime, "requestAdapter", WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, []) {
            auto promise = new Promise<HandleRequestAdapterData>(runtime);
            return promise->jsPromise([promise]() {
                auto instance = wgpuCreateInstance(nullptr);

                if (instance == nullptr) {
                    promise->reject(makeJSError(promise->runtime, "Failed to make wgpu instance"));
                    delete promise;
                    return;
                }

                WGPURequestAdapterOptions adapterOptions = {
                    .nextInChain = nullptr,
                    .compatibleSurface = nullptr,
                    .powerPreference = WGPUPowerPreference_Undefined,
                    .backendType = WGPUBackendType_Undefined,
                    .forceFallbackAdapter = false,
                };

                wgpuInstanceRequestAdapter(instance, &adapterOptions, wgpuHandleRequestAdapter, promise);
            });
        }));
        auto navigator = Object(runtime);
        navigator.setProperty(runtime, "gpu", std::move(gpu));

        result.setProperty(runtime, "navigator", std::move(navigator));

        return std::move(result);
    });

    auto webgpu = Object(runtime);
    webgpu.setProperty(runtime, "createImageBitmap", createImageBitmap(runtime));
    webgpu.setProperty(runtime, "getSurfaceBackedWebGPU", std::move(getSurfaceBackedWebGPU));
    webgpu.setProperty(runtime, "getHeadlessWebGPU", std::move(getHeadlessWebGPU));

    runtime.global().setProperty(runtime, "__reactNativeWebGPU", std::move(webgpu));
}

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata) {
    auto promise = (Promise<HandleRequestAdapterData> *)userdata;
    Runtime &runtime = promise->runtime;

    if (status == WGPURequestAdapterStatus_Success) {
        auto adapterWrapper = std::make_shared<AdapterWrapper>(adapter);
        if (promise->data.optionalSurface != nullptr) {
            promise->data.optionalSurface->setAdapter(adapterWrapper);
        }
        auto adapterHostObject = Object::createFromHostObject(runtime, std::make_shared<AdapterHostObject>(adapterWrapper));
        promise->resolve(std::move(adapterHostObject));
    } else {
        std::ostringstream ss;
        ss << __FILE__ << ":" << __LINE__ << " navigator.gpu.requestAdapter() failed with status " << status << ". " << message;
        promise->reject(String::createFromUtf8(runtime, ss.str()));
    }
    delete promise;
}
