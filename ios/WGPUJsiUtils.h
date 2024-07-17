#pragma once

#include <jsi/jsi.h>
#include "AutoReleasePool.h"
#include "webgpu.h"
#include <iostream>

#define WGPU_FUNC_FROM_HOST_FUNC(__name, __argCount, ...) Function::createFromHostFunction(runtime, PropNameID::forAscii(runtime, #__name), __argCount, __VA_ARGS__(Runtime &runtime, const Value &thisValue, const Value *arguments, size_t count) -> Value

// Optional jsi getters
#define WGPU_BOOL_OPT(__obj, __propName, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getProperty(runtime, #__propName).asBool() : __default
#define WGPU_UTF8_OPT(__obj, __propName, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getProperty(runtime, #__propName).asString(runtime).utf8(runtime) : __default
#define WGPU_NUMBER_OPT(__obj, __propName, __type, __default) __obj.hasProperty(runtime, #__propName) ? (__type)__obj.getProperty(runtime, #__propName).asNumber() : __default
#define WGPU_HOST_OBJ_VALUE_OPT(__obj, __propName, __type, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getPropertyAsObject(runtime, #__propName).asHostObject<__type>(runtime)->getValue() : __default

// Required jsi getters
#define WGPU_UTF8(__obj, __propName) __obj.getProperty(runtime, #__propName).asString(runtime).utf8(runtime)
#define WGPU_NUMBER(__obj, __propName, __type) (__type)__obj.getProperty(runtime, #__propName).asNumber()
#define WGPU_OBJ(__obj, __propName) __obj.getPropertyAsObject(runtime, #__propName)
#define WGPU_HOST_OBJ(__obj, __propName, __type) __obj.getPropertyAsObject(runtime, #__propName).asHostObject<__type>(runtime)
#define WGPU_ARRAY(__obj, __propName) __obj.getPropertyAsObject(runtime, #__propName).asArray(runtime)

using namespace facebook::jsi;

namespace wgpu {

template<typename T, typename Transform>
std::vector<T> jsiArrayToVector(Runtime &runtime, Array jsiArray, Transform transform) {
    std::vector<T> vec;
    size_t arraySize = jsiArray.size(runtime);
    vec.reserve(arraySize);

    for (size_t i = 0; i < arraySize; ++i) {
        vec.emplace_back(transform(runtime, jsiArray.getValueAtIndex(runtime, i)));
    }

    return vec;
}

template<typename T, typename Transform>
Array cArrayToJsi(Runtime &runtime, T *array, size_t size, Transform transform) {
    auto result = Array(runtime, size);

    for (size_t i = 0; i < size; ++i) {
        result.setValueAtIndex(runtime, i, transform(runtime, *array));
        array++;
    }

    return result;
}

template<typename T>
class Promise {
public:
    explicit Promise(Runtime &runtime): runtime(runtime) {}
    Value jsPromise(std::function<void()> &&fn);
    void resolve(Value value) {
        _resolve->call(runtime, std::move(value));
    }
    void reject(Value value) {
        _reject->call(runtime, std::move(value));
    }
    Runtime &runtime;
    T data;
private:
    std::shared_ptr<Function> _resolve;
    std::shared_ptr<Function> _reject;
};

template <typename T>
Value Promise<T>::jsPromise(std::function<void()> &&fn) {
    auto promiseCallbackFn = WGPU_FUNC_FROM_HOST_FUNC(promiseCallbackFn, 2, [this, fn = std::move(fn)]) {
        _resolve = std::make_shared<Function>(arguments[0].asObject(runtime).asFunction(runtime));
        _reject = std::make_shared<Function>(arguments[1].asObject(runtime).asFunction(runtime));
        fn();
        return Value::undefined();
    });

    return runtime.global()
        .getProperty(runtime, "Promise")
        .asObject(runtime)
        .asFunction(runtime)
        .callAsConstructor(runtime, promiseCallbackFn);
}

// TODO: remove
inline std::shared_ptr<std::string> getUTF8(Runtime &runtime, AutoReleasePool *pool, Value value) {
    auto str = std::make_shared<std::string>(value.asString(runtime).utf8(runtime));
    pool->add(str);
    return str;
}

inline bool isArray(Runtime &runtime, Value *value) {
    return value->isObject() && value->asObject(runtime).isArray(runtime);
}

class UnownedMutableBuffer: public MutableBuffer {
public:
    UnownedMutableBuffer(void *data, size_t size): _data(data), _size(size) {}
    ~UnownedMutableBuffer() {}
    size_t size() const override {
        return _size;
    }
    uint8_t* data() override {
        return (uint8_t *)_data;
    }
private:
    void *_data;
    size_t _size;
};

class OwnedMutableBuffer: public MutableBuffer {
public:
    OwnedMutableBuffer(void *data, size_t size): _data(data), _size(size) {}
    ~OwnedMutableBuffer() { free(_data); }
    size_t size() const override {
        return _size;
    }
    uint8_t* data() override {
        return (uint8_t *)_data;
    }
private:
    void *_data;
    size_t _size;
};

inline ArrayBuffer createUnownedArrayBuffer(Runtime &runtime, void *bytes, size_t size) {
    auto buffer = std::make_shared<UnownedMutableBuffer>(bytes, size);
    return ArrayBuffer(runtime, buffer);
}

inline ArrayBuffer createOwnedArrayBuffer(Runtime &runtime, void *bytes, size_t size) {
    auto buffer = std::make_shared<OwnedMutableBuffer>(bytes, size);
    return ArrayBuffer(runtime, buffer);
}

inline Value makeJSError(Runtime &runtime, std::string message) {
    auto errorConstructor = runtime.global().getPropertyAsFunction(runtime, "Error");
    return errorConstructor.callAsConstructor(runtime, message);
}

inline Value makeJSSet(Runtime &runtime, Value *items, size_t size) {
    auto setConstructor = runtime.global().getPropertyAsFunction(runtime, "Set");
    auto set = setConstructor.callAsConstructor(runtime).asObject(runtime);
    auto add = set.getPropertyAsFunction(runtime, "add");
    for (int i = 0; i < size; i++) {
        add.callWithThis(runtime, set, items[i]);
    }
    return std::move(set);
}

inline ArrayBuffer getArrayBufferFromArrayBufferLike(Runtime &runtime, Object arrayBufferLike) {
    if (arrayBufferLike.isArrayBuffer(runtime)) {
        return arrayBufferLike.getArrayBuffer(runtime);
    } else if (arrayBufferLike.hasProperty(runtime, "buffer")) {
        return arrayBufferLike.getPropertyAsObject(runtime, "buffer").getArrayBuffer(runtime);
    }
    throw new JSError(runtime, "Unsupported ArrayBufferLike object");
}

// Wrapper around c pointer to manage lifetime
class DeviceWrapper {
public:
    explicit DeviceWrapper(WGPUDevice device): _device(device) {}
    ~DeviceWrapper() { wgpuDeviceRelease(_device); }
    WGPUDevice _device;
};

class AdapterWrapper {
public:
    explicit AdapterWrapper(WGPUAdapter adapter): _adapter(adapter) {}
    ~AdapterWrapper() { wgpuAdapterRelease(_adapter); }
    WGPUAdapter _adapter;
};

template<typename T, typename U>
bool cArrayContains(T *data, size_t size, U searchItem) {
    for (auto i = 0; i < size; i++) {
        if (data[0] == searchItem) {
            return true;
        }
    }
    return false;
}

inline void jsLog(Runtime &runtime, const char *functionName, std::initializer_list<std::string> message) {
#ifdef DEBUG
    std::vector<Value> args;
    args.reserve(args.size());
    for (auto str : message) {
        args.emplace_back(String::createFromUtf8(runtime, str));
    }

    runtime.global()
        .getPropertyAsObject(runtime, "console")
        .getPropertyAsFunction(runtime, functionName)
        .call(runtime, (const Value*)args.data(), args.size());
#endif
}

} // namespace
