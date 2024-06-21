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
