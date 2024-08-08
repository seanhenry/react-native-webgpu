#include "WGPUAndroidInstance.h"

#include "JNIBitmapLoaderFactory.h"
#include "WGPULog.h"

using namespace wgpu;

std::unique_ptr<WGPUAndroidInstance> WGPUAndroidInstance::instance = nullptr;

void WGPUAndroidInstance::setBitmapLoaderFactory(JNIEnv *env, jobject factory) {
  auto httpMethodIDs = std::make_shared<JNIHTTPBitmapLoaderMethodIDs>(env);
  auto blobMethodIDs = std::make_shared<JNIBlobBitmapLoaderMethodIDs>(env);
  auto factoryMethodIDs = std::make_shared<JNIImageLoaderFactoryMethodIDs>(env);
  _imageLoaderFactory =
    std::make_shared<JNIImageLoaderFactory>(factoryMethodIDs, httpMethodIDs, blobMethodIDs, env->NewGlobalRef(factory));
}
