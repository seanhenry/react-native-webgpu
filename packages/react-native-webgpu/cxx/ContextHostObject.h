#pragma once

#include <jsi/jsi.h>

#include "Surface.h"
#include "WGPUContext.h"

using namespace facebook::jsi;

namespace wgpu {

class ContextHostObject : public HostObject {
 public:
  explicit ContextHostObject(std::weak_ptr<Surface> surface) : _surface(surface) {}
  ~ContextHostObject() {}
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;
  inline bool isSurfaceConfigured() { return _configuredContext != nullptr; }
  void throwPendingJSIError();

 private:
  std::weak_ptr<Surface> _surface;
  std::shared_ptr<Surface> _strongSurface;
  std::shared_ptr<WGPUContext> _configuredContext;
  WGPUTexture _texture = nullptr;
};

}  // namespace wgpu
