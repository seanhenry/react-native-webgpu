#import <UIKit/UIKit.h>
#import <React/RCTComponent.h>

NS_ASSUME_NONNULL_BEGIN

@interface WGPUWebGPUView : UIView

@property (nonatomic, copy) RCTBubblingEventBlock onInit;
@property (nonatomic, copy) NSString *identifier;
- (CAMetalLayer *)metalLayer;

@end

NS_ASSUME_NONNULL_END
