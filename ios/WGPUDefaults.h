#pragma once

#include "wgpu.h"

// Defaults can often be found
// https://developer.mozilla.org/en-US/docs/Web/API/WebGPU_API
// https://github.com/gfx-rs/wgpu
namespace wgpu {

WGPUMultisampleState makeDefaultWGPUMultisampleState();
WGPUStencilFaceState makeDefaultWGPUStencilFaceState();
WGPUExtent3D makeDefaultWGPUExtent3D();
WGPUTextureDescriptor makeDefaultWGPUTextureDescriptor(WGPUTextureFormat format);

}
