#include <jsi/jsi.h>

#include <memory>

#include "JSIInstance.h"

using namespace facebook::jsi;

namespace wgpu {
void installExperimentalJSI(Runtime& runtime, std::shared_ptr<JSIInstance> jsiInstance);
}
