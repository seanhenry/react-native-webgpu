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

namespace wgpu {

WGPUDepthStencilState makeWGPUDepthStencilState(Runtime &runtime, const Object &obj) {
  WGPUDepthStencilState state{
    .format = StringToWGPUTextureFormat(WGPU_UTF8(obj, format)),
    .depthWriteEnabled = WGPU_BOOL_OPT(obj, depthWriteEnabled, false),
    .depthCompare = StringToWGPUCompareFunction(WGPU_UTF8_OPT(obj, depthCompare, "always")),
    .stencilFront = makeDefaultWGPUStencilFaceState(),
    .stencilBack = makeDefaultWGPUStencilFaceState(),
    .stencilReadMask = WGPU_NUMBER_OPT(obj, stencilReadMask, uint32_t, 0xFFFFFFFF),
    .stencilWriteMask = WGPU_NUMBER_OPT(obj, stencilWriteMask, uint32_t, 0xFFFFFFFF),
    .depthBias = WGPU_NUMBER_OPT(obj, depthBias, int32_t, 0),
    .depthBiasClamp = WGPU_NUMBER_OPT(obj, depthBiasClamp, float, 0),
    .depthBiasSlopeScale = WGPU_NUMBER_OPT(obj, depthBiasSlopeScale, float, 0),
  };
  if (WGPU_HAS_PROP(obj, stencilBack)) {
    auto stencilBack = WGPU_OBJ(obj, stencilBack);
    state.stencilBack = makeWGPUStencilFaceState(runtime, stencilBack);
  }
  if (WGPU_HAS_PROP(obj, stencilFront)) {
    auto stencilFront = WGPU_OBJ(obj, stencilFront);
    state.stencilFront = makeWGPUStencilFaceState(runtime, stencilFront);
  }
  return state;
}

WGPUVertexAttribute makeWGPUVertexAttribute(Runtime &runtime, const Value &value) {
  auto obj = value.asObject(runtime);
  return {
    .format = StringToWGPUVertexFormat(WGPU_UTF8(obj, format)),
    .offset = WGPU_NUMBER(obj, offset, uint64_t),
    .shaderLocation = WGPU_NUMBER(obj, shaderLocation, uint32_t),
  };
}

WGPUVertexBufferLayout makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool &pool, const Value &value) {
  auto obj = value.asObject(runtime);
  auto attributesIn = WGPU_ARRAY(obj, attributes);
  auto attributes = jsiArrayToVector<WGPUVertexAttribute>(runtime, std::move(attributesIn), makeWGPUVertexAttribute);
  auto sharedAttributes = std::make_shared<std::vector<WGPUVertexAttribute>>(attributes);
  pool.add(sharedAttributes);

  return {
    .arrayStride = WGPU_NUMBER(obj, arrayStride, uint64_t),
    .stepMode = StringToWGPUVertexStepMode(WGPU_UTF8_OPT(obj, stepMode, "vertex")),
    .attributeCount = attributes.size(),
    .attributes = sharedAttributes->data(),
  };
}

ConstantEntries makeWGPUConstantEntries(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj) {
  ConstantEntries constants;
  if (obj.hasProperty(runtime, "constants")) {
    auto constantsIn = obj.getPropertyAsObject(runtime, "constants");
    auto constantsOut = jsiArrayToVector<WGPUConstantEntry>(
      runtime, constantsIn.getPropertyNames(runtime),
      [constantsIn = std::move(constantsIn), &autoReleasePool](Runtime &runtime, const Value &value) {
      auto key = autoReleasePool.addString(runtime, value);
      return (WGPUConstantEntry){
        .key = key->data(),
        .value = constantsIn.getProperty(runtime, key->data()).asNumber(),
      };
    });
    constants = std::make_shared<std::vector<WGPUConstantEntry>>(constantsOut);
  }
  autoReleasePool.add(constants);
  return constants;
}

WGPUVertexState makeGPUVertexState(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj) {
  auto vertexModule = WGPU_HOST_OBJ(obj, module, ShaderModuleHostObject);
  auto entryPoint = autoReleasePool.addString(runtime, obj.getProperty(runtime, "entryPoint"));
  auto constants = makeWGPUConstantEntries(runtime, autoReleasePool, obj);
  WGPUVertexState state = {
    .module = vertexModule->getValue(),
    .entryPoint = entryPoint->data(),
    .constantCount = constants != nullptr ? constants->size() : 0,
    .constants = constants != nullptr ? constants->data() : nullptr,
  };
  if (obj.hasProperty(runtime, "buffers")) {
    auto buffersIn = WGPU_ARRAY(obj, buffers);
    auto buffers = jsiArrayToVector<WGPUVertexBufferLayout>(runtime, std::move(buffersIn),
                                                            [&autoReleasePool](Runtime &runtime, const Value &value) {
      return makeWGPUVertexBufferLayout(runtime, autoReleasePool, value);
    });
    auto sharedBuffers = std::make_shared<std::vector<WGPUVertexBufferLayout>>(buffers);
    autoReleasePool.add(sharedBuffers);
    state.buffers = sharedBuffers->data();
    state.bufferCount = buffers.size();
  }
  return state;
}

WGPUFragmentState makeGPUFragmentState(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj) {
  auto targets = jsiArrayToVector<WGPUColorTargetState>(runtime, WGPU_ARRAY(obj, targets),
                                                        [&autoReleasePool](Runtime &runtime, const Value &value) {
    if (value.isNull()) {
      // TODO: handle null state
      return (const WGPUColorTargetState){nullptr};
    }
    auto target = value.asObject(runtime);
    WGPUColorTargetState state = {
      .format = StringToWGPUTextureFormat(WGPU_UTF8(target, format)),
      .writeMask = WGPU_NUMBER_OPT(target, writeMask, WGPUColorWriteMaskFlags, WGPUColorWriteMask_All),
    };
    if (WGPU_HAS_PROP(target, blend)) {
      auto blendIn = WGPU_OBJ(target, blend);
      auto blend = std::make_shared<WGPUBlendState>(makeGPUBlendState(runtime, blendIn));
      autoReleasePool.add(blend);
      state.blend = blend.get();
    }
    return state;
  });
  auto sharedTargets = std::make_shared<std::vector<WGPUColorTargetState>>(targets);
  autoReleasePool.add(sharedTargets);
  auto entryPoint = autoReleasePool.addString(runtime, obj.getProperty(runtime, "entryPoint"));
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

WGPUProgrammableStageDescriptor makeWGPUProgrammableStageDescriptor(Runtime &runtime, AutoReleasePool &autoReleasePool,
                                                                    const Object &obj) {
  auto vertexModule = WGPU_HOST_OBJ(obj, module, ShaderModuleHostObject);
  auto entryPoint = autoReleasePool.addString(runtime, obj.getProperty(runtime, "entryPoint"));
  auto constants = makeWGPUConstantEntries(runtime, autoReleasePool, obj);
  return {
    .module = vertexModule->getValue(),
    .entryPoint = entryPoint->data(),
    .constantCount = constants != nullptr ? constants->size() : 0,
    .constants = constants != nullptr ? constants->data() : nullptr,
  };
}

WGPUExtent3D makeGPUExtent3D(Runtime &runtime, const Object &obj) {
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

void addWGPUBindingResource(Runtime &runtime, const Value &value, WGPUBindGroupEntry &entry) {
  if (value.isObject() && value.asObject(runtime).isHostObject<SamplerHostObject>(runtime)) {
    entry.sampler = value.asObject(runtime).asHostObject<SamplerHostObject>(runtime)->getValue();
  } else if (value.isObject() && value.asObject(runtime).isHostObject<TextureViewHostObject>(runtime)) {
    entry.textureView = value.asObject(runtime).asHostObject<TextureViewHostObject>(runtime)->getValue();
  } else if (value.isObject() && value.asObject(runtime).hasProperty(runtime, "buffer")) {
    auto obj = value.asObject(runtime);
    auto buffer = WGPU_HOST_OBJ(obj, buffer, BufferHostObject);
    entry.buffer = buffer->getValue();
    entry.offset = WGPU_NUMBER_OPT(obj, offset, size_t, 0);
    entry.size = WGPU_NUMBER_OPT(obj, size, size_t, wgpuBufferGetSize(buffer->getValue()) - entry.offset);
  } else {
    // TODO: GPUExternalTexture
    throw JSError(runtime, "GPUExternalTexture is not supported");
  }
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUCommandEncoder/beginRenderPass#color_attachment_object_structure
WGPUColor makeWGPUColor(Runtime &runtime, const Value &value) {
  WGPUColor color = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
  if (isJSIArray(runtime, value)) {
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

WGPUColor makeWGPUColorFromProp(Runtime &runtime, const Object &obj, const char *propName) {
  if (obj.hasProperty(runtime, propName)) {
    auto clearValueIn = obj.getProperty(runtime, propName);
    return makeWGPUColor(runtime, clearValueIn);
  }
  return {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
}

WGPUImageCopyTexture makeWGPUImageCopyTexture(Runtime &runtime, const Object &obj) {
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

WGPUImageCopyBuffer makeWGPUImageCopyBuffer(Runtime &runtime, const Object &obj, const WGPUExtent3D &extent) {
  return {
    .nextInChain = nullptr,
    .layout = makeWGPUTextureDataLayout(runtime, obj, extent),
    .buffer = WGPU_HOST_OBJ(obj, buffer, BufferHostObject)->getValue(),
  };
}

WGPUOrigin3D makeWGPUOrigin3D(Runtime &runtime, const Object &obj) {
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

// wgpu returns undefined data along with supported features so we need to filter those out
Value makeJsiFeatures(Runtime &runtime, const std::vector<WGPUFeatureName> &features) {
  std::vector<Value> values;
  values.reserve(features.size());
  for (WGPUFeatureName featureName : features) {
    auto name = WGPUFeatureNameToString(featureName);
    if (name != nullptr) {
      values.emplace_back(String::createFromUtf8(runtime, name));
    }
  }
  return makeJSSet(runtime, values.data(), values.size());
}

// TODO: figure out defaults https://www.w3.org/TR/webgpu/#timestamp
WGPUComputePassTimestampWrites makeWGPUComputePassTimestampWrites(Runtime &runtime, const Object &obj) {
  return {
    .querySet = WGPU_HOST_OBJ(obj, querySet, QuerySetHostObject)->getValue(),
    .beginningOfPassWriteIndex = WGPU_NUMBER(obj, beginningOfPassWriteIndex, uint32_t),
    .endOfPassWriteIndex = WGPU_NUMBER(obj, endOfPassWriteIndex, uint32_t),
  };
}

// TODO: figure out defaults https://www.w3.org/TR/webgpu/#timestamp
WGPURenderPassTimestampWrites makeWGPURenderPassTimestampWrites(Runtime &runtime, const Object &obj) {
  return {
    .querySet = WGPU_HOST_OBJ(obj, querySet, QuerySetHostObject)->getValue(),
    .beginningOfPassWriteIndex = WGPU_NUMBER(obj, beginningOfPassWriteIndex, uint32_t),
    .endOfPassWriteIndex = WGPU_NUMBER(obj, endOfPassWriteIndex, uint32_t),
  };
}

WGPUTextureDataLayout makeWGPUTextureDataLayout(Runtime &runtime, const Object &obj, const WGPUExtent3D &extent) {
  return {
    .nextInChain = nullptr,
    .offset = WGPU_NUMBER_OPT(obj, offset, uint64_t, 0),
    // TODO: bytesPerRow cannot be optional - extent->width * 8?
    .bytesPerRow = WGPU_NUMBER_OPT(obj, bytesPerRow, uint32_t, 0),
    .rowsPerImage = WGPU_NUMBER_OPT(obj, rowsPerImage, uint32_t, extent.height),
  };
}

WGPUBlendState makeGPUBlendState(Runtime &runtime, Object &obj) {
  auto color = WGPU_OBJ(obj, color);
  auto alpha = WGPU_OBJ(obj, alpha);
  return {
    .color = makeGPUBlendComponent(runtime, color),
    .alpha = makeGPUBlendComponent(runtime, alpha),
  };
}

WGPUBlendComponent makeGPUBlendComponent(Runtime &runtime, Object &obj) {
  auto operation = WGPU_UTF8_OPT(obj, operation, "add");
  auto srcFactor = WGPU_UTF8_OPT(obj, srcFactor, "one");
  auto dstFactor = WGPU_UTF8_OPT(obj, dstFactor, "zero");
  return {
    .operation = StringToWGPUBlendOperation(operation),
    .srcFactor = StringToWGPUBlendFactor(srcFactor),
    .dstFactor = StringToWGPUBlendFactor(dstFactor),
  };
}

WGPUStencilFaceState makeWGPUStencilFaceState(Runtime &runtime, Object &obj) {
  auto compare = WGPU_UTF8_OPT(obj, compare, "always");
  auto failOp = WGPU_UTF8_OPT(obj, failOp, "keep");
  auto depthFailOp = WGPU_UTF8_OPT(obj, depthFailOp, "keep");
  auto passOp = WGPU_UTF8_OPT(obj, passOp, "keep");
  return {
    .compare = StringToWGPUCompareFunction(compare),
    .failOp = StringToWGPUStencilOperation(failOp),
    .depthFailOp = StringToWGPUStencilOperation(depthFailOp),
    .passOp = StringToWGPUStencilOperation(passOp),
  };
}

#define LIMITS                                                  \
  __WGPU_PRINT_LIMIT(maxTextureDimension1D)                     \
  __WGPU_PRINT_LIMIT(maxTextureDimension2D)                     \
  __WGPU_PRINT_LIMIT(maxTextureDimension3D)                     \
  __WGPU_PRINT_LIMIT(maxTextureArrayLayers)                     \
  __WGPU_PRINT_LIMIT(maxBindGroups)                             \
  __WGPU_PRINT_LIMIT(maxBindGroupsPlusVertexBuffers)            \
  __WGPU_PRINT_LIMIT(maxBindingsPerBindGroup)                   \
  __WGPU_PRINT_LIMIT(maxDynamicUniformBuffersPerPipelineLayout) \
  __WGPU_PRINT_LIMIT(maxDynamicStorageBuffersPerPipelineLayout) \
  __WGPU_PRINT_LIMIT(maxSampledTexturesPerShaderStage)          \
  __WGPU_PRINT_LIMIT(maxSamplersPerShaderStage)                 \
  __WGPU_PRINT_LIMIT(maxStorageBuffersPerShaderStage)           \
  __WGPU_PRINT_LIMIT(maxStorageTexturesPerShaderStage)          \
  __WGPU_PRINT_LIMIT(maxUniformBuffersPerShaderStage)           \
  __WGPU_PRINT_LIMIT(maxUniformBufferBindingSize)               \
  __WGPU_PRINT_LIMIT(maxStorageBufferBindingSize)               \
  __WGPU_PRINT_LIMIT(minUniformBufferOffsetAlignment)           \
  __WGPU_PRINT_LIMIT(minStorageBufferOffsetAlignment)           \
  __WGPU_PRINT_LIMIT(maxVertexBuffers)                          \
  __WGPU_PRINT_LIMIT(maxBufferSize)                             \
  __WGPU_PRINT_LIMIT(maxVertexAttributes)                       \
  __WGPU_PRINT_LIMIT(maxVertexBufferArrayStride)                \
  __WGPU_PRINT_LIMIT(maxInterStageShaderComponents)             \
  __WGPU_PRINT_LIMIT(maxInterStageShaderVariables)              \
  __WGPU_PRINT_LIMIT(maxColorAttachments)                       \
  __WGPU_PRINT_LIMIT(maxColorAttachmentBytesPerSample)          \
  __WGPU_PRINT_LIMIT(maxComputeWorkgroupStorageSize)            \
  __WGPU_PRINT_LIMIT(maxComputeInvocationsPerWorkgroup)         \
  __WGPU_PRINT_LIMIT(maxComputeWorkgroupSizeX)                  \
  __WGPU_PRINT_LIMIT(maxComputeWorkgroupSizeY)                  \
  __WGPU_PRINT_LIMIT(maxComputeWorkgroupSizeZ)                  \
  __WGPU_PRINT_LIMIT(maxComputeWorkgroupsPerDimension)

Value makeJsiSupportedLimits(Runtime &runtime, const WGPULimits &limits) {
  Object obj(runtime);
  WGPU_SET_BRAND(obj, GPUSupportedLimits);
#define __WGPU_PRINT_LIMIT(__limit) obj.setProperty(runtime, #__limit, Value((int)limits.__limit));
  LIMITS
#undef __WGPU_PRINT_LIMIT
  return std::move(obj);
}

WGPULimits makeWGPULimits(Runtime &runtime, const Object &limits, WGPUSupportedLimits &supportedLimits) {
  return {
#define __WGPU_PRINT_LIMIT(__limit) \
  .__limit = WGPU_NUMBER_OPT(limits, __limit, uint32_t, supportedLimits.limits.__limit),
    LIMITS
#undef __WGPU_PRINT_LIMIT
  };
}

}  // namespace wgpu
