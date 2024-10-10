#pragma once

#include <jsi/jsi.h>

#include "JSIInstance.h"
#include "WGPUWrappers.h"

using namespace facebook::jsi;

namespace wgpu {

class AdapterHostObject : public HostObject {
 public:
  explicit AdapterHostObject(std::shared_ptr<AdapterWrapper> adapter, std::shared_ptr<JSIInstance> jsiInstance)
    : _adapter(adapter), _jsiInstance(jsiInstance) {}
  ~AdapterHostObject() {}
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  std::shared_ptr<AdapterWrapper> _adapter;

 private:
  std::shared_ptr<JSIInstance> _jsiInstance;
};

}  // namespace wgpu
