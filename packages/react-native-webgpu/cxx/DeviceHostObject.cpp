#include "DeviceHostObject.h"

#include "AutoReleasePool.h"
#include "BindGroupHostObject.h"
#include "BindGroupLayoutHostObject.h"
#include "BufferHostObject.h"
#include "CommandEncoderHostObject.h"
#include "ComputePipelineHostObject.h"
#include "ConstantConversion.h"
#include "PipelineLayoutHostObject.h"
#include "QuerySetHostObject.h"
#include "QueueHostObject.h"
#include "RenderPipelineHostObject.h"
#include "SamplerHostObject.h"
#include "ShaderModuleHostObject.h"
#include "TextureHostObject.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value DeviceHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "createRenderPipeline") {
    return WGPU_FUNC_FROM_HOST_FUNC(createRenderPipeline, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createRenderPipeline);
      AutoReleasePool autoReleasePool;

      auto options = arguments[0].asObject(runtime);
      auto vertex = makeGPUVertexState(runtime, &autoReleasePool, WGPU_OBJ(options, vertex));
      auto label = WGPU_UTF8_OPT(options, label, "");

      WGPURenderPipelineDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
        .layout = options.hasProperty(runtime, "layout") && options.getProperty(runtime, "layout").isObject()
                    ? WGPU_HOST_OBJ(options, layout, PipelineLayoutHostObject)->getValue()
                    : nullptr,
        .vertex = vertex,
        .primitive = {nullptr},
        .depthStencil = nullptr,
        .multisample = {nullptr},
        .fragment = nullptr,
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

      auto pipeline = wgpuDeviceCreateRenderPipeline(getValue(), &descriptor);

      return Object::createFromHostObject(
        runtime, std::make_shared<RenderPipelineHostObject>(pipeline, _context, std::move(label)));
    });
  }

  if (name == "createComputePipeline") {
    return WGPU_FUNC_FROM_HOST_FUNC(createComputePipeline, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createComputePipeline);
      AutoReleasePool autoReleasePool;
      auto options = arguments[0].asObject(runtime);
      auto label = WGPU_UTF8_OPT(options, label, "");
      WGPUComputePipelineDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
        .layout = options.hasProperty(runtime, "layout") && options.getProperty(runtime, "layout").isObject()
                    ? WGPU_HOST_OBJ(options, layout, PipelineLayoutHostObject)->getValue()
                    : nullptr,
        .compute = makeWGPUProgrammableStageDescriptor(runtime, &autoReleasePool, WGPU_OBJ(options, compute)),
      };
      auto pipeline = wgpuDeviceCreateComputePipeline(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<ComputePipelineHostObject>(pipeline, _context, std::move(label)));
    });
  }

  if (name == "createShaderModule") {
    return WGPU_FUNC_FROM_HOST_FUNC(createShaderModule, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createShaderModule);

      WGPUShaderModuleDescriptor descriptor = {nullptr};
      auto options = arguments[0].asObject(runtime);

      auto label = WGPU_UTF8_OPT(options, label, "");
      descriptor.label = label.data();

      auto code = WGPU_UTF8(options, code);
      WGPUShaderModuleWGSLDescriptor desc = {
        .chain =
          {
            .next = nullptr,
            .sType = WGPUSType_ShaderModuleWGSLDescriptor,
          },
        .code = code.data(),
      };
      descriptor.nextInChain = (const WGPUChainedStruct *)&desc;

      auto shader = wgpuDeviceCreateShaderModule(getValue(), &descriptor);
      return Object::createFromHostObject(runtime,
                                          std::make_shared<ShaderModuleHostObject>(shader, _context, std::move(label)));
    });
  }

  if (name == "createCommandEncoder") {
    return WGPU_FUNC_FROM_HOST_FUNC(createCommandEncoder, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createCommandEncoder);
      std::string label;
      if (count > 0) {
        label = WGPU_UTF8_OPT(arguments[0].asObject(runtime), label, "");
      }
      WGPUCommandEncoderDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
      };
      auto command_encoder = wgpuDeviceCreateCommandEncoder(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<CommandEncoderHostObject>(command_encoder, _context, std::move(label)));
    });
  }

  if (name == "queue") {
    auto queue = wgpuDeviceGetQueue(getValue());
    return Object::createFromHostObject(runtime, std::make_shared<QueueHostObject>(queue, _context));
  }

  if (name == "createBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(createBuffer, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createBuffer);
      auto desc = arguments[0].asObject(runtime);
      auto label = WGPU_UTF8_OPT(desc, label, "");
      WGPUBufferDescriptor descriptor = {
        .label = label.data(),
        .usage = (WGPUBufferUsageFlags)desc.getProperty(runtime, "usage").asNumber(),
        .size = (uint64_t)desc.getProperty(runtime, "size").asNumber(),
        .mappedAtCreation = WGPU_BOOL_OPT(desc, mappedAtCreation, false),
      };
      auto buffer = wgpuDeviceCreateBuffer(getValue(), &descriptor);
      return Object::createFromHostObject(runtime,
                                          std::make_shared<BufferHostObject>(buffer, _context, std::move(label)));
    });
  }

  if (name == "createTexture") {
    return WGPU_FUNC_FROM_HOST_FUNC(createTexture, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createTexture);
      auto desc = arguments[0].asObject(runtime);
      auto label = WGPU_UTF8_OPT(desc, label, "");
      auto format = WGPU_UTF8(desc, format);
      auto dimension = WGPU_UTF8_OPT(desc, dimension, "2d");
      WGPUTextureDescriptor descriptor = makeDefaultWGPUTextureDescriptor(StringToWGPUTextureFormat(format));
      descriptor.label = label.data();
      descriptor.size = makeGPUExtent3D(runtime, WGPU_OBJ(desc, size));
      descriptor.usage = WGPU_NUMBER(desc, usage, WGPUTextureUsageFlags);
      descriptor.sampleCount = WGPU_NUMBER_OPT(desc, sampleCount, uint32_t, descriptor.sampleCount);
      descriptor.dimension = StringToWGPUTextureDimension(dimension.data());
      descriptor.mipLevelCount = WGPU_NUMBER_OPT(desc, mipLevelCount, uint32_t, descriptor.mipLevelCount);
      auto texture = wgpuDeviceCreateTexture(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<TextureHostObject>(texture, _context, std::move(label), false));
    });
  }

  if (name == "createBindGroup") {
    return WGPU_FUNC_FROM_HOST_FUNC(createBindGroup, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createBindGroup);
      auto desc = arguments[0].asObject(runtime);
      auto layout = WGPU_HOST_OBJ(desc, layout, BindGroupLayoutHostObject);
      auto entries =
        jsiArrayToVector<WGPUBindGroupEntry>(runtime, WGPU_ARRAY(desc, entries), [](Runtime &runtime, Value value) {
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
        .layout = layout->getValue(),
        .entryCount = entries.size(),
        .entries = entries.data(),
      };

      auto bindGroup = wgpuDeviceCreateBindGroup(getValue(), &descriptor);
      return Object::createFromHostObject(runtime,
                                          std::make_shared<BindGroupHostObject>(bindGroup, _context, std::move(label)));
    });
  }

  if (name == "createSampler") {
    return WGPU_FUNC_FROM_HOST_FUNC(createSampler, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createSampler);
      WGPUSamplerDescriptor descriptor = makeDefaultSamplerDescriptor();
      std::string label;
      if (count > 0) {
        auto desc = arguments[0].asObject(runtime);
        label = WGPU_UTF8_OPT(desc, label, "");
        auto magFilter = WGPU_UTF8_OPT(desc, magFilter, "nearest");
        auto minFilter = WGPU_UTF8_OPT(desc, minFilter, "nearest");
        auto mipmapFilter = WGPU_UTF8_OPT(desc, mipmapFilter, "nearest");
        auto addressModeU = WGPU_UTF8_OPT(desc, addressModeU, "repeat");
        auto addressModeV = WGPU_UTF8_OPT(desc, addressModeV, "repeat");
        auto addressModeW = WGPU_UTF8_OPT(desc, addressModeW, "repeat");
        descriptor.label = label.data();
        descriptor.magFilter = StringToWGPUFilterMode(magFilter.data());
        descriptor.minFilter = StringToWGPUFilterMode(minFilter.data());
        descriptor.mipmapFilter = StringToWGPUMipmapFilterMode(mipmapFilter.data());
        descriptor.maxAnisotropy = WGPU_NUMBER_OPT(desc, maxAnisotropy, uint16_t, descriptor.maxAnisotropy);
        descriptor.addressModeU = StringToWGPUAddressMode(addressModeU.data());
        descriptor.addressModeV = StringToWGPUAddressMode(addressModeV.data());
        descriptor.addressModeW = StringToWGPUAddressMode(addressModeW.data());
      }
      auto sampler = wgpuDeviceCreateSampler(getValue(), &descriptor);
      return Object::createFromHostObject(runtime,
                                          std::make_shared<SamplerHostObject>(sampler, _context, std::move(label)));
    });
  }

  if (name == "createBindGroupLayout") {
    return WGPU_FUNC_FROM_HOST_FUNC(createBindGroupLayout, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createBindGroupLayout);
      auto desc = arguments[0].asObject(runtime);
      auto entries = jsiArrayToVector<WGPUBindGroupLayoutEntry>(
        runtime, WGPU_ARRAY(desc, entries), [](Runtime &runtime, Value value) {
          auto obj = value.asObject(runtime);
          WGPUBindGroupLayoutEntry entry = {
            .binding = WGPU_NUMBER(obj, binding, uint32_t),
            .visibility = WGPU_NUMBER(obj, visibility, WGPUShaderStageFlags),
          };
          if (obj.hasProperty(runtime, "buffer")) {
            auto buffer = obj.getPropertyAsObject(runtime, "buffer");
            auto type = WGPU_UTF8_OPT(buffer, type, "uniform");
            entry.buffer = {
              .type = StringToWGPUBufferBindingType(type.data()),
              .hasDynamicOffset = WGPU_BOOL_OPT(buffer, hasDynamicOffset, false),
              .minBindingSize = WGPU_NUMBER_OPT(buffer, minBindingSize, uint64_t, 0),
            };
          }
          if (obj.hasProperty(runtime, "texture")) {
            auto texture = obj.getPropertyAsObject(runtime, "texture");
            auto viewDimension = WGPU_UTF8_OPT(texture, viewDimension, "2d");
            auto sampleType = WGPU_UTF8_OPT(texture, sampleType, "float");
            entry.texture = {
              .nextInChain = nullptr,
              .sampleType = StringToWGPUTextureSampleType(sampleType.data()),
              .viewDimension = StringToWGPUTextureViewDimension(viewDimension.data()),
              .multisampled = WGPU_BOOL_OPT(texture, multisampled, false),
            };
          }
          if (obj.hasProperty(runtime, "sampler")) {
            auto sampler = obj.getPropertyAsObject(runtime, "sampler");
            auto type = WGPU_UTF8_OPT(sampler, type, "filtering");
            entry.sampler = {
              .nextInChain = nullptr,
              .type = StringToWGPUSamplerBindingType(type.data()),
            };
          }
          return entry;
        });

      auto label = WGPU_UTF8_OPT(desc, label, "");
      WGPUBindGroupLayoutDescriptor descriptor = {
        .label = label.data(),
        .entryCount = entries.size(),
        .entries = entries.data(),
      };
      auto layout = wgpuDeviceCreateBindGroupLayout(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<BindGroupLayoutHostObject>(layout, _context, std::move(label)));
    });
  }

  if (name == "createPipelineLayout") {
    return WGPU_FUNC_FROM_HOST_FUNC(createPipelineLayout, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createPipelineLayout);
      auto desc = arguments[0].asObject(runtime);
      auto layouts = jsiArrayToVector<WGPUBindGroupLayout>(
        runtime, WGPU_ARRAY(desc, bindGroupLayouts), [](Runtime &runtime, Value value) {
          return value.asObject(runtime).asHostObject<BindGroupLayoutHostObject>(runtime)->getValue();
        });
      auto label = WGPU_UTF8_OPT(desc, label, "");
      WGPUPipelineLayoutDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
        .bindGroupLayoutCount = layouts.size(),
        .bindGroupLayouts = layouts.data(),
      };
      auto layout = wgpuDeviceCreatePipelineLayout(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<PipelineLayoutHostObject>(layout, _context, std::move(label)));
    });
  }

  if (name == "features") {
    auto size = wgpuDeviceEnumerateFeatures(getValue(), nullptr);
    std::vector<WGPUFeatureName> features;
    features.resize(size);
    wgpuDeviceEnumerateFeatures(getValue(), features.data());
    return makeJsiFeatures(runtime, &features);
  }

  if (name == "limits") {
    WGPUSupportedLimits limits = {nullptr};
    wgpuDeviceGetLimits(getValue(), &limits);
    return makeJsiLimits(runtime, &limits.limits);
  }

  if (name == "createQuerySet") {
    return WGPU_FUNC_FROM_HOST_FUNC(createQuerySet, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(createQuerySet);
      auto desc = arguments[0].asObject(runtime);
      auto type = WGPU_UTF8(desc, type);
      auto label = WGPU_UTF8_OPT(desc, label, "");
      WGPUQuerySetDescriptor descriptor = {
        .nextInChain = nullptr,
        .label = label.data(),
        .type = StringToWGPUQueryType(type.data()),
        .count = WGPU_NUMBER(desc, count, uint32_t),
      };
      auto querySet = wgpuDeviceCreateQuerySet(getValue(), &descriptor);
      return Object::createFromHostObject(
        runtime, std::make_shared<QuerySetHostObject>(querySet, _context, std::move(std::move(label))));
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> DeviceHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "createRenderPipeline", "createShaderModule", "createCommandEncoder", "queue",
                           "createBuffer", "createTexture", "createBindGroup", "createSampler", "createBindGroupLayout",
                           "createPipelineLayout", "features", "limits", "createQuerySet");
}