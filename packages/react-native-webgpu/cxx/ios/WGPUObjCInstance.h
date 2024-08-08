#import <Foundation/Foundation.h>
#import <React-Core/React/RCTBlobManager.h>
#import <React-Core/React/RCTBridgeModule.h>

NS_ASSUME_NONNULL_BEGIN

@interface WGPUObjCInstance : NSObject

+ (WGPUObjCInstance *)shared;

@property(nonatomic, weak) RCTBlobManager *blobManager;

- (void)loadModules:(RCTModuleRegistry *)moduleRegistry;

@end

NS_ASSUME_NONNULL_END
