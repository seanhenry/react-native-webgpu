#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface WGPUMetalLayers : NSObject

+ (instancetype)instance;

@property (nonatomic) NSMapTable<NSString *, CAMetalLayer *> *layers;

@end

NS_ASSUME_NONNULL_END
