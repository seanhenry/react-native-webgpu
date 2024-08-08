#pragma once

#include <jni.h>

#include <memory>
#include <string>
#include <utility>

#include "JNIBlobBitmapLoader.h"
#include "JSIHTTPBitmapLoader.h"

namespace wgpu {

class JNIImageLoaderFactoryMethodIDs {
   public:
    explicit JNIImageLoaderFactoryMethodIDs(JNIEnv *env) {
        auto factoryClass =
            (jclass)env->NewGlobalRef(env->FindClass("com/webgpu/BitmapLoaderFactory"));
        if (factoryClass == nullptr) {
            throw std::runtime_error("Could not find java class BitmapLoaderFactory");
        }
        makeHttpBitmapLoaderId =
            env->GetMethodID(factoryClass, "makeHttpBitmapLoader",
                             "(Ljava/lang/String;)Lcom/webgpu/HTTPBitmapLoader;");
        makeBlobBitmapLoaderId = env->GetMethodID(factoryClass, "makeBlobBitmapLoader",
                                                  "()Lcom/webgpu/BlobBitmapLoader;");
    }

    jmethodID makeHttpBitmapLoaderId;
    jmethodID makeBlobBitmapLoaderId;
};

class JNIImageLoaderFactory {
   public:
    JNIImageLoaderFactory(std::shared_ptr<JNIImageLoaderFactoryMethodIDs> factoryMethodIDs,
                          std::shared_ptr<JNIHTTPBitmapLoaderMethodIDs> httpMethodIDs,
                          std::shared_ptr<JNIBlobBitmapLoaderMethodIDs> blobMethodIDs,
                          jobject instance)
        : _factoryMethodIDs(std::move(factoryMethodIDs)),
          _httpMethodIDs(std::move(httpMethodIDs)),
          _blobMethodIDs(std::move(blobMethodIDs)),
          _instance(instance) {}

    std::shared_ptr<JNIHTTPBitmapLoader> makeHttpBitmapLoader(JNIEnv *env, std::string uri) {
        auto uriString = env->NewStringUTF(uri.data());
        auto instance =
            env->CallObjectMethod(_instance, _factoryMethodIDs->makeHttpBitmapLoaderId, uriString);
        return std::make_shared<JNIHTTPBitmapLoader>(_httpMethodIDs, instance);
    }

    std::shared_ptr<JNIBlobBitmapLoader> makeBlobBitmapLoader(JNIEnv *env) {
        auto instance = env->CallObjectMethod(_instance, _factoryMethodIDs->makeBlobBitmapLoaderId);
        return std::make_shared<JNIBlobBitmapLoader>(_blobMethodIDs, instance);
    }

   private:
    std::shared_ptr<JNIImageLoaderFactoryMethodIDs> _factoryMethodIDs;
    std::shared_ptr<JNIHTTPBitmapLoaderMethodIDs> _httpMethodIDs;
    std::shared_ptr<JNIBlobBitmapLoaderMethodIDs> _blobMethodIDs;
    jobject _instance;
};

}  // namespace wgpu
