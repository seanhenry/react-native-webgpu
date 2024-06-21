#include "WGPUConversions.h"
#include "ConstantConversion.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"
#include "ShaderModuleHostObject.h"
#include "BufferHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

WGPUDepthStencilState wgpu::makeWGPUDepthStencilState(Runtime &runtime, Object obj) {
    auto format = WGPU_UTF8(obj, format);
    auto depthCompare = WGPU_UTF8_OPT(obj, depthCompare, "always");
    return {
        .format = StringToWGPUTextureFormat(format),
        .depthWriteEnabled = WGPU_BOOL_OPT(obj, depthWriteEnabled, false),
        .depthCompare = StringToWGPUCompareFunction(depthCompare.data()),
        .stencilFront = makeDefaultWGPUStencilFaceState(),
        .stencilBack = makeDefaultWGPUStencilFaceState(),
    };
}

WGPUPrimitiveState wgpu::makeWGPUPrimitiveState(Runtime &runtime, Object obj) {
    WGPUPrimitiveState state = {0};
    auto topology = WGPU_UTF8_OPT(obj, topology, "point-list");
    state.topology = StringToWGPUPrimitiveTopology(topology.data());
    auto cullMode = WGPU_UTF8_OPT(obj, cullMode, "none");
    state.cullMode = StringToWGPUCullMode(cullMode.data());
    return state;
}

WGPUVertexAttribute wgpu::makeWGPUVertexAttribute(Runtime &runtime, Value value) {
    auto obj = value.asObject(runtime);
    auto format = WGPU_UTF8(obj, format);
    return {
        .format = StringToWGPUVertexFormat(format),
        .offset = WGPU_NUMBER(obj, offset, uint64_t),
        .shaderLocation = WGPU_NUMBER(obj, shaderLocation, uint32_t),
    };
}

WGPUVertexBufferLayout wgpu::makeWGPUVertexBufferLayout(Runtime &runtime, AutoReleasePool *pool, Value value) {
    auto obj = value.asObject(runtime);
    auto attributesIn = WGPU_ARRAY(obj, attributes);
    auto attributes = jsiArrayToVector<WGPUVertexAttribute>(runtime, std::move(attributesIn), makeWGPUVertexAttribute);
    auto sharedAttributes = std::make_shared<std::vector<WGPUVertexAttribute>>(attributes);
    pool->add(sharedAttributes);
    auto stepMode = WGPU_UTF8_OPT(obj, stepMode, "vertex");

    return {
        .arrayStride = WGPU_NUMBER(obj, arrayStride, uint64_t),
        .attributes = sharedAttributes->data(),
        .attributeCount = attributes.size(),
        .stepMode = StringToWGPUVertexStepMode(stepMode.data()),
    };
}

WGPUVertexState wgpu::makeGPUVertexState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj) {
    auto vertexModule = WGPU_HOST_OBJ(obj, module, ShaderModuleHostObject);
    auto entryPoint = getUTF8(runtime, autoReleasePool, obj.getProperty(runtime, "entryPoint"));
    WGPUVertexState state = {
        .module = vertexModule->_value,
        .entryPoint = entryPoint->data(),
    };
    if (obj.hasProperty(runtime, "buffers")) {
        auto buffersIn = WGPU_ARRAY(obj, buffers);
        auto buffers = jsiArrayToVector<WGPUVertexBufferLayout>(runtime, std::move(buffersIn), [autoReleasePool](Runtime &runtime, Value value) {
            return makeWGPUVertexBufferLayout(runtime, autoReleasePool, std::move(value));
        });
        auto sharedBuffers = std::make_shared<std::vector<WGPUVertexBufferLayout>>(buffers);
        autoReleasePool->add(sharedBuffers);
        state.buffers = sharedBuffers->data();
        state.bufferCount = buffers.size();
    }
    return state;
}

WGPUFragmentState wgpu::makeGPUFragmentState(Runtime &runtime, AutoReleasePool *autoReleasePool, Object obj) {
    auto targets = jsiArrayToVector<WGPUColorTargetState>(runtime, WGPU_ARRAY(obj, targets), [](Runtime &runtime, Value value) {
        if (value.isNull()) {
            // TODO: handle null state
            return (const WGPUColorTargetState){0};
        }
        auto target = value.asObject(runtime);
        auto format = WGPU_UTF8(target, format);
        return (const WGPUColorTargetState){
            .format = StringToWGPUTextureFormat(format),
            .writeMask = WGPUColorWriteMask_All,
        };
    });
    auto sharedTargets = std::make_shared<std::vector<WGPUColorTargetState>>(targets);
    autoReleasePool->add(sharedTargets);
    auto entryPoint = getUTF8(runtime, autoReleasePool, obj.getProperty(runtime, "entryPoint"));

    return {
        .module = obj.getPropertyAsObject(runtime, "module").asHostObject<ShaderModuleHostObject>(runtime)->_value,
        .entryPoint = entryPoint->data(),
        .targets = sharedTargets->data(),
        .targetCount = sharedTargets->size(),
    };
}

WGPUExtent3D wgpu::makeGPUExtent3D(Runtime &runtime, Object obj) {
    WGPUExtent3D extent = makeDefaultWGPUExtent3D();
    if (obj.isArray(runtime)) {
        auto array = obj.asArray(runtime);
        auto size = array.size(runtime);
        if (size > 0) { extent.width = (uint32_t)array.getValueAtIndex(runtime, 0).asNumber(); }
        if (size > 1) { extent.height = (uint32_t)array.getValueAtIndex(runtime, 1).asNumber(); }
        if (size > 2) { extent.depthOrArrayLayers = (uint32_t)array.getValueAtIndex(runtime, 2).asNumber(); }
    } else {
        extent.width = (uint32_t)obj.getProperty(runtime, "width").asNumber();
        if (obj.hasProperty(runtime, "height")) { extent.height = (uint32_t)obj.getProperty(runtime, "height").asNumber(); }
        // deprecated:
        if (obj.hasProperty(runtime, "depth")) { extent.depthOrArrayLayers = (uint32_t)obj.getProperty(runtime, "depth").asNumber(); }
        if (obj.hasProperty(runtime, "depthOrArrayLayers")) { extent.depthOrArrayLayers = (uint32_t)obj.getProperty(runtime, "depthOrArrayLayers").asNumber(); }
    }
    return extent;
}

void wgpu::makeWGPUBufferBinding(Runtime &runtime, Object obj, WGPUBindGroupEntry *entry) {
    auto buffer = WGPU_HOST_OBJ(obj, buffer, BufferHostObject);
    entry->buffer = buffer->_value;
    entry->offset = WGPU_NUMBER_OPT(obj, offset, size_t, 0);
    entry->size = WGPU_NUMBER_OPT(obj, size, size_t, wgpuBufferGetSize(buffer->_value));
}

void wgpu::makeWGPUBindingResource(Runtime &runtime, Value value, WGPUBindGroupEntry *entry) {
    if (value.isObject()) { // GPUBufferBinding
        makeWGPUBufferBinding(runtime, value.asObject(runtime), entry);
    } else {
        throw JSError(runtime, "Only WGPUBufferBinding is supported");
    }
    // TODO: GPUSampler
    // TODO: GPUTextureView
    // TODO: GPUExternalTexture
}

// https://developer.mozilla.org/en-US/docs/Web/API/GPUCommandEncoder/beginRenderPass#color_attachment_object_structure
WGPUColor wgpu::makeWGPUColor(Runtime &runtime, Value &value) {
    WGPUColor color = {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
    if (wgpu::isArray(runtime, &value)) {
        auto array = value.asObject(runtime).asArray(runtime);
        auto size = array.size(runtime);
        color.r = size > 0 ? array.getValueAtIndex(runtime, 0).asNumber() : 0.0;
        color.g = size > 1 ? array.getValueAtIndex(runtime, 1).asNumber() : 0.0;
        color.b = size > 2 ? array.getValueAtIndex(runtime, 2).asNumber() : 0.0;
        color.a = size > 3 ? array.getValueAtIndex(runtime, 3).asNumber() : 0.0;
    } else if (value.isObject()) {
        auto obj = value.asObject(runtime);
        color.r = obj.getProperty(runtime, "r").asNumber();
        color.g = obj.getProperty(runtime, "g").asNumber();
        color.b = obj.getProperty(runtime, "b").asNumber();
        color.a = obj.getProperty(runtime, "a").asNumber();
    }
    return color;
}

WGPUColor wgpu::makeWGPUColorFromProp(Runtime &runtime, Object &obj, const char *propName) {
    if (obj.hasProperty(runtime, propName)) {
        auto clearValueIn = obj.getProperty(runtime, propName);
        return makeWGPUColor(runtime, clearValueIn);
    }
    return {.r = 0.0, .g = 0.0, .b = 0.0, .a = 0.0};
}
