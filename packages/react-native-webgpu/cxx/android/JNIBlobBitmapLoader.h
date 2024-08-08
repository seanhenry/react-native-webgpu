#pragma once

#include <jni.h>

#include <memory>

namespace wgpu {

class JNIBlobBitmapLoaderMethodIDs {
   public:
    explicit JNIBlobBitmapLoaderMethodIDs(JNIEnv *env) {
        auto loaderClass = env->FindClass("com/webgpu/BlobBitmapLoader");
        if (loaderClass == nullptr) {
            throw std::runtime_error("Could not find java class BlobBitmapLoader");
        }
        loadBitmapId = env->GetMethodID(loaderClass, "loadBitmap", "(Ljava/lang/String;II)I");
        getSharedMemoryId =
            env->GetMethodID(loaderClass, "getSharedMemory", "()Landroid/os/SharedMemory;");
        getWidthId = env->GetMethodID(loaderClass, "getWidth", "()I");
        getHeightId = env->GetMethodID(loaderClass, "getHeight", "()I");
    }

    jmethodID loadBitmapId;
    jmethodID getSharedMemoryId;
    jmethodID getWidthId;
    jmethodID getHeightId;
};

class JNIBlobBitmapLoader {
   public:
    explicit JNIBlobBitmapLoader(std::shared_ptr<JNIBlobBitmapLoaderMethodIDs> methodIDs,
                                 jobject instance)
        : _methodIDs(std::move(methodIDs)), _this(instance) {}

    int loadBitmap(JNIEnv *env, const char *blobID, int offset, int size) {
        auto id = env->NewStringUTF(blobID);
        return env->CallIntMethod(_this, _methodIDs->loadBitmapId, id, offset, size);
    }

    jobject getSharedMemory(JNIEnv *env) {
        return env->CallObjectMethod(_this, _methodIDs->getSharedMemoryId);
    }

    jint getHeight(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getHeightId); }

    jint getWidth(JNIEnv *env) { return env->CallIntMethod(_this, _methodIDs->getWidthId); }

   private:
    std::shared_ptr<JNIBlobBitmapLoaderMethodIDs> _methodIDs;
    jobject _this;
};

}  // namespace wgpu