#import "WGPUJsi.h"
#import "React/RCTBridge+Private.h"
#import "webgpu.h"
#import <jsi/jsi.h>
#import <React-callinvoker/ReactCommon/CallInvoker.h>
#include "JSIInstance.h"
#include "InstallRootJSI.h"

using namespace facebook::react;
using namespace facebook::jsi;
using namespace wgpu;

@interface RCTBridge ()
- (std::shared_ptr<CallInvoker>)jsCallInvoker;
@end

@implementation WGPUJsi

RCT_EXPORT_MODULE(WGPUJsi)

@synthesize bridge;

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
    RCTCxxBridge *cxxBridge = (RCTCxxBridge*)self.bridge;
    if (cxxBridge == nil) {
        NSLog(@"Cxx bridge not found");
        return @(NO);
    }

    auto &runtime = *(Runtime *)cxxBridge.runtime;

    auto surfaces = std::make_shared<std::unordered_map<std::string, std::shared_ptr<Surface>>>();
    JSIInstance::instance = std::make_unique<JSIInstance>(runtime, std::make_shared<Thread>([cxxBridge jsCallInvoker]));
    JSIInstance::instance->onCreateSurface = [surfaces](std::string uuid, std::shared_ptr<Surface> surface) {
        surfaces->insert_or_assign(uuid, surface);
    };

    installRootJSI(runtime, surfaces);

    return @(YES);
}

@end
