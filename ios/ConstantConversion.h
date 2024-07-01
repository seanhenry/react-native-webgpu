#pragma once

#include <jsi/jsi.h>
#include "wgpu.h"
#include "webgpu.h"

using namespace facebook::jsi;

const char* WGPUTextureFormatToString(WGPUTextureFormat format);
WGPUTextureFormat StringToWGPUTextureFormat(std::string &format);
const char* WGPUCompositeAlphaModeToString(WGPUCompositeAlphaMode alphaMode);
WGPUCompositeAlphaMode StringToWGPUCompositeAlphaMode(const char *alphaMode);
WGPUPrimitiveTopology StringToWGPUPrimitiveTopology(const char *topology);
WGPULoadOp StringToWGPULoadOp(const char *str);
WGPUStoreOp StringToWGPUStoreOp(const char *str);
WGPUVertexFormat StringToWGPUVertexFormat(std::string &str);
WGPUVertexStepMode StringToWGPUVertexStepMode(const char *str);
WGPUCullMode StringToWGPUCullMode(const char *str);
WGPUCompareFunction StringToWGPUCompareFunction(const char *str);
WGPUFilterMode StringToWGPUFilterMode(const char *str);
WGPUTextureDimension StringToWGPUTextureDimension(const char *str);
WGPUTextureViewDimension StringToWGPUTextureViewDimension(const char *str);
WGPUTextureAspect StringToWGPUTextureAspect(const char *str);
