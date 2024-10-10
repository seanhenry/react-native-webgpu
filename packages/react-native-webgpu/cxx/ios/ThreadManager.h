#ifdef WGPU_ENABLE_THREADS

#pragma once

#include <jsi/jsi.h>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "JSIInstance.h"

using namespace facebook::jsi;

@class WGPUThreadInstance;

namespace wgpu {

class ThreadInstance;

class ThreadManager {
 public:
  explicit ThreadManager() {}
  static inline ThreadManager *getInstance() { return _instance; }
  void installJsi(Runtime &runtime);
  void spawn(std::string &bundleId, std::string &threadId);
  void attachSurface(std::string &uuid, std::string &threadId);
  void setJSIInstance(std::shared_ptr<JSIInstance> jsiInstance, std::string &threadId);

 private:
  static ThreadManager *_instance;
  std::unordered_map<std::string, std::shared_ptr<ThreadInstance>> _threads;
  std::mutex _mutex;
};

class ThreadInstance {
 public:
  explicit ThreadInstance(WGPUThreadInstance *instance) : _instance(instance) {}
  friend class ThreadManager;

 private:
  static std::shared_ptr<ThreadInstance> make(WGPUThreadInstance *instance) {
    return std::make_shared<ThreadInstance>(instance);
  }
  void attachSurface(std::string &uuid);
  inline void setJSIInstance(std::shared_ptr<JSIInstance> jsiInstance) { _jsiInstance = jsiInstance; }

  WGPUThreadInstance *_instance;
  std::shared_ptr<JSIInstance> _jsiInstance = nullptr;
};
}  // namespace wgpu

@interface WGPUThreadInstance : NSObject

@end

#endif
