#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class CommandEncoderHostObject : public HostObject {
 public:
  explicit CommandEncoderHostObject(WGPUCommandEncoder value, std::shared_ptr<WGPUContext> context, std::string label)
    : _value(value), _context(context), _label(label) {}
  ~CommandEncoderHostObject() { wgpuCommandEncoderRelease(_value); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;

 private:
  WGPUCommandEncoder _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
