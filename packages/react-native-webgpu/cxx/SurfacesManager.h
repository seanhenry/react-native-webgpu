#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "Surface.h"

namespace wgpu {

class SurfacesManager {
 public:
  static inline SurfacesManager *getInstance() { return _instance; }
  void set(std::string &uuid, std::shared_ptr<Surface> surface);
  void remove(std::string &uuid);
  std::weak_ptr<Surface> get(std::string &uuid);

 private:
  static SurfacesManager *_instance;
  std::unordered_map<std::string, std::shared_ptr<Surface>> _surfaces;
  std::mutex _mutex;
};

}  // namespace wgpu
