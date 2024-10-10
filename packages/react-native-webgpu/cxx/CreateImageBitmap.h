#pragma once

#include <jsi/jsi.h>

#include "JSIInstance.h"
#include "Promise.h"

using namespace facebook::jsi;

namespace wgpu {

Function createImageBitmap(Runtime &runtime, std::shared_ptr<JSIInstance> jsiInstance);

}
