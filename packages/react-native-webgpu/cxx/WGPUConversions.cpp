#include "WGPUConversions.h"

#include "BufferHostObject.h"
#include "ConstantConversion.h"
#include "QuerySetHostObject.h"
#include "SamplerHostObject.h"
#include "ShaderModuleHostObject.h"
#include "TextureHostObject.h"
#include "TextureViewHostObject.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

WGPUDepthStencilState wgpu::makeWGPUDepthStencilState(Runtime &runtime, Object obj) {
  auto format = WGPU_UTF8(obj, format);
  auto depthCompare = WGPU_UTF8_OPT(obj, depthCompare, "always");
  return {
    .format = StringToWGPUTextureFormat(format),
    .depthWriteEnabled = WGPU_BOOL_OPT(obj, depthWriteEnabled, false),
    .depthCompare = StringToWGPUCompareFunction(depthCompare.data()),
    .stencilFront = makeDefaultWGPUStencilFaceState(),
    .stencilBack = makeDefaultWGPUStencilFaceState(),
  };
}

WGPUVertexAttribute wgpu::makeWGPUVertexAttribute(Runtime &runtime, Value value) {
  auto obj = value.asObject(runtime);
  auto format = WGPU_UTF8(obj, format);
  return {
    .format = StringToWGPUVertexFormat(format),
    .offset = WGPU_NUMBER(obj, offset, uint64_t),
    .shaderLocation = WGPU_NUMBER(obj, shaderLocation, uint32_t),
  };
}

WGPUVertexBufferLayout wgpu::makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool *pool, Value value) {
  auto obj = value.asObject(runtime);
  auto attributesIn = WGPU_ARRAY(obj, attributes);
  auto attributes = jsiArrayToVector<WGPUVertexAttribute>(runtime, std::move(attributesIn), makeWGPUVertexAttribute);
  auto sharedAttributes = std::make_shared<std::vector<WGPUVertexAttribute>>(attributes);
  pool->add(sharedAttributes);
  auto stepMode = WGPU_UTF8_OPT(obj, stepMode, "vertex");

  return {
    .arrayStride = WGPU_NUMBER(obj, arrayStride, uint64_t),
    .stepMode = StringToWGPUVertexStepMode(stepMode.data()),
    .attributeCount = attributes.size(),
    .attributes = sharedAttributes->data(),
  };
}

ConstantEntries wgpu::makeWGPUConstantEntries(Runtime &runtime, AutoReleasePool *autoReleasePool, Object &obj) {
  ConstantEntries constants;
  if (obj.hasProperty(runtime, "constants")) {
    auto constantsIn = obj.getPropertyAsObject(runtime, "constants");
    auto constantsOut = jsiArrayToVector<WGPUConstantEntry>(
      runtime, constantsIn.getPropertyNames(runtime),
      [constantsIn = std::move(constantsIn), autoReleasePool](Runtime &runtime, Value value) {
        auto key = getUTF8(runtime, autoReleasePool, std::move(value));
        return (WGPUConstantEntry){
          .key = key->data(),
          .value = constantsIn.getProperty(runtime, key->data()).asNumber(),
        };
      });
    constants = std::make_shared<std::vector<WGPUConstantEntry>>(constantsOut);
  }
  autoReleasePool->add(constants);
  return constants;
}

WGPUVertexState wgpu::makeGPUVertexState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj) {
  auto vertexModule = WGPU_HOST_OBJ(obj, module, ShaderModuleHostObject);
  auto entryPoint = getUTF8(runtime, autoReleasePool, obj.getProperty(runtime, "entryPoint"));
  auto constants = makeWGPUConstantEntries(runtime, autoReleasePool, obj);
  WGPUVertexState state = {
    .module = vertexModule->getValue(),
    .entryPoint = entryPoint->data(),
    .constantCount = constants != nullptr ? constants->size() : 0,
    .constants = constants != nullptr ? constants->data() : nullptr,
  };
  if (obj.hasProperty(runtime, "buffers")) {
    auto buffersIn = WGPU_ARRAY(obj, buffers);
    auto buffers = jsiArrayToVector<WGPUVertexBufferLayout>(
      runtime, std::move(buffersIn), [autoReleasePool](Runtime &runtime, Value value) {
        return makeWGPUVertexBufferLayout(runtime, autoReleasePool, std::move(value));
      });
    auto sharedBuffers = std::make_shared<std::vector<WGPUVertexBufferLayout>>(buffers);
    autoReleasePool->add(sharedBuffers);
    state.buffers = sharedBuffers->data();
    state.bufferCount = buffers.size();
  }
  return state;
}

WGPUFragmentState wgpu::makeGPUFragmentState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj) {
  auto targets =
    jsiArrayToVector<WGPUColorTargetState>(runtime, WGPU_ARRAY(obj, targets), [](Runtime &runtime, Value value) {
      if (value.isNull()) {
        // TODO: handle null state
        return (const WGPUColorTargetState){nullptr};
      }
      auto target = value.asObject(runtime);
      auto format = WGPU_UTF8(target, format);
      return (const WGPUColorTargetState){
        .format = StringToWGPUTextureFormat(format),
        .writeMask = WGPU_NUMBER_OPT(target, writeMask, WGPUColorWriteMaskFlags, WGPUColorWriteMask_All),
      };
    });
  auto sharedTargets = std::make_shared<std::vector<WGPUColorTargetState>>(targets);
  autoReleasePool->add(sharedTargets);
  auto entryPoint = getUTF8(runtime, autoReleasePool, obj.getProperty(runtime, "entryPoint"));
  auto constants = makeWGPUConstantEntries(runtime, autoReleasePool, obj);

  return {
    .module = obj.getPropertyAsObject(runtime, "module").asHostObject<ShaderModuleHostObject>(runtime)->getValue(),
    .entryPoint = entryPoint->data(),
    .constantCount = constants != nullptr ? constants->size() : 0,
    .constants = constants != nullptr ? constants->data() : nullptr,
    .targetCount = sharedTargets->size(),
    .targets = sharedTargets->data(),
  };
}

WGPUProgrammableStageDescriptor wgpu::makeWGPUProgrammableStageDescriptor(Runtime &runtime,
                                                                          AutoReleasePool *autoReleasePool,
                                                                          Object obj) {
  auto vertexModule = WGPU_HOST_OBJ(obj, module, ShaderModuleHostObject);
  auto entryPoint = getUTF8(runtime, autoReleasePool, obj.getProperty(runtime, "entryPoint"));
  auto constants = makeWGPUConstantEntries(runtime, autoReleasePool, obj);
  return {
    .module = vertexModule->getValue(),
    .entryPoint = entryPoint->data(),
    .constantCount = constants != nullptr ? constants->size() : 0,
    .constants = constants != nullptr ? constants->data() : nullptr,
  };
}

WGPUExtent3D wgpu::makeGPUExtent3D(Runtime &runtime, Object obj) {
  WGPUExtent3D extent = makeDefaultWGPUExtent3D();
  if (obj.isArray(runtime)) {
    auto array = obj.asArray(runtime);
    auto size = array.size(runtime);
    if (size > 0) {
      extent.width = (uint32_t)array.getValueAtIndex(runtime, 0).asNumber();
    }
    if (size > 1) {
      extent.height = (uint32_t)array.getValueAtIndex(runtime, 1).asNumber();
    }
    if (size > 2) {
      extent.depthOrArrayLayers = (uint32_t)array.getValueAtIndex(runtime, 2).asNumber();
    }
  } else {
    extent.width = (uint32_t)obj.getProperty(runtime, "width").asNumber();
    extent.height = WGPU_NUMBER_OPT(obj, height, uint32_t, extent.height);
    extent.depthOrArrayLayers = WGPU_NUMBER_OPT(obj, depth, uint32_t, extent.depthOrArrayLayers);
    extent.depthOrArrayLayers = WGPU_NUMBER_OPT(obj, depthOrArrayLayers, uint32_t, extent.depthOrArrayLayers);
  }
  return extent;
}

void wgpu::makeWGPUBindingResource(Runtime &runtime, Value value, WGPUBindGroupEntry *entry) {
  if (value.isObject() && value.asObject(runtime).isHostObject<SamplerHostObject>(runtime)) {
    entry->sampler = value.asObject(runtime).asHostObject<SamplerHostObject>(runtime)->getValue();
  } else if (value.isObject() && value.asObject(runtime).isHostObject<TextureViewHostObject>(runtime)) {
    entry->textureView = value.asObject(runtime).asHostObject<TextureViewHostObject>(runtime)->getValue();
  } else if (value.isObject() && value.asObject(runtime).hasProperty(runtime, "buffer")) {
    auto obj = value.asObject(runtime);
    auto buffer = WGPU_HOST_OBJ(obj, buffer, BufferHostObject);
    entry->buffer = buffer->getValue();
    entry->offset = WGPU_NUMBER_OPT(obj, offset, size_t, 0);
    entry->size = WGPU_NUMBER_OPT(obj, size, size_t, wgpuBufferGetSize(buffer->getValue()) - entry->offset);
  } else {
    // TODO: GPUExternalTexture
    throw JSError(runtime, "GPUExternalTexture is not supported");
  }
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUCommandEncoder/beginRenderPass#color_attachment_object_structure
WGPUColor wgpu::makeWGPUColor(Runtime &runtime, Value &value) {
  WGPUColor color = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
  if (wgpu::isArray(runtime, &value)) {
    auto array = value.asObject(runtime).asArray(runtime);
    auto size = array.size(runtime);
    color.r = size > 0 ? array.getValueAtIndex(runtime, 0).asNumber() : 0.0;
    color.g = size > 1 ? array.getValueAtIndex(runtime, 1).asNumber() : 0.0;
    color.b = size > 2 ? array.getValueAtIndex(runtime, 2).asNumber() : 0.0;
    color.a = size > 3 ? array.getValueAtIndex(runtime, 3).asNumber() : 0.0;
  } else if (value.isObject()) {
    auto obj = value.asObject(runtime);
    color.r = obj.getProperty(runtime, "r").asNumber();
    color.g = obj.getProperty(runtime, "g").asNumber();
    color.b = obj.getProperty(runtime, "b").asNumber();
    color.a = obj.getProperty(runtime, "a").asNumber();
  }
  return color;
}

WGPUColor wgpu::makeWGPUColorFromProp(Runtime &runtime, Object &obj, const char *propName) {
  if (obj.hasProperty(runtime, propName)) {
    auto clearValueIn = obj.getProperty(runtime, propName);
    return makeWGPUColor(runtime, clearValueIn);
  }
  return {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
}

WGPUImageCopyTexture wgpu::makeWGPUImageCopyTexture(Runtime &runtime, Object obj) {
  auto textureIn = WGPU_HOST_OBJ(obj, texture, TextureHostObject)->getValue();
  auto textureOut = makeDefaultImageCopyTexture(textureIn);
  if (obj.hasProperty(runtime, "origin")) {
    textureOut.origin = makeWGPUOrigin3D(runtime, WGPU_OBJ(obj, origin));
  }
  if (obj.hasProperty(runtime, "mipLevel")) {
    textureOut.mipLevel = WGPU_NUMBER(obj, mipLevel, uint32_t);
  }
  return textureOut;
}

WGPUImageCopyBuffer wgpu::makeWGPUImageCopyBuffer(Runtime &runtime, Object &obj, WGPUExtent3D *extent) {
  auto bufferIn = WGPU_HOST_OBJ(obj, buffer, BufferHostObject)->getValue();
  WGPUImageCopyBuffer bufferOut = {
    .nextInChain = nullptr,
    .layout = makeWGPUTextureDataLayout(runtime, obj, extent),
    .buffer = bufferIn,
  };
  return bufferOut;
}

WGPUOrigin3D wgpu::makeWGPUOrigin3D(Runtime &runtime, Object obj) {
  WGPUOrigin3D origin = {0};
  if (obj.isArray(runtime)) {
    auto array = obj.asArray(runtime);
    auto size = array.size(runtime);
    origin.x = size > 0 ? (uint32_t)array.getValueAtIndex(runtime, 0).asNumber() : 0;
    origin.y = size > 1 ? (uint32_t)array.getValueAtIndex(runtime, 1).asNumber() : 0;
    origin.z = size > 2 ? (uint32_t)array.getValueAtIndex(runtime, 2).asNumber() : 0;
  } else {
    origin.x = WGPU_NUMBER_OPT(obj, x, uint32_t, 0);
    origin.y = WGPU_NUMBER_OPT(obj, y, uint32_t, 0);
    origin.z = WGPU_NUMBER_OPT(obj, z, uint32_t, 0);
  }
  return origin;
}

Value wgpu::makeJsiLimits(Runtime &runtime, WGPULimits *limits) {
  Object obj(runtime);
  obj.setProperty(runtime, "maxTextureDimension1D", Value((int)limits->maxTextureDimension1D));
  obj.setProperty(runtime, "maxTextureDimension2D", Value((int)limits->maxTextureDimension2D));
  obj.setProperty(runtime, "maxTextureDimension3D", Value((int)limits->maxTextureDimension3D));
  obj.setProperty(runtime, "maxTextureArrayLayers", Value((int)limits->maxTextureArrayLayers));
  obj.setProperty(runtime, "maxBindGroups", Value((int)limits->maxBindGroups));
  obj.setProperty(runtime, "maxBindGroupsPlusVertexBuffers", Value((int)limits->maxBindGroupsPlusVertexBuffers));
  obj.setProperty(runtime, "maxBindingsPerBindGroup", Value((int)limits->maxBindingsPerBindGroup));
  obj.setProperty(runtime, "maxDynamicUniformBuffersPerPipelineLayout",
                  Value((int)limits->maxDynamicUniformBuffersPerPipelineLayout));
  obj.setProperty(runtime, "maxDynamicStorageBuffersPerPipelineLayout",
                  Value((int)limits->maxDynamicStorageBuffersPerPipelineLayout));
  obj.setProperty(runtime, "maxSampledTexturesPerShaderStage", Value((int)limits->maxSampledTexturesPerShaderStage));
  obj.setProperty(runtime, "maxSamplersPerShaderStage", Value((int)limits->maxSamplersPerShaderStage));
  obj.setProperty(runtime, "maxStorageBuffersPerShaderStage", Value((int)limits->maxStorageBuffersPerShaderStage));
  obj.setProperty(runtime, "maxStorageTexturesPerShaderStage", Value((int)limits->maxStorageTexturesPerShaderStage));
  obj.setProperty(runtime, "maxUniformBuffersPerShaderStage", Value((int)limits->maxUniformBuffersPerShaderStage));
  obj.setProperty(runtime, "maxUniformBufferBindingSize", Value((int)limits->maxUniformBufferBindingSize));
  obj.setProperty(runtime, "maxStorageBufferBindingSize", Value((int)limits->maxStorageBufferBindingSize));
  obj.setProperty(runtime, "minUniformBufferOffsetAlignment", Value((int)limits->minUniformBufferOffsetAlignment));
  obj.setProperty(runtime, "minStorageBufferOffsetAlignment", Value((int)limits->minStorageBufferOffsetAlignment));
  obj.setProperty(runtime, "maxVertexBuffers", Value((int)limits->maxVertexBuffers));
  obj.setProperty(runtime, "maxBufferSize", Value((int)limits->maxBufferSize));
  obj.setProperty(runtime, "maxVertexAttributes", Value((int)limits->maxVertexAttributes));
  obj.setProperty(runtime, "maxVertexBufferArrayStride", Value((int)limits->maxVertexBufferArrayStride));
  obj.setProperty(runtime, "maxInterStageShaderComponents", Value((int)limits->maxInterStageShaderComponents));
  obj.setProperty(runtime, "maxInterStageShaderVariables", Value((int)limits->maxInterStageShaderVariables));
  obj.setProperty(runtime, "maxColorAttachments", Value((int)limits->maxColorAttachments));
  obj.setProperty(runtime, "maxColorAttachmentBytesPerSample", Value((int)limits->maxColorAttachmentBytesPerSample));
  obj.setProperty(runtime, "maxComputeWorkgroupStorageSize", Value((int)limits->maxComputeWorkgroupStorageSize));
  obj.setProperty(runtime, "maxComputeInvocationsPerWorkgroup", Value((int)limits->maxComputeInvocationsPerWorkgroup));
  obj.setProperty(runtime, "maxComputeWorkgroupSizeX", Value((int)limits->maxComputeWorkgroupSizeX));
  obj.setProperty(runtime, "maxComputeWorkgroupSizeY", Value((int)limits->maxComputeWorkgroupSizeY));
  obj.setProperty(runtime, "maxComputeWorkgroupSizeZ", Value((int)limits->maxComputeWorkgroupSizeZ));
  obj.setProperty(runtime, "maxComputeWorkgroupsPerDimension", Value((int)limits->maxComputeWorkgroupsPerDimension));
  return std::move(obj);
}

// wgpu returns undefined data along with supported features so we need to filter those out
Value wgpu::makeJsiFeatures(Runtime &runtime, std::vector<WGPUFeatureName> *features) {
  std::vector<Value> values;
  values.reserve(features->size());
  for (WGPUFeatureName featureName : *features) {
    auto name = WGPUFeatureNameToString(featureName);
    if (name != nullptr) {
      values.emplace_back(String::createFromUtf8(runtime, name));
    }
  }
  return makeJSSet(runtime, values.data(), values.size());
}

// TODO: figure out defaults https://www.w3.org/TR/webgpu/#timestamp
WGPUComputePassTimestampWrites wgpu::makeWGPUComputePassTimestampWrites(Runtime &runtime, Object obj) {
  return {
    .querySet = WGPU_HOST_OBJ(obj, querySet, QuerySetHostObject)->getValue(),
    .beginningOfPassWriteIndex = WGPU_NUMBER(obj, beginningOfPassWriteIndex, uint32_t),
    .endOfPassWriteIndex = WGPU_NUMBER(obj, endOfPassWriteIndex, uint32_t),
  };
}

// TODO: figure out defaults https://www.w3.org/TR/webgpu/#timestamp
WGPURenderPassTimestampWrites wgpu::makeWGPURenderPassTimestampWrites(Runtime &runtime, Object obj) {
  return {
    .querySet = WGPU_HOST_OBJ(obj, querySet, QuerySetHostObject)->getValue(),
    .beginningOfPassWriteIndex = WGPU_NUMBER(obj, beginningOfPassWriteIndex, uint32_t),
    .endOfPassWriteIndex = WGPU_NUMBER(obj, endOfPassWriteIndex, uint32_t),
  };
}

WGPUTextureDataLayout wgpu::makeWGPUTextureDataLayout(Runtime &runtime, Object &obj, WGPUExtent3D *extent) {
  return {
    .nextInChain = nullptr,
    .offset = WGPU_NUMBER_OPT(obj, offset, uint64_t, 0),
    // TODO: bytesPerRow cannot be optional - extent->width * 8?
    .bytesPerRow = WGPU_NUMBER_OPT(obj, bytesPerRow, uint32_t, 0),
    .rowsPerImage = WGPU_NUMBER_OPT(obj, rowsPerImage, uint32_t, extent->height),
  };
}
