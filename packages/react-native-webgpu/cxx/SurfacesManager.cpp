#include "SurfacesManager.h"

namespace wgpu {

SurfacesManager *SurfacesManager::_instance = new SurfacesManager;

void SurfacesManager::set(std::string &uuid, std::shared_ptr<Surface> surface) {
  std::lock_guard<std::mutex> lock(_mutex);
  _surfaces.insert_or_assign(uuid, surface);
}

void SurfacesManager::remove(std::string &uuid) {
  std::lock_guard<std::mutex> lock(_mutex);
  _surfaces.erase(uuid);
}

std::weak_ptr<Surface> SurfacesManager::get(std::string &uuid) {
  std::lock_guard<std::mutex> lock(_mutex);
  std::weak_ptr<Surface> result;
  auto it = _surfaces.find(uuid);
  if (it != _surfaces.end()) {
    return it->second;
  }
  return result;
}

}  // namespace wgpu
