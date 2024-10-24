#include "ShaderModuleHostObject.h"

#include "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

struct WGPUShaderCompilationMessage {
  std::string message;
  uint64_t lineNum;
  uint64_t linePos;
  uint64_t offset;
  uint64_t length;
};

void wgpuShaderModuleGetCompilationInfoCallback(WGPUCompilationInfoRequestStatus status,
                                                struct WGPUCompilationInfo const *compilationInfo,
                                                WGPU_NULLABLE void *userdata);

Value ShaderModuleHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "getCompilationInfo") {
    return WGPU_FUNC_FROM_HOST_FUNC(getCompilationInfo, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(getCompilationInfo);
      return Promise::makeJSPromise(_context->getJSIInstance(), [this](auto &runtime, auto &promise) {
        wgpuShaderModuleGetCompilationInfo(_value, wgpuShaderModuleGetCompilationInfoCallback, promise->toCData());
      });
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> ShaderModuleHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "getCompilationInfo", "label");
}

void wgpuShaderModuleGetCompilationInfoCallback(WGPUCompilationInfoRequestStatus status,
                                                struct WGPUCompilationInfo const *compilationInfo,
                                                WGPU_NULLABLE void *userdata) {
  Promise::fromCData(userdata, [status, compilationInfo](auto &promise) {
    if (status == WGPUCompilationInfoRequestStatus_Success) {
      std::vector<WGPUShaderCompilationMessage> messages;
      for (auto i = 0; i < compilationInfo->messageCount; i++) {
        auto message = compilationInfo->messages[i];
        messages.emplace_back((WGPUShaderCompilationMessage const){
          .message = message.message,
          .lineNum = message.lineNum,
          .linePos = message.linePos,
          .offset = message.offset,
          .length = message.length,
        });
      }
      promise->resolve([messagesIn = std::move(messages)](auto &runtime) {
        auto messages = cArrayToJsi(runtime, messagesIn.data(), messagesIn.size(),
                                    [](Runtime &runtime, WGPUShaderCompilationMessage item) {
          auto obj = Object(runtime);
          WGPU_SET_UTF8(obj, message, item.message);
          WGPU_SET_INT(obj, lineNum, item.lineNum);
          WGPU_SET_INT(obj, linePos, item.linePos);
          WGPU_SET_INT(obj, offset, item.offset);
          WGPU_SET_INT(obj, length, item.length);
          return obj;
        });

        auto result = Object(runtime);
        result.setProperty(runtime, "messages", messages);
        return result;
      });
    } else {
      promise->reject([](auto &runtime) { return makeJSError(runtime, "getCompilationInfo failed"); });
    }
  });
}
