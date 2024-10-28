#include "RenderPassEncoderHostObject.h"

#include "BindGroupHostObject.h"
#include "BufferHostObject.h"
#include "ConstantConversion.h"
#include "Mixins.h"
#include "RenderBundleHostObject.h"
#include "RenderPipelineHostObject.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderPassEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_PIPELINE(wgpuRenderPassEncoderSetPipeline, RenderPipelineHostObject)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_INDEX_BUFFER(wgpuRenderPassEncoderSetIndexBuffer)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_VERTEX_BUFFER(wgpuRenderPassEncoderSetVertexBuffer)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW(wgpuRenderPassEncoderDraw)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW_INDEXED(wgpuRenderPassEncoderDrawIndexed)

  WGPU_GPU_BINDING_COMMANDS_MIXIN_SET_BIND_GROUP(wgpuRenderPassEncoderSetBindGroup)

  if (name == "end") {
    return WGPU_FUNC_FROM_HOST_FUNC(end, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(end);
      wgpuRenderPassEncoderEnd(this->_value);
      // https://github.com/gfx-rs/wgpu-native/issues/412
      release();
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
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
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "setScissorRect") {
    return WGPU_FUNC_FROM_HOST_FUNC(setScissorRect, 4, [this]) {
      WGPU_LOG_FUNC_ARGS(setViewport);
      auto x = (uint32_t)arguments[0].asNumber();
      auto y = (uint32_t)arguments[1].asNumber();
      auto width = (uint32_t)arguments[2].asNumber();
      auto height = (uint32_t)arguments[3].asNumber();

      wgpuRenderPassEncoderSetScissorRect(_value, x, y, width, height);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "executeBundles") {
    return WGPU_FUNC_FROM_HOST_FUNC(executeBundles, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(executeBundles);
      auto bundlesIn = arguments[0].asObject(runtime).asArray(runtime);
      auto bundles =
        jsiArrayToVector<WGPURenderBundle>(runtime, std::move(bundlesIn), [](Runtime &runtime, Value value) {
        return value.asObject(runtime).asHostObject<RenderBundleHostObject>(runtime)->getValue();
      });
      wgpuRenderPassEncoderExecuteBundles(_value, bundles.size(), bundles.data());
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "beginOcclusionQuery") {
    return WGPU_FUNC_FROM_HOST_FUNC(beginOcclusionQuery, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(beginOcclusionQuery);
      auto queryIndex = arguments[0].asNumber();
      wgpuRenderPassEncoderBeginOcclusionQuery(_value, queryIndex);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "endOcclusionQuery") {
    return WGPU_FUNC_FROM_HOST_FUNC(endOcclusionQuery, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(beginOcclusionQuery);
      wgpuRenderPassEncoderEndOcclusionQuery(_value);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  WGPU_GET_LABEL()

  WGPU_GET_BRAND(GPURenderPassEncoder)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> RenderPassEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "end", "label", "setViewport", "executeBundles", "beginOcclusionQuery",
                           "endOcclusionQuery", "setScissorRect", "__brand", WGPU_GPU_RENDER_COMMANDS_MIXIN_PROP_NAMES,
                           WGPU_GPU_BINDING_COMMANDS_MIXIN_PROP_NAMES);
}
