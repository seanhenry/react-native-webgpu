#include "CreateImageBitmap.h"
#import <Foundation/Foundation.h>
#import <React/RCTBlobManager.h>
#include <boost/format.hpp>
#include "ImageBitmapHostObject.h"
#include "JSIInstance.h"
#include "Promise.h"
#import "UIImage+Bitmap.h"
#import "WGPUObjCInstance.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

Value fetchImageBitmap(Runtime &runtime, std::string uri);
Value getImageBitmapFromBlob(Runtime &runtime, Object obj);
void makeBitmap(NSData *data, Promise<void *> *promise);

}  // namespace wgpu

Function wgpu::createImageBitmap(Runtime &runtime) {
  return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, []) {
    // https://developer.mozilla.org/en-US/docs/Web/API/createImageBitmap
    // TODO: Support other types, options, cropping
    auto obj = arguments[0].asObject(runtime);
    if (obj.hasProperty(runtime, "uri")) {
      auto uri = obj.getProperty(runtime, "uri").asString(runtime).utf8(runtime);
      return fetchImageBitmap(runtime, std::move(uri));
    } else if (obj.hasProperty(runtime, "_data")) {
      return getImageBitmapFromBlob(runtime, std::move(obj));
    }
    auto promise = new Promise<void *>(runtime);
    return promise->jsPromise([promise]() {
      promise->reject(makeJSError(promise->runtime, "Unsupported object passed to createImageBitmap"));
      delete promise;
    });
  });
}

Value wgpu::fetchImageBitmap(Runtime &runtime, std::string uri) {
  auto promise = new Promise<void *>(runtime);
  return promise->jsPromise([promise, uri = std::move(uri)]() {
    // TODO: Support file urls
    NSURL *url = [NSURL URLWithString:[NSString stringWithCString:uri.data() encoding:NSUTF8StringEncoding]];
    NSURLSession *session = NSURLSession.sharedSession;
    NSURLSessionTask *task =
      [session dataTaskWithURL:url
             completionHandler:^(NSData *_Nullable data, NSURLResponse *_Nullable response, NSError *_Nullable error) {
               if (error != nil) {
                 auto message =
                   (boost::format("%s %s") %
                    [error.userInfo[NSLocalizedFailureReasonErrorKey] cStringUsingEncoding:NSUTF8StringEncoding] %
                    [error.userInfo[NSLocalizedRecoverySuggestionErrorKey] cStringUsingEncoding:NSUTF8StringEncoding])
                     .str();

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
                 makeBitmap(data, promise);
               }
             }];
    [task resume];
  });
}

Value wgpu::getImageBitmapFromBlob(Runtime &runtime, Object obj) {
  auto jsDataPtr = std::make_shared<Object>(obj.getPropertyAsObject(runtime, "_data"));
  auto promise = new Promise<void *>(runtime);
  return promise->jsPromise([promise, jsDataPtr]() {
    auto jsData = std::move(*jsDataPtr.get());
    auto &runtime = promise->runtime;
    RCTBlobManager *blobManager = [WGPUObjCInstance shared].blobManager;
    if (blobManager == nil) {
      NSString *message = [NSString stringWithFormat:@"%s:%i Blob manager was not found", __FILE__, __LINE__];
      promise->reject(makeJSError(promise->runtime, [message cStringUsingEncoding:NSUTF8StringEncoding]));
      delete promise;
    }
    auto blobId = WGPU_UTF8(jsData, blobId);
    auto data = [blobManager resolve:[NSString stringWithCString:blobId.data() encoding:NSUTF8StringEncoding]
                              offset:WGPU_NUMBER_OPT(jsData, offset, NSInteger, 0)
                                size:WGPU_NUMBER_OPT(jsData, size, NSInteger, -1)];
    makeBitmap(data, promise);
  });
}

void wgpu::makeBitmap(NSData *data, Promise<void *> *promise) {
  BitmapImage bitmapImage = {nullptr};
  auto result = [[UIImage imageWithData:data] createBitmapImage:&bitmapImage runtime:promise->runtime];

  if (!result) {
    JSIInstance::instance->jsThread->run([promise]() {
      promise->reject(makeJSError(promise->runtime, "Failed to create bitmap"));
      delete promise;
    });
  } else {
    JSIInstance::instance->jsThread->run([promise, bitmapImage]() {
      auto obj = Object::createFromHostObject(
        promise->runtime, std::make_shared<wgpu::ImageBitmapHostObject>(bitmapImage.data, bitmapImage.size,
                                                                        bitmapImage.width, bitmapImage.height));
      promise->resolve(std::move(obj));
      delete promise;
    });
  }
}
