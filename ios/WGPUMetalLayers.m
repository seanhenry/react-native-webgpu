#import "WGPUMetalLayers.h"

@implementation WGPUMetalLayers

+ (instancetype)instance {
    static WGPUMetalLayers *instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[WGPUMetalLayers alloc] init];
    });
    return instance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _layers = [NSMapTable mapTableWithKeyOptions:NSMapTableStrongMemory valueOptions:NSMapTableWeakMemory];
    }
    return self;
}

@end
