#include "CommandEncoderHostObject.h"
#include "TextureViewHostObject.h"
#include "RenderPassEncoderHostObject.h"
#include "CommandBufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include "WGPUConversions.h"

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

            WGPURenderPassDescriptor descriptor = {
                .colorAttachmentCount = colorAttachments.size(),
                .colorAttachments = colorAttachments.data(),
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
            return Object::createFromHostObject(runtime, std::make_shared<RenderPassEncoderHostObject>(encoder, _context));
        });
    }

    if (name == "finish") {
        return WGPU_FUNC_FROM_HOST_FUNC(finish, 0, [this]) {
            WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(_value, NULL);
            return Object::createFromHostObject(runtime, std::make_shared<CommandBufferHostObject>(buffer, _context));
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> CommandEncoderHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "beginRenderPass", "finish");
}