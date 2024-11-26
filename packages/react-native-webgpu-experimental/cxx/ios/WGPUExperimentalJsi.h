#ifdef RCT_NEW_ARCH_ENABLED

#import <WebgpuExperimentalSpec/WebgpuExperimentalSpec.h>

@interface WGPUExperimentalJsi : NSObject <NativeWebgpuExperimentalModuleSpec>
#else
#import <React/RCTBridgeModule.h>

@interface WGPUExperimentalJsi : NSObject <RCTBridgeModule>
#endif

@end
