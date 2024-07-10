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
#include "WGPUConversions.h"
#include "ConstantConversion.h"
#include "QuerySetHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value DeviceHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "createRenderPipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(createRenderPipeline, 1, [this]) {
            AutoReleasePool autoReleasePool;

            auto options = arguments[0].asObject(runtime);
            auto vertex = makeGPUVertexState(runtime, &autoReleasePool, WGPU_OBJ(options, vertex));
            auto label = WGPU_UTF8_OPT(options, label, "");

            WGPURenderPipelineDescriptor descriptor = {
                .label = label.data(),
                .layout = options.hasProperty(runtime, "layout") && options.getProperty(runtime, "layout").isObject() ? WGPU_HOST_OBJ(options, layout, PipelineLayoutHostObject)->_value : NULL,
                .vertex = vertex,
                .primitive = NULL,
                .depthStencil = NULL,
                .fragment = NULL,
                .multisample = NULL,
                .nextInChain = NULL,
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

            return Object::createFromHostObject(runtime, std::make_shared<RenderPipelineHostObject>(pipeline, _context, label));
        });
    }

    if (name == "createComputePipeline") {
        return WGPU_FUNC_FROM_HOST_FUNC(createComputePipeline, 1, [this]) {
            AutoReleasePool autoReleasePool;
            auto options = arguments[0].asObject(runtime);
            auto label = WGPU_UTF8_OPT(options, label, "");
            WGPUComputePipelineDescriptor descriptor = {
                .label = label.data(),
                .compute = makeWGPUProgrammableStageDescriptor(runtime, &autoReleasePool, WGPU_OBJ(options, compute)),
                .layout = options.hasProperty(runtime, "layout") && options.getProperty(runtime, "layout").isObject() ? WGPU_HOST_OBJ(options, layout, PipelineLayoutHostObject)->_value : NULL,
                .nextInChain = NULL,
            };
            auto pipeline = wgpuDeviceCreateComputePipeline(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<ComputePipelineHostObject>(pipeline, _context, label));
        });
    }

    if (name == "createShaderModule") {
        return WGPU_FUNC_FROM_HOST_FUNC(createShaderModule, 1, [this]) {

            WGPUShaderModuleDescriptor descriptor = {0};
            auto options = arguments[0].asObject(runtime);

            auto label = WGPU_UTF8_OPT(options, label, "");
            descriptor.label = label.data();

            auto code = WGPU_UTF8(options, code);
            WGPUShaderModuleWGSLDescriptor desc = {
                .code = code.data(),
                .chain = {
                    .next = NULL,
                    .sType = WGPUSType_ShaderModuleWGSLDescriptor,
                },
            };
            descriptor.nextInChain = (const WGPUChainedStruct *)&desc;

            auto shader = wgpuDeviceCreateShaderModule(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<ShaderModuleHostObject>(shader, _context, label));
        });
    }

    if (name == "createCommandEncoder") {
        return WGPU_FUNC_FROM_HOST_FUNC(createCommandEncoder, 1, [this]) {
            std::string label;
            if (count > 0) {
                label = WGPU_UTF8_OPT(arguments[0].asObject(runtime), label, "");
            }
            WGPUCommandEncoderDescriptor descriptor = {
                .label = label.data(),
                .nextInChain = NULL,
            };
            auto command_encoder = wgpuDeviceCreateCommandEncoder(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<CommandEncoderHostObject>(command_encoder, _context, label));
        });
    }

    if (name == "queue") {
        auto queue = wgpuDeviceGetQueue(_value);
        return Object::createFromHostObject(runtime, std::make_shared<QueueHostObject>(queue, _context));
    }

    if (name == "createBuffer") {
        return WGPU_FUNC_FROM_HOST_FUNC(createBuffer, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto label = WGPU_UTF8_OPT(desc, label, "");
            WGPUBufferDescriptor descriptor = {
                .label = label.data(),
                .mappedAtCreation = WGPU_BOOL_OPT(desc, mappedAtCreation, false),
                .size = (uint64_t)desc.getProperty(runtime, "size").asNumber(),
                .usage = (WGPUBufferUsageFlags)desc.getProperty(runtime, "usage").asNumber(),
            };
            auto buffer = wgpuDeviceCreateBuffer(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BufferHostObject>(buffer, _context, label));
        });
    }

    if (name == "createTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(createTexture, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto label = WGPU_UTF8_OPT(desc, label, "");
            auto format = desc.getProperty(runtime, "format").asString(runtime).utf8(runtime);
            auto dimension = WGPU_UTF8_OPT(desc, dimension, "2d");
            WGPUTextureDescriptor descriptor = makeDefaultWGPUTextureDescriptor(StringToWGPUTextureFormat(format));
            descriptor.label = label.data();
            descriptor.size = makeGPUExtent3D(runtime, WGPU_OBJ(desc, size));
            descriptor.usage = WGPU_NUMBER(desc, usage, WGPUTextureUsageFlags);
            descriptor.sampleCount = WGPU_NUMBER_OPT(desc, sampleCount, uint32_t, 1);
            descriptor.dimension = StringToWGPUTextureDimension(dimension.data());
            auto texture = wgpuDeviceCreateTexture(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<TextureHostObject>(texture, _context, label));
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

            auto label = WGPU_UTF8_OPT(desc, label, "");
            WGPUBindGroupDescriptor descriptor = {
                .label = label.data(),
                .layout = layout->_value,
                .entries = entries.data(),
                .entryCount = entries.size(),
            };

            auto bindGroup = wgpuDeviceCreateBindGroup(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BindGroupHostObject>(bindGroup, _context, label));
        });
    }

    if (name == "createSampler") {
        return WGPU_FUNC_FROM_HOST_FUNC(createSampler, 1, [this]) {
            WGPUSamplerDescriptor descriptor = makeDefaultSamplerDescriptor();
            std::string label;
            if (count > 0) {
                auto desc = arguments[0].asObject(runtime);
                label = WGPU_UTF8_OPT(desc, label, "");
                auto magFilter = WGPU_UTF8_OPT(desc, magFilter, "nearest");
                auto minFilter = WGPU_UTF8_OPT(desc, minFilter, "nearest");
                descriptor.label = label.data();
                descriptor.magFilter = StringToWGPUFilterMode(magFilter.data());
                descriptor.minFilter = StringToWGPUFilterMode(minFilter.data());
            }
            auto sampler = wgpuDeviceCreateSampler(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<SamplerHostObject>(sampler, _context, label));
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

            auto label = WGPU_UTF8_OPT(desc, label, "");
            WGPUBindGroupLayoutDescriptor descriptor = {
                .label = label.data(),
                .entries = entries.data(),
                .entryCount = entries.size(),
            };
            auto layout = wgpuDeviceCreateBindGroupLayout(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<BindGroupLayoutHostObject>(layout, _context, label));
        });
    }

    if (name == "createPipelineLayout") {
        return WGPU_FUNC_FROM_HOST_FUNC(createPipelineLayout, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto layouts = jsiArrayToVector<WGPUBindGroupLayout>(runtime, WGPU_ARRAY(desc, bindGroupLayouts), [](Runtime &runtime, Value value) {
                return value.asObject(runtime).asHostObject<BindGroupLayoutHostObject>(runtime)->_value;
            });
            auto label = WGPU_UTF8_OPT(desc, label, "");
            WGPUPipelineLayoutDescriptor descriptor = {
                .label = label.data(),
                .bindGroupLayouts = layouts.data(),
                .bindGroupLayoutCount = layouts.size(),
                .nextInChain = NULL,
            };
            auto layout = wgpuDeviceCreatePipelineLayout(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<PipelineLayoutHostObject>(layout, _context, label));
        });
    }

    if (name == "features") {
        auto size = wgpuDeviceEnumerateFeatures(_value, NULL);
        std::vector<WGPUFeatureName> features;
        features.resize(size);
        wgpuDeviceEnumerateFeatures(_value, features.data());
        return makeJsiFeatures(runtime, &features);
    }

    if (name == "limits") {
        WGPUSupportedLimits limits = {0};
        wgpuDeviceGetLimits(_value, &limits);
        return makeJsiLimits(runtime, &limits.limits);
    }

    if (name == "createQuerySet") {
        return WGPU_FUNC_FROM_HOST_FUNC(createQuerySet, 1, [this]) {
            auto desc = arguments[0].asObject(runtime);
            auto type = WGPU_UTF8(desc, type);
            auto label = WGPU_UTF8_OPT(desc, label, "");
            WGPUQuerySetDescriptor descriptor = {
                .label = label.data(),
                .type = StringToWGPUQueryType(type.data()),
                .count = WGPU_NUMBER(desc, count, uint32_t),
                .nextInChain = NULL,
            };
            auto querySet = wgpuDeviceCreateQuerySet(_value, &descriptor);
            return Object::createFromHostObject(runtime, std::make_shared<QuerySetHostObject>(querySet, _context, std::move(label)));
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> DeviceHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "createRenderPipeline", "createShaderModule", "createCommandEncoder", "queue", "createBuffer", "createTexture", "createBindGroup", "createSampler", "createBindGroupLayout", "createPipelineLayout", "features", "limits", "createQuerySet");
}
