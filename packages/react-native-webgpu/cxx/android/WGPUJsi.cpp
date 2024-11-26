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
#include "SurfaceSize.h"
#include "SurfacesManager.h"
#include "WGPUAndroidInstance.h"
#include "WGPULog.h"
#include "webgpu.h"
#include "wgpu.h"

using namespace wgpu;
using namespace facebook::jsi;
using namespace facebook::jni;
using namespace facebook::react;

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved) {
  JNIEnv *env;
  if (jvm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  WGPUAndroidInstance::instance = std::make_unique<WGPUAndroidInstance>(jvm, env);

  return JNI_VERSION_1_6;
}
extern "C" JNIEXPORT void JNICALL JNI_OnUnLoad(JavaVM *jvm, void *reserved) {}

extern "C" JNIEXPORT void JNICALL Java_com_webgpu_CxxBridge_00024Companion_setJavaModules(JNIEnv *env, jobject obj,
                                                                                          jobject bitmapLoaderFactory,
                                                                                          jobject exceptionHandler) {
  if (WGPUAndroidInstance::instance == nullptr) {
    WGPU_LOG_ERROR("WGPUAndroidInstance was null");
    return;
  }

  WGPUAndroidInstance::instance->setBitmapLoaderFactory(env, bitmapLoaderFactory);
  WGPUAndroidInstance::instance->setExceptionHandler(env, exceptionHandler);
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_webgpu_CxxBridge_00024Companion_installJsi(
  JNIEnv *env, jobject obj, jstring threadId, jlong jsiRuntimeRef, jobject jsCallInvokerHolder,
  jobject bitmapLoaderFactory, jobject exceptionHandler) {
  auto runtime{reinterpret_cast<Runtime *>(jsiRuntimeRef)};
  auto jsCallInvoker{alias_ref<CallInvokerHolder::javaobject>{reinterpret_cast<CallInvokerHolder::javaobject>(
                       jsCallInvokerHolder)} -> cthis()->getCallInvoker()};

  auto jsiInstance = std::make_shared<JSIInstance>(*runtime, std::make_shared<Thread>(jsCallInvoker));

  installRootJSI(*runtime, jsiInstance);

  return true;
}

extern "C" JNIEXPORT jboolean JNICALL Java_com_webgpu_CxxBridge_00024Companion_onSurfaceCreated(
  JNIEnv *env, jobject obj, jobject inSurface, jstring inUUID, jfloat density, jint backends) {
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

  auto extras = (const WGPUInstanceExtras){
    .backends = (uint32_t)backends,
    .flags = WGPUInstanceFlag_Default,
    .dx12ShaderCompiler = WGPUDx12Compiler_Undefined,
    .gles3MinorVersion = WGPUGles3MinorVersion_Automatic,
    .dxilPath = nullptr,
    .dxcPath = nullptr,
  };
  extras.chain.sType = (WGPUSType)WGPUSType_InstanceExtras;
  auto instanceDescriptor = (const WGPUInstanceDescriptor){
    .nextInChain = (const WGPUChainedStruct *)&extras,
  };

  WGPUInstance instance = wgpuCreateInstance(&instanceDescriptor);

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

  auto surfaceSize = std::make_shared<PullSurfaceSize>(window, density);
  auto managedSurface = std::make_shared<Surface>(instance, surface, surfaceSize, window);

  SurfacesManager::getInstance()->set(uuid, managedSurface);

  ANativeWindow_release(window);

  return true;
}

extern "C" JNIEXPORT void JNICALL Java_com_webgpu_CxxBridge_00024Companion_onSurfaceDestroyed(JNIEnv *env, jobject obj,
                                                                                              jstring inUUID) {
  const char *uuidChars = env->GetStringUTFChars(inUUID, nullptr);
  std::string uuid(uuidChars);
  env->ReleaseStringUTFChars(inUUID, uuidChars);

  auto weakSurface = SurfacesManager::getInstance()->get(uuid);
  auto surface = weakSurface.lock();
  if (surface != nullptr) {
    surface->invalidateTimer();
    SurfacesManager::getInstance()->remove(uuid);
  }
}

#if WGPU_NEW_ARCHITECTURE

#include "WGPUJsi.h"

namespace facebook::react {
WGPUJsi::WGPUJsi(std::shared_ptr<CallInvoker> jsInvoker) : NativeWebgpuModuleCxxSpec(std::move(jsInvoker)) {}

bool WGPUJsi::installWithThreadId(Runtime &runtime, String threadId) {
  auto jsiInstance = std::make_shared<JSIInstance>(runtime, std::make_shared<Thread>(jsInvoker_));
  installRootJSI(runtime, jsiInstance);
  return true;
}
}  // namespace facebook::react

#endif
