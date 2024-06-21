#include "AdapterHostObject.h"
#include "DeviceHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include <stdio.h>
#include <boost/format.hpp>

using namespace facebook::jsi;
using namespace wgpu;

static void handle_request_device(WGPURequestDeviceStatus status,
                                  WGPUDevice device, char const *message,
                                  void *userdata) {
    auto promise = (Promise *)userdata;
    auto context = (WGPUContext *)promise->userData;
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

Value AdapterHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "requestDevice") {
        return WGPU_FUNC_FROM_HOST_FUNC(requestDevice, 1, [this]) {
            return makePromise(runtime, [this](Promise *promise) {
                promise->userData = (void *)_context;
                wgpuAdapterRequestDevice(_value, NULL, handle_request_device, promise);
            });
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> AdapterHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "requestDevice");
}
