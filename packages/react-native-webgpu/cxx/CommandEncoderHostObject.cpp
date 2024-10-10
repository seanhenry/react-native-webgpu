#include "CommandEncoderHostObject.h"

#include "BufferHostObject.h"
#include "CommandBufferHostObject.h"
#include "ComputePassEncoderHostObject.h"
#include "ConstantConversion.h"
#include "Mixins.h"
#include "QuerySetHostObject.h"
#include "RenderPassEncoderHostObject.h"
#include "TextureViewHostObject.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value CommandEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "beginRenderPass") {
    return WGPU_FUNC_FROM_HOST_FUNC(beginRenderPass, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(beginRenderPass);
      auto desc = arguments[0].asObject(runtime);
      auto colorAttachments = jsiArrayToVector<WGPURenderPassColorAttachment>(
        runtime, WGPU_ARRAY(desc, colorAttachments), [](Runtime &runtime, Value value) {
        if (value.isNull()) {
          // TODO: handle null state
          return (const WGPURenderPassColorAttachment){0};
        }
        auto attachment = value.asObject(runtime);
        return (const WGPURenderPassColorAttachment){
          .view = WGPU_HOST_OBJ(attachment, view, TextureViewHostObject)->getValue(),
          .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
          .resolveTarget = WGPU_HOST_OBJ_VALUE_OPT(attachment, resolveTarget, TextureViewHostObject, NULL),
          .loadOp = StringToWGPULoadOp(WGPU_UTF8(attachment, loadOp)),
          .storeOp = StringToWGPUStoreOp(WGPU_UTF8(attachment, storeOp)),
          .clearValue = makeWGPUColorFromProp(runtime, attachment, "clearValue"),
        };
      });

      auto label = WGPU_UTF8_OPT(desc, label, "");

      WGPURenderPassDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
        .colorAttachmentCount = colorAttachments.size(),
        .colorAttachments = colorAttachments.data(),
        .depthStencilAttachment = nullptr,
        .occlusionQuerySet = WGPU_HOST_OBJ_VALUE_OPT(desc, occlusionQuerySet, QuerySetHostObject, nullptr),
        .timestampWrites = nullptr,
      };

      WGPURenderPassDepthStencilAttachment depthStencilAttachment = {0};
      if (desc.hasProperty(runtime, "depthStencilAttachment")) {
        auto attachment = desc.getPropertyAsObject(runtime, "depthStencilAttachment");
        depthStencilAttachment.view = WGPU_HOST_OBJ(attachment, view, TextureViewHostObject)->getValue();

        depthStencilAttachment.depthClearValue = WGPU_NUMBER_OPT(attachment, depthClearValue, float, 0.0);

        auto depthLoadOp = WGPU_UTF8_OPT(attachment, depthLoadOp, "undefined");
        depthStencilAttachment.depthLoadOp = StringToWGPULoadOp(depthLoadOp);

        auto depthStoreOp = WGPU_UTF8_OPT(attachment, depthStoreOp, "undefined");
        depthStencilAttachment.depthStoreOp = StringToWGPUStoreOp(depthStoreOp);

        depthStencilAttachment.depthReadOnly = WGPU_BOOL_OPT(attachment, depthReadOnly, false);

        depthStencilAttachment.stencilClearValue = WGPU_NUMBER_OPT(attachment, stencilClearValue, uint32_t, 0);

        auto stencilLoadOp = WGPU_UTF8_OPT(attachment, stencilLoadOp, "undefined");
        depthStencilAttachment.stencilLoadOp = StringToWGPULoadOp(stencilLoadOp);

        auto stencilStoreOp = WGPU_UTF8_OPT(attachment, stencilStoreOp, "undefined");
        depthStencilAttachment.stencilStoreOp = StringToWGPUStoreOp(stencilStoreOp);

        depthStencilAttachment.stencilReadOnly = WGPU_BOOL_OPT(attachment, stencilReadOnly, false);

        descriptor.depthStencilAttachment = &depthStencilAttachment;
      }

      WGPURenderPassTimestampWrites timestampWrites;
      if (desc.hasProperty(runtime, "timestampWrites")) {
        timestampWrites =
          makeWGPURenderPassTimestampWrites(runtime, desc.getPropertyAsObject(runtime, "timestampWrites"));
        descriptor.timestampWrites = &timestampWrites;
      }

      auto encoder = wgpuCommandEncoderBeginRenderPass(_value, &descriptor);
      _context->getErrorHandler()->throwPendingJSIError();
      return Object::createFromHostObject(
        runtime, std::make_shared<RenderPassEncoderHostObject>(encoder, _context, std::move(label)));
    });
  }

  WGPU_ENCODER_MIXIN_FINISH(wgpuCommandEncoderFinish, WGPUCommandBufferDescriptor, CommandBufferHostObject)

  if (name == "copyTextureToTexture") {
    return WGPU_FUNC_FROM_HOST_FUNC(copyTextureToTexture, 3, [this]) {
      WGPU_LOG_FUNC_ARGS(copyTextureToTexture);
      auto source = arguments[0].asObject(runtime);
      auto destination = arguments[1].asObject(runtime);
      auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

      auto sourceCopyTexture = makeWGPUImageCopyTexture(runtime, std::move(source));
      auto destCopyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));
      wgpuCommandEncoderCopyTextureToTexture(_value, &sourceCopyTexture, &destCopyTexture, &copySize);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "beginComputePass") {
    return WGPU_FUNC_FROM_HOST_FUNC(beginComputePass, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(beginComputePass);
      std::string label;
      WGPUComputePassTimestampWrites timestampWrites;
      WGPUComputePassDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = nullptr,
        .timestampWrites = nullptr,
      };
      if (count > 0) {
        auto desc = arguments[0].asObject(runtime);
        label = WGPU_UTF8_OPT(desc, label, "");
        descriptor.label = label.data();

        if (desc.hasProperty(runtime, "timestampWrites")) {
          timestampWrites =
            makeWGPUComputePassTimestampWrites(runtime, desc.getPropertyAsObject(runtime, "timestampWrites"));
          descriptor.timestampWrites = &timestampWrites;
        }
      }
      auto encoder = wgpuCommandEncoderBeginComputePass(_value, &descriptor);
      _context->getErrorHandler()->throwPendingJSIError();
      return Object::createFromHostObject(
        runtime, std::make_shared<ComputePassEncoderHostObject>(encoder, _context, std::move(label)));
    });
  }

  if (name == "copyBufferToBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(copyBufferToBuffer, 5, [this]) {
      WGPU_LOG_FUNC_ARGS(copyBufferToBuffer);
      auto source = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue();
      auto sourceOffset = (uint64_t)arguments[1].asNumber();
      auto destination = arguments[2].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue();
      auto destinationOffset = (uint64_t)arguments[3].asNumber();
      auto size = (uint64_t)arguments[4].asNumber();
      wgpuCommandEncoderCopyBufferToBuffer(_value, source, sourceOffset, destination, destinationOffset, size);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "copyTextureToBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(copyTextureToBuffer, 3, [this]) {
      WGPU_LOG_FUNC_ARGS(copyTextureToBuffer);
      auto source = arguments[0].asObject(runtime);
      auto destination = arguments[1].asObject(runtime);
      auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

      auto sourceCopyTexture = makeWGPUImageCopyTexture(runtime, std::move(source));
      auto destCopyBuffer = makeWGPUImageCopyBuffer(runtime, destination, copySize);
      wgpuCommandEncoderCopyTextureToBuffer(_value, &sourceCopyTexture, &destCopyBuffer, &copySize);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "resolveQuerySet") {
    return WGPU_FUNC_FROM_HOST_FUNC(resolveQuerySet, 5, [this]) {
      WGPU_LOG_FUNC_ARGS(resolveQuerySet);
      auto querySet = arguments[0].asObject(runtime).asHostObject<QuerySetHostObject>(runtime)->getValue();
      auto firstQuery = (uint32_t)arguments[1].asNumber();
      auto queryCount = (uint32_t)arguments[2].asNumber();
      auto destination = arguments[3].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue();
      auto destinationOffset = (uint64_t)arguments[4].asNumber();
      wgpuCommandEncoderResolveQuerySet(_value, querySet, firstQuery, queryCount, destination, destinationOffset);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> CommandEncoderHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "beginRenderPass", "finish", "copyTextureToTexture", "beginComputePass",
                           "copyTextureToBuffer", "copyBufferToBuffer", "resolveQuerySet", "label");
}
