#pragma once

#include <jsi/jsi.h>

#include "webgpu.h"

// TODO: rename to WGPUConstantConverion (maybe also make it namespaced?)
using namespace facebook::jsi;

const char *WGPUTextureFormatToString(WGPUTextureFormat format);
WGPUTextureFormat StringToWGPUTextureFormat(std::string &format);
const char *WGPUCompositeAlphaModeToString(WGPUCompositeAlphaMode alphaMode);
WGPUCompositeAlphaMode StringToWGPUCompositeAlphaMode(const char *alphaMode);
WGPUPrimitiveTopology StringToWGPUPrimitiveTopology(const char *topology);
WGPULoadOp StringToWGPULoadOp(const char *str);
WGPUStoreOp StringToWGPUStoreOp(const char *str);
WGPUVertexFormat StringToWGPUVertexFormat(std::string &str);
WGPUVertexStepMode StringToWGPUVertexStepMode(const char *str);
WGPUCullMode StringToWGPUCullMode(const char *str);
WGPUCompareFunction StringToWGPUCompareFunction(const char *str);
WGPUFilterMode StringToWGPUFilterMode(const char *str);
WGPUMipmapFilterMode StringToWGPUMipmapFilterMode(const char *str);
WGPUTextureDimension StringToWGPUTextureDimension(const char *str);
const char *WGPUTextureDimensionToString(WGPUTextureDimension dim);
WGPUTextureViewDimension StringToWGPUTextureViewDimension(const char *str);
WGPUTextureAspect StringToWGPUTextureAspect(const char *str);
const char *WGPUFeatureNameToString(WGPUFeatureName name);
WGPUFeatureName StringToWGPUFeatureName(const char *name);
WGPUBufferBindingType StringToWGPUBufferBindingType(const char *name);
WGPUQueryType StringToWGPUQueryType(const char *name);
const char *WGPUQuerySetToString(WGPUQueryType type);
WGPUIndexFormat StringToWGPUIndexFormat(const char *str);
WGPUSamplerBindingType StringToWGPUSamplerBindingType(const char *str);
WGPUTextureSampleType StringToWGPUTextureSampleType(const char *str);
WGPUAddressMode StringToWGPUAddressMode(const char *str);
