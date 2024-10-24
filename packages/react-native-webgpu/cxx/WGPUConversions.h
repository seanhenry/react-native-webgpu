#pragma once
#include <jsi/jsi.h>

#include "AutoReleasePool.h"
#include "webgpu.h"

using namespace facebook::jsi;

namespace wgpu {

typedef std::shared_ptr<std::vector<WGPUConstantEntry>> ConstantEntries;

WGPUDepthStencilState makeWGPUDepthStencilState(Runtime &runtime, const Object &obj);
WGPUVertexAttribute makeWGPUVertexAttribute(Runtime &runtime, const Value &value);
WGPUVertexBufferLayout makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool &pool, const Value &value);
ConstantEntries makeWGPUConstantEntries(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj);
WGPUVertexState makeGPUVertexState(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj);
WGPUFragmentState makeGPUFragmentState(Runtime &runtime, AutoReleasePool &autoReleasePool, const Object &obj);
WGPUProgrammableStageDescriptor makeWGPUProgrammableStageDescriptor(Runtime &runtime, AutoReleasePool &autoReleasePool,
                                                                    const Object &obj);
WGPUExtent3D makeGPUExtent3D(Runtime &runtime, const Object &obj);
void addWGPUBindingResource(Runtime &runtime, const Value &value, WGPUBindGroupEntry &entry);
WGPUColor makeWGPUColor(Runtime &runtime, const Value &value);
WGPUColor makeWGPUColorFromProp(Runtime &runtime, const Object &obj, const char *propName);
WGPUImageCopyTexture makeWGPUImageCopyTexture(Runtime &runtime, const Object &obj);
WGPUImageCopyBuffer makeWGPUImageCopyBuffer(Runtime &runtime, const Object &obj, const WGPUExtent3D &extent);
WGPUOrigin3D makeWGPUOrigin3D(Runtime &runtime, const Object &obj);
Value makeJsiFeatures(Runtime &runtime, const std::vector<WGPUFeatureName> &features);
WGPUComputePassTimestampWrites makeWGPUComputePassTimestampWrites(Runtime &runtime, const Object &obj);
WGPURenderPassTimestampWrites makeWGPURenderPassTimestampWrites(Runtime &runtime, const Object &obj);
WGPUTextureDataLayout makeWGPUTextureDataLayout(Runtime &runtime, const Object &obj, const WGPUExtent3D &extent);
WGPUBlendState makeGPUBlendState(Runtime &runtime, Object &obj);
WGPUBlendComponent makeGPUBlendComponent(Runtime &runtime, Object &obj);

Value makeJsiLimits(Runtime &runtime, const WGPULimits &limits);
WGPULimits makeWGPULimits(Runtime &runtime, const Object &limits, WGPUSupportedLimits &supporedLimits);

}  // namespace wgpu
