#import "WGPUJsi.h"
#import "React/RCTBridge+Private.h"
#import "webgpu.h"
#import <jsi/jsi.h>
#import "WGPUJsiUtils.h"
#import "AdapterHostObject.h"
#import <boost/format.hpp>
#import "ConstantConversion.h"
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#include "JSIInstance.h"
#include "CreateImageBitmap.h"

using namespace facebook::react;
using namespace facebook::jsi;
using namespace wgpu;

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata);
typedef struct HandleRequestAdapterData {
    std::shared_ptr<Surface> optionalSurface;
} HandleRequestAdapterData;

@interface RCTBridge ()
- (std::shared_ptr<CallInvoker>)jsCallInvoker;
@end

@implementation WGPUJsi

RCT_EXPORT_MODULE(WGPUJsi)

@synthesize bridge;

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
    RCTCxxBridge *cxxBridge = (RCTCxxBridge*)self.bridge;
    if (cxxBridge == nil) {
        NSLog(@"Cxx bridge not found");
        return @(NO);
    }

    auto &runtime = *(Runtime *)cxxBridge.runtime;

    auto surfaces = std::make_shared<std::unordered_map<std::string, std::shared_ptr<Surface>>>();
    JSIInstance::instance = std::make_unique<JSIInstance>(runtime, std::make_shared<Thread>([cxxBridge jsCallInvoker]));
    JSIInstance::instance->onCreateSurface = [surfaces](std::string uuid, std::shared_ptr<Surface> surface) {
        surfaces->insert_or_assign(uuid, surface);
    };

    auto getSurfaceBackedWebGPU = WGPU_FUNC_FROM_HOST_FUNC(getWebGPUForSurface, 1, [surfaces]) {
        auto uuid = arguments[0].asString(runtime).utf8(runtime);
        auto surfaceHandle = surfaces->extract(uuid);
        if (surfaceHandle.empty()) {
            throw JSError(runtime, "getWebGPUForSurface failed to find surface");
        }
        auto surface = std::move(surfaceHandle.mapped());

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
                    .compatibleSurface = surface->getWGPUSurface(),
                    .backendType = WGPUBackendType_Undefined,
                    .powerPreference = WGPUPowerPreference_Undefined,
                    .forceFallbackAdapter = false,
                    .nextInChain = NULL,
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
                    .compatibleSurface = NULL,
                    .backendType = WGPUBackendType_Undefined,
                    .powerPreference = WGPUPowerPreference_Undefined,
                    .forceFallbackAdapter = false,
                    .nextInChain = NULL,
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

    return @(YES);
}

@end

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
        auto error = boost::format("[%s] %#.8x %s") % __FILE_NAME__ % status % message;
        promise->reject(String::createFromUtf8(runtime, error.str()));
    }
    delete promise;
}
