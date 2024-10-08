#pragma once

#include <functional>
#include <memory>

namespace facebook::react {
class CallInvoker;
}

namespace wgpu {

class Thread {
 public:
  explicit Thread(std::shared_ptr<facebook::react::CallInvoker> invoker) : _invoker(invoker) {}
  void run(std::function<void()>&&);

 private:
  std::shared_ptr<facebook::react::CallInvoker> _invoker;
};

}  // namespace wgpu
