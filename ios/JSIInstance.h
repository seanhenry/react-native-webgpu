#pragma once

#include "Surface.h"
#include "Thread.h"
#include <jsi/jsi.h>

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {

class JSIInstance {
public:
    static std::unique_ptr<JSIInstance> instance;
    explicit JSIInstance(Runtime &rt, std::shared_ptr<Thread> thread): runtime(rt), jsThread(thread) {}
    Runtime &runtime;
    std::shared_ptr<Thread> jsThread;
    std::function<void(std::string, std::shared_ptr<Surface>)> onCreateSurface;
};

}
