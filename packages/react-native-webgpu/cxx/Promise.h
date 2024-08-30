#pragma once

#include <jsi/jsi.h>
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

namespace wgpu {

template <typename T>
class Promise {
 public:
  explicit Promise(Runtime &runtime) : runtime(runtime) {}
  Value jsPromise(std::function<void()> &&fn);
  void resolve(Value &&value) { _resolve->call(runtime, std::move(value)); }
  void reject(Value &&value) { _reject->call(runtime, std::move(value)); }
  Runtime &runtime;
  T data;

 private:
  std::shared_ptr<Function> _resolve;
  std::shared_ptr<Function> _reject;
};

template <typename T>
Value Promise<T>::jsPromise(std::function<void()> &&fn) {
  auto promiseCallbackFn = WGPU_FUNC_FROM_HOST_FUNC(promiseCallbackFn, 2, [ this, fn = std::move(fn) ]) {
    _resolve = std::make_shared<Function>(arguments[0].asObject(runtime).asFunction(runtime));
    _reject = std::make_shared<Function>(arguments[1].asObject(runtime).asFunction(runtime));
    fn();
    return Value::undefined();
  });

  return runtime.global().getPropertyAsFunction(runtime, "Promise").callAsConstructor(runtime, promiseCallbackFn);
}

}  // namespace wgpu
