/**
 * This code was generated by [react-native-codegen](https://www.npmjs.com/package/react-native-codegen).
 *
 * Do not edit this file as changes may cause incorrect behavior and will be lost
 * once the code is regenerated.
 *
 * @generated by codegen project: GenerateModuleCpp.js
 */

#include "WebgpuSpecJSI.h"

namespace facebook::react {

static jsi::Value __hostFunction_NativeWebgpuModuleCxxSpecJSI_getConstants(jsi::Runtime &rt, TurboModule &turboModule,
                                                                           const jsi::Value *args, size_t count) {
  return static_cast<NativeWebgpuModuleCxxSpecJSI *>(&turboModule)->getConstants(rt);
}
static jsi::Value __hostFunction_NativeWebgpuModuleCxxSpecJSI_installWithThreadId(jsi::Runtime &rt,
                                                                                  TurboModule &turboModule,
                                                                                  const jsi::Value *args,
                                                                                  size_t count) {
  return static_cast<NativeWebgpuModuleCxxSpecJSI *>(&turboModule)
    ->installWithThreadId(
      rt, count <= 0 ? throw jsi::JSError(rt, "Expected argument in position 0 to be passed") : args[0].asString(rt));
}

NativeWebgpuModuleCxxSpecJSI::NativeWebgpuModuleCxxSpecJSI(std::shared_ptr<CallInvoker> jsInvoker)
  : TurboModule("WGPUJsi", jsInvoker) {
  methodMap_["getConstants"] = MethodMetadata{0, __hostFunction_NativeWebgpuModuleCxxSpecJSI_getConstants};
  methodMap_["installWithThreadId"] =
    MethodMetadata{1, __hostFunction_NativeWebgpuModuleCxxSpecJSI_installWithThreadId};
}

}  // namespace facebook::react