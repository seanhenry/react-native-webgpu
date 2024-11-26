#import "WGPUExperimentalJsi.h"
#import <JSIInstance.h>
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#import "InstallExperimentalJSI.h"
#import "React/RCTBridge+Private.h"
#import "ThreadManager.h"

using namespace facebook::react;
using namespace facebook::jsi;

#if RCT_NEW_ARCH_ENABLED && DEBUG
const BOOL WGPU_ENABLE_THREADS = YES;
#else
const BOOL WGPU_ENABLE_THREADS = NO;
#endif

@interface RCTBridge ()
- (std::shared_ptr<CallInvoker>)jsCallInvoker;
@end

@implementation WGPUExperimentalJsi

@synthesize bridge;

RCT_EXPORT_MODULE(WGPUExperimentalJsi)

- (NSDictionary *)constantsToExport {
  return @{
    @"ENABLE_THREADS" : @(WGPU_ENABLE_THREADS),
  };
}

RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSNumber *, installWithThreadId:(NSString *)threadId) {
  RCTCxxBridge *cxxBridge = (RCTCxxBridge *)self.bridge;
  if (cxxBridge == nil) {
    NSLog(@"Cxx bridge not found");
    return @(NO);
  }

  auto &runtime = *(Runtime *)cxxBridge.runtime;
  auto jsiInstance = std::make_shared<JSIInstance>(runtime, std::make_shared<Thread>([cxxBridge jsCallInvoker]));

#ifdef RCT_NEW_ARCH_ENABLED
  std::string threadIdStr = threadId.UTF8String;
  ThreadManager::getInstance()->setJSIInstance(jsiInstance, threadIdStr);
  ThreadManager::getInstance()->installJsi(runtime);
#endif

  installExperimentalJSI(runtime, jsiInstance);
  return @(YES);
}

#ifdef RCT_NEW_ARCH_ENABLED

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
  (const facebook::react::ObjCTurboModule::InitParams &)params {
  return std::make_shared<facebook::react::NativeWebgpuExperimentalModuleSpecJSI>(params);
}

- (facebook::react::ModuleConstants<JS::NativeWebgpuExperimentalModule::Constants::Builder>)getConstants {
  return [self constantsToExport];
}

#endif

@end
