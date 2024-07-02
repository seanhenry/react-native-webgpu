#pragma once
#include <jsi/jsi.h>
#include "webgpu.h"
#include "AutoReleasePool.h"

using namespace facebook::jsi;

typedef std::shared_ptr<std::vector<WGPUConstantEntry>> ConstantEntries;

namespace wgpu {

WGPUDepthStencilState makeWGPUDepthStencilState(Runtime &runtime, Object obj);
WGPUPrimitiveState makeWGPUPrimitiveState(Runtime &runtime, Object obj);
WGPUVertexAttribute makeWGPUVertexAttribute(Runtime &runtime, Value value);
WGPUVertexBufferLayout makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool *pool, Value value);
ConstantEntries makeWGPUConstantEntries(Runtime &runtime, AutoReleasePool *autoReleasePool, Object &obj);
WGPUVertexState makeGPUVertexState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj);
WGPUFragmentState makeGPUFragmentState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj);
WGPUProgrammableStageDescriptor makeWGPUProgrammableStageDescriptor(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj);
WGPUExtent3D makeGPUExtent3D(Runtime &runtime, Object obj);
void makeWGPUBindingResource(Runtime &runtime, Value value, WGPUBindGroupEntry *entry);
WGPUColor makeWGPUColor(Runtime &runtime, Value &value);
WGPUColor makeWGPUColorFromProp(Runtime &runtime, Object &obj, const char *propName);
WGPUImageCopyTexture makeWGPUImageCopyTexture(Runtime &runtime, Object obj);
WGPUOrigin3D makeWGPUOrigin3D(Runtime &runtime, Object obj);

}
