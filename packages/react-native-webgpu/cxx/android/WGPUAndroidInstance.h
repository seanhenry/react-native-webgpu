#pragma once

#include <jni.h>

#include <memory>

#include "JNIBitmapLoaderFactory.h"
#include "JNIBlobBitmapLoader.h"
#include "JNIExceptionHandler.h"

namespace wgpu {

class WGPUAndroidInstance {
 public:
  static std::unique_ptr<WGPUAndroidInstance> instance;
  explicit WGPUAndroidInstance(JavaVM *jvm, JNIEnv *env) : _jvm(jvm), _env(env) {};
  void setBitmapLoaderFactory(JNIEnv *env, jobject factory);
  void setExceptionHandler(JNIEnv *env, jobject handler);
  JavaVM *getJVM() { return _jvm; }
  JNIEnv *getMainThreadEnv() { return _env; }
  std::shared_ptr<JNIImageLoaderFactory> getImageLoaderFactory() { return _imageLoaderFactory; }
  std::shared_ptr<JNIExceptionHandler> getExceptionHandler() { return _exceptionHandler; }

 private:
  JavaVM *_jvm = nullptr;
  JNIEnv *_env = nullptr;
  std::shared_ptr<JNIImageLoaderFactory> _imageLoaderFactory;
  std::shared_ptr<JNIExceptionHandler> _exceptionHandler;
};

}  // namespace wgpu
