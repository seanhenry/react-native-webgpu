#pragma once

#include <memory>
#include <shared_mutex>

namespace wgpu {

struct SurfaceSize {
  uint32_t pixelWidth;
  uint32_t pixelHeight;
  float scale;
  float pointWidth;
  float pointHeight;
};

/**
 On iOS, we cannot request the size on the JS thread because UIKit is main thread access only.
 Instead, we push the surface size on the UI thread when it changes.
 */
class PushSurfaceSize : public SurfaceSize {
 public:
  PushSurfaceSize(SurfaceSize size) : _size(std::move(size)) {}
  void setSize(SurfaceSize size) {
    std::unique_lock<std::shared_mutex> lock(_mutex);
    _size = std::move(size);
  }
  inline uint32_t getPixelWidth() {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _size.pixelWidth;
  }
  inline uint32_t getPixelHeight() {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _size.pixelHeight;
  }
  inline float getScale() {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _size.scale;
  }
  inline float getPointWidth() {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _size.pointWidth;
  }
  inline float getPointHeight() {
    std::shared_lock<std::shared_mutex> lock(_mutex);
    return _size.pointHeight;
  }

 private:
  SurfaceSize _size;
  mutable std::shared_mutex _mutex;
};

#ifdef ANDROID

#include <android/native_window_jni.h>

/**
 On Android, we can request the surface size directly.
 */
class PullSurfaceSize : public SurfaceSize {
 public:
  PullSurfaceSize(ANativeWindow *window, float density) : _window(window), _density(density) {
    ANativeWindow_acquire(window);
  }
  ~PullSurfaceSize() { ANativeWindow_release(_window); }
  inline uint32_t getPixelWidth() { return (uint32_t)ANativeWindow_getWidth(_window); }
  inline uint32_t getPixelHeight() { return (uint32_t)ANativeWindow_getHeight(_window); }
  inline float getScale() { return _density; }
  inline float getPointWidth() { return (float)getPixelWidth() / _density; }
  inline float getPointHeight() { return (float)getPixelHeight() / _density; }

 private:
  ANativeWindow *_window;
  float _density;
};

#endif

}  // namespace wgpu
