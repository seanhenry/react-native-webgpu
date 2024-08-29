#pragma once

#include <jsi/jsi.h>

#include <memory>
#include <queue>
#include <string>

#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

typedef struct Error {
  WGPUErrorType type;
  std::string message;
} Error;

class ErrorHandler {
 public:
  explicit ErrorHandler() {}
  void pushError(WGPUErrorType type, char const *message) {
    _errors.push({
      .type = type,
      .message = std::string(message),
    });
  }
  bool hasError() { return !_errors.empty(); }
  Error popError() {
    auto error = std::move(_errors.front());
    _errors.pop();
    return error;
  }
  void throwPendingJSIError() {
    if (hasError()) {
      auto error = popError();
      throw JSINativeException(error.message.data());
    }
  }

 private:
  std::queue<Error> _errors;
};

}  // namespace wgpu
