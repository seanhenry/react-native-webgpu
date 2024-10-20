#import "WGPUWebGPUView.h"
#import <React/RCTBridgeConstants.h>
#include <memory>
#include "Surface.h"
#include "SurfacesManager.h"
#include "webgpu.h"

#ifdef RCT_NEW_ARCH_ENABLED
#import "generated/WebgpuSpec/ComponentDescriptors.h"
#import "generated/WebgpuSpec/EventEmitters.h"
#import "generated/WebgpuSpec/Props.h"
#import "generated/WebgpuSpec/RCTComponentViewHelpers.h"
#endif

using namespace wgpu;
using namespace facebook::react;

@interface WGPUWebGPUView ()

@property(nonatomic, readonly) std::string uuidCxxString;
@property(nonatomic, readonly, nullable) CAMetalLayer *metalLayer;

@end

@implementation WGPUWebGPUView {
  std::shared_ptr<Surface> _surface;
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
#ifdef RCT_NEW_ARCH_ENABLED
    static const auto defaultProps = std::make_shared<const WGPUWebGPUViewProps>();
    _props = defaultProps;
    self.contentView = [[UIView alloc] init];
#endif
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
  return CGRectGetWidth(self.layer.frame) > 0 && CGRectGetHeight(self.layer.frame) > 0;
}

- (void)createSurface {
  if (self->_surface != nullptr || ![self hasNonZeroSize]) {
    return;
  }
  _metalLayer = [CAMetalLayer layer];
  self.metalLayer.frame = self.layer.bounds;

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
    _metalLayer = nil;
    [self callOnCreateSurfaceWithError:@"Failed to create wgpu instance"];
    return;
  }

  auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

  if (surface == nullptr) {
    _metalLayer = nil;
    [self callOnCreateSurfaceWithError:@"Failed to create surface"];
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
  [self.layer addSublayer:self.metalLayer];
  [self callOnCreateSurfaceWithError:nil];
}

- (void)deleteSurface {
  if (self->_surface != nullptr) {
    auto uuidStr = self.uuidCxxString;
    SurfacesManager::getInstance()->remove(uuidStr);
    self->_surface->invalidateTimer();
    self->_surface = nullptr;
  }
  if (self.metalLayer != nil) {
    [self.metalLayer removeFromSuperlayer];
    _metalLayer = nil;
  }
}

- (std::string)uuidCxxString {
  return [self.uuid cStringUsingEncoding:NSUTF8StringEncoding];
}

- (void)layoutSublayersOfLayer:(CALayer *)layer {
  [super layoutSublayersOfLayer:layer];
  if (self.metalLayer != nil) {
    self.metalLayer.frame = layer.bounds;
  }
}

// TODO: update surface size
//- (void)layoutSubviews {}

#ifdef RCT_NEW_ARCH_ENABLED

+ (ComponentDescriptorProvider)componentDescriptorProvider {
  return concreteComponentDescriptorProvider<WGPUWebGPUViewComponentDescriptor>();
}

- (void)updateProps:(Props::Shared const &)props oldProps:(Props::Shared const &)oldProps {
  //  const auto &oldViewProps = *std::static_pointer_cast<WGPUWebGPUViewProps const>(_props);
  //  const auto &newViewProps = *std::static_pointer_cast<WGPUWebGPUViewProps const>(props);

  [super updateProps:props oldProps:oldProps];
}

- (void)callOnCreateSurfaceWithError:(NSString *)error {
  if (self->_eventEmitter == nullptr) {
    return;
  }

  assert(std::dynamic_pointer_cast<WGPUWebGPUViewEventEmitter const>(self->_eventEmitter));
  auto emitter = std::static_pointer_cast<WGPUWebGPUViewEventEmitter const>(self->_eventEmitter);
  if (error != nil) {
    emitter->onCreateSurface({
      .error = error.UTF8String,
    });
  } else {
    emitter->onCreateSurface({
      .uuid = self.uuidCxxString,
    });
  }
}

#else

- (void)callOnCreateSurfaceWithError:(NSString *)error {
  if (self.onCreateSurface != nil) {
    if (error != nil) {
      self.onCreateSurface(@{@"error" : error});
    } else {
      self.onCreateSurface(@{@"uuid" : self.uuid});
    }
  }
}

#endif

@end

#ifdef RCT_NEW_ARCH_ENABLED
extern "C" {
Class<RCTComponentViewProtocol> WGPUWebGPUViewCls(void) { return WGPUWebGPUView.class; }
}
#endif
