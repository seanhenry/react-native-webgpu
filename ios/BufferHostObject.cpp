#include "BufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include <boost/format.hpp>

using namespace facebook::jsi;
using namespace wgpu;

static void handle_map_async(WGPUBufferMapAsyncStatus status, void *userdata);

Value BufferHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "getMappedRange") {
        return WGPU_FUNC_FROM_HOST_FUNC(getMappedRange, 2, [this]) {
            auto offset = (size_t)count > 0 ? arguments[0].asNumber() : 0;
            auto size = (size_t)count > 1 ? arguments[1].asNumber() : wgpuBufferGetSize(_value);
            auto range = wgpuBufferGetMappedRange(_value, offset, size);
            
            return createSharedArrayBuffer(runtime, range, size);
        });
    }

    if (name == "unmap") {
        return WGPU_FUNC_FROM_HOST_FUNC(unmap, 0, [this]) {
            wgpuBufferUnmap(_value);
            return Value::undefined();
        });
    }

    if (name == "mapAsync") {
        return WGPU_FUNC_FROM_HOST_FUNC(mapAsync, 3, [this]) {
            auto buffer = _value;
            auto device = _context->_device;
            WGPUMapModeFlags mode = (WGPUMapModeFlags)arguments[0].asNumber();
            uint64_t offset = count > 1 ? (uint64_t)arguments[1].asNumber() : 0;
            uint64_t size = count > 2 ? (uint64_t)arguments[2].asNumber() : (wgpuBufferGetSize(_value) - offset);
            return makePromise(runtime, [buffer, mode, offset, size, device](Promise *promise) {
                wgpuBufferMapAsync(buffer, mode, offset, size, handle_map_async, promise);
                wgpuDevicePoll(device, true, NULL);
            });
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> BufferHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "getMappedRange", "unmap", "mapAsync");
}

static std::string mapAsyncStatusToString(WGPUBufferMapAsyncStatus status) {
    switch (status) {
        case WGPUBufferMapAsyncStatus_DeviceLost: return "DeviceLost";
        case WGPUBufferMapAsyncStatus_ValidationError: return "ValidationError";
        case WGPUBufferMapAsyncStatus_SizeOutOfRange: return "SizeOutOfRange";
        case WGPUBufferMapAsyncStatus_OffsetOutOfRange: return "OffsetOutOfRange";
        case WGPUBufferMapAsyncStatus_MappingAlreadyPending: return "MappingAlreadyPending";
        case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback: return "UnmappedBeforeCallback";
        case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback: return "DestroyedBeforeCallback";
        case WGPUBufferMapAsyncStatus_Success: return "Success";
        case WGPUBufferMapAsyncStatus_Unknown:
        default: return "unknown";
    }
}

static void handle_map_async(WGPUBufferMapAsyncStatus status, void *userdata) {
    auto promise = (Promise *)userdata;
    if (status == WGPUBufferMapAsyncStatus_Success) {
        promise->resolve->call(promise->runtime, Value::undefined());
    } else {
        auto messageFmt = boost::format("GPUBuffer.mapAsync error: %s") % mapAsyncStatusToString(status);
        auto error = makeJSError(promise->runtime, messageFmt.str());
        promise->reject->call(promise->runtime, std::move(error));
    }
    delete promise;
}
