#include "Thread.h"

#include <ReactCommon/CallInvokerHolder.h>

using namespace wgpu;

void Thread::run(std::function<void()> &&fn) { _invoker->invokeAsync(std::move(fn)); }
