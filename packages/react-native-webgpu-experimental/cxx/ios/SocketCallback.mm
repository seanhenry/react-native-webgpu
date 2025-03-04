#import "SocketCallback.h"
#import <CFNetwork/CFNetwork.h>
#import "WGPUJsiUtils.h"

using namespace facebook::jsi;

#define HOST "localhost"
#define PORT 8888

namespace wgpu {

Value socketCallback(Runtime &runtime) {
  return WGPU_FUNC_FROM_HOST_FUNC(socketCallback, 1, []) {
    auto exampleName = arguments[0].asString(runtime).utf8(runtime);
    static dispatch_queue_t queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      queue = dispatch_queue_create("com.example.socketCallback", DISPATCH_QUEUE_SERIAL);
    });
    dispatch_async(queue, ^{
      CFWriteStreamRef writeStream;
      CFDataRef messageData = CFDataCreate(NULL, (UInt8 *)exampleName.c_str(), exampleName.size());
      CFStringRef hostString = CFStringCreateWithCString(NULL, HOST, kCFStringEncodingUTF8);
      CFHostRef host = CFHostCreateWithName(NULL, hostString);
      CFStreamCreatePairWithSocketToCFHost(NULL, host, PORT, NULL, &writeStream);
      if (CFWriteStreamOpen(writeStream)) {
        CFWriteStreamWrite(writeStream, CFDataGetBytePtr(messageData), CFDataGetLength(messageData));
        CFWriteStreamClose(writeStream);
      }
      CFRelease(writeStream);
      CFRelease(messageData);
      CFRelease(host);
      CFRelease(hostString);
    });
    return Value::undefined();
  });
}
}  // namespace wgpu
