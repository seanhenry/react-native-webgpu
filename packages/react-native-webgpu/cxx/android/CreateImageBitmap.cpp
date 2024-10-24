#include "CreateImageBitmap.h"

#include <android/sharedmem.h>
#include <android/sharedmem_jni.h>
#include <jni.h>
#include <unistd.h>

#include <sstream>
#include <thread>

#include "ImageBitmapHostObject.h"
#include "JSIInstance.h"
#include "WGPUAndroidInstance.h"
#include "WGPUJsiUtils.h"
#include "WGPULog.h"
#include "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

Value fetchImageBitmap(std::string uri, std::shared_ptr<JSIInstance> jsiInstance);

Value getImageBitmapFromBlob(Runtime &runtime, Object obj, std::shared_ptr<JSIInstance> jsiInstance);

size_t getBufferFromSharedMemory(JNIEnv *env, jobject sharedMemory, jint width, jint height,
                                 const std::shared_ptr<std::string> &errorMessage, std::shared_ptr<Promise> promise,
                                 uint8_t **dataOut);

}  // namespace wgpu

Function wgpu::createImageBitmap(Runtime &runtime, std::shared_ptr<JSIInstance> jsiInstance) {
  return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, [jsiInstance]) {
    auto obj = arguments[0].asObject(runtime);
    if (obj.hasProperty(runtime, "uri")) {
      auto uri = WGPU_UTF8(obj, uri);
      return fetchImageBitmap(std::move(uri), jsiInstance);
    } else if (obj.hasProperty(runtime, "_data")) {
      return getImageBitmapFromBlob(runtime, std::move(obj), jsiInstance);
    }
    return makeRejectedJSPromise(runtime,
                                 makeJSError(runtime, "[WebGPU] Unsupported type passed to createImageBitmap"));
  });
}

Value wgpu::fetchImageBitmap(std::string uri, std::shared_ptr<JSIInstance> jsiInstance) {
  return Promise::makeJSPromise(jsiInstance, [uri = std::move(uri)](auto &runtime, auto &promise) {
    auto thread = std::thread([promise, uri]() {
      auto jvm = WGPUAndroidInstance::instance->getJVM();
      if (jvm == nullptr) {
        promise->reject([](auto &runtime) { return makeJSError(runtime, "The JVM is not available"); });
        return;
      }
      JNIEnv *env = nullptr;
      auto getEngStatus = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
      if (getEngStatus == JNI_EDETACHED) {
        jvm->AttachCurrentThread(&env, nullptr);
      } else if (getEngStatus != JNI_OK) {
        WGPU_LOG_ERROR("%s:%i jvm->GetEnv() returned status %i", __FILE__, __LINE__, getEngStatus);
        promise->reject([](auto &runtime) { return makeJSError(runtime, "Failed to get JNIEnv"); });
        return;
      }

      jobject sharedMemory;
      jint width;
      jint height;
      jstring errorMessageIn;

      if (uri.find("https://") == 0 || uri.find("http://") == 0) {
        auto bitmapLoader = WGPUAndroidInstance::instance->getImageLoaderFactory()->makeHttpBitmapLoader(env, uri);
        bitmapLoader->fetchBitmap(env);

        sharedMemory = bitmapLoader->getSharedMemory(env);
        width = bitmapLoader->getWidth(env);
        height = bitmapLoader->getHeight(env);

        errorMessageIn = bitmapLoader->getErrorMessage(env);
      } else {
        auto bitmapLoader = WGPUAndroidInstance::instance->getImageLoaderFactory()->makeDrawableBitmapLoader(env, uri);
        bitmapLoader->fetchBitmap(env);

        sharedMemory = bitmapLoader->getSharedMemory(env);
        width = bitmapLoader->getWidth(env);
        height = bitmapLoader->getHeight(env);

        errorMessageIn = bitmapLoader->getErrorMessage(env);
      }

      std::shared_ptr<std::string> errorMessage;
      if (errorMessageIn != nullptr) {
        const char *uuidChars = env->GetStringUTFChars(errorMessageIn, nullptr);
        errorMessage = std::make_shared<std::string>(uuidChars);
        env->ReleaseStringUTFChars(errorMessageIn, uuidChars);
      }

      uint8_t *buffer = nullptr;
      auto size = getBufferFromSharedMemory(env, sharedMemory, width, height, errorMessage, promise, &buffer);

      jvm->DetachCurrentThread();
      if (size > 0) {
        promise->resolve([buffer, size, width, height](auto &runtime) {
          return Object::createFromHostObject(
            runtime, std::make_shared<wgpu::ImageBitmapHostObject>(buffer, size, (uint32_t)width, (uint32_t)height));
        });
      }
    });
    thread.detach();
  });
}

Value wgpu::getImageBitmapFromBlob(Runtime &runtime, Object obj, std::shared_ptr<JSIInstance> jsiInstance) {
  auto jsDataPtr = std::make_shared<Object>(obj.getPropertyAsObject(runtime, "_data"));
  return Promise::makeJSPromise(jsiInstance, [jsDataPtr](auto &runtime, auto &promise) {
    auto jsData = std::move(*jsDataPtr.get());
    auto env = WGPUAndroidInstance::instance->getMainThreadEnv();
    auto loader = WGPUAndroidInstance::instance->getImageLoaderFactory()->makeBlobBitmapLoader(env);
    if (loader == nullptr) {
      std::ostringstream ss;
      ss << __FILE__ << ":" << __LINE__ << " BlobBitmapLoader was not found";
      promise->reject([message = ss.str()](auto &runtime) { return makeJSError(runtime, message); });
      return;
    }
    auto blobId = WGPU_UTF8(jsData, blobId);
    auto offset = WGPU_NUMBER_OPT(jsData, offset, int, 0);
    auto sizeIn = WGPU_NUMBER_OPT(jsData, size, int, -1);
    auto result = loader->loadBitmap(env, blobId.data(), offset, sizeIn);
    if (result != 0) {
      promise->reject([](auto &runtime) { return makeJSError(runtime, "Failed to load blob image"); });
      return;
    }

    auto sharedMemory = loader->getSharedMemory(env);
    auto width = loader->getWidth(env);
    auto height = loader->getHeight(env);
    uint8_t *buffer = nullptr;
    auto size = getBufferFromSharedMemory(env, sharedMemory, width, height, nullptr, promise, &buffer);
    if (size > 0) {
      promise->resolve([buffer, size, width, height](auto &runtime) {
        auto bitmap = std::make_shared<wgpu::ImageBitmapHostObject>(buffer, size, (uint32_t)width, (uint32_t)height);
        return Object::createFromHostObject(runtime, bitmap);
      });
    }
  });
}

size_t wgpu::getBufferFromSharedMemory(JNIEnv *env, jobject sharedMemory, jint width, jint height,
                                       const std::shared_ptr<std::string> &errorMessage,
                                       std::shared_ptr<Promise> promise, uint8_t **dataOut) {
  if (sharedMemory == nullptr || width == 0 || height == 0) {
    WGPU_LOG_ERROR("%s:%i Failed to get bitmap %s", __FILE__, __LINE__,
                   errorMessage != nullptr ? errorMessage->data() : "");
    promise->reject([](auto &runtime) { return makeJSError(runtime, "Failed to fetch bitmap"); });
    return 0;
  }

  auto sharedMemoryFd = ASharedMemory_dupFromJava(env, sharedMemory);
  auto size = ASharedMemory_getSize(sharedMemoryFd);
  auto buffer = (uint8_t *)malloc(size);
  auto bytesRead = read(sharedMemoryFd, buffer, size);
  close(sharedMemoryFd);

  if (bytesRead != (ssize_t)size) {
    WGPU_LOG_ERROR(
      "%s:%i Copying shared memory bitmap to buffer failed. Bytes read: "
      "%zi shared memory size: %zu",
      __FILE__, __LINE__, bytesRead, size);
    promise->reject(
      [](auto &runtime) { return makeJSError(runtime, "Copying shared memory bitmap to buffer failed"); });
    return 0;
  }

  *dataOut = buffer;
  return size;
}
