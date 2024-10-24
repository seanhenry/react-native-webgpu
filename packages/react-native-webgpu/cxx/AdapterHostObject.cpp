#include "AdapterHostObject.h"

#include <stdio.h>

#include <sstream>

#include "ConstantConversion.h"
#include "DeviceHostObject.h"
#include "ErrorHandler.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"
#include "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

static void wgpuErrorCallback(WGPUErrorType type, char const *message, void *userdata);
static void wgpuHandleRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const *message,
                                    void *userdata);
static Object wgpuMakeJsAdapterInfo(Runtime &runtime, WGPUAdapter adapter);

typedef struct HandleRequestDeviceUserData {
  std::shared_ptr<AdapterWrapper> adapter;
  std::shared_ptr<ErrorHandler> errorHandler;
} HandleRequestDeviceUserData;

Value AdapterHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "requestDevice") {
    return WGPU_FUNC_FROM_HOST_FUNC(requestDevice, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(requestDevice);
      std::shared_ptr<Object> sharedObj;
      if (count > 0 && arguments[0].isObject()) {
        sharedObj = std::make_shared<Object>(arguments[0].asObject(runtime));
      }
      return Promise::makeJSPromise(_jsiInstance,
                                    [adapterWrapper = this->_adapter, sharedObj](auto &runtime, auto &promise) {
        auto errorHandler = std::make_shared<ErrorHandler>();
        std::vector<WGPUFeatureName> requiredFeatures;
        auto requiredLimits = std::shared_ptr<WGPURequiredLimits>(nullptr);
        WGPUDeviceDescriptor descriptor = {nullptr};
        descriptor.uncapturedErrorCallbackInfo = {
          .nextInChain = nullptr,
          .callback = wgpuErrorCallback,
          .userdata = errorHandler.get(),
        };

        if (sharedObj != nullptr) {
          auto obj = std::move(*sharedObj.get());

          if (WGPU_HAS_PROP(obj, requiredFeatures)) {
            auto requiredFeaturesIn = WGPU_ARRAY(obj, requiredFeatures);
            requiredFeatures = jsiArrayToVector<WGPUFeatureName>(runtime, std::move(requiredFeaturesIn),
                                                                 [](Runtime &runtime, Value value) {
              auto str = value.asString(runtime).utf8(runtime);
              return StringToWGPUFeatureName(str);
            });
            descriptor.requiredFeatures = requiredFeatures.data();
            descriptor.requiredFeatureCount = requiredFeatures.size();
          }
          if (WGPU_HAS_PROP(obj, requiredLimits)) {
            auto requiredLimitsIn = WGPU_OBJ(obj, requiredLimits);
            WGPUSupportedLimits supportedLimits;
            wgpuAdapterGetLimits(adapterWrapper->_adapter, &supportedLimits);
            requiredLimits = std::make_shared<WGPURequiredLimits>((const WGPURequiredLimits){
              .nextInChain = nullptr,
              .limits = makeWGPULimits(runtime, requiredLimitsIn, supportedLimits),
            });
            descriptor.requiredLimits = requiredLimits.get();
          }
        }
        HandleRequestDeviceUserData data = {
          .adapter = adapterWrapper,
          .errorHandler = errorHandler,
        };
        wgpuAdapterRequestDevice(adapterWrapper->_adapter, &descriptor, wgpuHandleRequestDevice,
                                 promise->toCDataWithExtras(std::move(data)));
      });
    });
  }

  if (name == "features") {
    auto size = wgpuAdapterEnumerateFeatures(_adapter->_adapter, nullptr);
    std::vector<WGPUFeatureName> features;
    features.resize(size);
    wgpuAdapterEnumerateFeatures(_adapter->_adapter, features.data());
    return makeJsiFeatures(runtime, features);
  }

  if (name == "limits") {
    WGPUSupportedLimits limits = {};
    wgpuAdapterGetLimits(_adapter->_adapter, &limits);
    return makeJsiLimits(runtime, limits.limits);
  }

  if (name == "info") {
    return wgpuMakeJsAdapterInfo(runtime, _adapter->_adapter);
  }

  if (name == "requestAdapterInfo") {
    return WGPU_FUNC_FROM_HOST_FUNC(requestAdapterInfo, 0, [this]) {
      return Promise::makeJSPromise(_jsiInstance, [this](auto &runtime, auto &promise) {
        promise->resolve(
          [adapter = _adapter->_adapter](auto &runtime) { return wgpuMakeJsAdapterInfo(runtime, adapter); });
      });
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> AdapterHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "requestDevice", "features", "limits", "info", "requestAdapterInfo");
}

static void wgpuErrorCallback(WGPUErrorType type, char const *message, void *userdata) {
  // ErrorHandler is retained in WGPUContext so will survive as long as the device, and therefore this callback
  auto errorHandler = (ErrorHandler *)userdata;
  errorHandler->pushError(type, message);
}

static void wgpuHandleRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const *message,
                                    void *userdata) {
  Promise::fromCDataWithExtras<HandleRequestDeviceUserData>(userdata,
                                                            [status, device, message](auto &promise, auto data) {
    if (status == WGPURequestDeviceStatus_Success) {
      auto deviceWrapper = std::make_shared<DeviceWrapper>(device);
      auto context =
        std::make_shared<WGPUContext>(data.adapter, deviceWrapper, data.errorHandler, promise->getJSIInstance());
      auto deviceHostObject = std::make_shared<DeviceHostObject>(deviceWrapper, context);

      promise->resolve(
        [deviceHostObject](auto &runtime) { return Object::createFromHostObject(runtime, deviceHostObject); });
    } else {
      std::ostringstream ss;
      ss << __FILE__ << ":" << __LINE__ << " Adapter.requestDevice() failed with status " << status << ". " << message;
      promise->reject([message = ss.str()](auto &runtime) { return makeJSError(runtime, message); });
    }
  });
}

static Object wgpuMakeJsAdapterInfo(Runtime &runtime, WGPUAdapter adapter) {
  WGPUAdapterInfo info;
  wgpuAdapterGetInfo(adapter, &info);
  Object jsInfo(runtime);
  WGPU_SET_UTF8(jsInfo, vendor, info.vendor);
  WGPU_SET_UTF8(jsInfo, architecture, info.architecture);
  WGPU_SET_UTF8(jsInfo, device, info.device);
  WGPU_SET_UTF8(jsInfo, description, info.description);
  return jsInfo;
}
