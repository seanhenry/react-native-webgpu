#pragma once

#include "BindGroupHostObject.h"
#include "BufferHostObject.h"
#include "ConstantConversion.h"
#include "RenderPipelineHostObject.h"
#include "WGPUJsiUtils.h"

// GPURenderCommandsMixin
#define WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_PIPELINE(__func, __pipeline)                 \
  if (name == "setPipeline") {                                                          \
    return WGPU_FUNC_FROM_HOST_FUNC(setPipeline, 1, [this]) {                           \
      WGPU_LOG_FUNC_ARGS(setPipeline);                                                  \
      auto pipeline = arguments[0].asObject(runtime).asHostObject<__pipeline>(runtime); \
      __func(_value, pipeline->getValue());                                             \
      return Value::undefined();                                                        \
    });                                                                                 \
  }

#define WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_INDEX_BUFFER(__func)                                         \
  if (name == "setIndexBuffer") {                                                                       \
    return WGPU_FUNC_FROM_HOST_FUNC(setIndexBuffer, 4, [this]) {                                        \
      WGPU_LOG_FUNC_ARGS(setIndexBuffer);                                                               \
      auto buffer = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue(); \
      auto format = arguments[1].asString(runtime).utf8(runtime);                                       \
      auto offset = count > 2 ? arguments[2].asNumber() : 0;                                            \
      auto size = count > 3 ? arguments[3].asNumber() : wgpuBufferGetSize(buffer) - offset;             \
      __func(_value, buffer, StringToWGPUIndexFormat(format.data()), offset, size);                     \
      return Value::undefined();                                                                        \
    });                                                                                                 \
  }

#define WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_VERTEX_BUFFER(__func)                                \
  if (name == "setVertexBuffer") {                                                              \
    return WGPU_FUNC_FROM_HOST_FUNC(setVertexBuffer, 4, [this]) {                               \
      WGPU_LOG_FUNC_ARGS(setVertexBuffer);                                                      \
      auto slot = (uint32_t)arguments[0].asNumber();                                            \
      WGPUBuffer buffer = nullptr;                                                              \
      if (arguments[1].isObject()) {                                                            \
        auto bufferIn = arguments[1].asObject(runtime).asHostObject<BufferHostObject>(runtime); \
        buffer = bufferIn->getValue();                                                          \
      }                                                                                         \
      auto offset = count > 2 ? (size_t)arguments[2].asNumber() : 0;                            \
      auto size = count > 3           ? (size_t)arguments[3].asNumber()                         \
                  : buffer != nullptr ? wgpuBufferGetSize(buffer) - offset                      \
                                      : 0;                                                      \
      __func(_value, slot, buffer, offset, size);                                               \
      return Value::undefined();                                                                \
    });                                                                                         \
  }

#define WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW(__func)                                 \
  if (name == "draw") {                                                             \
    return WGPU_FUNC_FROM_HOST_FUNC(draw, 4, [this]) {                              \
      WGPU_LOG_FUNC_ARGS(draw);                                                     \
      auto vertexCount = (uint32_t)arguments[0].asNumber();                         \
      auto instanceCount = (uint32_t)count > 1 ? arguments[1].asNumber() : 1;       \
      auto firstVertex = (uint32_t)count > 2 ? arguments[2].asNumber() : 0;         \
      auto firstInstance = (uint32_t)count > 3 ? arguments[3].asNumber() : 0;       \
      __func(this->_value, vertexCount, instanceCount, firstVertex, firstInstance); \
      return Value::undefined();                                                    \
    });                                                                             \
  }

#define WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW_INDEXED(__func)                             \
  if (name == "drawIndexed") {                                                          \
    return WGPU_FUNC_FROM_HOST_FUNC(drawIndexed, 5, [this]) {                           \
      WGPU_LOG_FUNC_ARGS(drawIndexed);                                                  \
      auto indexCount = (uint32_t)arguments[0].asNumber();                              \
      auto instanceCount = count > 1 ? (uint32_t)arguments[1].asNumber() : 1;           \
      auto firstIndex = count > 2 ? (uint32_t)arguments[2].asNumber() : 0;              \
      auto baseVertex = count > 3 ? (uint32_t)arguments[3].asNumber() : 0;              \
      auto firstInstance = count > 4 ? (uint32_t)arguments[4].asNumber() : 0;           \
      __func(_value, indexCount, instanceCount, firstIndex, baseVertex, firstInstance); \
      return Value::undefined();                                                        \
    });                                                                                 \
  }

#define WGPU_GPU_RENDER_COMMANDS_MIXIN_PROP_NAMES \
  "setPipeline", "setIndexBuffer", "setVertexBuffer", "draw", "drawIndexed"

// GPUBindingCommandsMixin

// TODO: see overloaded version with 5 args
#define WGPU_GPU_BINDING_COMMANDS_MIXIN_SET_BIND_GROUP(__func)                                    \
  if (name == "setBindGroup") {                                                                   \
    return WGPU_FUNC_FROM_HOST_FUNC(setBindGroup, 5, [this]) {                                    \
      WGPU_LOG_FUNC_ARGS(setBindGroup);                                                           \
      auto index = (uint32_t)arguments[0].asNumber();                                             \
      WGPUBindGroup bindGroup = nullptr;                                                          \
      if (arguments[1].isObject()) {                                                              \
        auto groupIn = arguments[1].asObject(runtime).asHostObject<BindGroupHostObject>(runtime); \
        bindGroup = groupIn->getValue();                                                          \
      }                                                                                           \
      __func(_value, index, bindGroup, 0, NULL);                                                  \
      return Value::undefined();                                                                  \
    });                                                                                           \
  }

#define WGPU_GPU_BINDING_COMMANDS_MIXIN_PROP_NAMES "setBindGroup"

// Other encoder

#define WGPU_ENCODER_MIXIN_FINISH(__func, __descriptor, __return)                                                   \
  if (name == "finish") {                                                                                           \
    return WGPU_FUNC_FROM_HOST_FUNC(finish, 1, [this]) {                                                            \
      WGPU_LOG_FUNC_ARGS(finish);                                                                                   \
      std::string label;                                                                                            \
      if (count > 0) {                                                                                              \
        label = WGPU_UTF8_OPT(arguments[0].asObject(runtime), label, "");                                           \
      }                                                                                                             \
      __descriptor descriptor = {                                                                                   \
        .nextInChain = nullptr,                                                                                     \
        .label = label.data(),                                                                                      \
      };                                                                                                            \
      auto buffer = __func(_value, &descriptor);                                                                    \
      return Object::createFromHostObject(runtime, std::make_shared<__return>(buffer, _context, std::move(label))); \
    });                                                                                                             \
  }
