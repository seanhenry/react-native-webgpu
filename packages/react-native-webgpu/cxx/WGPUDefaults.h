#pragma once

#include <jsi/jsi.h>

#include "webgpu.h"

using namespace facebook::jsi;

// Defaults can often be found
// https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API
// https://github.com/gfx-rs/wgpu
namespace wgpu {

WGPUMultisampleState makeDefaultWGPUMultisampleState();
WGPUStencilFaceState makeDefaultWGPUStencilFaceState();
WGPUExtent3D makeDefaultWGPUExtent3D();
WGPUTextureDescriptor makeDefaultWGPUTextureDescriptor(WGPUTextureFormat format);
WGPUSamplerDescriptor makeDefaultSamplerDescriptor();
WGPUImageCopyTexture makeDefaultImageCopyTexture(WGPUTexture texture);
WGPUTextureViewDescriptor makeDefaultWGPUTextureViewDescriptor(Runtime &runtime, Object &desc, WGPUTexture texture);

}  // namespace wgpu
