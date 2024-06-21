#include "DeviceHostObject.h"
#include "ShaderModuleHostObject.h"
#include "RenderPipelineHostObject.h"
#include "CommandEncoderHostObject.h"
#include "QueueHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "ConstantConversion.h"
#include "AutoReleasePool.h"
#include "BufferHostObject.h"
#include "WGPUDefaults.h"
#include "WGPUConversions.h"
#include "TextureHostObject.h"
#include "BindGroupLayoutHostObject.h"
#include "BindGroupHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value DeviceHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "createRenderPipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(createRenderPipeline, 1, [this]) {
            AutoReleasePool autoReleasePool;

            auto options = arguments[0].asObject(runtime);
            auto vertex = makeGPUVertexState(runtime, &autoReleasePool, WGPU_OBJ(options, vertex));

            WGPURenderPipelineDescriptor descriptor = {
                .layout = NULL, // TODO: GPUPipelineLayout
                .vertex = vertex,
                .multisample = makeDefaultWGPUMultisampleState(),
            };
            if (options.hasProperty(runtime, "primitive")) {
                descriptor.primitive = makeWGPUPrimitiveState(runtime, WGPU_OBJ(options, primitive));
            }
            WGPUDepthStencilState depthStencil;
            if (options.hasProperty(runtime, "depthStencil")) {
                depthStencil = makeWGPUDepthStencilState(runtime, WGPU_OBJ(options, depthStencil));
                descriptor.depthStencil = &depthStencil;
            }
            WGPUFragmentState fragment;
            if (options.hasProperty(runtime, "fragment")) {
                fragment = makeGPUFragmentState(runtime, &autoReleasePool, WGPU_OBJ(options, fragment));
                descriptor.fragment = &fragment;
            }
            auto pipeline = wgpuDeviceCreateRenderPipeline(_value, &descriptor);

            return Object::createFromHostObject(runtime, std::make_shared<RenderPipelineHostObject>(pipeline, _context));
        });
    }

    if (name == "createShaderModule") {
        return WGPU_FUNC_FROM_HOST_FUNC(createShaderModule, 1, [this]) {

            WGPUShaderModuleDescriptor descriptor;
            auto options = arguments[0].asObject(runtime);

            auto label = WGPU_UTF8_OPT(options, label, "");
            if (label != "") {
                descriptor.label = label.data();
            }

            auto code = WGPU_UTF8(options, code);
            WGPUShaderModuleWGSLDescriptor desc = {
                .code = code.data(),
                .chain = {
                    .sType = WGPUSType_ShaderModuleWGSLDescriptor,
                }
            };
            descriptor.nextInChain = (const WGPUChainedStruct *)&desc;

            auto shader = wgpuDeviceCreateShaderModule(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<ShaderModuleHostObject>(shader, _context));
        });
    }

    if (name == "createCommandEncoder") {
        return WGPU_FUNC_FROM_HOST_FUNC(createCommandEncoder, 1, [this]) {
            WGPUCommandEncoderDescriptor descriptor = {0};
            auto command_encoder = wgpuDeviceCreateCommandEncoder(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<CommandEncoderHostObject>(command_encoder, _context));
        });
    }

    if (name == "queue") {
        auto queue = wgpuDeviceGetQueue(_value);
        return Object::createFromHostObject(runtime, std::make_shared<QueueHostObject>(queue, _context));
    }

    if (name == "createBuffer") {
        return WGPU_FUNC_FROM_HOST_FUNC(createBuffer, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            WGPUBufferDescriptor descriptor = {
                .mappedAtCreation = WGPU_BOOL_OPT(desc, mappedAtCreation, false),
                .size = (uint64_t)desc.getProperty(runtime, "size").asNumber(),
                .usage = (WGPUBufferUsageFlags)desc.getProperty(runtime, "usage").asNumber(),
            };
            auto buffer = wgpuDeviceCreateBuffer(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BufferHostObject>(buffer, _context));
        });
    }

    if (name == "createTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(createTexture, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto format = desc.getProperty(runtime, "format").asString(runtime).utf8(runtime);
            WGPUTextureDescriptor descriptor = makeDefaultWGPUTextureDescriptor(StringToWGPUTextureFormat(format));
            descriptor.size = makeGPUExtent3D(runtime, WGPU_OBJ(desc, size));
            descriptor.usage = WGPU_NUMBER(desc, usage, WGPUTextureUsageFlags);
            auto texture = wgpuDeviceCreateTexture(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<TextureHostObject>(texture, _context));
        });
    }


    if (name == "createBindGroup") {
        return WGPU_FUNC_FROM_HOST_FUNC(createBindGroup, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto layout = WGPU_HOST_OBJ(desc, layout, BindGroupLayoutHostObject);
            auto entries = jsiArrayToVector<WGPUBindGroupEntry>(runtime, WGPU_ARRAY(desc, entries), [](Runtime &runtime, Value value){
                auto obj = value.asObject(runtime);
                auto resource = obj.getProperty(runtime, "resource");

                WGPUBindGroupEntry entry = {
                    .binding = WGPU_NUMBER(obj, binding, uint32_t),
                };
                makeWGPUBindingResource(runtime, std::move(resource), &entry);
                return entry;
            });

            WGPUBindGroupDescriptor descriptor = {
                .layout = layout->_value,
                .entries = entries.data(),
                .entryCount = entries.size(),
            };
            auto bindGroup = wgpuDeviceCreateBindGroup(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BindGroupHostObject>(bindGroup, _context));
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> DeviceHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createRenderPipeline", "createShaderModule", "createCommandEncoder", "queue");
}
