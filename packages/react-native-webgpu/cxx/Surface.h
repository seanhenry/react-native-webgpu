#pragma once

#include <jsi/jsi.h>

#include <functional>
#include <memory>

#include "JSIInstance.h"
#include "SurfaceSize.h"
#include "WGPUWrappers.h"
#include "webgpu.h"

#ifdef ANDROID
#include <android/choreographer.h>
#include <android/native_window_jni.h>
#endif

using namespace facebook::jsi;

namespace wgpu {
class Surface;
}

#ifdef ANDROID
typedef struct WGPUSurfaceCallbackData {
  // Keeps a strong reference to self whilst a frame has been requested
  std::shared_ptr<wgpu::Surface> surface;
  std::shared_ptr<JSIInstance> jsiInstance;
  std::vector<Function> animationCallbacks;
  std::vector<Function> animationCallbacksForProcessing;
} WGPUSurfaceCallbackData;
#endif

namespace wgpu {

class Surface : public std::enable_shared_from_this<Surface> {
 public:
#ifdef ANDROID
  explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, std::shared_ptr<PullSurfaceSize> surfaceSize,
                   ANativeWindow *window)
    : _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _window(window), _surfaceSize(surfaceSize) {
    ANativeWindow_acquire(window);
  }
#else
  explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, std::shared_ptr<PushSurfaceSize> surfaceSize)
    : _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _surfaceSize(surfaceSize) {}
#endif
  ~Surface();
  inline WGPUSurface getWGPUSurface() { return _wgpuSurface; }
  inline WGPUInstance getWGPUInstance() { return _wgpuInstance; }
  inline std::weak_ptr<AdapterWrapper> getUnownedWGPUAdapter() { return _unownedWGPUAdapter; }
  inline void setAdapter(const std::shared_ptr<AdapterWrapper> &adapter) { _unownedWGPUAdapter = adapter; }
  inline uint32_t getPixelWidth() { return _surfaceSize->getPixelWidth(); }
  inline uint32_t getPixelHeight() { return _surfaceSize->getPixelHeight(); }
  inline float getScale() { return _surfaceSize->getScale(); }
  inline float getPointWidth() { return _surfaceSize->getPointWidth(); }
  inline float getPointHeight() { return _surfaceSize->getPointHeight(); }
  void requestAnimationFrame(Function fn);
  void createTimer(std::shared_ptr<JSIInstance> jsiInstance);
  void invalidateTimer();

 private:
  WGPUInstance _wgpuInstance = nullptr;
  WGPUSurface _wgpuSurface = nullptr;
  std::weak_ptr<AdapterWrapper> _unownedWGPUAdapter;
#ifdef ANDROID
  ANativeWindow *_window = nullptr;
  AChoreographer *_choreographer = nullptr;
  WGPUSurfaceCallbackData _callbackData;
  std::shared_ptr<PullSurfaceSize> _surfaceSize;
#else
  void *_timer = nullptr;
  std::shared_ptr<PushSurfaceSize> _surfaceSize;
#endif
};

}  // namespace wgpu
