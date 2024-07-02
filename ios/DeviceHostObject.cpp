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
#include "SamplerHostObject.h"
#include "WGPUDefaults.h"
#include "ComputePipelineHostObject.h"
#include "PipelineLayoutHostObject.h"

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
                .layout = WGPU_HOST_OBJ_VALUE_OPT(options, layout, PipelineLayoutHostObject, NULL),
                .vertex = vertex,
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
            WGPUMultisampleState multisample = makeDefaultWGPUMultisampleState();
            if (options.hasProperty(runtime, "multisample")) {
                auto multisampleIn = options.getPropertyAsObject(runtime, "multisample");
                multisample.alphaToCoverageEnabled = WGPU_BOOL_OPT(multisampleIn, alphaToCoverageEnabled, false);
                multisample.count = WGPU_NUMBER_OPT(multisampleIn, count, uint32_t, 1);
                multisample.mask = WGPU_NUMBER_OPT(multisampleIn, mask, uint32_t, 0xFFFFFFFF);
            }
            descriptor.multisample = multisample;

            auto pipeline = wgpuDeviceCreateRenderPipeline(_value, &descriptor);

            return Object::createFromHostObject(runtime, std::make_shared<RenderPipelineHostObject>(pipeline, _context));
        });
    }

    if (name == "createComputePipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(createComputePipeline, 1, [this]) {
            AutoReleasePool autoReleasePool;
            auto options = arguments[0].asObject(runtime);
            WGPUComputePipelineDescriptor descriptor = {
                .compute = makeWGPUProgrammableStageDescriptor(runtime, &autoReleasePool, WGPU_OBJ(options, compute)),
                .layout = WGPU_HOST_OBJ_VALUE_OPT(options, layout, PipelineLayoutHostObject, NULL),
            };
            auto pipeline = wgpuDeviceCreateComputePipeline(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<ComputePipelineHostObject>(pipeline, _context));
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
            auto dimension = WGPU_UTF8_OPT(desc, dimension, "2d");
            WGPUTextureDescriptor descriptor = makeDefaultWGPUTextureDescriptor(StringToWGPUTextureFormat(format));
            descriptor.size = makeGPUExtent3D(runtime, WGPU_OBJ(desc, size));
            descriptor.usage = WGPU_NUMBER(desc, usage, WGPUTextureUsageFlags);
            descriptor.sampleCount = WGPU_NUMBER_OPT(desc, sampleCount, uint32_t, 1);
            descriptor.dimension = StringToWGPUTextureDimension(dimension.data());
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

    if (name == "createSampler") {
        return WGPU_FUNC_FROM_HOST_FUNC(createSampler, 1, [this]) {
            WGPUSamplerDescriptor descriptor = makeDefaultSamplerDescriptor();
            if (count > 0) {
                auto desc = arguments[0].asObject(runtime);
                auto magFilter = WGPU_UTF8_OPT(desc, magFilter, "nearest");
                auto minFilter = WGPU_UTF8_OPT(desc, minFilter, "nearest");
                descriptor.magFilter = StringToWGPUFilterMode(magFilter.data());
                descriptor.minFilter = StringToWGPUFilterMode(minFilter.data());
            }
            auto sampler = wgpuDeviceCreateSampler(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<SamplerHostObject>(sampler, _context));
        });
    }

    if (name == "createBindGroupLayout") {
        return WGPU_FUNC_FROM_HOST_FUNC(createBindGroupLayout, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto entries = jsiArrayToVector<WGPUBindGroupLayoutEntry>(runtime, WGPU_ARRAY(desc, entries), [](Runtime &runtime, Value value) {
                auto obj = value.asObject(runtime);
                WGPUBindGroupLayoutEntry entry = {
                    .binding = WGPU_NUMBER(obj, binding, uint32_t),
                    .visibility = WGPU_NUMBER(obj, visibility, WGPUShaderStageFlags),
                };
                if (obj.hasProperty(runtime, "buffer")) {
                    auto buffer = obj.getPropertyAsObject(runtime, "buffer");
                    auto type = WGPU_UTF8_OPT(buffer, type, "uniform");
                    entry.buffer = {
                        .hasDynamicOffset = false,
                        .minBindingSize = 0,
                        .type = StringToWGPUBufferBindingType(type.data()),
                    };
                }
                return entry;
            });

            WGPUBindGroupLayoutDescriptor descriptor = {
                .entries = entries.data(),
                .entryCount = entries.size(),
            };
            auto layout = wgpuDeviceCreateBindGroupLayout(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BindGroupLayoutHostObject>(layout, _context));
        });
    }

    if (name == "createPipelineLayout") {
        return WGPU_FUNC_FROM_HOST_FUNC(createPipelineLayout, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto layouts = jsiArrayToVector<WGPUBindGroupLayout>(runtime, WGPU_ARRAY(desc, bindGroupLayouts), [](Runtime &runtime, Value value) {
                return value.asObject(runtime).asHostObject<BindGroupLayoutHostObject>(runtime)->_value;
            });
            WGPUPipelineLayoutDescriptor descriptor = {
                .bindGroupLayouts = layouts.data(),
                .bindGroupLayoutCount = layouts.size(),
            };
            auto layout = wgpuDeviceCreatePipelineLayout(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<PipelineLayoutHostObject>(layout, _context));
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> DeviceHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createRenderPipeline", "createShaderModule", "createCommandEncoder", "queue", "createBuffer", "createTexture", "createBindGroup", "createSampler", "createBindGroupLayout", "createPipelineLayout");
}
