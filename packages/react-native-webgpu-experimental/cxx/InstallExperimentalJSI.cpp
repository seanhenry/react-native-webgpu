#include "InstallExperimentalJSI.h"

#include "Compression.h"
#include "VideoPlayer.h"

namespace wgpu {
void installExperimentalJSI(Runtime& runtime, std::shared_ptr<JSIInstance> jsiInstance) {
  auto experimental = Object(runtime);
  experimental.setProperty(runtime, "inflate", inflate(runtime));
  experimental.setProperty(runtime, "makeVideoPlayer", VideoPlayer::factory(runtime));
  runtime.global().setProperty(runtime, "reactNativeWebGPUExperimental", std::move(experimental));
}
}  // namespace wgpu
