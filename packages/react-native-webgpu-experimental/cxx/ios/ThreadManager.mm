#ifdef RCT_NEW_ARCH_ENABLED

#include "ThreadManager.h"
#import <React/RCTBundleURLProvider.h>
#import <ReactCommon/RCTHermesInstance.h>
#import <ReactCommon/RCTHost.h>
#import <ReactCommon/RCTTurboModule.h>
#import <ReactCommon/RCTTurboModuleManager.h>
#include <jsi/jsi.h>
#include <functional>
#include "WGPUJsiUtils.h"

using namespace facebook::react;
using namespace facebook::jsi;

@interface RCTHost ()

- (void)setContextContainerHandler:(id<RCTContextContainerHandling>)contextContainerHandler;
- (void)setBundleURLProvider:(RCTHostBundleURLProvider)bundleURLProvider;

@end

@interface WGPUThreadInstance () <RCTHostRuntimeDelegate,
                                  RCTHostDelegate,
                                  RCTTurboModuleManagerDelegate,
                                  RCTContextContainerHandling>

@property(class, nonatomic, readonly) BOOL bridgelessEnabled;
@property(class, nonatomic, readonly) BOOL newArchEnabled;
@property(nonatomic) RCTHost *reactHost;
@property(nonatomic, copy) NSString *threadId;

- (instancetype)initWithBundleURL:(NSURL *)url threadId:(NSString *)threadId;
- (void)start;
- (void)attachSurface:(NSString *)uuid jsiInstance:(std::shared_ptr<JSIInstance>)jsiInstance;

@end

namespace wgpu {

ThreadManager *ThreadManager::_instance = new ThreadManager;

void ThreadManager::installJsi(Runtime &runtime) {
  auto isEnabled = WGPUThreadInstance.newArchEnabled && WGPUThreadInstance.bridgelessEnabled;
  if (!isEnabled) {
    jsLog(runtime, "error", {"Threads are only available with the new architecture and bridgeless mode"});
  }
  Object reactNativeWebGPUThreads(runtime);
  reactNativeWebGPUThreads.setProperty(runtime, "spawn", WGPU_FUNC_FROM_HOST_FUNC(preload, 1, [isEnabled]) {
    if (!isEnabled) return Value::undefined();
    auto obj = arguments[0].asObject(runtime);
    auto bundleId = WGPU_UTF8(obj, bundleId);
    auto threadId = WGPU_UTF8(obj, threadId);

    ThreadManager::getInstance()->spawn(bundleId, threadId);

    return Value::undefined();
  }));
  reactNativeWebGPUThreads.setProperty(runtime, "attachSurface", WGPU_FUNC_FROM_HOST_FUNC(preload, 1, [isEnabled]) {
    if (!isEnabled) return Value::undefined();
    auto obj = arguments[0].asObject(runtime);
    auto uuid = WGPU_UTF8(obj, uuid);
    auto threadId = WGPU_UTF8(obj, threadId);

    ThreadManager::getInstance()->attachSurface(uuid, threadId);

    return Value::undefined();
  }));
  runtime.global().setProperty(runtime, "reactNativeWebGPUThreads", reactNativeWebGPUThreads);
}

void ThreadManager::spawn(std::string &bundleId, std::string &threadId) {
  NSString *objcBundleId = [NSString stringWithCString:bundleId.data() encoding:NSUTF8StringEncoding];
  NSString *objcThreadId = [NSString stringWithCString:threadId.data() encoding:NSUTF8StringEncoding];
  NSURL *url = [[RCTBundleURLProvider sharedSettings] jsBundleURLForBundleRoot:objcBundleId];
  WGPUThreadInstance *instance = [[WGPUThreadInstance alloc] initWithBundleURL:url threadId:objcThreadId];

  {
    std::lock_guard<std::mutex> lock(_mutex);
    _threads[threadId] = std::make_shared<ThreadInstance>(instance);
  }

  [instance start];
}

void ThreadManager::attachSurface(std::string &uuid, std::string &threadId) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _threads.find(threadId);
    if (it != _threads.end()) {
      it->second->attachSurface(uuid);
    } else {
      throw JSINativeException("Attempted to attach surface to thread but thread doesn't exist");
    }
  }
}

void ThreadManager::setJSIInstance(std::shared_ptr<JSIInstance> jsiInstance, std::string &threadId) {
  std::lock_guard<std::mutex> lock(_mutex);
  auto it = _threads.find(threadId);
  if (it != _threads.end() && it->second != nullptr) {
    it->second->setJSIInstance(jsiInstance);
  }
}

void ThreadInstance::attachSurface(std::string &uuid) {
  [_instance attachSurface:[NSString stringWithCString:uuid.data() encoding:NSUTF8StringEncoding]
               jsiInstance:_jsiInstance];
}

}  // namespace wgpu

@implementation WGPUThreadInstance

- (instancetype)initWithBundleURL:(NSURL *)url threadId:(NSString *)threadId {
  self = [super init];
  if (self) {
    self.threadId = threadId;
    self.reactHost = [[RCTHost alloc]
      initWithBundleURLProvider:^{
        return url;
      }
      hostDelegate:self
      turboModuleManagerDelegate:self
      jsEngineProvider:^std::shared_ptr<JSRuntimeFactory>() {
        return std::make_shared<RCTHermesInstance>();
      }
      launchOptions:@{}];
    self.reactHost.runtimeDelegate = self;
    [self.reactHost setBundleURLProvider:^{
      return url;
    }];
    [self.reactHost setContextContainerHandler:self];
  }
  return self;
}

- (void)start {
  [self.reactHost start];
}

- (void)attachSurface:(NSString *)uuid jsiInstance:(std::shared_ptr<JSIInstance>)jsiInstance {
  if (jsiInstance == nullptr) {
    throw JSINativeException("Tried to attach a surface before the thread was installed");
  }
  std::string uuidStr = uuid.UTF8String;
  jsiInstance->jsThread->run([&runtime = jsiInstance->runtime, uuidStr]() {
    Object payload(runtime);
    payload.setProperty(runtime, "uuid", uuidStr);
    auto instance = WGPU_OBJ(runtime.global(), reactNativeWebGPUThreadsInstance);
    try {
      auto onAttachSurface = WGPU_OBJ(instance, onAttachSurface).asFunction(runtime);
      onAttachSurface.call(runtime, std::move(payload));
    } catch (...) {
      throw JSINativeException("Tried to attach a surface but onAttachSurface was not set");
    }
  });
}

#pragma mark - RCTHostRuntimeDelegate

- (void)host:(RCTHost *)host didInitializeRuntime:(Runtime &)runtime {
  Object instance(runtime);
  __weak WGPUThreadInstance *weakSelf = self;
  instance.setProperty(runtime, "getContext", WGPU_FUNC_FROM_HOST_FUNC(getContext, 0, [weakSelf]) {
    auto result = Object(runtime);
    result.setProperty(runtime, "threadId", [weakSelf.threadId UTF8String]);
    return result;
  }));
  runtime.global().setProperty(runtime, "reactNativeWebGPUThreadsInstance", instance);
}

#pragma mark - RCTHostDelegate

- (void)host:(RCTHost *)host
  didReceiveJSErrorStack:(NSArray<NSDictionary<NSString *, id> *> *)stack
                 message:(NSString *)message
             exceptionId:(NSUInteger)exceptionId
                 isFatal:(BOOL)isFatal {
}

- (void)hostDidStart:(RCTHost *)host {
}

#pragma mark - RCTContextContainerHandling

- (void)didCreateContextContainer:(std::shared_ptr<ContextContainer>)contextContainer {
}

#pragma mark - RCTTurboModuleManagerDelegate

- (Class)getModuleClassFromName:(const char *)name {
  return [((id)UIApplication.sharedApplication.delegate) getModuleClassFromName:name];
}

- (std::shared_ptr<TurboModule>)getTurboModule:(const std::string &)name
                                     jsInvoker:(std::shared_ptr<CallInvoker>)jsInvoker {
  return [((id)UIApplication.sharedApplication.delegate) getTurboModule:name jsInvoker:jsInvoker];
}

- (std::shared_ptr<TurboModule>)getTurboModule:(const std::string &)name
                                    initParams:(const ObjCTurboModule::InitParams &)params {
  return [((id)UIApplication.sharedApplication.delegate) getTurboModule:name initParams:params];
}

- (id<RCTTurboModule>)getModuleInstanceFromClass:(Class)moduleClass {
  return [((id)UIApplication.sharedApplication.delegate) getModuleInstanceFromClass:moduleClass];
}

+ (BOOL)bridgelessEnabled {
  return [((id)UIApplication.sharedApplication.delegate) bridgelessEnabled];
}

+ (BOOL)newArchEnabled {
  return [((id)UIApplication.sharedApplication.delegate) newArchEnabled];
}

@end

#endif
