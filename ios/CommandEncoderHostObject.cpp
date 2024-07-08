#include "CommandEncoderHostObject.h"
#include "TextureViewHostObject.h"
#include "RenderPassEncoderHostObject.h"
#include "CommandBufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include "WGPUConversions.h"
#include "ComputePassEncoderHostObject.h"
#include "BufferHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value CommandEncoderHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "beginRenderPass") {
        return WGPU_FUNC_FROM_HOST_FUNC(beginRenderPass, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto colorAttachments = jsiArrayToVector<WGPURenderPassColorAttachment>(runtime, WGPU_ARRAY(desc, colorAttachments), [](Runtime &runtime, Value value) {
                if (value.isNull()) {
                    // TODO: handle null state
                    return (const WGPURenderPassColorAttachment){0};
                }
                auto attachment = value.asObject(runtime);
                auto loadOp = WGPU_UTF8(attachment, loadOp);
                auto storeOp = WGPU_UTF8(attachment, storeOp);
                return (const WGPURenderPassColorAttachment){
                    .view = WGPU_HOST_OBJ(attachment, view, TextureViewHostObject)->_value,
                    .loadOp = StringToWGPULoadOp(loadOp.data()),
                    .storeOp = StringToWGPUStoreOp(storeOp.data()),
                    .depthSlice = WGPU_DEPTH_SLICE_UNDEFINED,
                    .clearValue = makeWGPUColorFromProp(runtime, attachment, "clearValue"),
                    .resolveTarget = WGPU_HOST_OBJ_VALUE_OPT(attachment, resolveTarget, TextureViewHostObject, NULL),
                };
            });

            auto label = WGPU_UTF8_OPT(desc, label, "");

            WGPURenderPassDescriptor descriptor = {
                .label = label.data(),
                .colorAttachmentCount = colorAttachments.size(),
                .colorAttachments = colorAttachments.data(),
                .depthStencilAttachment = NULL,
                .nextInChain = NULL,
                .occlusionQuerySet = NULL,
                .timestampWrites = NULL,
            };

            WGPURenderPassDepthStencilAttachment depthStencilAttachment = {0};
            if (desc.hasProperty(runtime, "depthStencilAttachment")) {
                auto attachment = desc.getPropertyAsObject(runtime, "depthStencilAttachment");
                depthStencilAttachment.view = WGPU_HOST_OBJ(attachment, view, TextureViewHostObject)->_value;

                depthStencilAttachment.depthClearValue = WGPU_NUMBER_OPT(attachment, depthClearValue, float, 0.0);

                auto depthLoadOp = WGPU_UTF8_OPT(attachment, depthLoadOp, "undefined");
                depthStencilAttachment.depthLoadOp = StringToWGPULoadOp(depthLoadOp.data());

                auto depthStoreOp = WGPU_UTF8_OPT(attachment, depthStoreOp, "undefined");
                depthStencilAttachment.depthStoreOp = StringToWGPUStoreOp(depthStoreOp.data());

                descriptor.depthStencilAttachment = &depthStencilAttachment;
            }

            auto encoder = wgpuCommandEncoderBeginRenderPass(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<RenderPassEncoderHostObject>(encoder, _context, label));
        });
    }

    if (name == "finish") {
        return WGPU_FUNC_FROM_HOST_FUNC(finish, 1, [this]) {
            std::string label;
            if (count > 0) {
                label = WGPU_UTF8_OPT(arguments[0].asObject(runtime), label, "");
            }
            WGPUCommandBufferDescriptor descriptor = {
                .label = label.data(),
                .nextInChain = NULL,
            };
            WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<CommandBufferHostObject>(buffer, _context, label));
        });
    }

    if (name == "copyTextureToTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(copyTextureToTexture, 3, [this]) {
            auto source = arguments[0].asObject(runtime);
            auto destination = arguments[1].asObject(runtime);
            auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

            auto sourceCopyTexture = makeWGPUImageCopyTexture(runtime, std::move(source));
            auto destCopyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));
            wgpuCommandEncoderCopyTextureToTexture(_value, &sourceCopyTexture, &destCopyTexture, &copySize);
            return Value::undefined();
        });
    }

    if (name == "beginComputePass") {
        return WGPU_FUNC_FROM_HOST_FUNC(beginComputePass, 1, [this]) {
            std::string label;
            if (count > 0) {
                label = WGPU_UTF8_OPT(arguments[0].asObject(runtime), label, "");
            }
            WGPUComputePassDescriptor descriptor = {
                .label = label.data(),
                .nextInChain = NULL,
                .timestampWrites = NULL,
            };
            auto encoder = wgpuCommandEncoderBeginComputePass(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<ComputePassEncoderHostObject>(encoder, _context, label));
        });
    }

    if (name == "copyBufferToBuffer") {
        return WGPU_FUNC_FROM_HOST_FUNC(copyBufferToBuffer, 5, [this]) {
            auto source = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->_value;
            auto sourceOffset = (uint64_t)arguments[1].asNumber();
            auto destination = arguments[2].asObject(runtime).asHostObject<BufferHostObject>(runtime)->_value;
            auto destinationOffset = (uint64_t)arguments[3].asNumber();
            auto size = (uint64_t)arguments[4].asNumber();
            wgpuCommandEncoderCopyBufferToBuffer(_value, source, sourceOffset, destination, destinationOffset, size);
            return Value::undefined();
        });
    }

    if (name == "label") {
        return String::createFromUtf8(runtime, _label);
    }

    return Value::undefined();
}

std::vector<PropNameID> CommandEncoderHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "beginRenderPass", "finish", "copyTextureToTexture", "beginComputePass", "copyBufferToBuffer", "label");
}
