#pragma once

#include <jsi/jsi.h>

#include "WGPUJsiUtils.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

class AdapterHostObject : public HostObject {
 public:
  explicit AdapterHostObject(std::shared_ptr<AdapterWrapper> adapter) : _adapter(adapter) {}
  ~AdapterHostObject() {}
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  std::shared_ptr<AdapterWrapper> _adapter;
};

}  // namespace wgpu
