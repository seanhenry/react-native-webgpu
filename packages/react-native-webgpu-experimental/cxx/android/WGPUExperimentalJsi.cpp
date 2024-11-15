#include <ReactCommon/CallInvokerHolder.h>
#include <jni.h>
#include <jsi/jsi.h>

#include <memory>

#include "InstallExperimentalJSI.h"
#include "JSIInstance.h"
#include "WGPULog.h"

using namespace wgpu;
using namespace facebook::jsi;
using namespace facebook::jni;
using namespace facebook::react;

extern "C" JNIEXPORT jboolean JNICALL Java_com_webgpu_experimental_CxxBridge_00024Companion_installJsi(
  JNIEnv *env, jobject obj, jstring threadId, jlong jsiRuntimeRef, jobject jsCallInvokerHolder) {
  auto runtime{reinterpret_cast<Runtime *>(jsiRuntimeRef)};
  auto jsCallInvoker{alias_ref<CallInvokerHolder::javaobject>{reinterpret_cast<CallInvokerHolder::javaobject>(
                       jsCallInvokerHolder)} -> cthis()->getCallInvoker()};

  auto jsiInstance = std::make_shared<JSIInstance>(*runtime, std::make_shared<Thread>(jsCallInvoker));

  installExperimentalJSI(*runtime, jsiInstance);

  return true;
}

#if WGPU_NEW_ARCHITECTURE

#include "WGPUExperimentalJsi.h"

namespace facebook::react {
WGPUExperimentalJsi::WGPUExperimentalJsi(std::shared_ptr<CallInvoker> jsInvoker)
  : NativeWebgpuExperimentalModuleCxxSpec(std::move(jsInvoker)) {}

jsi::Object WGPUExperimentalJsi::getConstants(Runtime &runtime) {
  Object constants(runtime);
  constants.setProperty(runtime, "ENABLE_THREADS", Value(false));
  return constants;
}
bool WGPUExperimentalJsi::installWithThreadId(Runtime &runtime, String threadId) {
  auto jsiInstance = std::make_shared<JSIInstance>(runtime, std::make_shared<Thread>(jsInvoker_));
  installExperimentalJSI(runtime, jsiInstance);
  return true;
}
}  // namespace facebook::react

#endif
