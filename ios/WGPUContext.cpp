#import "WGPUContext.h"
#import "wgpu.h"

bool wgpu::WGPUContext::poll(bool wait) {
    return wgpuDevicePoll(_device->_device, (WGPUBool)wait, NULL);
}
