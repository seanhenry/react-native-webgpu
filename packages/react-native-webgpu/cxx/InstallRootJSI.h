#pragma once
#include <jsi/jsi.h>
#include <unordered_map>
#include "Surface.h"

using namespace facebook::jsi;

namespace wgpu {

void installRootJSI(Runtime &runtime, const std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Surface>>>& surfaces);

}
