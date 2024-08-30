#pragma once

#include "webgpu.h"

namespace wgpu {

// The device's lifetime is managed by the context and the device's host object.
// To ensure it only gets deleted once, and at the right time, we wrap in this class.
class DeviceWrapper {
 public:
  explicit DeviceWrapper(WGPUDevice device) : _device(device) {}
  ~DeviceWrapper() { wgpuDeviceRelease(_device); }
  WGPUDevice _device;
};

// The adapter's lifetime is managed by the context and the adapter's host object.
// To ensure it only gets deleted once, and at the right time, we wrap in this class.
class AdapterWrapper {
 public:
  explicit AdapterWrapper(WGPUAdapter adapter) : _adapter(adapter) {}
  ~AdapterWrapper() { wgpuAdapterRelease(_adapter); }
  WGPUAdapter _adapter;
};

}  // namespace wgpu
