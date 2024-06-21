#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#import "webgpu.h"
#import "wgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class DeviceHostObject : public HostObject {
public:
    explicit DeviceHostObject(WGPUDevice value, WGPUContext *context): _value(value), _context(context) { }
    ~DeviceHostObject() { wgpuDeviceRelease(_value); }
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
    WGPUDevice _value;
    WGPUContext *_context;
};

}
