#import "WGPUJsi.h"
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#import <jsi/jsi.h>
#import <memory>
#import <string>
#import "InstallRootJSI.h"
#import "JSIInstance.h"
#import "React/RCTBridge+Private.h"
#import "WGPUObjCInstance.h"
#import "webgpu.h"

using namespace facebook::react;
using namespace facebook::jsi;
using namespace wgpu;

@interface RCTBridge ()
- (std::shared_ptr<CallInvoker>)jsCallInvoker;
@end

@implementation WGPUJsi

RCT_EXPORT_MODULE(WGPUJsi)

@synthesize bridge;
@synthesize moduleRegistry;

RCT_EXPORT_SYNCHRONOUS_TYPED_METHOD(NSNumber *, installWithThreadId:(NSString *)threadId) {
  RCTCxxBridge *cxxBridge = (RCTCxxBridge *)self.bridge;
  if (cxxBridge == nil) {
    NSLog(@"Cxx bridge not found");
    return @(NO);
  }

  auto &runtime = *(Runtime *)cxxBridge.runtime;

  auto jsiInstance = std::make_shared<JSIInstance>(runtime, std::make_shared<Thread>([cxxBridge jsCallInvoker]));
  [[WGPUObjCInstance shared] loadModules:moduleRegistry];

  installRootJSI(runtime, jsiInstance);

  return @(YES);
}

#ifdef RCT_NEW_ARCH_ENABLED

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
  (const facebook::react::ObjCTurboModule::InitParams &)params {
  return std::make_shared<facebook::react::NativeWebgpuModuleSpecJSI>(params);
}

#endif

@end
