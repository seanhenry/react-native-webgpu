#import "CreateImageBitmap.h"
#import <Foundation/Foundation.h>
#import <React/RCTBlobManager.h>
#import <sstream>
#import "ImageBitmapHostObject.h"
#import "UIImage+Bitmap.h"
#import "WGPUJsiUtils.h"
#import "WGPUObjCInstance.h"
#import "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

Value fetchImageBitmap(Runtime &runtime, std::string uri, std::shared_ptr<JSIInstance> jsiInstance);
Value getImageBitmapFromBlob(Runtime &runtime, Object obj, std::shared_ptr<JSIInstance> jsiInstance);
void makeBitmap(NSData *data, std::shared_ptr<Promise> promise);

}  // namespace wgpu

Function wgpu::createImageBitmap(Runtime &runtime, std::shared_ptr<JSIInstance> jsiInstance) {
  return WGPU_FUNC_FROM_HOST_FUNC(createImageBitmap, 1, [jsiInstance]) {
    // https://developer.mozilla.org/en-US/docs/Web/API/createImageBitmap
    // TODO: Support other types, options, cropping
    auto obj = arguments[0].asObject(runtime);
    if (obj.hasProperty(runtime, "uri")) {
      auto uri = obj.getProperty(runtime, "uri").asString(runtime).utf8(runtime);
      return fetchImageBitmap(runtime, std::move(uri), jsiInstance);
    } else if (obj.hasProperty(runtime, "_data")) {
      return getImageBitmapFromBlob(runtime, std::move(obj), jsiInstance);
    }
    return makeRejectedJSPromise(runtime, makeJSError(runtime, "Unsupported object passed to createImageBitmap"));
  });
}

Value wgpu::fetchImageBitmap(Runtime &runtime, std::string uri, std::shared_ptr<JSIInstance> jsiInstance) {
  return Promise::makeJSPromise(jsiInstance, [uri = std::move(uri)](auto &runtime, auto promise) {
    // TODO: Support file urls
    NSURL *url = [NSURL URLWithString:[NSString stringWithCString:uri.data() encoding:NSUTF8StringEncoding]];
    NSURLSession *session = NSURLSession.sharedSession;
    NSURLSessionTask *task = [session
        dataTaskWithURL:url
      completionHandler:^(NSData *_Nullable data, NSURLResponse *_Nullable response, NSError *_Nullable error) {
        if (error != nil) {
          std::ostringstream ss;
          ss << [error.userInfo[NSLocalizedFailureReasonErrorKey] cStringUsingEncoding:NSUTF8StringEncoding] << " "
             << [error.userInfo[NSLocalizedRecoverySuggestionErrorKey] cStringUsingEncoding:NSUTF8StringEncoding];
          promise->reject([message = ss.str()](auto &runtime) { return makeJSError(runtime, message); });
        } else if (data == nil) {
          NSHTTPURLResponse *httpResponse = (NSHTTPURLResponse *)response;

          std::ostringstream ss;
          ss << "[" << httpResponse.statusCode << "] No response";
          promise->reject([message = ss.str()](auto &runtime) { return makeJSError(runtime, message); });
        } else {
          makeBitmap(data, promise);
        }
      }];
    [task resume];
  });
}

Value wgpu::getImageBitmapFromBlob(Runtime &runtime, Object obj, std::shared_ptr<JSIInstance> jsiInstance) {
  auto jsDataPtr = std::make_shared<Object>(obj.getPropertyAsObject(runtime, "_data"));
  return Promise::makeJSPromise(jsiInstance, [jsDataPtr](auto &runtime, auto &promise) {
    auto jsData = std::move(*jsDataPtr.get());
    RCTBlobManager *blobManager = [WGPUObjCInstance shared].blobManager;
    if (blobManager == nil) {
      NSString *message = [NSString stringWithFormat:@"%s:%i Blob manager was not found", __FILE__, __LINE__];
      promise->reject(
        [message](auto &runtime) { return makeJSError(runtime, [message cStringUsingEncoding:NSUTF8StringEncoding]); });
    }
    auto blobId = WGPU_UTF8(jsData, blobId);
    auto data = [blobManager resolve:[NSString stringWithCString:blobId.data() encoding:NSUTF8StringEncoding]
                              offset:WGPU_NUMBER_OPT(jsData, offset, NSInteger, 0)
                                size:WGPU_NUMBER_OPT(jsData, size, NSInteger, -1)];
    makeBitmap(data, promise);
  });
}

void wgpu::makeBitmap(NSData *data, std::shared_ptr<Promise> promise) {
  BitmapImage bitmapImage = {nullptr};
  auto result = [[UIImage imageWithData:data] createBitmapImage:&bitmapImage];

  if (!result) {
    promise->reject([](auto &runtime) { return makeJSError(runtime, "Failed to create bitmap"); });
  } else {
    promise->resolve([bitmapImage = std::move(bitmapImage)](auto &runtime) {
      auto bitmap = std::make_shared<wgpu::ImageBitmapHostObject>(bitmapImage.data, bitmapImage.size, bitmapImage.width,
                                                                  bitmapImage.height);
      return Object::createFromHostObject(runtime, std::move(bitmap));
    });
  }
}
