#import <UIKit/UIKit.h>
#import <jsi/jsi.h>

using namespace facebook::jsi;

NS_ASSUME_NONNULL_BEGIN

@interface UIImage (Bitmap)

- (Value)bitmapImageWithRuntime:(Runtime&)runtime;

@end

NS_ASSUME_NONNULL_END
