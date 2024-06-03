
#ifdef RCT_NEW_ARCH_ENABLED
#import "RNWebgpuSpec.h"

@interface Webgpu : NSObject <NativeWebgpuSpec>
#else
#import <React/RCTBridgeModule.h>

@interface Webgpu : NSObject <RCTBridgeModule>
#endif

@end
