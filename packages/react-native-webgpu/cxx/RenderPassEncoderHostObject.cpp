#include "RenderPassEncoderHostObject.h"

#include "BindGroupHostObject.h"
#include "BufferHostObject.h"
#include "ConstantConversion.h"
#include "RenderPipelineHostObject.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderPassEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "setPipeline") {
    return WGPU_FUNC_FROM_HOST_FUNC(setPipeline, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(setPipeline);
      auto pipeline = arguments[0].asObject(runtime).asHostObject<RenderPipelineHostObject>(runtime);
      wgpuRenderPassEncoderSetPipeline(this->_value, pipeline->getValue());
      return Value(123);
    });
  }

  if (name == "draw") {
    return WGPU_FUNC_FROM_HOST_FUNC(draw, 4, [this]) {
      WGPU_LOG_FUNC_ARGS(draw);
      auto vertexCount = (uint32_t)arguments[0].asNumber();
      auto instanceCount = (uint32_t)count > 1 ? arguments[1].asNumber() : 1;
      auto firstVertex = (uint32_t)count > 2 ? arguments[2].asNumber() : 0;
      auto firstInstance = (uint32_t)count > 3 ? arguments[3].asNumber() : 0;
      wgpuRenderPassEncoderDraw(this->_value, vertexCount, instanceCount, firstVertex, firstInstance);
      return Value::undefined();
    });
  }

  if (name == "end") {
    return WGPU_FUNC_FROM_HOST_FUNC(end, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(end);
      wgpuRenderPassEncoderEnd(this->_value);
      return Value::undefined();
    });
  }

  if (name == "setBindGroup") {
    // TODO: see overloaded version with 5 args
    return WGPU_FUNC_FROM_HOST_FUNC(setBindGroup, 5, [this]) {
      WGPU_LOG_FUNC_ARGS(setBindGroup);
      auto index = (uint32_t)arguments[0].asNumber();
      WGPUBindGroup bindGroup = NULL;
      if (arguments[1].isObject()) {
        auto groupIn = arguments[1].asObject(runtime).asHostObject<BindGroupHostObject>(runtime);
        bindGroup = groupIn->getValue();
      }
      wgpuRenderPassEncoderSetBindGroup(_value, index, bindGroup, 0, NULL);
      return Value::undefined();
    });
  }

  if (name == "setVertexBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(setVertexBuffer, 4, [this]) {
      WGPU_LOG_FUNC_ARGS(setVertexBuffer);
      auto slot = (uint32_t)arguments[0].asNumber();
      WGPUBuffer buffer = NULL;
      if (arguments[1].isObject()) {
        auto bufferIn = arguments[1].asObject(runtime).asHostObject<BufferHostObject>(runtime);
        buffer = bufferIn->getValue();
      }
      auto offset = count > 2 ? (size_t)arguments[2].asNumber() : 0;
      auto size = count > 3 ? (size_t)arguments[3].asNumber() : buffer != NULL ? wgpuBufferGetSize(buffer) - offset : 0;
      wgpuRenderPassEncoderSetVertexBuffer(_value, slot, buffer, offset, size);
      return Value::undefined();
    });
  }

  if (name == "setIndexBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(setIndexBuffer, 4, [this]) {
      WGPU_LOG_FUNC_ARGS(setIndexBuffer);
      auto buffer = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue();
      auto format = arguments[1].asString(runtime).utf8(runtime);
      auto offset = count > 2 ? arguments[2].asNumber() : 0;
      auto size = count > 3 ? arguments[3].asNumber() : wgpuBufferGetSize(buffer) - offset;
      wgpuRenderPassEncoderSetIndexBuffer(_value, buffer, StringToWGPUIndexFormat(format.data()), offset, size);
      return Value::undefined();
    });
  }

  if (name == "drawIndexed") {
    return WGPU_FUNC_FROM_HOST_FUNC(drawIndexed, 5, [this]) {
      WGPU_LOG_FUNC_ARGS(drawIndexed);
      auto indexCount = (uint32_t)arguments[0].asNumber();
      auto instanceCount = count > 1 ? (uint32_t)arguments[1].asNumber() : 1;
      auto firstIndex = count > 2 ? (uint32_t)arguments[2].asNumber() : 0;
      auto baseVertex = count > 3 ? (uint32_t)arguments[3].asNumber() : 0;
      auto firstInstance = count > 4 ? (uint32_t)arguments[4].asNumber() : 0;
      wgpuRenderPassEncoderDrawIndexed(_value, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
      return Value::undefined();
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  if (name == "setViewport") {
    return WGPU_FUNC_FROM_HOST_FUNC(setViewport, 6, [this]) {
      WGPU_LOG_FUNC_ARGS(setViewport);
      auto x = (float)arguments[0].asNumber();
      auto y = (float)arguments[1].asNumber();
      auto width = (float)arguments[2].asNumber();
      auto height = (float)arguments[3].asNumber();
      auto minDepth = (float)arguments[4].asNumber();
      auto maxDepth = (float)arguments[5].asNumber();
      wgpuRenderPassEncoderSetViewport(_value, x, y, width, height, minDepth, maxDepth);
      return Value::undefined();
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> RenderPassEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "setPipeline", "draw", "end", "setBindGroup", "setVertexBuffer", "label",
                           "setIndexBuffer", "drawIndexed");
}
