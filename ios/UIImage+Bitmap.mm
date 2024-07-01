#import "UIImage+Bitmap.h"
#import "ImageBitmapHostObject.h"

@implementation UIImage (Bitmap)

- (Value)bitmapImageWithRuntime:(Runtime&)runtime {
    CGImageRef imageRef = [self CGImage];
    NSUInteger width = CGImageGetWidth(imageRef);
    NSUInteger height = CGImageGetHeight(imageRef);
    NSUInteger bytesPerPixel = 4;
    NSUInteger bytesPerRow = bytesPerPixel * width;
    NSUInteger bitsPerComponent = 8;

    size_t size = height * width * bytesPerPixel;
    unsigned char *rawData = (unsigned char*) calloc(size, sizeof(unsigned char));
    if (!rawData) {
        return Value::null();
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
        return Value::null();
    }

    CGContextDrawImage(context, CGRectMake(0, 0, width, height), imageRef);
    CGContextRelease(context);

    return Object::createFromHostObject(runtime, std::make_shared<wgpu::ImageBitmapHostObject>(rawData, size, width, height));
}

@end
