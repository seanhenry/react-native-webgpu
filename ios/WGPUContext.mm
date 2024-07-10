#import "WGPUContext.h"
#import "React-Core/React/RCTMessageThread.h"

using namespace facebook::react;

void wgpu::WGPUContext::runOnJsThread(std::function<void()>&& fn) {
    if (_jsThread != nullptr) {
        _jsThread->runOnQueue(std::move(fn));
    }
}
