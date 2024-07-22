#pragma once

#include <jni.h>
#include <memory>
#include <string>
#include <utility>

// Looking up method ids so they can be used from any thread later
class JNIMethodIDs {
public:
    explicit JNIMethodIDs(JNIEnv *env) {
        // Note, not deleting this global ref as it has the same lifetime as the app
        bitmapLoaderFactoryClass = (jclass)env->NewGlobalRef(env->FindClass("com/webgpu/BitmapLoaderFactory"));
        if (bitmapLoaderFactoryClass == nullptr) {
            throw std::runtime_error("Could not find java class BitmapLoaderFactory");
        }
        makeHttpBitmapLoaderId = env->GetStaticMethodID(bitmapLoaderFactoryClass,
                                                        "makeHttpBitmapLoader",
                                                        "(Ljava/lang/String;)Lcom/webgpu/BitmapLoader;");

        auto bitmapLoaderClass = env->FindClass("com/webgpu/BitmapLoader");
        if (bitmapLoaderClass == nullptr) {
            throw std::runtime_error("Could not find java class BitmapLoader");
        }
        fetchBitmapId = env->GetMethodID(bitmapLoaderClass, "fetchBitmap", "()V");
        getSharedMemoryId = env->GetMethodID(bitmapLoaderClass, "getSharedMemory",
                                             "()Landroid/os/SharedMemory;");
        getWidthId = env->GetMethodID(bitmapLoaderClass, "getWidth", "()I");
        getHeightId = env->GetMethodID(bitmapLoaderClass, "getHeight", "()I");
        getErrorMessage = env->GetMethodID(bitmapLoaderClass, "getErrorMessage",
                                           "()Ljava/lang/String;");
    }

    jclass bitmapLoaderFactoryClass;
    jmethodID makeHttpBitmapLoaderId;

    jmethodID getErrorMessage;
    jmethodID fetchBitmapId;
    jmethodID getSharedMemoryId;
    jmethodID getWidthId;
    jmethodID getHeightId;
};

class JNIImageLoader {

public:
    explicit JNIImageLoader(std::shared_ptr<JNIMethodIDs> methodIDs, jobject instance): _methodIDs(std::move(methodIDs)), _this(instance) {
    }

    static std::shared_ptr<JNIImageLoader> fromUri(JNIEnv *env, std::shared_ptr<JNIMethodIDs> methodIDs, std::string uri) {
        auto uriString = env->NewStringUTF(uri.data());
        auto instance = env->CallStaticObjectMethod(methodIDs->bitmapLoaderFactoryClass, methodIDs->makeHttpBitmapLoaderId, uriString);
        return std::make_shared<JNIImageLoader>(methodIDs, instance);
    }

    void fetchBitmap(JNIEnv *env) {
        env->CallVoidMethod(_this, _methodIDs->fetchBitmapId);
    }

    jstring getErrorMessage(JNIEnv *env) {
        return (jstring) env->CallObjectMethod(_this, _methodIDs->getErrorMessage);
    }

    jobject getSharedMemory(JNIEnv *env) {
        return env->CallObjectMethod(_this, _methodIDs->getSharedMemoryId);
    }

    jint getHeight(JNIEnv *env) {
        return env->CallIntMethod(_this, _methodIDs->getHeightId);
    }

    jint getWidth(JNIEnv *env) {
        return env->CallIntMethod(_this, _methodIDs->getWidthId);
    }

private:
    std::shared_ptr<JNIMethodIDs> _methodIDs;
    jobject _this;
};
