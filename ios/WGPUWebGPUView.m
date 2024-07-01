#import "WGPUWebGPUView.h"
#import "WGPUMetalLayers.h"

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
        // TODO: add deinit
        self.onInit(@{@"identifier": self.identifier});
    }
}

@end
