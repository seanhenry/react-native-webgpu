#pragma once

#include <jsi/jsi.h>

#include "WGPUContext.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class ComputePipelineHostObject : public HostObject {
 public:
  explicit ComputePipelineHostObject(WGPUComputePipeline value, std::shared_ptr<WGPUContext> context, std::string label)
    : _value(value), _context(context), _label(label) {}
  ~ComputePipelineHostObject() { wgpuComputePipelineRelease(_value); }
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline WGPUComputePipeline getValue() { return _value; }

 private:
  WGPUComputePipeline _value;
  std::shared_ptr<WGPUContext> _context;
  std::string _label;
};

}  // namespace wgpu
