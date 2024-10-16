#include <ReactCommon/CallInvokerHolder.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <jsi/jsi.h>

#include <memory>
#include <unordered_map>

#include "InstallRootJSI.h"
#include "JNIBitmapLoaderFactory.h"
#include "JSIInstance.h"
#include "Surface.h"
#include "WGPUAndroidInstance.h"
#include "WGPULog.h"
#include "webgpu.h"

using namespace wgpu;
using namespace facebook::jsi;
using namespace facebook::jni;
using namespace facebook::react;

extern "C" JNIEXPORT jboolean JNICALL Java_com_webgpu_CxxBridge_00024Companion_installJsi(JNIEnv *env, jobject obj,
                                                                                          jlong jsiRuntimeRef,
                                                                                          jobject jsCallInvokerHolder,
                                                                                          jobject bitmapLoaderFactory) {
  JavaVM *jvm = nullptr;
  auto status = env->GetJavaVM(&jvm);
  if (status != JNI_OK) {
    WGPU_LOG_ERROR("%s:%i env->GetJavaVM failed.", __FILE__, __LINE__);
    return false;
  }

  auto runtime{reinterpret_cast<Runtime *>(jsiRuntimeRef)};
  auto jsCallInvoker{alias_ref<CallInvokerHolder::javaobject>{
    reinterpret_cast<CallInvokerHolder::javaobject>(jsCallInvokerHolder)} -> cthis()
                       ->getCallInvoker()};

  auto newSurfaces = std::make_shared<std::unordered_map<std::string, std::shared_ptr<Surface>>>();
  JSIInstance::instance = std::make_unique<JSIInstance>(*runtime, std::make_shared<Thread>(jsCallInvoker));
  JSIInstance::instance->onCreateSurface = [newSurfaces](std::string uuid, std::shared_ptr<Surface> surface) {
    newSurfaces->insert_or_assign(uuid, surface);
  };

  WGPUAndroidInstance::instance = std::make_unique<WGPUAndroidInstance>(jvm, env);
  WGPUAndroidInstance::instance->setBitmapLoaderFactory(env, bitmapLoaderFactory);

  installRootJSI(*runtime, newSurfaces);
  return true;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_webgpu_CxxBridge_00024Companion_onSurfaceCreated(
  JNIEnv *env, jobject obj, jobject inSurface, jstring inUUID, jfloat density) {
  ANativeWindow *window = ANativeWindow_fromSurface(env, inSurface);
  const char *uuidChars = env->GetStringUTFChars(inUUID, nullptr);
  std::string uuid(uuidChars);
  env->ReleaseStringUTFChars(inUUID, uuidChars);

  struct WGPUSurfaceDescriptorFromAndroidNativeWindow descriptorFromNativeWindow = {
    .chain =
      (const WGPUChainedStruct){
        .sType = WGPUSType_SurfaceDescriptorFromAndroidNativeWindow,
      },
    .window = window,
  };
  struct WGPUSurfaceDescriptor descriptor = {
    .nextInChain = (const WGPUChainedStruct *)&descriptorFromNativeWindow,
  };

  WGPUInstance instance = wgpuCreateInstance(nullptr);

  if (instance == nullptr) {
    WGPU_LOG_ERROR("%s:%i wgpuCreateInstance() failed.", __FILE__, __LINE__);
    ANativeWindow_release(window);
    return false;
  }

  auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

  if (surface == nullptr) {
    WGPU_LOG_ERROR("%s:%i wgpuInstanceCreateSurface() failed.", __FILE__, __LINE__);
    ANativeWindow_release(window);
    return false;
  }

  auto width = ANativeWindow_getWidth(window);
  auto height = ANativeWindow_getHeight(window);

  SurfaceSize surfaceSize = {
    .pixelWidth = (uint32_t)width,
    .pixelHeight = (uint32_t)height,
    .scale = density,
    .pointWidth = (float)width / density,
    .pointHeight = (float)height / density,
  };
  auto managedSurface = std::make_shared<Surface>(instance, surface, surfaceSize, window);

  JSIInstance::instance->jsThread->run(
    [uuid = std::move(uuid), managedSurface]() { JSIInstance::instance->onCreateSurface(uuid, managedSurface); });
  return true;
}

extern "C" JNIEXPORT void JNICALL Java_com_webgpu_CxxBridge_00024Companion_onSurfaceDestroyed(JNIEnv *env, jobject obj,
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
