#pragma once

#include <jsi/jsi.h>

#include <functional>

#include "WGPUJsiUtils.h"
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
  std::vector<Function> animationCallbacks;
  std::vector<Function> animationCallbacksForProcessing;
} WGPUSurfaceCallbackData;
#endif

namespace wgpu {

typedef struct SurfaceSize {
  uint32_t pixelWidth;
  uint32_t pixelHeight;
  float scale;
  float pointWidth;
  float pointHeight;
} SurfaceSize;

class Surface : public std::enable_shared_from_this<Surface> {
 public:
#ifdef ANDROID
  explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, SurfaceSize size, ANativeWindow *window)
    : _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _size(size), _window(window) {}
#else
  explicit Surface(WGPUInstance wgpuInstance, WGPUSurface wgpuSurface, SurfaceSize size)
    : _wgpuInstance(wgpuInstance), _wgpuSurface(wgpuSurface), _size(size) {}
#endif
  ~Surface();
  inline WGPUSurface getWGPUSurface() { return _wgpuSurface; }
  inline WGPUInstance getWGPUInstance() { return _wgpuInstance; }
  inline std::weak_ptr<AdapterWrapper> getUnownedWGPUAdapter() { return _unownedWGPUAdapter; }
  inline void setAdapter(const std::shared_ptr<AdapterWrapper> &adapter) { _unownedWGPUAdapter = adapter; }
  inline void setSize(SurfaceSize size) { _size = size; }
  inline uint32_t getPixelWidth() { return _size.pixelWidth; }
  inline uint32_t getPixelHeight() { return _size.pixelHeight; }
  inline float getScale() { return _size.scale; }
  inline float getPointWidth() { return _size.pointWidth; }
  inline float getPointHeight() { return _size.pointHeight; }
  void requestAnimationFrame(Function fn);
  void createTimer();
  void invalidateTimer();

 private:
  WGPUInstance _wgpuInstance = nullptr;
  WGPUSurface _wgpuSurface = nullptr;
  std::weak_ptr<AdapterWrapper> _unownedWGPUAdapter;
  SurfaceSize _size;
#ifdef ANDROID
  ANativeWindow *_window = nullptr;
  AChoreographer *_choreographer = nullptr;
  WGPUSurfaceCallbackData _callbackData;
#else
  void *_timer = nullptr;
#endif
};

}  // namespace wgpu
