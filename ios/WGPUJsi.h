#import "webgpu.h"
#import "wgpu.h"
#import <iostream>
#import <UIKit/UIKit.h>
#import "WGPUContext.h"
#import "ContextHostObject.h"
#import <jsi/jsi.h>

#ifdef RCT_NEW_ARCH_ENABLED
#import "RNWebgpuSpec.h"

@interface WGPUJsi : NSObject <NativeWebgpuSpec>
#else
#import <React/RCTBridgeModule.h>

@interface WGPUJsi : NSObject <RCTBridgeModule>
#endif

@end
