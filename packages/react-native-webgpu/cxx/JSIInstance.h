#pragma once

#include <jsi/jsi.h>

#include <unordered_map>
#include <utility>

#include "Surface.h"
#include "Thread.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

class JSIInstance {
 public:
  static std::unique_ptr<JSIInstance> instance;
  explicit JSIInstance(Runtime &rt, std::shared_ptr<Thread> thread)
    : runtime(rt),
      jsThread(std::move(thread)),
      weakSurfaces(std::make_shared<std::unordered_map<std::string, std::weak_ptr<Surface>>>()) {}
  Runtime &runtime;
  std::shared_ptr<Thread> jsThread;
  std::shared_ptr<std::unordered_map<std::string, std::weak_ptr<Surface>>> weakSurfaces;

  std::function<void(std::string, std::shared_ptr<Surface>)> onCreateSurface;
};

}  // namespace wgpu
