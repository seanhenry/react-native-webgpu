#pragma once

#include <jsi/jsi.h>
#include "WGPUContext.h"
#include "wgpu.h"

using namespace facebook::jsi;

@class WGPUTimer;

namespace wgpu {

class TimerHostObject : public HostObject {
public:
    explicit TimerHostObject(Runtime *runtime);
    ~TimerHostObject();
    std::vector<PropNameID> getPropertyNames(Runtime& runtime) override;
    Value get(Runtime &runtime, const PropNameID &name) override;
private:
    __strong WGPUTimer *_timer;
};

}
