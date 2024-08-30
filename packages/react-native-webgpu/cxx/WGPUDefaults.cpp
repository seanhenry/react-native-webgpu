#include "WGPUDefaults.h"

#include "ConstantConversion.h"
#include "WGPUJsiUtils.h"

WGPUMultisampleState wgpu::makeDefaultWGPUMultisampleState() {
  return {
    .count = 1,
    .mask = 0xFFFFFFFF,
    .alphaToCoverageEnabled = false,
  };
}

WGPUStencilFaceState wgpu::makeDefaultWGPUStencilFaceState() {
  return {
    .compare = WGPUCompareFunction_Always,
    .failOp = WGPUStencilOperation_Keep,
    .depthFailOp = WGPUStencilOperation_Keep,
    .passOp = WGPUStencilOperation_Keep,
  };
}

WGPUExtent3D wgpu::makeDefaultWGPUExtent3D() {
  return {
    .width = 1,
    .height = 1,
    .depthOrArrayLayers = 1,
  };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUDevice/createTexture#parameters
WGPUTextureDescriptor wgpu::makeDefaultWGPUTextureDescriptor(WGPUTextureFormat format) {
  return {
    .label = nullptr,
    .usage = 0,
    .dimension = WGPUTextureDimension_2D,
    .size = makeDefaultWGPUExtent3D(),
    .format = format,
    .mipLevelCount = 1,
    .sampleCount = 1,
    .viewFormatCount = 0,
    .viewFormats = nullptr,
  };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUDevice/createSampler#parameters
WGPUSamplerDescriptor wgpu::makeDefaultSamplerDescriptor() {
  return {
    .addressModeU = WGPUAddressMode_ClampToEdge,
    .addressModeV = WGPUAddressMode_ClampToEdge,
    .addressModeW = WGPUAddressMode_ClampToEdge,
    .magFilter = WGPUFilterMode_Nearest,
    .minFilter = WGPUFilterMode_Nearest,
    .mipmapFilter = WGPUMipmapFilterMode_Nearest,
    .lodMinClamp = 0,
    .lodMaxClamp = 32,
    .compare = WGPUCompareFunction_Undefined,
    .maxAnisotropy = 1,
  };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUQueue/copyExternalImageToTexture#destination
WGPUImageCopyTexture wgpu::makeDefaultImageCopyTexture(WGPUTexture texture) {
  return {
    .texture = texture,
    .mipLevel = 0,
    .origin = (WGPUOrigin3D){0},
    .aspect = WGPUTextureAspect_All,
  };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUTexture/createView#descriptor
WGPUTextureViewDescriptor wgpu::makeDefaultWGPUTextureViewDescriptor(Runtime &runtime, Object &desc,
                                                                     WGPUTexture texture) {
  auto arrayLayerCount = WGPU_NUMBER_OPT(desc, arrayLayerCount, uint32_t, 0);
  auto aspectStr = WGPU_UTF8_OPT(desc, aspect, "all");
  auto aspect = StringToWGPUTextureAspect(aspectStr);
  auto baseArrayLayer = WGPU_NUMBER_OPT(desc, baseArrayLayer, uint32_t, 0);
  auto baseMipLevel = WGPU_NUMBER_OPT(desc, baseMipLevel, uint32_t, 0);
  auto dimensionStr = WGPU_UTF8_OPT(desc, dimension, "undefined");
  auto dimension = StringToWGPUTextureViewDimension(dimensionStr);
  auto mipLevelCount =
    WGPU_NUMBER_OPT(desc, mipLevelCount, uint32_t, wgpuTextureGetMipLevelCount(texture) - baseMipLevel);
  auto format = WGPUTextureFormat_Undefined;

  // https://developer.mozilla.org/en-US/docs/Web/API/GPUTexture/createView#dimension
  if (dimension == WGPUTextureViewDimension_Undefined) {
    auto texDimension = wgpuTextureGetDimension(texture);
    if (texDimension == WGPUTextureDimension_1D) {
      dimension = WGPUTextureViewDimension_1D;
    } else if (texDimension == WGPUTextureDimension_2D) {
      auto texDepthOrArrayLayers = wgpuTextureGetDepthOrArrayLayers(texture);
      dimension = texDepthOrArrayLayers > 1 ? WGPUTextureViewDimension_2DArray : WGPUTextureViewDimension_2D;
    } else if (texDimension == WGPUTextureDimension_3D) {
      dimension = WGPUTextureViewDimension_3D;
    }
  }

  // https://developer.mozilla.org/en-US/docs/Web/API/GPUTexture/createView#arraylayercount
  if (arrayLayerCount == 0) {
    if (dimension == WGPUTextureViewDimension_1D || dimension == WGPUTextureViewDimension_2D ||
        dimension == WGPUTextureViewDimension_3D) {
      arrayLayerCount = 1;
    } else if (dimension == WGPUTextureViewDimension_Cube) {
      arrayLayerCount = 6;
    } else if (dimension == WGPUTextureViewDimension_2DArray || dimension == WGPUTextureViewDimension_CubeArray) {
      arrayLayerCount = wgpuTextureGetDepthOrArrayLayers(texture) - baseArrayLayer;
    } else {
      throw JSError(runtime, "Unknown dimension");
    }
  }

  if (desc.hasProperty(runtime, "format")) {
    auto formatStr = WGPU_UTF8(desc, format);
    format = StringToWGPUTextureFormat(formatStr);
  } else {
    // https://developer.mozilla.org/en-US/docs/Web/API/GPUTexture/createView#format
    auto texFormat = wgpuTextureGetFormat(texture);
    format = texFormat;
    // TODO: needs checking
    if (aspect == WGPUTextureAspect_DepthOnly || aspect == WGPUTextureAspect_StencilOnly) {
      if (texFormat == WGPUTextureFormat_Depth24PlusStencil8) {
        format = aspect == WGPUTextureAspect_DepthOnly ? WGPUTextureFormat_Depth24Plus : WGPUTextureFormat_Stencil8;
      } else if (texFormat == WGPUTextureFormat_Depth32FloatStencil8) {
        format = aspect == WGPUTextureAspect_DepthOnly ? WGPUTextureFormat_Depth32Float : WGPUTextureFormat_Stencil8;
      }
    }
  }

  return {
    .nextInChain = nullptr,
    .label = nullptr,
    .format = format,
    .dimension = dimension,
    .baseMipLevel = baseMipLevel,
    .mipLevelCount = mipLevelCount,
    .baseArrayLayer = baseArrayLayer,
    .arrayLayerCount = arrayLayerCount,
    .aspect = aspect,
  };
}
