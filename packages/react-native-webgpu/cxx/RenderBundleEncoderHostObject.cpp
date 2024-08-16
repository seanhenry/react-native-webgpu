#include "RenderBundleEncoderHostObject.h"

#include "Mixins.h"
#include "RenderBundleHostObject.h"
#include "WGPUContext.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value RenderBundleEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_PIPELINE(wgpuRenderBundleEncoderSetPipeline, RenderPipelineHostObject)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_INDEX_BUFFER(wgpuRenderBundleEncoderSetIndexBuffer)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_SET_VERTEX_BUFFER(wgpuRenderBundleEncoderSetVertexBuffer)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW(wgpuRenderBundleEncoderDraw)
  WGPU_GPU_RENDER_COMMANDS_MIXIN_DRAW_INDEXED(wgpuRenderBundleEncoderDrawIndexed)

  WGPU_GPU_BINDING_COMMANDS_MIXIN_SET_BIND_GROUP(wgpuRenderBundleEncoderSetBindGroup)
  WGPU_ENCODER_MIXIN_FINISH(wgpuRenderBundleEncoderFinish, WGPURenderBundleDescriptor, RenderBundleHostObject)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> RenderBundleEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "label", "finish", WGPU_GPU_RENDER_COMMANDS_MIXIN_PROP_NAMES,
                           WGPU_GPU_BINDING_COMMANDS_MIXIN_PROP_NAMES);
}
