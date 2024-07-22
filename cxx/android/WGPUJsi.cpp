#include "Surface.h"
#include "JSIInstance.h"
#include "InstallRootJSI.h"
#include "webgpu.h"
#include "WGPULog.h"
#include "JNIImageLoader.h"
#include <jni.h>
#include <jsi/jsi.h>
#include <ReactCommon/CallInvokerHolder.h>
#include <unordered_map>
#include <memory>
#include <android/native_window_jni.h>

using namespace wgpu;
using namespace facebook::jsi;
using namespace facebook::jni;
using namespace facebook::react;

extern "C" JNIEXPORT jboolean JNICALL
Java_com_webgpu_CxxBridge_00024Companion_installJsi(JNIEnv *env, jobject obj, jlong jsiRuntimeRef, jobject jsCallInvokerHolder) {
    JavaVM *jvm = nullptr;
    auto status = env->GetJavaVM(&jvm);
    if (status != JNI_OK) {
        LOG_ERROR("%s:%i env->GetJavaVM failed.", __FILE__, __LINE__);
        return false;
    }

    auto runtime{reinterpret_cast<Runtime *>(jsiRuntimeRef)};
    auto jsCallInvoker{alias_ref<CallInvokerHolder::javaobject>{
            reinterpret_cast<CallInvokerHolder::javaobject>(jsCallInvokerHolder)}->cthis()->getCallInvoker()};

    auto newSurfaces = std::make_shared<std::unordered_map<std::string, std::shared_ptr<Surface>>>();
    JSIInstance::instance = std::make_unique<JSIInstance>(*runtime,
                                                          std::make_shared<Thread>(jsCallInvoker));
    JSIInstance::instance->onCreateSurface = [newSurfaces](std::string uuid,
                                                           std::shared_ptr<Surface> surface) {
        newSurfaces->insert_or_assign(uuid, surface);
    };
    JSIInstance::instance->jvm = jvm;

    try {
        JSIInstance::instance->jniMethodIDs = std::make_shared<JNIMethodIDs>(env);
    } catch (std::exception &exception) {
        LOG_ERROR("%s:%i Failed to look up jni method ids. %s", __FILE__, __LINE__, exception.what());
        return false;
    }

    installRootJSI(*runtime, newSurfaces);
    return true;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_webgpu_CxxBridge_00024Companion_onSurfaceCreated(JNIEnv *env, jobject obj,
                                                          jobject inSurface, jstring inUUID) {
    ANativeWindow *window = ANativeWindow_fromSurface(env, inSurface);
    const char *uuidChars = env->GetStringUTFChars(inUUID, nullptr);
    std::string uuid(uuidChars);
    env->ReleaseStringUTFChars(inUUID, uuidChars);


    struct WGPUSurfaceDescriptorFromAndroidNativeWindow descriptorFromNativeWindow = {
            .chain = (const WGPUChainedStruct) {
                    .sType = WGPUSType_SurfaceDescriptorFromAndroidNativeWindow,
            },
            .window = window,
    };
    struct WGPUSurfaceDescriptor descriptor = {
            .nextInChain = (const WGPUChainedStruct *) &descriptorFromNativeWindow,
    };

    WGPUInstance instance = wgpuCreateInstance(nullptr);

    if (instance == nullptr) {
        LOG_ERROR("%s:%i wgpuCreateInstance() failed.", __FILE__, __LINE__);
        ANativeWindow_release(window);
        return false;
    }

    auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

    if (surface == nullptr) {
        LOG_ERROR("%s:%i wgpuInstanceCreateSurface() failed.", __FILE__, __LINE__);
        ANativeWindow_release(window);
        return false;
    }

    auto width = (uint32_t) ANativeWindow_getWidth(window);
    auto height = (uint32_t) ANativeWindow_getHeight(window);
    auto managedSurface = std::make_shared<Surface>(instance, surface, width, height, window);

    JSIInstance::instance->jsThread->run([uuid = std::move(uuid), managedSurface]() {
        JSIInstance::instance->onCreateSurface(uuid, managedSurface);
    });
    return true;
}

extern "C" JNIEXPORT void JNICALL
Java_com_webgpu_CxxBridge_00024Companion_onSurfaceDestroyed(JNIEnv *env, jobject obj,
                                                            jstring inUUID) {
    const char *uuidChars = env->GetStringUTFChars(inUUID, nullptr);
    std::string uuid(uuidChars);
    env->ReleaseStringUTFChars(inUUID, uuidChars);

    JSIInstance::instance->jsThread->run([uuid = std::move(uuid)]() {

        auto surfaces = JSIInstance::instance->weakSurfaces;
        auto iterator = surfaces->find(uuid);
        if (iterator != surfaces->end()) {
            auto weakSurface = surfaces->at(uuid);
            auto surface = weakSurface.lock();
            if (surface != nullptr) {
                surface->invalidateTimer();
            }
        }
    });
}
