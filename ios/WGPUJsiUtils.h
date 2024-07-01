#include <jsi/jsi.h>
#include "AutoReleasePool.h"

#define WGPU_FUNC_FROM_HOST_FUNC(__name, __argCount, ...) Function::createFromHostFunction(runtime, PropNameID::forAscii(runtime, #__name), __argCount, __VA_ARGS__(Runtime &runtime, const Value &thisValue, const Value *arguments, size_t count) -> Value

// Optional jsi getters
#define WGPU_BOOL_OPT(__obj, __propName, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getProperty(runtime, #__propName).asBool() : __default
#define WGPU_UTF8_OPT(__obj, __propName, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getProperty(runtime, #__propName).asString(runtime).utf8(runtime) : __default
#define WGPU_NUMBER_OPT(__obj, __propName, __type, __default) __obj.hasProperty(runtime, #__propName) ? (__type)__obj.getProperty(runtime, #__propName).asNumber() : __default
#define WGPU_HOST_OBJ_VALUE_OPT(__obj, __propName, __type, __default) __obj.hasProperty(runtime, #__propName) ? __obj.getPropertyAsObject(runtime, #__propName).asHostObject<__type>(runtime)->_value : __default

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

class Promise {
public:
    Promise(Runtime &runtime): runtime(runtime) {}
    Runtime &runtime;
    std::unique_ptr<Function> resolve;
    std::unique_ptr<Function> reject;
    void * userData;
};

template<typename Callback>
Value makePromise(Runtime &runtime, Callback cb) {
    auto promiseConstructor = WGPU_FUNC_FROM_HOST_FUNC(promiseConstructor, 2, [cb]) {
        auto promise = new Promise(runtime);
        promise->resolve = std::make_unique<Function>(arguments[0].asObject(runtime).asFunction(runtime));
        promise->reject = std::make_unique<Function>(arguments[1].asObject(runtime).asFunction(runtime));
        cb(promise);
        return Value::undefined();
    });

    return runtime.global()
        .getProperty(runtime, "Promise")
        .asObject(runtime)
        .asFunction(runtime)
        .callAsConstructor(runtime, promiseConstructor);
}

std::shared_ptr<std::string> getUTF8(Runtime &runtime, AutoReleasePool *pool, Value value) {
    auto str = std::make_shared<std::string>(value.asString(runtime).utf8(runtime));
    pool->add(str);
    return str;
}

bool isArray(Runtime &runtime, Value *value) {
    return value->isObject() && value->asObject(runtime).isArray(runtime);
}

class SharedMutableBuffer: public MutableBuffer {
public:
    SharedMutableBuffer(void *data, size_t size): _data(data), _size(size) {}
    ~SharedMutableBuffer() {}
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

ArrayBuffer createSharedArrayBuffer(Runtime &runtime, void *bytes, size_t size) {
    auto buffer = std::make_shared<SharedMutableBuffer>(bytes, size);
    return ArrayBuffer(runtime, buffer);
}

}
