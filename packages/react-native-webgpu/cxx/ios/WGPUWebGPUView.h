#import <React/RCTComponent.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface WGPUWebGPUView : UIView

@property(nonatomic, copy) RCTBubblingEventBlock onCreateSurface;
@property(nonatomic, copy) NSString *uuid;
- (CAMetalLayer *)metalLayer;

@end

NS_ASSUME_NONNULL_END
