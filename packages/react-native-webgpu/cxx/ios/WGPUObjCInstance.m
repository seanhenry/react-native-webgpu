#import "WGPUObjCInstance.h"
#include "WGPULog.h"

@implementation WGPUObjCInstance

+ (WGPUObjCInstance *)shared {
  static WGPUObjCInstance *instance;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    instance = [[WGPUObjCInstance alloc] init];
  });
  return instance;
}

- (void)loadModules:(RCTModuleRegistry *)moduleRegistry {
  self.blobManager = [moduleRegistry moduleForName:"BlobModule"];
  if (self.blobManager == nil) {
    WGPU_LOG_ERROR("Missing BlobModule. Blobs will not be supported.");
  }
}

@end
