#include "CreateImageBitmap.h"
#include "WGPUJsiUtils.h"
#include "JSIInstance.h"
#include "ImageBitmapHostObject.h"
#include <boost/format.hpp>
#import <Foundation/Foundation.h>
#import "UIImage+Bitmap.h"

using namespace facebook::jsi;
using namespace wgpu;

Function wgpu::createImageBitmap(Runtime &runtime) {
    return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, []) {
        // https://developer.mozilla.org/en-US/docs/Web/API/createImageBitmap
        // TODO: Support blob, etc
        // TODO: Support options
        // TODO: Support cropping
        auto uri = arguments[0].asObject(runtime).getProperty(runtime, "uri").asString(runtime).utf8(runtime);

        auto promise = new Promise<void *>(runtime);
        return promise->jsPromise([promise, uri = std::move(uri)]() {
            // TODO: Support file urls
            NSURL *url = [NSURL URLWithString:[NSString stringWithCString:uri.data() encoding:NSUTF8StringEncoding]];
            NSURLSession *session = NSURLSession.sharedSession;
            NSURLSessionTask *task = [session dataTaskWithURL:url completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
                if (error != nil) {
                    auto message = (boost::format("%s %s")
                        % [error.userInfo[NSLocalizedFailureReasonErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]
                        % [error.userInfo[NSLocalizedRecoverySuggestionErrorKey] cStringUsingEncoding:NSUTF8StringEncoding]).str();

                    JSIInstance::instance->jsThread->run([promise, message]() {
                        promise->reject(makeJSError(promise->runtime, message));
                        delete promise;
                    });
                } else if (data == nil) {
                    NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;
                    auto message = (boost::format("[%li] No response") % httpResponse.statusCode).str();
                    JSIInstance::instance->jsThread->run([promise, message]() {
                        promise->reject(makeJSError(promise->runtime, message));
                        delete promise;
                    });
                } else {
                    BitmapImage bitmapImage = {0};
                    auto result = [[UIImage imageWithData:data] createBitmapImage:&bitmapImage runtime:promise->runtime];

                    if (!result) {
                        JSIInstance::instance->jsThread->run([promise]() {
                            promise->reject(makeJSError(promise->runtime, "Failed to create bitmap"));
                            delete promise;
                        });
                    } else {
                        JSIInstance::instance->jsThread->run([promise, bitmapImage]() {
                            auto obj = Object::createFromHostObject(promise->runtime, std::make_shared<wgpu::ImageBitmapHostObject>(bitmapImage.data, bitmapImage.size, bitmapImage.width, bitmapImage.height));
                            promise->resolve(std::move(obj));
                            delete promise;
                        });
                    }
                }
            }];
            [task resume];
        });
    });
}
