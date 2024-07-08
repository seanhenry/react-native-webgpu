#include "RenderPassEncoderHostObject.h"
#include "RenderPipelineHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "BindGroupHostObject.h"
#include "BufferHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderPassEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "setPipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(setPipeline, 1, [this]) {
            auto pipeline = arguments[0].asObject(runtime).asHostObject<RenderPipelineHostObject>(runtime);
            wgpuRenderPassEncoderSetPipeline(this->_value, pipeline->_value);
            return Value(123);
        });
    }

    if (name == "draw") {
        return WGPU_FUNC_FROM_HOST_FUNC(draw, 4, [this]) {
            auto vertexCount = (uint32_t)arguments[0].asNumber();
            auto instanceCount = (uint32_t) count > 1 ? arguments[1].asNumber() : 1;
            auto firstVertex = (uint32_t) count > 2 ? arguments[2].asNumber() : 0;
            auto firstInstance = (uint32_t) count > 3 ? arguments[3].asNumber() : 0;
            wgpuRenderPassEncoderDraw(this->_value, vertexCount, instanceCount, firstVertex, firstInstance);
            return Value::undefined();
        });
    }

    if (name == "end") {
        return WGPU_FUNC_FROM_HOST_FUNC(end, 0, [this]) {
            wgpuRenderPassEncoderEnd(this->_value);
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
                bindGroup = groupIn->_value;
            }
            wgpuRenderPassEncoderSetBindGroup(_value, index, bindGroup, 0, NULL);
            return Value::undefined();
        });
    }

    if (name == "setVertexBuffer") {
        return WGPU_FUNC_FROM_HOST_FUNC(setVertexBuffer, 4, [this]) {
            auto slot = (uint32_t)arguments[0].asNumber();
            WGPUBuffer buffer = NULL;
            if (arguments[1].isObject()) {
                auto bufferIn = arguments[1].asObject(runtime).asHostObject<BufferHostObject>(runtime);
                buffer = bufferIn->_value;
            }
            auto offset = count > 2 ? (size_t)arguments[2].asNumber() : 0;
            auto size = count > 3 ? (size_t)arguments[3].asNumber() :
                buffer != NULL ? wgpuBufferGetSize(buffer) - offset : 0;
            wgpuRenderPassEncoderSetVertexBuffer(_value, slot, buffer, offset, size);
            return Value::undefined();
        });
    }

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> RenderPassEncoderHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "setPipeline", "draw", "end", "setBindGroup", "setVertexBuffer", "label");
}
