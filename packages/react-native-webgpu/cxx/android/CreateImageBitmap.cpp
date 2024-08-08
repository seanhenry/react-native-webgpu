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

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

Value fetchImageBitmap(Runtime &runtime, std::string uri);

Value getImageBitmapFromBlob(Runtime &runtime, Object obj);

size_t getBufferFromSharedMemory(JNIEnv *env, jobject sharedMemory, jint width, jint height,
                                 const std::shared_ptr<std::string> &errorMessage,
                                 Promise<void *> *promise, uint8_t **dataOut);

}  // namespace wgpu

Function wgpu::createImageBitmap(Runtime &runtime) {
    return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, []) {
        auto obj = arguments[0].asObject(runtime);
        if (obj.hasProperty(runtime, "uri")) {
            auto uri = WGPU_UTF8(obj, uri);
            return fetchImageBitmap(runtime, std::move(uri));
        } else if (obj.hasProperty(runtime, "_data")) {
            return getImageBitmapFromBlob(runtime, std::move(obj));
        }
        throw JSError(runtime, "[WebGPU] Unsupported type passed to createImageBitmap");
    });
}

Value wgpu::fetchImageBitmap(Runtime &runtime, std::string uri) {
    auto promise = new Promise<void *>(runtime);
    return promise->jsPromise([promise, uri = std::move(uri)]() {
        auto thread = std::thread([promise, uri]() {
            auto jvm = WGPUAndroidInstance::instance->getJVM();
            if (jvm == nullptr) {
                JSIInstance::instance->jsThread->run([promise]() {
                    promise->reject(makeJSError(promise->runtime, "The JVM is not available"));
                    delete promise;
                });
                return;
            }
            JNIEnv *env = nullptr;
            auto getEngStatus = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
            if (getEngStatus == JNI_EDETACHED) {
                jvm->AttachCurrentThread(&env, nullptr);
            } else if (getEngStatus != JNI_OK) {
                WGPU_LOG_ERROR("%s:%i jvm->GetEnv() returned status %i", __FILE__, __LINE__,
                               getEngStatus);
                JSIInstance::instance->jsThread->run([promise]() {
                    promise->reject(makeJSError(promise->runtime, "Failed to get JNIEnv"));
                    delete promise;
                });
                return;
            }

            auto bitmapLoader =
                WGPUAndroidInstance::instance->getImageLoaderFactory()->makeHttpBitmapLoader(env,
                                                                                             uri);
            bitmapLoader->fetchBitmap(env);

            auto sharedMemory = bitmapLoader->getSharedMemory(env);
            auto width = bitmapLoader->getWidth(env);
            auto height = bitmapLoader->getHeight(env);

            std::shared_ptr<std::string> errorMessage;
            auto errorMessageIn = bitmapLoader->getErrorMessage(env);
            if (errorMessageIn != nullptr) {
                const char *uuidChars = env->GetStringUTFChars(errorMessageIn, nullptr);
                errorMessage = std::make_shared<std::string>(uuidChars);
                env->ReleaseStringUTFChars(errorMessageIn, uuidChars);
            }

            uint8_t *buffer = nullptr;
            auto size = getBufferFromSharedMemory(env, sharedMemory, width, height, errorMessage,
                                                  promise, &buffer);

            jvm->DetachCurrentThread();
            JSIInstance::instance->jsThread->run([promise, buffer, size, width, height]() {
                auto obj = Object::createFromHostObject(
                    promise->runtime, std::make_shared<wgpu::ImageBitmapHostObject>(
                                          buffer, size, (uint32_t)width, (uint32_t)height));
                promise->resolve(std::move(obj));
                delete promise;
            });
        });
        thread.detach();
    });
}

Value wgpu::getImageBitmapFromBlob(Runtime &runtime, Object obj) {
    auto promise = new Promise<void *>(runtime);
    auto jsDataPtr = std::make_shared<Object>(obj.getPropertyAsObject(runtime, "_data"));
    return promise->jsPromise([promise, jsDataPtr] {
        auto jsData = std::move(*jsDataPtr.get());
        auto &runtime = promise->runtime;
        auto env = WGPUAndroidInstance::instance->getMainThreadEnv();
        auto loader =
            WGPUAndroidInstance::instance->getImageLoaderFactory()->makeBlobBitmapLoader(env);
        if (loader == nullptr) {
            std::ostringstream ss;
            ss << __FILE__ << ":" << __LINE__ << " BlobBitmapLoader was not found";
            promise->reject(makeJSError(promise->runtime, ss.str()));
            delete promise;
        }
        auto blobId = WGPU_UTF8(jsData, blobId);
        auto offset = WGPU_NUMBER_OPT(jsData, offset, int, 0);
        auto sizeIn = WGPU_NUMBER_OPT(jsData, size, int, -1);
        auto result = loader->loadBitmap(env, blobId.data(), offset, sizeIn);
        if (result != 0) {
            promise->reject(makeJSError(runtime, "Failed to load blob image"));
            delete promise;
            return;
        }

        auto sharedMemory = loader->getSharedMemory(env);
        auto width = loader->getWidth(env);
        auto height = loader->getHeight(env);
        uint8_t *buffer = nullptr;
        auto size =
            getBufferFromSharedMemory(env, sharedMemory, width, height, nullptr, promise, &buffer);
        if (size > 0) {
            auto bitmap = std::make_shared<wgpu::ImageBitmapHostObject>(
                buffer, size, (uint32_t)width, (uint32_t)height);
            auto obj = Object::createFromHostObject(promise->runtime, bitmap);
            promise->resolve(std::move(obj));
            delete promise;
        }
    });
}

size_t wgpu::getBufferFromSharedMemory(JNIEnv *env, jobject sharedMemory, jint width, jint height,
                                       const std::shared_ptr<std::string> &errorMessage,
                                       Promise<void *> *promise, uint8_t **dataOut) {
    if (sharedMemory == nullptr || width == 0 || height == 0) {
        WGPU_LOG_ERROR("%s:%i Failed to get bitmap %s", __FILE__, __LINE__,
                       errorMessage != nullptr ? errorMessage->data() : "");
        JSIInstance::instance->jsThread->run([promise]() {
            promise->reject(makeJSError(promise->runtime, "Failed to fetch bitmap"));
            delete promise;
        });
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
        JSIInstance::instance->jsThread->run([promise]() {
            promise->reject(
                makeJSError(promise->runtime, "Copying shared memory bitmap to buffer failed"));
            delete promise;
        });
        return 0;
    }

    *dataOut = buffer;
    return size;
}
