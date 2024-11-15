#pragma once

#include <WebgpuSpecJSI.h>

using namespace facebook::jsi;

namespace facebook::react {

class WGPUJsi : public NativeWebgpuModuleCxxSpec<WGPUJsi> {
 public:
  WGPUJsi(std::shared_ptr<CallInvoker> jsInvoker);

  bool installWithThreadId(Runtime &rt, String threadId);
};
}  // namespace facebook::react
