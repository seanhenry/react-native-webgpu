#include "InstallRootJSI.h"

#include <sstream>

#include "AdapterHostObject.h"
#include "Compression.h"
#include "ConstantConversion.h"
#include "ContextHostObject.h"
#include "CreateImageBitmap.h"
#include "Promise.h"
#include "Surface.h"
#include "SurfacesManager.h"
#include "VideoPlayer.h"
#include "WGPUJsiUtils.h"

#ifdef ANDROID
const char *defaultTextureFormat = "rgba8unorm-srgb";
#else
const char *defaultTextureFormat = "bgra8unorm-srgb";
#endif

using namespace facebook::jsi;

static void wgpuHandleRequestAdapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message,
                                     void *userdata);
typedef struct HandleRequestAdapterData {
  std::weak_ptr<Surface> optionalSurface;
  std::shared_ptr<JSIInstance> jsiInstance;
} HandleRequestAdapterData;

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
      auto promise = new Promise<HandleRequestAdapterData>(runtime);
      return promise->jsPromise([promise, weakSurface, jsiInstance]() {
        auto surface = weakSurface.lock();
        if (surface == nullptr) {
          promise->reject(makeJSError(promise->runtime, "Surface was released from memory"));
          delete promise;
          return;
        }
        promise->data = {
          .optionalSurface = weakSurface,
          .jsiInstance = jsiInstance,
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
      auto promise = new Promise<HandleRequestAdapterData>(runtime);
      promise->data = {
        .optionalSurface = std::shared_ptr<Surface>(),
        .jsiInstance = jsiInstance,
      };
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
  auto promise = (Promise<HandleRequestAdapterData> *)userdata;
  Runtime &runtime = promise->runtime;

  if (status == WGPURequestAdapterStatus_Success) {
    auto adapterWrapper = std::make_shared<AdapterWrapper>(adapter);
    auto surface = promise->data.optionalSurface.lock();
    if (surface != nullptr) {
      surface->setAdapter(adapterWrapper);
    }
    auto adapterHostObject = Object::createFromHostObject(
      runtime, std::make_shared<AdapterHostObject>(adapterWrapper, promise->data.jsiInstance));
    promise->resolve(std::move(adapterHostObject));
  } else {
    std::ostringstream ss;
    ss << __FILE__ << ":" << __LINE__ << " navigator.gpu.requestAdapter() failed with status " << status << ". "
       << message;
    promise->reject(makeJSError(runtime, ss.str()));
  }
  delete promise;
}
