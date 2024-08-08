#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class PipelineLayoutHostObject : public HostObject {
 public:
  explicit PipelineLayoutHostObject(WGPUPipelineLayout value, std::shared_ptr<WGPUContext> context, std::string label)
    : _value(value), _context(context), _label(label) {}
  ~PipelineLayoutHostObject() { wgpuPipelineLayoutRelease(_value); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline WGPUPipelineLayout getValue() { return _value; }

 private:
  WGPUPipelineLayout _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
