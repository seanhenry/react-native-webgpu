#pragma once

#include <jsi/jsi.h>

#include <iostream>
#include <sstream>

#include "WGPULog.h"
#include "webgpu.h"

#define WGPU_FUNC_FROM_HOST_FUNC(__name, __argCount, ...) Function::createFromHostFunction(runtime, PropNameID::forAscii(runtime, #__name), __argCount, __VA_ARGS__([[maybe_unused]] Runtime &runtime, [[maybe_unused]] const Value &thisValue, [[maybe_unused]] const Value *arguments, [[maybe_unused]] size_t count) -> Value

// Optional jsi getters
#define WGPU_HAS_PROP(__obj, __propName)                                                           \
  (__obj.hasProperty(runtime, #__propName) && !__obj.getProperty(runtime, #__propName).isNull() && \
   !__obj.getProperty(runtime, #__propName).isUndefined())
#define WGPU_BOOL_OPT(__obj, __propName, __default) \
  WGPU_HAS_PROP(__obj, __propName) ? __obj.getProperty(runtime, #__propName).asBool() : __default
#define WGPU_UTF8_OPT(__obj, __propName, __default) \
  WGPU_HAS_PROP(__obj, __propName)                  \
  ? __obj.getProperty(runtime, #__propName).asString(runtime).utf8(runtime) : __default
#define WGPU_NUMBER_OPT(__obj, __propName, __type, __default) \
  WGPU_HAS_PROP(__obj, __propName) ? (__type)__obj.getProperty(runtime, #__propName).asNumber() : __default
#define WGPU_HOST_OBJ_VALUE_OPT(__obj, __propName, __type, __default) \
  WGPU_HAS_PROP(__obj, __propName)                                    \
  ? __obj.getPropertyAsObject(runtime, #__propName).asHostObject<__type>(runtime)->getValue() : __default

// Required jsi getters
#define WGPU_UTF8(__obj, __propName) __obj.getProperty(runtime, #__propName).asString(runtime).utf8(runtime)
#define WGPU_NUMBER(__obj, __propName, __type) (__type) __obj.getProperty(runtime, #__propName).asNumber()
#define WGPU_OBJ(__obj, __propName) __obj.getPropertyAsObject(runtime, #__propName)
#define WGPU_HOST_OBJ(__obj, __propName, __type) \
  __obj.getPropertyAsObject(runtime, #__propName).asHostObject<__type>(runtime)
#define WGPU_ARRAY(__obj, __propName) __obj.getPropertyAsObject(runtime, #__propName).asArray(runtime)

// Jsi setters
#define WGPU_SET_UTF8(__obj, __propName, __str) \
  __obj.setProperty(runtime, #__propName, String::createFromUtf8(runtime, __str))

// Uncomment to display JSI logs
// #define WGPU_DEBUG_LOG_PROPS
// #define WGPU_DEBUG_LOG_UNIMPLEMENTED_PROPS
// #define WGPU_DEBUG_LOG_FUNC_ARGS

#ifdef WGPU_DEBUG_LOG_PROPS
#define WGPU_LOG_GET_PROP WGPU_LOG_INFO("%s.%s", __FILE__, name.data())
#else
#define WGPU_LOG_GET_PROP
#endif

#ifdef WGPU_DEBUG_LOG_UNIMPLEMENTED_PROPS
#define WGPU_LOG_UNIMPLEMENTED_GET_PROP WGPU_LOG_ERROR("%s.%s is unimplemented", __FILE__, name.data())
#else
#define WGPU_LOG_UNIMPLEMENTED_GET_PROP
#endif

#ifdef WGPU_DEBUG_LOG_FUNC_ARGS
#define WGPU_LOG_FUNC_ARGS(__name) jsLogFuncArgs(runtime, "log", #__name, arguments, count, __FILE_NAME__, __LINE__)
#else
#define WGPU_LOG_FUNC_ARGS(__name)
#endif

using namespace facebook::jsi;

namespace wgpu {

template <typename T, typename Transform>
std::vector<T> jsiArrayToVector(Runtime &runtime, Array jsiArray, Transform transform) {
  std::vector<T> vec;
  size_t arraySize = jsiArray.size(runtime);
  vec.reserve(arraySize);

  for (size_t i = 0; i < arraySize; ++i) {
    vec.emplace_back(transform(runtime, jsiArray.getValueAtIndex(runtime, i)));
  }

  return vec;
}

template <typename T, typename Transform>
Array cArrayToJsi(Runtime &runtime, T *array, size_t size, Transform transform) {
  auto result = Array(runtime, size);

  for (size_t i = 0; i < size; ++i) {
    result.setValueAtIndex(runtime, i, transform(runtime, *array));
    array++;
  }

  return result;
}

inline bool isJSIArray(Runtime &runtime, const Value &value) {
  return value.isObject() && value.asObject(runtime).isArray(runtime);
}

inline Value makeJSError(Runtime &runtime, std::string message) {
  auto errorConstructor = runtime.global().getPropertyAsFunction(runtime, "Error");
  return errorConstructor.callAsConstructor(runtime, message);
}

inline Value makeJSSet(Runtime &runtime, Value *items, size_t size) {
  auto setConstructor = runtime.global().getPropertyAsFunction(runtime, "Set");
  auto set = setConstructor.callAsConstructor(runtime).asObject(runtime);
  auto add = set.getPropertyAsFunction(runtime, "add");
  for (size_t i = 0; i < size; i++) {
    add.callWithThis(runtime, set, items[i]);
  }
  return std::move(set);
}

template <typename T, typename U>
bool cArrayContains(T *data, size_t size, U searchItem) {
  for (size_t i = 0; i < size; i++) {
    if (data[0] == searchItem) {
      return true;
    }
  }
  return false;
}

inline void jsLogValues([[maybe_unused]] Runtime &runtime, [[maybe_unused]] const char *functionName,
                        [[maybe_unused]] const Value *arguments, [[maybe_unused]] size_t size) {
#ifdef DEBUG
  runtime.global()
    .getPropertyAsObject(runtime, "console")
    .getPropertyAsFunction(runtime, functionName)
    .call(runtime, arguments, size);
#endif
}

inline void jsLog([[maybe_unused]] Runtime &runtime, [[maybe_unused]] const char *functionName,
                  [[maybe_unused]] std::initializer_list<std::string> message) {
#ifdef DEBUG
  std::vector<Value> args;
  args.reserve(args.size());
  for (auto str : message) {
    args.emplace_back(String::createFromUtf8(runtime, str));
  }

  wgpu::jsLogValues(runtime, functionName, (const Value *)args.data(), args.size());
#endif
}

inline void jsLogFuncArgs([[maybe_unused]] Runtime &runtime, [[maybe_unused]] const char *logFunctionName,
                          [[maybe_unused]] const char *functionName, [[maybe_unused]] const Value *arguments,
                          [[maybe_unused]] size_t size, [[maybe_unused]] const char *file,
                          [[maybe_unused]] size_t line) {
#ifdef DEBUG
  std::vector<Value> args;
  std::ostringstream ss;
  ss << file << ":" << line << " " << functionName;
  args.emplace_back(String::createFromUtf8(runtime, ss.str()));

  wgpu::jsLogValues(runtime, logFunctionName, args.data(), args.size());
  wgpu::jsLogValues(runtime, logFunctionName, arguments, size);
#endif
}

}  // namespace wgpu
