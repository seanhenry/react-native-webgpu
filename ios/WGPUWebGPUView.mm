#import "WGPUWebGPUView.h"
#include "webgpu.h"
#include "JSIInstance.h"
#include "Surface.h"
#include <memory>

using namespace wgpu;

typedef struct SurfaceObjCWrapper {
    std::shared_ptr<Surface> surface;
} SurfaceObjCWrapper;

@implementation WGPUWebGPUView {
    SurfaceObjCWrapper wrapper;
}

+ (Class)layerClass {
    return [CAMetalLayer self];
}

- (CAMetalLayer *)metalLayer {
    return (CAMetalLayer *)self.layer;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self != nil) {
        _uuid = [[NSUUID UUID] UUIDString];
    }
    return self;
}

- (void)willMoveToWindow:(UIWindow *)newWindow {
    if (newWindow != nil) {
        [self deleteSurface];
        [self createSurface];
    }
}

- (void)didMoveToWindow {
    if (self.window == nil) {
        [self deleteSurface];
    }
}

- (void)createSurface {
    struct WGPUSurfaceDescriptorFromMetalLayer descriptorFromMetalLayer = {
        .chain = (const WGPUChainedStruct){
            .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
        },
            .layer = (__bridge void *)self.metalLayer,
    };
    struct WGPUSurfaceDescriptor descriptor = {
        .nextInChain = (const WGPUChainedStruct *)&descriptorFromMetalLayer,
    };

    WGPUInstance instance = wgpuCreateInstance(NULL);

    if (instance == nullptr) {
        self.onCreateSurface(@{ @"error": @"Failed to create wgpu instance" });
        return;
    }

    auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

    if (surface == nullptr) {
        self.onCreateSurface(@{ @"error": @"Failed to create surface" });
        return;
    }

    auto width = (uint32_t)CGRectGetWidth(self.metalLayer.frame);
    auto height = (uint32_t)CGRectGetHeight(self.metalLayer.frame);
    auto managedSurface = std::make_shared<Surface>(instance, surface, width, height);

    self->wrapper.surface = managedSurface;

    __weak WGPUWebGPUView *weakSelf = self;
    JSIInstance::instance->jsThread->run([managedSurface, weakSelf]() {
        WGPUWebGPUView *strongSelf = weakSelf;
        if (strongSelf == nil) {
            return;
        }
        std::string uuidStr = [strongSelf.uuid cStringUsingEncoding:NSUTF8StringEncoding];
        JSIInstance::instance->onCreateSurface(std::move(uuidStr), managedSurface);
        managedSurface->createTimer();
        strongSelf.onCreateSurface(@{ @"uuid": strongSelf.uuid });
    });
}

- (void)deleteSurface {
    if (self->wrapper.surface != nullptr) {
        self->wrapper.surface->invalidateTimer();
        self->wrapper.surface = nullptr;
    }
}

// TODO: update surface width/height
//- (void)layoutSubviews {}

@end
