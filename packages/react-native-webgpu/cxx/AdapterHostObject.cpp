#include "AdapterHostObject.h"

#include <stdio.h>

#include <sstream>

#include "ConstantConversion.h"
#include "DeviceHostObject.h"
#include "ErrorHandler.h"
#include "Promise.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"

using namespace facebook::jsi;
using namespace wgpu;

static void wgpuErrorCallback(WGPUErrorType type, char const *message, void *userdata);
static void wgpuHandleRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const *message,
                                    void *userdata);
typedef struct HandleRequestDeviceUserData {
  std::shared_ptr<AdapterWrapper> adapter;
  std::shared_ptr<ErrorHandler> errorHandler;
  std::shared_ptr<JSIInstance> jsiInstance;
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
      auto promise = new Promise<HandleRequestDeviceUserData>(runtime);
      return promise->jsPromise([this, promise, sharedObj]() {
        auto errorHandler = std::make_shared<ErrorHandler>();
        promise->data = (const HandleRequestDeviceUserData){
          .adapter = _adapter,
          .errorHandler = errorHandler,
          .jsiInstance = _jsiInstance,
        };
        std::vector<WGPUFeatureName> requiredFeatures;
        WGPUDeviceDescriptor descriptor = {nullptr};
        descriptor.uncapturedErrorCallbackInfo = {
          .nextInChain = nullptr,
          .callback = wgpuErrorCallback,
          .userdata = errorHandler.get(),
        };

        if (sharedObj != nullptr) {
          auto &runtime = promise->runtime;
          auto obj = std::move(*sharedObj.get());

          if (obj.hasProperty(runtime, "requiredFeatures")) {
            auto requiredFeaturesIn = obj.getPropertyAsObject(runtime, "requiredFeatures").asArray(runtime);
            requiredFeatures = jsiArrayToVector<WGPUFeatureName>(runtime, std::move(requiredFeaturesIn),
                                                                 [](Runtime &runtime, Value value) {
              auto str = value.asString(runtime).utf8(runtime);
              return StringToWGPUFeatureName(str);
            });
            descriptor.requiredFeatures = requiredFeatures.data();
            descriptor.requiredFeatureCount = requiredFeatures.size();
          }
        }
        wgpuAdapterRequestDevice(_adapter->_adapter, &descriptor, wgpuHandleRequestDevice, promise);
      });
    });
  }

  if (name == "features") {
    auto size = wgpuAdapterEnumerateFeatures(_adapter->_adapter, NULL);
    std::vector<WGPUFeatureName> features;
    features.resize(size);
    wgpuAdapterEnumerateFeatures(_adapter->_adapter, features.data());
    return makeJsiFeatures(runtime, features);
  }

  if (name == "limits") {
    WGPUSupportedLimits limits = {0};
    wgpuAdapterGetLimits(_adapter->_adapter, &limits);
    return makeJsiLimits(runtime, limits.limits);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> AdapterHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "requestDevice", "features", "limits");
}

static void wgpuErrorCallback(WGPUErrorType type, char const *message, void *userdata) {
  // ErrorHandler is retained in WGPUContext so will survive as long as the device, and therefore this callback
  auto errorHandler = (ErrorHandler *)userdata;
  errorHandler->pushError(type, message);
}

static void wgpuHandleRequestDevice(WGPURequestDeviceStatus status, WGPUDevice device, char const *message,
                                    void *userdata) {
  auto promise = (Promise<HandleRequestDeviceUserData> *)userdata;
  Runtime &runtime = promise->runtime;

  if (status == WGPURequestDeviceStatus_Success) {
    auto deviceWrapper = std::make_shared<DeviceWrapper>(device);
    auto context = std::make_shared<WGPUContext>(promise->data.adapter, deviceWrapper, promise->data.errorHandler,
                                                 promise->data.jsiInstance);
    auto deviceHostObject =
      Object::createFromHostObject(runtime, std::make_shared<DeviceHostObject>(deviceWrapper, context));
    promise->resolve(std::move(deviceHostObject));
  } else {
    std::ostringstream ss;
    ss << __FILE__ << ":" << __LINE__ << " Adapter.requestDevice() failed with status " << status << ". " << message;
    promise->reject(String::createFromUtf8(runtime, ss.str()));
  }
  delete promise;
}
