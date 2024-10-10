#pragma once

#include <jsi/jsi.h>

#include <utility>

#include "Thread.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

class JSIInstance {
 public:
  explicit JSIInstance(Runtime &rt, std::shared_ptr<Thread> thread) : runtime(rt), jsThread(std::move(thread)) {}
  Runtime &runtime;
  std::shared_ptr<Thread> jsThread;
};

}  // namespace wgpu
