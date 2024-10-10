#pragma once
#include <jsi/jsi.h>

#include "Surface.h"

using namespace facebook::jsi;

namespace wgpu {

void installRootJSI(Runtime& runtime, std::shared_ptr<JSIInstance> jsiInstance);

}
