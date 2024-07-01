#pragma once
#include <jsi/jsi.h>
#include "webgpu.h"
#include "AutoReleasePool.h"

using namespace facebook::jsi;

namespace wgpu {

WGPUDepthStencilState makeWGPUDepthStencilState(Runtime &runtime, Object obj);
WGPUPrimitiveState makeWGPUPrimitiveState(Runtime &runtime, Object obj);
WGPUVertexAttribute makeWGPUVertexAttribute(Runtime &runtime, Value value);
WGPUVertexBufferLayout makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool *pool, Value value);
WGPUVertexState makeGPUVertexState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj);
WGPUFragmentState makeGPUFragmentState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj);
WGPUExtent3D makeGPUExtent3D(Runtime &runtime, Object obj);
void makeWGPUBindingResource(Runtime &runtime, Value value, WGPUBindGroupEntry *entry);
WGPUColor makeWGPUColor(Runtime &runtime, Value &value);
WGPUColor makeWGPUColorFromProp(Runtime &runtime, Object &obj, const char *propName);

}
