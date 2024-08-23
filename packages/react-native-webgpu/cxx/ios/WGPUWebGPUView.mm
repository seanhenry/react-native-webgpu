#import "WGPUWebGPUView.h"
#import <React/RCTBridgeConstants.h>
#include <memory>
#include "JSIInstance.h"
#include "Surface.h"
#include "webgpu.h"

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
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(onJSReload)
                                               name:RCTBridgeWillReloadNotification
                                             object:nil];
    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(onJSReload)
                                               name:RCTBridgeFastRefreshNotification
                                             object:nil];
  }
  return self;
}

- (void)onJSReload {
  [self deleteSurface];
  if (self.window != nil) {
    [self createSurface];
  }
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

- (void)layoutSubviews {
  [super layoutSubviews];
  if (self.window != nil) {
    [self createSurface];
  }
}

- (BOOL)hasNonZeroSize {
  return CGRectGetWidth(self.metalLayer.frame) > 0 && CGRectGetHeight(self.metalLayer.frame) > 0;
}

- (void)createSurface {
  if (self->wrapper.surface != nullptr || ![self hasNonZeroSize]) {
    return;
  }
  struct WGPUSurfaceDescriptorFromMetalLayer descriptorFromMetalLayer = {
    .chain =
      (const WGPUChainedStruct){
        .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
      },
    .layer = (__bridge void *)self.metalLayer,
  };
  struct WGPUSurfaceDescriptor descriptor = {
    .nextInChain = (const WGPUChainedStruct *)&descriptorFromMetalLayer,
  };

  WGPUInstance instance = wgpuCreateInstance(NULL);

  if (instance == nullptr) {
    self.onCreateSurface(@{@"error" : @"Failed to create wgpu instance"});
    return;
  }

  auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

  if (surface == nullptr) {
    self.onCreateSurface(@{@"error" : @"Failed to create surface"});
    return;
  }

  auto width = CGRectGetWidth(self.metalLayer.frame);
  auto height = CGRectGetHeight(self.metalLayer.frame);
  auto scale = UIScreen.mainScreen.scale;
  SurfaceSize surfaceSize = {
    .pixelWidth = (uint32_t)(width * scale),
    .pixelHeight = (uint32_t)(height * scale),
    .scale = (float)scale,
    .pointWidth = (float)width,
    .pointHeight = (float)height,
  };

  auto managedSurface = std::make_shared<Surface>(instance, surface, surfaceSize);

  self->wrapper.surface = managedSurface;

  __weak WGPUWebGPUView *weakSelf = self;
  JSIInstance::instance->jsThread->run([managedSurface, weakSelf]() {
    WGPUWebGPUView *strongSelf = weakSelf;
    if (strongSelf == nil) {
      return;
    }
    std::string uuidStr = [strongSelf.uuid cStringUsingEncoding:NSUTF8StringEncoding];
    JSIInstance::instance->onCreateSurface(std::move(uuidStr), managedSurface);
    strongSelf.onCreateSurface(@{@"uuid" : strongSelf.uuid});
  });
}

- (void)deleteSurface {
  if (self->wrapper.surface != nullptr) {
    self->wrapper.surface->invalidateTimer();
    self->wrapper.surface = nullptr;
  }
}

// TODO: update surface size
//- (void)layoutSubviews {}

@end
