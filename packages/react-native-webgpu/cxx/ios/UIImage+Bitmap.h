#import <UIKit/UIKit.h>
#import <jsi/jsi.h>

using namespace facebook::jsi;

typedef struct BitmapImage {
  uint8_t* _Nonnull data;
  size_t size;
  uint32_t width;
  uint32_t height;
} BitmapImage;

NS_ASSUME_NONNULL_BEGIN

@interface UIImage (Bitmap)

- (BOOL)createBitmapImage:(BitmapImage*)bitmapImage;

@end

NS_ASSUME_NONNULL_END
