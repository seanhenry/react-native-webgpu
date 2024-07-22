#include "ComputePassEncoderHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "BindGroupHostObject.h"
#include "ComputePipelineHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value ComputePassEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "setPipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(setPipeline, 1, [this]) {
            auto pipeline = arguments[0].asObject(runtime).asHostObject<ComputePipelineHostObject>(runtime);
            wgpuComputePassEncoderSetPipeline(_value, pipeline->getValue());
            return Value::undefined();
        });
    }

    if (name == "setBindGroup") {
        // TODO: see overloaded version with 5 args
        return WGPU_FUNC_FROM_HOST_FUNC(setBindGroup, 5, [this]) {
            auto index = (uint32_t)arguments[0].asNumber();
            WGPUBindGroup bindGroup = NULL;
            if (arguments[1].isObject()) {
                auto groupIn = arguments[1].asObject(runtime).asHostObject<BindGroupHostObject>(runtime);
                bindGroup = groupIn->getValue();
            }
            wgpuComputePassEncoderSetBindGroup(_value, index, bindGroup, 0, NULL);
            return Value::undefined();
        });
    }

    if (name == "dispatchWorkgroups") {
        return WGPU_FUNC_FROM_HOST_FUNC(dispatchWorkgroups, 3, [this]) {
            uint32_t x = (uint32_t)arguments[0].asNumber();
            uint32_t y = count > 1 ? (uint32_t)arguments[1].asNumber() : 1;
            uint32_t z = count > 2 ? (uint32_t)arguments[2].asNumber() : 1;
            wgpuComputePassEncoderDispatchWorkgroups(_value, x, y, z);
            return Value::undefined();
        });
    }

    if (name == "end") {
        return WGPU_FUNC_FROM_HOST_FUNC(end, 0, [this]) {
            wgpuComputePassEncoderEnd(_value);
            return Value::undefined();
        });
    }

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> ComputePassEncoderHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "setPipeline", "setBindGroup", "dispatchWorkgroups", "end", "label");
}
