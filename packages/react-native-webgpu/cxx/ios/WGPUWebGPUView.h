#import <React/RCTComponent.h>
#import <UIKit/UIKit.h>

#ifdef RCT_NEW_ARCH_ENABLED
#import <React/RCTViewComponentView.h>
#endif

NS_ASSUME_NONNULL_BEGIN

#ifdef RCT_NEW_ARCH_ENABLED
@interface WGPUWebGPUView : RCTViewComponentView
#else
@interface WGPUWebGPUView : UIView
#endif

@property(nonatomic, copy) RCTBubblingEventBlock onCreateSurface;
@property(nonatomic, copy) NSString *uuid;
- (CAMetalLayer *)metalLayer;

@end

NS_ASSUME_NONNULL_END
