#import <React/RCTViewManager.h>
#import <UIKit/UIKit.h>
#import "WGPUWebGPUView.h"

@interface WGPUWebGPUViewManager : RCTViewManager
@end

@implementation WGPUWebGPUViewManager

RCT_EXPORT_MODULE(WGPUWebGPUView)
RCT_EXPORT_VIEW_PROPERTY(onCreateSurface, RCTBubblingEventBlock)
RCT_EXPORT_VIEW_PROPERTY(pollSize, BOOL)

- (UIView*)view {
  WGPUWebGPUView* view = [[WGPUWebGPUView alloc] init];
  return view;
}

@end
