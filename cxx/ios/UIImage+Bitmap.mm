#import "UIImage+Bitmap.h"
#import "ImageBitmapHostObject.h"
#import <tuple>

@implementation UIImage (Bitmap)

- (BOOL)createBitmapImage:(BitmapImage *)bitmapImage runtime:(Runtime&)runtime {
    CGImageRef imageRef = [self CGImage];
    NSUInteger width = CGImageGetWidth(imageRef);
    NSUInteger height = CGImageGetHeight(imageRef);
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;

    size_t size = height * width * bytesPerPixel;
    unsigned char *rawData = (unsigned char*) calloc(size, sizeof(unsigned char));
    if (!rawData) {
        return NO;
    }

    CGContextRef context = CGBitmapContextCreate(
        rawData,
        width,
        height,
        bitsPerComponent,
        bytesPerRow,
        CGImageGetColorSpace(imageRef),
        kCGImageAlphaPremultipliedLast | kCGImageByteOrder32Big
    );
    if (!context) {
        free(rawData);
        return NO;
    }

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(context);

    bitmapImage->data = rawData;
    bitmapImage->size = size;
    bitmapImage->width = width;
    bitmapImage->height = height;

    return YES;
}

@end
