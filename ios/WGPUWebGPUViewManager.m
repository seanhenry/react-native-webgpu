#import <React/RCTViewManager.h>
#import <UIKit/UIKit.h>
#import "WGPUMetalLayers.h"
#import "WGPUWebGPUView.h"

@interface WGPUWebGPUViewManager : RCTViewManager
@end

@implementation WGPUWebGPUViewManager

RCT_EXPORT_MODULE(WGPUWebGPUView)
RCT_EXPORT_VIEW_PROPERTY(onInit, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(identifier, NSString)

- (UIView *)view {
    WGPUWebGPUView* view = [[WGPUWebGPUView alloc] init];
    return view;
}

@end
