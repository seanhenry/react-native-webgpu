#pragma once

#include <jsi/jsi.h>

using namespace facebook::jsi;

namespace wgpu {

class AutoReleasePool {
 public:
  AutoReleasePool() = default;

  template <typename T>
  void add(std::shared_ptr<T> ptr) {
    pool.push_back(ptr);
  }

  std::shared_ptr<std::string> addString(Runtime &runtime, const Value &value) {
    auto str = std::make_shared<std::string>(value.asString(runtime).utf8(runtime));
    pool.push_back(str);
    return str;
  }

  void clear() { pool.clear(); }

  ~AutoReleasePool() { pool.clear(); }

 private:
  std::vector<std::shared_ptr<void>> pool;
};

}  // namespace wgpu
