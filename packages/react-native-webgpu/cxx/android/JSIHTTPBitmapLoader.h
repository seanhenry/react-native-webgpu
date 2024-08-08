#pragma once

#include <jni.h>

#include <memory>
#include <string>

namespace wgpu {

class JNIHTTPBitmapLoaderMethodIDs {
   public:
    explicit JNIHTTPBitmapLoaderMethodIDs(JNIEnv *env) {
        auto bitmapLoaderClass = env->FindClass("com/webgpu/HTTPBitmapLoader");
        if (bitmapLoaderClass == nullptr) {
            throw std::runtime_error("Could not find java class HTTPBitmapLoader");
        }
        fetchBitmapId = env->GetMethodID(bitmapLoaderClass, "fetchBitmap", "()V");
        getSharedMemoryId =
            env->GetMethodID(bitmapLoaderClass, "getSharedMemory", "()Landroid/os/SharedMemory;");
        getWidthId = env->GetMethodID(bitmapLoaderClass, "getWidth", "()I");
        getHeightId = env->GetMethodID(bitmapLoaderClass, "getHeight", "()I");
        getErrorMessage =
            env->GetMethodID(bitmapLoaderClass, "getErrorMessage", "()Ljava/lang/String;");
    }

    jmethodID getErrorMessage;
    jmethodID fetchBitmapId;
    jmethodID getSharedMemoryId;
    jmethodID getWidthId;
    jmethodID getHeightId;
};

class JNIHTTPBitmapLoader {
   public:
    explicit JNIHTTPBitmapLoader(std::shared_ptr<JNIHTTPBitmapLoaderMethodIDs> methodIDs,
                                 jobject instance)
        : _methodIDs(std::move(methodIDs)), _this(instance) {}

    void fetchBitmap(JNIEnv *env) { env->CallVoidMethod(_this, _methodIDs->fetchBitmapId); }

    jstring getErrorMessage(JNIEnv *env) {
        return (jstring)env->CallObjectMethod(_this, _methodIDs->getErrorMessage);
    }

    jobject getSharedMemory(JNIEnv *env) {
        return env->CallObjectMethod(_this, _methodIDs->getSharedMemoryId);
    }

    jint getHeight(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getHeightId); }

    jint getWidth(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getWidthId); }

   private:
    std::shared_ptr<JNIHTTPBitmapLoaderMethodIDs> _methodIDs;
    jobject _this;
};

}  // namespace wgpu
