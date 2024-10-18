#pragma once

#include <WebgpuSpecJSI.h>

using namespace facebook::jsi;

namespace facebook::react {

class WGPUJsi : public NativeWebgpuModuleCxxSpec<WGPUJsi> {
 public:
  WGPUJsi(std::shared_ptr<CallInvoker> jsInvoker);

  Object getConstants(Runtime &rt);
  bool installWithThreadId(Runtime &rt, String threadId);
};
}  // namespace facebook::react
