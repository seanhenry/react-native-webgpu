#include "AdapterHostObject.h"
#include "DeviceHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include "WGPUConversions.h"
#include <stdio.h>
#include <boost/format.hpp>

using namespace facebook::jsi;
using namespace wgpu;

static void handle_request_device(WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata);

Value AdapterHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "requestDevice") {
        return WGPU_FUNC_FROM_HOST_FUNC(requestDevice, 1, [this]) {
            std::shared_ptr<Object> sharedObj;
            if (count > 0 && arguments[0].isObject()) {
                sharedObj = std::make_shared<Object>(arguments[0].asObject(runtime));
            }
            return makePromise(runtime, _context, [this, sharedObj](Promise *promise) {
                if (sharedObj != nullptr) {
                    auto &runtime = promise->runtime;
                    auto obj = std::move(*sharedObj.get());
                    WGPUDeviceDescriptor descriptor = {0};

                    std::vector<WGPUFeatureName> requiredFeatures;
                    if (obj.hasProperty(runtime, "requiredFeatures")) {
                        auto requiredFeaturesIn = obj.getPropertyAsObject(runtime, "requiredFeatures").asArray(runtime);
                        requiredFeatures = jsiArrayToVector<WGPUFeatureName>(runtime, std::move(requiredFeaturesIn), [](Runtime &runtime, Value value) {
                            auto str = value.asString(runtime).utf8(runtime);
                            return StringToWGPUFeatureName(str.data());
                        });
                        descriptor.requiredFeatures = requiredFeatures.data();
                        descriptor.requiredFeatureCount = requiredFeatures.size();
                    }

                    wgpuAdapterRequestDevice(_value, &descriptor, handle_request_device, promise);
                } else {
                    wgpuAdapterRequestDevice(_value, NULL, handle_request_device, promise);
                }

            });
        });
    }

    if (name == "features") {
        auto size = wgpuAdapterEnumerateFeatures(_value, NULL);
        std::vector<WGPUFeatureName> features;
        features.resize(size);
        wgpuAdapterEnumerateFeatures(_value, features.data());
        return makeJsiFeatures(runtime, &features);
    }

    if (name == "limits") {
        WGPUSupportedLimits limits = {0};
        wgpuAdapterGetLimits(_value, &limits);
        return makeJsiLimits(runtime, &limits.limits);
    }

    return Value::undefined();
}

std::vector<PropNameID> AdapterHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "requestDevice", "features", "limits");
}

static void handle_request_device(WGPURequestDeviceStatus status, WGPUDevice device, char const *message, void *userdata) {
    auto promise = (Promise *)userdata;
    auto context = promise->context;
    Runtime &runtime = promise->runtime;
    if (status == WGPURequestDeviceStatus_Success) {
        auto deviceHostObject = Object::createFromHostObject(runtime, std::make_shared<DeviceHostObject>(device, context));
        promise->resolve->call(runtime, std::move(deviceHostObject));
    } else {
        auto error = boost::format("[%s] %#.8x %s") % __FILE_NAME__ % status % message;
        promise->reject->call(runtime, String::createFromUtf8(runtime, error.str()));
    }
    delete promise;
}
