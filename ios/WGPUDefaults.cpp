#include "WGPUDefaults.h"

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
        .format = format,
        .label = NULL,
        .size = makeDefaultWGPUExtent3D(),
        .mipLevelCount = 1,
        .sampleCount = 1,
        .dimension = WGPUTextureDimension_2D,
        .usage = 0,
        .viewFormats = NULL,
        .viewFormatCount = 0,
    };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUDevice/createSampler#parameters
WGPUSamplerDescriptor wgpu::makeDefaultSamplerDescriptor() {
    return {
        .addressModeU = WGPUAddressMode_ClampToEdge,
        .addressModeV = WGPUAddressMode_ClampToEdge,
        .addressModeW = WGPUAddressMode_ClampToEdge,
        .compare = WGPUCompareFunction_Undefined,
        .lodMinClamp = 0,
        .lodMaxClamp = 32,
        .maxAnisotropy = 1,
        .magFilter = WGPUFilterMode_Nearest,
        .minFilter = WGPUFilterMode_Nearest,
        .mipmapFilter = WGPUMipmapFilterMode_Nearest,
    };
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUQueue/copyExternalImageToTexture#destination
WGPUImageCopyTexture wgpu::makeDefaultImageCopyTexture(WGPUTexture texture) {
    return {
        .aspect = WGPUTextureAspect_All,
        .origin = (WGPUOrigin3D){0},
        .mipLevel = 0,
        .texture = texture,
    };
}
