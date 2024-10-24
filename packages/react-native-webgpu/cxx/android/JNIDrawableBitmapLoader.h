#pragma once
#include <jni.h>

#include <memory>
#include <string>
namespace wgpu {

class JNIDrawableBitmapLoaderMethodIDs {
 public:
  explicit JNIDrawableBitmapLoaderMethodIDs(JNIEnv *env) {
    auto bitmapLoaderClass = env->FindClass("com/webgpu/DrawableBitmapLoader");
    if (bitmapLoaderClass == nullptr) {
      throw std::runtime_error("Could not find java class DrawableBitmapLoader");
    }
    fetchBitmapId = env->GetMethodID(bitmapLoaderClass, "fetchBitmap", "()V");
    getSharedMemoryId = env->GetMethodID(bitmapLoaderClass, "getSharedMemory", "()Landroid/os/SharedMemory;");
    getWidthId = env->GetMethodID(bitmapLoaderClass, "getWidth", "()I");
    getHeightId = env->GetMethodID(bitmapLoaderClass, "getHeight", "()I");
    getErrorMessage = env->GetMethodID(bitmapLoaderClass, "getErrorMessage", "()Ljava/lang/String;");
  }

  jmethodID getErrorMessage;
  jmethodID fetchBitmapId;
  jmethodID getSharedMemoryId;
  jmethodID getWidthId;
  jmethodID getHeightId;
};

class JNIDrawableBitmapLoader {
 public:
  explicit JNIDrawableBitmapLoader(std::shared_ptr<JNIDrawableBitmapLoaderMethodIDs> methodIDs, jobject instance)
    : _methodIDs(std::move(methodIDs)), _this(instance) {}

  void fetchBitmap(JNIEnv *env) { env->CallVoidMethod(_this, _methodIDs->fetchBitmapId); }

  jstring getErrorMessage(JNIEnv *env) { return (jstring)env->CallObjectMethod(_this, _methodIDs->getErrorMessage); }

  jobject getSharedMemory(JNIEnv *env) { return env->CallObjectMethod(_this, _methodIDs->getSharedMemoryId); }

  jint getHeight(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getHeightId); }

  jint getWidth(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getWidthId); }

 private:
  std::shared_ptr<JNIDrawableBitmapLoaderMethodIDs> _methodIDs;
  jobject _this;
};

}  // namespace wgpu
