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

  if (name == "executeBundles") {
    return WGPU_FUNC_FROM_HOST_FUNC(executeBundles, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(executeBundles);
      auto bundlesIn = arguments[0].asObject(runtime).asArray(runtime);
      auto bundles =
        jsiArrayToVector<WGPURenderBundle>(runtime, std::move(bundlesIn), [](Runtime &runtime, Value value) {
          return value.asObject(runtime).asHostObject<RenderBundleHostObject>(runtime)->getValue();
        });
      wgpuRenderPassEncoderExecuteBundles(_value, bundles.size(), bundles.data());
      return Value::undefined();
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> RenderPassEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "end", "label", "setViewport", "executeBundles",
                           WGPU_GPU_RENDER_COMMANDS_MIXIN_PROP_NAMES, WGPU_GPU_BINDING_COMMANDS_MIXIN_PROP_NAMES);
}
