#include "Thread.h"
#import "React-callinvoker/ReactCommon/CallInvoker.h"

using namespace wgpu;

void Thread::run(std::function<void()> &&fn) { _invoker->invokeAsync(std::move(fn)); }
