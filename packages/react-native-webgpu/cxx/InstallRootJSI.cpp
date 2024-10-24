#include "InstallRootJSI.h"

#include <sstream>

#include "AdapterHostObject.h"
#include "Compression.h"
#include "ConstantConversion.h"
#include "ContextHostObject.h"
#include "CreateImageBitmap.h"
#include "Surface.h"
#include "SurfacesManager.h"
#include "VideoPlayer.h"
#include "WGPUJsiUtils.h"
#include "WGPUPromise.h"

#ifdef ANDROID
const char *defaultTextureFormat = "rgba8unorm-srgb";
#else
const char *defaultTextureFormat = "bgra8unorm-srgb";
#endif

using namespace facebook::jsi;

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message,
                                     void *userdata);
using OptionalSurface = std::weak_ptr<Surface>;

void wgpu::installRootJSI(Runtime &runtime, std::shared_ptr<JSIInstance> jsiInstance) {
  auto getSurfaceBackedWebGPU = WGPU_FUNC_FROM_HOST_FUNC(getWebGPUForSurface, 1, [jsiInstance]) {
    auto uuid = arguments[0].asString(runtime).utf8(runtime);
    auto weakSurface = SurfacesManager::getInstance()->get(uuid);
    if (weakSurface.expired()) {
      throw JSINativeException("getWebGPUForSurface failed to find surface");
    }

    auto result = Object(runtime);

    result.setProperty(runtime, "context",
                       Object::createFromHostObject(runtime, std::make_shared<ContextHostObject>(weakSurface)));
    result.setProperty(runtime, "requestAnimationFrame", WGPU_FUNC_FROM_HOST_FUNC(requestAnimationFrame, 1, [weakSurface]) {
      auto surface = weakSurface.lock();
      if (surface != nullptr) {
        surface->requestAnimationFrame(arguments[0].asObject(runtime).asFunction(runtime));
      }
      return Value::undefined();
    }));

    auto gpu = Object(runtime);
    gpu.setProperty(runtime, "requestAdapter", WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, [weakSurface, jsiInstance]) {
      return Promise::makeJSPromise(jsiInstance, [weakSurface](auto &runtime, auto &promise) {
        auto surface = weakSurface.lock();
        if (surface == nullptr) {
          promise->reject([](auto &runtime) { return makeJSError(runtime, "Surface was released from memory"); });
          return;
        }
        WGPURequestAdapterOptions adapterOptions = {
          .nextInChain = nullptr,
          .compatibleSurface = surface->getWGPUSurface(),
          .powerPreference = WGPUPowerPreference_Undefined,
          .backendType = WGPUBackendType_Undefined,
          .forceFallbackAdapter = false,
        };

        wgpuInstanceRequestAdapter(surface->getWGPUInstance(), &adapterOptions, wgpuHandleRequestAdapter,
                                   promise->toCDataWithExtras(weakSurface));
      });
    }));
    gpu.setProperty(runtime, "getPreferredCanvasFormat", WGPU_FUNC_FROM_HOST_FUNC(getPreferredCanvasFormat, 0, [weakSurface]) {
      auto surface = weakSurface.lock();
      if (surface == nullptr) {
        jsLog(runtime, "warn", {"Surface was released from memory"});

        return String::createFromUtf8(runtime, defaultTextureFormat);
      }
      auto adapter = surface->getUnownedWGPUAdapter().lock();
      if (adapter == nullptr) {
        jsLog(runtime, "warn",
              {"Adapter not found. Call navigator.gpu.requestAdapter before "
               "navigator.gpu.getPreferredCanvasFormat"});

        return String::createFromUtf8(runtime, defaultTextureFormat);
      }
      WGPUSurfaceCapabilities caps = {nullptr};
      wgpuSurfaceGetCapabilities(surface->getWGPUSurface(), adapter->_adapter, &caps);
      if (caps.formatCount == 0) {
        jsLog(runtime, "warn", {"Surface capabilities didn't return any supported texture formats"});
        return String::createFromUtf8(runtime, defaultTextureFormat);
      }
      return String::createFromUtf8(runtime, WGPUTextureFormatToString(caps.formats[0]));
    }));

    auto navigator = Object(runtime);
    navigator.setProperty(runtime, "gpu", std::move(gpu));

    result.setProperty(runtime, "navigator", std::move(navigator));

    return std::move(result);
  });

  auto getHeadlessWebGPU = WGPU_FUNC_FROM_HOST_FUNC(getWebGPUForHeadless, 0, [jsiInstance]) {
    // context, requestAnimationFrame, getPreferredCanvasFormat don't make sense without a
    // surface
    auto result = Object(runtime);

    auto gpu = Object(runtime);
    gpu.setProperty(runtime, "requestAdapter", WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, [jsiInstance]) {
      return Promise::makeJSPromise(jsiInstance, [](auto &runtime, auto &promise) {
        auto instance = wgpuCreateInstance(nullptr);

        if (instance == nullptr) {
          promise->reject([](auto &runtime) { return makeJSError(runtime, "Failed to make wgpu instance"); });
          return;
        }

        WGPURequestAdapterOptions adapterOptions = {
          .nextInChain = nullptr,
          .compatibleSurface = nullptr,
          .powerPreference = WGPUPowerPreference_Undefined,
          .backendType = WGPUBackendType_Undefined,
          .forceFallbackAdapter = false,
        };

        wgpuInstanceRequestAdapter(instance, &adapterOptions, wgpuHandleRequestAdapter,
                                   promise->toCDataWithExtras((OptionalSurface)std::shared_ptr<Surface>()));
      });
    }));
    auto navigator = Object(runtime);
    navigator.setProperty(runtime, "gpu", std::move(gpu));

    result.setProperty(runtime, "navigator", std::move(navigator));

    return std::move(result);
  });

  auto webgpu = Object(runtime);
  webgpu.setProperty(runtime, "createImageBitmap", createImageBitmap(runtime, jsiInstance));
  webgpu.setProperty(runtime, "getSurfaceBackedWebGPU", std::move(getSurfaceBackedWebGPU));
  webgpu.setProperty(runtime, "getHeadlessWebGPU", std::move(getHeadlessWebGPU));
  auto experimental = Object(runtime);
  experimental.setProperty(runtime, "inflate", inflate(runtime));
  experimental.setProperty(runtime, "makeVideoPlayer", VideoPlayer::factory(runtime));
  webgpu.setProperty(runtime, "experimental", std::move(experimental));

  runtime.global().setProperty(runtime, "__reactNativeWebGPU", std::move(webgpu));
}

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message,
                                     void *userdata) {
  Promise::fromCDataWithExtras<OptionalSurface>(userdata,
                                                [status, adapter, message](auto &promise, auto optionalSurface) {
    if (status == WGPURequestAdapterStatus_Success) {
      promise->resolve([promise, adapter, optionalSurface](auto &runtime) {
        auto adapterWrapper = std::make_shared<AdapterWrapper>(adapter);
        auto surface = optionalSurface.lock();
        if (surface != nullptr) {
          surface->setAdapter(adapterWrapper);
        }
        return Object::createFromHostObject(
          runtime, std::make_shared<AdapterHostObject>(adapterWrapper, promise->getJSIInstance()));
      });
    } else {
      std::ostringstream ss;
      ss << __FILE__ << ":" << __LINE__ << " navigator.gpu.requestAdapter() failed with status " << status << ". "
         << message;
      promise->reject([message = ss.str()](auto &runtime) { return makeJSError(runtime, message); });
    }
  });
}
