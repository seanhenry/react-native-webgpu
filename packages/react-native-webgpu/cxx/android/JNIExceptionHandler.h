#pragma once

#include <jni.h>

#include <string>

namespace wgpu {
class JNIExceptionHandler {
 public:
  explicit JNIExceptionHandler(JNIEnv *env, jobject handler) {
    auto exceptionHandlerClass = env->FindClass("com/webgpu/ExceptionHandler");
    if (exceptionHandlerClass == nullptr) {
      throw std::runtime_error("Could not find java class ExceptionHandler");
    }
    _exceptionHandler = env->NewWeakGlobalRef(handler);
    _handleExceptionId = env->GetMethodID(exceptionHandlerClass, "handleException", "(Ljava/lang/String;)V");
  }

  void handleException(JNIEnv *env, std::string &message) {
    jstring jMessage = env->NewStringUTF(message.data());
    env->CallVoidMethod(_exceptionHandler, _handleExceptionId, jMessage);
    env->DeleteLocalRef(jMessage);
  }

 private:
  jobject _exceptionHandler = nullptr;
  jmethodID _handleExceptionId = nullptr;
};
}  // namespace wgpu
