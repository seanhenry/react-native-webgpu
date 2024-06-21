#import <React/RCTViewManager.h>
#import <UIKit/UIKit.h>
#import "WGPUMetalLayers.h"

@interface WGPUWebGPUViewManager : RCTViewManager
@end

@interface WGPUWebGPUView : UIView

@property (nonatomic, copy) RCTBubblingEventBlock onInit;
@property (nonatomic, copy) NSString *identifier;
- (CAMetalLayer *)metalLayer;

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

@implementation WGPUWebGPUView

+ (Class)layerClass {
    return [CAMetalLayer self];
}

- (CAMetalLayer *)metalLayer {
    return (CAMetalLayer *)self.layer;
}

- (void)willMoveToWindow:(UIWindow *)newWindow {
    if (newWindow != nil) {
        [[WGPUMetalLayers instance].layers setObject:self.metalLayer forKey:self.identifier];
        self.onInit(@{@"identifier": self.identifier});
    }
}

@end
