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
#import "ImageBitmapHostObject.h"
#import <boost/format.hpp>
#import <UIKit/UIKit.h>
#import "UIImage+Bitmap.h"
#import "React-Core/React/RCTMessageThread.h"
#import "TimerHostObject.h"
#import "ConstantConversion.h"

using namespace facebook::react;
using namespace facebook::jsi;
using namespace wgpu;

@implementation WGPUJsi

RCT_EXPORT_MODULE(WGPUJsi)

@synthesize bridge;

static void handle_request_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata);

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
    RCTCxxBridge *cxxBridge = (RCTCxxBridge*)self.bridge;
    if (cxxBridge == nil) {
        NSLog(@"Cxx bridge not found");
        return @(NO);
    }
    auto jsMessageThread = std::make_shared<RCTMessageThread>([NSRunLoop currentRunLoop], ^(NSError *error) {
      if (error) {
          NSLog(@"Error running on js thread: %@", error);
      }
    });

    auto &runtime = *(Runtime *)cxxBridge.runtime;
    auto getContext = WGPU_FUNC_FROM_HOST_FUNC("getContext", 1, [jsMessageThread]) {
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
        }, jsMessageThread);

        return Object::createFromHostObject(runtime, std::make_shared<ContextHostObject>(context));
    });

    auto requestAdapter = WGPU_FUNC_FROM_HOST_FUNC(requestAdapter, 1, []) {
        auto descriptor = arguments[0].asObject(runtime);
        auto context = WGPU_HOST_OBJ(descriptor, context, ContextHostObject);
        return makePromise(runtime, context->_context, [](Promise *promise) {
            auto surface = promise->context->_surface;
            auto instance = promise->context->_instance;

            const WGPURequestAdapterOptions adapterOptions = {
                .compatibleSurface = surface,
            };

            wgpuInstanceRequestAdapter(instance, &adapterOptions, handle_request_adapter, promise);
        });
    });

    auto createImageBitmap = WGPU_FUNC_FROM_HOST_FUNC(_createImageBitmap, 1, [jsMessageThread]) {
        // https://developer.mozilla.org/en-US/docs/Web/API/createImageBitmap
        // TODO: Support blob, etc
        // TODO: Support options
        // TODO: Support cropping
        auto uri = arguments[0].asObject(runtime).getProperty(runtime, "uri").asString(runtime).utf8(runtime);

        return makePromise(runtime, nullptr, [uri = std::move(uri), jsMessageThread](Promise *promise) {
            // TODO: Support file urls
            NSURL *url = [NSURL URLWithString:[NSString stringWithCString:uri.data() encoding:NSUTF8StringEncoding]];
            NSURLSession *session = NSURLSession.sharedSession;
            NSURLSessionTask *task = [session dataTaskWithURL:url completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
                auto &runtime = promise->runtime;
                if (error != nil) {
                    auto message = (boost::format("%s %s")
                        % [error.userInfo[NSLocalizedFailureReasonErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]
                        % [error.userInfo[NSLocalizedRecoverySuggestionErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]).str();

                    jsMessageThread->runOnQueue([promise, &runtime, message]() {
                        promise->reject->call(promise->runtime, makeJSError(runtime, message));
                        delete promise;
                    });
                } else if (data == nil) {
                    NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                    auto message = (boost::format("[%li] No response") % httpResponse.statusCode).str();
                    jsMessageThread->runOnQueue([promise, &runtime, message]() {
                        promise->reject->call(promise->runtime, makeJSError(runtime, message));
                        delete promise;
                    });
                } else {
                    BitmapImage bitmapImage = {0};
                    auto result = [[UIImage imageWithData:data] createBitmapImage:&bitmapImage runtime:runtime];

                    if (!result) {
                        jsMessageThread->runOnQueue([promise, &runtime]() {
                            promise->reject->call(promise->runtime, makeJSError(runtime, "Failed to create bitmap"));
                            delete promise;
                        });
                    } else {
                        jsMessageThread->runOnQueue([promise, &runtime, bitmapImage]() {
                            auto obj = Object::createFromHostObject(runtime, std::make_shared<wgpu::ImageBitmapHostObject>(bitmapImage.data, bitmapImage.size, bitmapImage.width, bitmapImage.height));
                            promise->resolve->call(promise->runtime, std::move(obj));
                            delete promise;
                        });
                    }
                }
            }];
            [task resume];
        });
    });

    auto makeTimer = WGPU_FUNC_FROM_HOST_FUNC(makeTimer, 1, []) {
        return Object::createFromHostObject(runtime, std::make_shared<TimerHostObject>(&runtime));
    });

    auto getPreferredCanvasFormat = WGPU_FUNC_FROM_HOST_FUNC(getPreferredCanvasFormat, 1, []) {
        if (count == 0) {
            throw makeJSError(runtime, "You must pass an adapter to getPreferredCanvasFormat for this native implementation");
        }
        auto adapter = arguments[0].asObject(runtime).asHostObject<AdapterHostObject>(runtime);
        auto format = wgpuSurfaceGetPreferredFormat(adapter->_context->_surface, adapter->_value);
        return String::createFromUtf8(runtime, WGPUTextureFormatToString(format));
    });

    auto gpu = Object(runtime);
    gpu.setProperty(runtime, "requestAdapter", std::move(requestAdapter));
    gpu.setProperty(runtime, "getPreferredCanvasFormat", std::move(getPreferredCanvasFormat));

    auto navigator = Object(runtime);
    navigator.setProperty(runtime, "gpu", std::move(gpu));

    auto webgpu = Object(runtime);
    webgpu.setProperty(runtime, "navigator", std::move(navigator));
    webgpu.setProperty(runtime, "getContext", std::move(getContext));
    webgpu.setProperty(runtime, "__createImageBitmap", std::move(createImageBitmap));
    webgpu.setProperty(runtime, "makeTimer", std::move(makeTimer));

    runtime.global().setProperty(runtime, "__webGPUNative", std::move(webgpu));

    return @(YES);
}

@end

static void handle_request_adapter(WGPURequestAdapterStatus status, WGPUAdapter adapter, char const *message, void *userdata) {
    Promise *promise = (Promise *)userdata;
    Runtime &runtime = promise->runtime;
    auto sharedContext = promise->context;

    if (status == WGPURequestAdapterStatus_Success) {
        auto adapterHostObject = Object::createFromHostObject(runtime, std::make_shared<AdapterHostObject>(adapter, sharedContext));
        promise->resolve->call(runtime, std::move(adapterHostObject));
    } else {
        auto error = boost::format("[%s] %#.8x %s") % __FILE_NAME__ % status % message;
        promise->reject->call(runtime, String::createFromUtf8(runtime, error.str()));
    }
    delete promise;
}
