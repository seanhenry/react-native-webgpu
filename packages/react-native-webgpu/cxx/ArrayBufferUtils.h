#pragma once

#include <jsi/jsi.h>

#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

namespace wgpu {

inline ArrayBufferView getArrayBufferViewFromArrayBufferLike(Runtime &runtime, const Object &arrayBufferLike) {
  if (arrayBufferLike.isArrayBuffer(runtime)) {
    auto arrayBuffer = arrayBufferLike.getArrayBuffer(runtime);
    return {
      .byteOffset = 0,
      .byteLength = arrayBuffer.size(runtime),
      .arrayBuffer = std::move(arrayBuffer),
    };
  } else if (arrayBufferLike.hasProperty(runtime, "buffer")) {  // TypedArray
    return {
      .byteOffset = WGPU_NUMBER(arrayBufferLike, byteOffset, size_t),
      .byteLength = WGPU_NUMBER(arrayBufferLike, byteLength, size_t),
      .arrayBuffer = WGPU_OBJ(arrayBufferLike, buffer).getArrayBuffer(runtime),
    };
  }
  throw new JSError(runtime, "Unsupported ArrayBufferLike object");
}

}  // namespace wgpu
