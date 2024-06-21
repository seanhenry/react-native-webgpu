#include "ShaderModuleHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

void wgpuShaderModuleGetCompilationInfoCallback(WGPUCompilationInfoRequestStatus status, struct WGPUCompilationInfo const *compilationInfo, WGPU_NULLABLE void *userdata) {
    Promise *promise = (Promise *)userdata;
    Runtime &runtime = promise->runtime;
    if (status == WGPUCompilationInfoRequestStatus_Success) {
        auto messages = cArrayToJsi(runtime, compilationInfo->messages, compilationInfo->messageCount, [](Runtime &runtime, WGPUCompilationMessage item) {
            auto obj = Object(runtime);
            obj.setProperty(runtime, "message", String::createFromUtf8(runtime, item.message));
            obj.setProperty(runtime, "lineNum", Value((int)item.lineNum));
            obj.setProperty(runtime, "linePos", Value((int)item.linePos));
            obj.setProperty(runtime, "offset", Value((int)item.offset));
            obj.setProperty(runtime, "length", Value((int)item.length));
            return obj;
        });
        
        auto result = Object(runtime);
        result.setProperty(runtime, "messages", messages);
        promise->resolve->call(runtime, std::move(result));
    } else {
        promise->reject->call(runtime, "getCompilationInfo error");
    }
    delete promise;
}

Value ShaderModuleHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    // Not supported on iOS
    if (name == "getCompilationInfo") {
        return WGPU_FUNC_FROM_HOST_FUNC(getCompilationInfo, 0, [this]) {
            return makePromise(runtime, [this](Promise *promise) {
                wgpuShaderModuleGetCompilationInfo(_value, wgpuShaderModuleGetCompilationInfoCallback, &promise);
            });
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> ShaderModuleHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "getCompilationInfo");
}
