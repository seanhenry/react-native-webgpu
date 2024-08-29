#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class RenderPassEncoderHostObject : public HostObject {
 public:
  explicit RenderPassEncoderHostObject(WGPURenderPassEncoder value, std::shared_ptr<WGPUContext> context,
                                       std::string label)
    : _value(value), _context(context), _label(label) {}
  ~RenderPassEncoderHostObject() { release(); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  void release() {
    if (_value != nullptr) {
      wgpuRenderPassEncoderRelease(_value);
      _value = nullptr;
    }
  }

 private:
  WGPURenderPassEncoder _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
