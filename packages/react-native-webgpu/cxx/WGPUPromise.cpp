#include "WGPUPromise.h"

#include "JSIInstance.h"

Value Promise::makeJSPromise(const std::shared_ptr<JSIInstance> &jsiInstance, Callback &&fn) {
  auto &runtime = jsiInstance->runtime;
  std::shared_ptr<ResolveReject> resolveReject;
  auto promiseCallbackFn =
    WGPU_FUNC_FROM_HOST_FUNC(promiseCallbackFn, 2, [&resolveReject, jsiInstance, fn = std::move(fn) ]) {
    auto promise = std::make_shared<Promise>(jsiInstance);
    resolveReject = std::make_shared<ResolveReject>(arguments[0].asObject(runtime).asFunction(runtime),
                                                    arguments[1].asObject(runtime).asFunction(runtime));
    promise->_resolveReject = resolveReject;
    fn(runtime, promise);
    return Value::undefined();
  });

  auto jsPromise =
    runtime.global().getPropertyAsFunction(runtime, "Promise").callAsConstructor(runtime, promiseCallbackFn);
  // We are setting the host object to the promise to retain it and pass back a weak reference to the native promise.
  // This ensures that the jsi runtime fully owns the lifecycle and will destroy it when required.
  // A strong reference to ResolveReject must only be retrieved on the JS thread. Otherwise crashes may occur,
  // especially during hot reloads.
  jsPromise.asObject(runtime).setProperty(runtime, "__wgpuNative",
                                          Object::createFromHostObject(runtime, resolveReject));
  resolveReject->_promise = std::make_shared<Object>(jsPromise.asObject(runtime));
  return jsPromise;
}

void Promise::resolve(std::function<Value(Runtime &)> &&fn) {
  finalize([fn = std::move(fn)](auto &runtime, auto &resolve, auto &reject) { resolve.call(runtime, fn(runtime)); });
}

void Promise::reject(std::function<Value(Runtime &)> &&fn) {
  finalize([fn = std::move(fn)](auto &runtime, auto &resolve, auto &reject) { reject.call(runtime, fn(runtime)); });
}

void Promise::finalize(std::function<void(Runtime &runtime, Function &resolve, Function &reject)> &&callback) {
  _jsiInstance->jsThread->run(
    [&runtime = _jsiInstance->runtime, rr = this->_resolveReject, callback = std::move(callback)] {
    auto resolveReject = rr.lock();
    if (resolveReject != nullptr) {
      callback(runtime, resolveReject->_resolve, resolveReject->_reject);
      // Important to delete ResolveReject here:
      // - Allows garbage collector to delete the promise
      // - Prevents accidental additional resolve/rejects
      resolveReject->_promise->setProperty(runtime, "__wgpuNative", Value::undefined());
    }
  });
}

void *Promise::toCData() { return toCDataWithExtras(nullptr); }

void Promise::fromCData(void *data, CDataCallback &&fn) {
  auto cData = (WGPUPromise_CData<void *> *)data;
  fn(cData->promise);
  delete cData;
}
