#pragma once

#include <jsi/jsi.h>

#include "webgpu.h"

// TODO: rename to WGPUConstantConverion (maybe also make it namespaced?)
using namespace facebook::jsi;

const char *WGPUTextureFormatToString(WGPUTextureFormat format);
WGPUTextureFormat StringToWGPUTextureFormat(const std::string &format);
const char *WGPUCompositeAlphaModeToString(WGPUCompositeAlphaMode alphaMode);
WGPUCompositeAlphaMode StringToWGPUCompositeAlphaMode(const std::string &alphaMode);
WGPUPrimitiveTopology StringToWGPUPrimitiveTopology(const std::string &topology);
WGPULoadOp StringToWGPULoadOp(const std::string &str);
WGPUStoreOp StringToWGPUStoreOp(const std::string &str);
WGPUVertexFormat StringToWGPUVertexFormat(const std::string &str);
WGPUVertexStepMode StringToWGPUVertexStepMode(const std::string &str);
WGPUCullMode StringToWGPUCullMode(const std::string &str);
WGPUCompareFunction StringToWGPUCompareFunction(const std::string &str);
WGPUFilterMode StringToWGPUFilterMode(const std::string &str);
WGPUMipmapFilterMode StringToWGPUMipmapFilterMode(const std::string &str);
WGPUTextureDimension StringToWGPUTextureDimension(const std::string &str);
const char *WGPUTextureDimensionToString(WGPUTextureDimension dim);
WGPUTextureViewDimension StringToWGPUTextureViewDimension(const std::string &str);
WGPUTextureAspect StringToWGPUTextureAspect(const std::string &str);
const char *WGPUFeatureNameToString(WGPUFeatureName name);
WGPUFeatureName StringToWGPUFeatureName(const std::string &name);
WGPUBufferBindingType StringToWGPUBufferBindingType(const std::string &name);
WGPUQueryType StringToWGPUQueryType(const std::string &name);
const char *WGPUQuerySetToString(WGPUQueryType type);
WGPUIndexFormat StringToWGPUIndexFormat(const std::string &str);
WGPUSamplerBindingType StringToWGPUSamplerBindingType(const std::string &str);
WGPUTextureSampleType StringToWGPUTextureSampleType(const std::string &str);
WGPUAddressMode StringToWGPUAddressMode(const std::string &str);
const char *WGPUBufferMapStateToString(WGPUBufferMapState state);
WGPUStorageTextureAccess StringToWGPUStorageTextureAccess(const std::string &str);
