#pragma once

#include <jsi/jsi.h>

#include <memory>

#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

namespace wgpu {

class VideoPlayer : public HostObject {
 public:
  static Object factory(Runtime &runtime) {
    return WGPU_FUNC_FROM_HOST_FUNC(makeVideoPlayer, 1, []) {
      auto url = arguments[0].asString(runtime).utf8(runtime);
      return Object::createFromHostObject(runtime, std::make_shared<VideoPlayer>(url));
    });
  }
  explicit VideoPlayer(const std::string &url);
  ~VideoPlayer();
  void release();
  std::vector<PropNameID> getPropertyNames(Runtime &runtime) override;
  Value get(Runtime &runtime, const PropNameID &name) override;

 private:
  void *videoPlayer = nullptr;
};

}  // namespace wgpu
