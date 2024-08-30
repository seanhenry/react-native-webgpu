#pragma once

#include <jsi/jsi.h>

#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

namespace wgpu {

class UnownedMutableBuffer : public MutableBuffer {
 public:
  UnownedMutableBuffer(void *data, size_t size) : _data(data), _size(size) {}
  ~UnownedMutableBuffer() {}
  size_t size() const override { return _size; }
  uint8_t *data() override { return (uint8_t *)_data; }

 private:
  void *_data;
  size_t _size;
};

class OwnedMutableBuffer : public MutableBuffer {
 public:
  OwnedMutableBuffer(void *data, size_t size) : _data(data), _size(size) {}
  ~OwnedMutableBuffer() { free(_data); }
  size_t size() const override { return _size; }
  uint8_t *data() override { return (uint8_t *)_data; }

 private:
  void *_data;
  size_t _size;
};

class OwnedVectorMutableBuffer : public MutableBuffer {
 public:
  OwnedVectorMutableBuffer(std::vector<uint8_t> &&vec) : _vec(std::move(vec)) {}
  size_t size() const override { return _vec.size(); }
  uint8_t *data() override { return _vec.data(); }

 private:
  std::vector<uint8_t> _vec;
};

inline ArrayBuffer createUnownedArrayBuffer(Runtime &runtime, void *bytes, size_t size) {
  auto buffer = std::make_shared<UnownedMutableBuffer>(bytes, size);
  return ArrayBuffer(runtime, buffer);
}

inline ArrayBuffer createOwnedArrayBuffer(Runtime &runtime, void *bytes, size_t size) {
  auto buffer = std::make_shared<OwnedMutableBuffer>(bytes, size);
  return ArrayBuffer(runtime, buffer);
}

inline ArrayBuffer createOwnedVectorArrayBuffer(Runtime &runtime, std::vector<uint8_t> &&vec) {
  auto buffer = std::make_shared<OwnedVectorMutableBuffer>(std::move(vec));
  return ArrayBuffer(runtime, buffer);
}

typedef struct ArrayBufferView {
  size_t byteOffset;
  size_t byteLength;
  ArrayBuffer arrayBuffer;
} ArrayBufferView;

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
