#pragma once

#include <WebgpuExperimentalSpecJSI.h>

using namespace facebook::jsi;

namespace facebook::react {

class WGPUExperimentalJsi : public NativeWebgpuExperimentalModuleCxxSpec<WGPUExperimentalJsi> {
 public:
  WGPUExperimentalJsi(std::shared_ptr<CallInvoker> jsInvoker);

  Object getConstants(Runtime &rt);
  bool installWithThreadId(Runtime &rt, String threadId);
};
}  // namespace facebook::react
