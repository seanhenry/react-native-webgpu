#import <UIKit/UIKit.h>
#import <jsi/jsi.h>
#import <iostream>
#import "ContextHostObject.h"
#import "WGPUContext.h"
#import "webgpu.h"
#import "wgpu.h"

#ifdef RCT_NEW_ARCH_ENABLED

#import <WebgpuSpec/WebgpuSpec.h>

@interface WGPUJsi : NSObject <NativeWebgpuModuleSpec>
#else
#import <React/RCTBridgeModule.h>

@interface WGPUJsi : NSObject <RCTBridgeModule>
#endif

@end
