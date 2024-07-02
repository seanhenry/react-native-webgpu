#import "WGPUJsi.h"
#import "React/RCTBridge+Private.h"
#import "webgpu.h"
#import "wgpu.h"
#import "WGPUMetalLayers.h"
#import <jsi/jsi.h>
#import <iostream>
#import <stdio.h>
#import "WGPUJsiUtils.h"
#import "AdapterHostObject.h"
#import <boost/format.hpp>
#import <UIKit/UIKit.h>
#import "UIImage+Bitmap.h"

using namespace facebook::jsi;
using namespace wgpu;

@implementation WGPUJsi

RCT_EXPORT_MODULE(WGPUJsi)

@synthesize bridge;

static void handle_request_adapter(WGPURequestAdapterStatus status,
                                   WGPUAdapter adapter,
                                   char const *message,
                                   void *userdata) {
    Promise *promise = (Promise *)userdata;
    Runtime &runtime = promise->runtime;
    auto sharedContext = (WGPUContext *)promise->userData;

    if (status == WGPURequestAdapterStatus_Success) {
        auto adapterHostObject = Object::createFromHostObject(runtime, std::make_shared<AdapterHostObject>(adapter, sharedContext));
        promise->resolve->call(runtime, std::move(adapterHostObject));
    } else {
        auto error = boost::format("[%s] %#.8x %s") % __FILE_NAME__ % status % message;
        promise->reject->call(runtime, String::createFromUtf8(runtime, error.str()));
    }
    delete promise;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
    RCTCxxBridge *cxxBridge = (RCTCxxBridge*)self.bridge;
    if (cxxBridge == nil) {
        NSLog(@"Cxx bridge not found");
        return @(NO);
    }

    auto &runtime = *(Runtime *)cxxBridge.runtime;
    auto getContext = WGPU_FUNC_FROM_HOST_FUNC("getContext", 1, []) {
        NSString *identifier = @"main";
        if (count > 0) {
            auto obj = arguments[0].asObject(runtime);
            if (obj.hasProperty(runtime, "identifier") && obj.getProperty(runtime, "identifier").isString()) {
                auto identifierStr = obj.getProperty(runtime, "identifier").asString(runtime).utf8(runtime);
                identifier = [NSString stringWithCString:identifierStr.data() encoding:NSUTF8StringEncoding];
            }
        }
        CAMetalLayer *metalLayer = [[WGPUMetalLayers instance].layers objectForKey:identifier];
        if (metalLayer == nil) {
            NSString *message = [NSString stringWithFormat:@"CAMetalLayer is nil for identifier %@", identifier];
            throw JSError(runtime, [message cStringUsingEncoding:NSUTF8StringEncoding]);
        }

        auto instance = wgpuCreateInstance(NULL);

        if (instance == nullptr) {
            throw JSError(runtime, "Failed to create WGPU instance");
        }

        struct WGPUSurfaceDescriptorFromMetalLayer descriptorFromMetalLayer = {
            .chain = (const WGPUChainedStruct){
                .sType = WGPUSType_SurfaceDescriptorFromMetalLayer,
            },
                .layer = (__bridge void *)metalLayer,
        };
        struct WGPUSurfaceDescriptor descriptor = {
            .nextInChain = (const WGPUChainedStruct *)&descriptorFromMetalLayer,
        };
        auto surface = wgpuInstanceCreateSurface(instance, &descriptor);

        if (surface == nullptr) {
            throw JSError(runtime, "Failed to create WGPU surface");
        }

        __weak CAMetalLayer *weakMetalLayer = metalLayer;
        auto context = new WGPUContext(instance, surface, [weakMetalLayer]() {
            auto layer = weakMetalLayer;
            if (layer != nil) {
                return (uint32_t)layer.frame.size.width;
            }
            return (uint32_t)0;
        }, [weakMetalLayer]() {
            auto layer = weakMetalLayer;
            if (layer != nil) {
                return (uint32_t)layer.frame.size.height;
            }
            return (uint32_t)0;
        });

        return Object::createFromHostObject(runtime, std::make_shared<ContextHostObject>(context));
    });

    auto requestAdapter = WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, []) {
        auto descriptor = arguments[0].asObject(runtime);
        auto context = WGPU_HOST_OBJ(descriptor, context, ContextHostObject);
        auto sharedContext = context->_context;
        return makePromise(runtime, [sharedContext](Promise *promise) {
            auto surface = sharedContext->_surface;
            auto instance = sharedContext->_instance;

            const WGPURequestAdapterOptions adapterOptions = {
                .compatibleSurface = surface,
            };

            promise->userData = (void *)sharedContext;
            wgpuInstanceRequestAdapter(instance, &adapterOptions, handle_request_adapter, promise);
        });
    });

    auto createImageBitmap = WGPU_FUNC_FROM_HOST_FUNC(_createImageBitmap, 1, []) {
        // https://developer.mozilla.org/en-US/docs/Web/API/createImageBitmap
        // TODO: Support blob, etc
        // TODO: Support options
        // TODO: Support cropping
        auto uri = arguments[0].asObject(runtime).getProperty(runtime, "uri").asString(runtime).utf8(runtime);

        return makePromise(runtime, [uri = std::move(uri)](Promise *promise){
            // TODO: Support file urls
            NSURL *url = [NSURL URLWithString:[NSString stringWithCString:uri.data() encoding:NSUTF8StringEncoding]];
            NSURLSession *session = NSURLSession.sharedSession;
            NSURLSessionTask *task = [session dataTaskWithURL:url completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
                auto &runtime = promise->runtime;
                if (error != nil) {
                    auto message = (boost::format("%s %s")
                        % [error.userInfo[NSLocalizedFailureReasonErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]
                        % [error.userInfo[NSLocalizedRecoverySuggestionErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]).str();
                    promise->reject->call(promise->runtime, makeJSError(runtime, message));
                } else if (data == nil) {
                    NSHTTPURLResponse *r = (NSHTTPURLResponse *)response;
                    auto message = (boost::format("[%li] No response") % r.statusCode).str();
                    promise->reject->call(promise->runtime, makeJSError(runtime, message));
                } else {
                    Value bitmap = [[UIImage imageWithData:data] bitmapImageWithRuntime:runtime];
                    if (bitmap.isNull()) {
                        promise->reject->call(promise->runtime, makeJSError(runtime, "Failed to create bitmap"));
                    } else {
                        promise->resolve->call(promise->runtime, std::move(bitmap));
                    }
                }
                delete promise;
            }];
            [task resume];
        });
    });

    auto gpu = Object(runtime);
    gpu.setProperty(runtime, "requestAdapter", std::move(requestAdapter));

    auto navigator = Object(runtime);
    navigator.setProperty(runtime, "gpu", std::move(gpu));

    auto webgpu = Object(runtime);
    webgpu.setProperty(runtime, "navigator", std::move(navigator));
    webgpu.setProperty(runtime, "getContext", std::move(getContext));
    webgpu.setProperty(runtime, "__createImageBitmap", std::move(createImageBitmap));

    runtime.global().setProperty(runtime, "__webGPUNative", std::move(webgpu));

    return @(YES);
}

@end



