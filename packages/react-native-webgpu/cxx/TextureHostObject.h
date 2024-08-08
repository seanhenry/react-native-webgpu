#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class TextureHostObject : public HostObject {
 public:
  explicit TextureHostObject(WGPUTexture value, std::shared_ptr<WGPUContext> context, std::string label,
                             bool isSurfaceTexture)
    : _value(value), _context(context), _label(label), _isSurfaceTexture(isSurfaceTexture) {}
  ~TextureHostObject() { release(); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  void release() {
    if (_value != nullptr && !_isSurfaceTexture) {
      wgpuTextureRelease(_value);
      _value = nullptr;
    }
  }
  inline WGPUTexture getValue() { return _value; }

 private:
  WGPUTexture _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
  bool _isSurfaceTexture;
};

}  // namespace wgpu
