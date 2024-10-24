#pragma once

#include <jsi/jsi.h>

#include "JSIInstance.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

namespace wgpu {

class Promise;

using Callback = std::function<void(Runtime &, const std::shared_ptr<Promise> &)>;
template <typename T>
using CDataCallbackWithExtras = std::function<void(const std::shared_ptr<Promise> &, const T data)>;
using CDataCallback = std::function<void(const std::shared_ptr<Promise> &)>;

template <typename T>
struct WGPUPromise_CData {
  std::shared_ptr<Promise> promise;
  T data;
};

class Promise : public std::enable_shared_from_this<Promise> {
 public:
  static Value makeJSPromise(const std::shared_ptr<JSIInstance> &jsiInstance, Callback &&fn);
  template <typename T>
  static void fromCDataWithExtras(void *data, CDataCallbackWithExtras<T> &&fn);
  static void fromCData(void *data, CDataCallback &&fn);

  explicit Promise(const std::shared_ptr<JSIInstance> &jsiInstance) : _jsiInstance(jsiInstance) {}

  void *toCData();
  template <typename T>
  void *toCDataWithExtras(T data);

  void finalize(std::function<void(Runtime &runtime, Function &resolve, Function &reject)> &&callback);
  void resolve(std::function<Value(Runtime &)> &&fn);
  void reject(std::function<Value(Runtime &)> &&fn);
  inline std::shared_ptr<JSIInstance> getJSIInstance() { return _jsiInstance; }

 private:
  class ResolveReject : public HostObject {
   public:
    ResolveReject(Function &&resolve, Function &&reject) : _resolve(std::move(resolve)), _reject(std::move(reject)) {}
    Function _resolve;
    Function _reject;
    std::shared_ptr<Object> _promise;
  };

  std::weak_ptr<ResolveReject> _resolveReject;
  std::shared_ptr<JSIInstance> _jsiInstance;
};

#pragma mark - C data interop

template <typename T>
void *Promise::toCDataWithExtras(const T data) {
  auto cData = new WGPUPromise_CData<T>{
    .promise = this->shared_from_this(),
    .data = data,
  };
  return (void *)cData;
}

template <typename T>
void Promise::fromCDataWithExtras(void *data, CDataCallbackWithExtras<T> &&fn) {
  auto cData = static_cast<WGPUPromise_CData<T> *>(data);
  fn(cData->promise, std::move(cData->data));
  delete cData;
}

#pragma mark - JS convenience

static Value makeRejectedJSPromise(Runtime &runtime, Value error) {
  return runtime.global()
    .getPropertyAsObject(runtime, "Promise")
    .getPropertyAsFunction(runtime, "reject")
    .call(runtime, std::move(error));
}

}  // namespace wgpu
