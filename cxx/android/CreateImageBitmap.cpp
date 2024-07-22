#include "CreateImageBitmap.h"
#include "WGPUJsiUtils.h"
#include "JSIInstance.h"
#include "WGPULog.h"
#include "ImageBitmapHostObject.h"
#include <jni.h>
#include <thread>
#include <android/sharedmem.h>
#include <android/sharedmem_jni.h>
#include <unistd.h>

using namespace facebook::jsi;
using namespace wgpu;

Function wgpu::createImageBitmap(Runtime &runtime) {
    return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, []) {
        auto uri = WGPU_UTF8(arguments[0].asObject(runtime), uri);

        auto promise = new Promise<void *>(runtime);
        return promise->jsPromise([promise, uri = std::move(uri)]() {
            auto thread = std::thread([promise, uri]() {
                auto jvm = JSIInstance::instance->jvm;
                if (jvm == nullptr) {
                    JSIInstance::instance->jsThread->run([promise]() {
                        promise->reject(makeJSError(promise->runtime, "The JVM is not available"));
                        delete promise;
                    });
                    return;
                }
                JNIEnv *env = nullptr;
                auto getEngStatus = jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
                if (getEngStatus == JNI_EDETACHED) {
                    jvm->AttachCurrentThread(&env, nullptr);
                } else if (getEngStatus != JNI_OK) {
                    LOG_ERROR("%s:%i jvm->GetEnv() returned status %i", __FILE__, __LINE__, getEngStatus);
                    JSIInstance::instance->jsThread->run([promise]() {
                        promise->reject(makeJSError(promise->runtime, "Failed to get JNIEnv"));
                        delete promise;
                    });
                    return;
                }

                auto bitmapLoader = JNIImageLoader::fromUri(env, JSIInstance::instance->jniMethodIDs, uri);
                bitmapLoader->fetchBitmap(env);

                auto sharedMemory = bitmapLoader->getSharedMemory(env);
                auto width = bitmapLoader->getWidth(env);
                auto height = bitmapLoader->getHeight(env);

                if (sharedMemory == nullptr || width == 0 || height == 0) {
                    auto errorMessageIn = bitmapLoader->getErrorMessage(env);
                    const char *uuidChars = env->GetStringUTFChars(errorMessageIn, nullptr);
                    std::string errorMessage(uuidChars);
                    env->ReleaseStringUTFChars(errorMessageIn, uuidChars);
                    LOG_ERROR("%s:%i Failed to fetch bitmap (uri: %s) %s ", __FILE__, __LINE__, uri.data(), errorMessage.data());
                    JSIInstance::instance->jsThread->run([promise]() {
                        promise->reject(makeJSError(promise->runtime, "Failed to fetch bitmap"));
                        delete promise;
                    });
                    delete promise;
                    return;
                }

                auto sharedMemoryFd = ASharedMemory_dupFromJava(env, sharedMemory);
                auto size = ASharedMemory_getSize(sharedMemoryFd);
                auto buffer = (uint8_t *)malloc(size);
                auto bytesRead = read(sharedMemoryFd, buffer, size);
                close(sharedMemoryFd);
                if (bytesRead != size) {
                    LOG_ERROR("%s:%i Copying shared memory bitmap to buffer failed. Bytes read: %zi shared memory size: %lu", __FILE__, __LINE__, bytesRead, size);
                    JSIInstance::instance->jsThread->run([promise]() {
                        promise->reject(makeJSError(promise->runtime, "Copying shared memory bitmap to buffer failed"));
                        delete promise;
                    });
                    return;
                }
                jvm->DetachCurrentThread();
                JSIInstance::instance->jsThread->run([promise, buffer, size, width, height](){
                    auto obj = Object::createFromHostObject(promise->runtime, std::make_shared<wgpu::ImageBitmapHostObject>(buffer, size, (uint32_t)width, (uint32_t)height));
                    promise->resolve(std::move(obj));
                    delete promise;
                });
           });
           thread.detach();
        });
    });
}
