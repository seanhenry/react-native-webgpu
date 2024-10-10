#import "WGPUWebGPUView.h"
#import <React/RCTBridgeConstants.h>
#include <memory>
#include "Surface.h"
#include "SurfacesManager.h"
#include "webgpu.h"

using namespace wgpu;

@interface WGPUWebGPUView ()

@property(nonatomic, readonly) std::string uuidCxxString;

@end

@implementation WGPUWebGPUView {
  std::shared_ptr<Surface> _surface;
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
  if (self->_surface != nullptr || ![self hasNonZeroSize]) {
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

  self->_surface = managedSurface;

  auto uuidStr = self.uuidCxxString;
  SurfacesManager::getInstance()->set(uuidStr, managedSurface);
  self.onCreateSurface(@{@"uuid" : self.uuid});
}

- (void)deleteSurface {
  if (self->_surface != nullptr) {
    auto uuidStr = self.uuidCxxString;
    SurfacesManager::getInstance()->remove(uuidStr);
    self->_surface->invalidateTimer();
    self->_surface = nullptr;
  }
}

- (std::string)uuidCxxString {
  return [self.uuid cStringUsingEncoding:NSUTF8StringEncoding];
}

// TODO: update surface size
//- (void)layoutSubviews {}

@end
