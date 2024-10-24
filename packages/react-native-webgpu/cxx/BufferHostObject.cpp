#include "BufferHostObject.h"

#include <thread>

#include "ArrayBufferUtils.h"
#include "ConstantConversion.h"
#include "JSIInstance.h"
#include "WGPUContext.h"
#include "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

static void wgpuHandleMapAsync(WGPUBufferMapAsyncStatus status, void *userdata);
typedef struct HandleMapAsyncData {
  volatile bool *isReady;
  std::shared_ptr<WGPUContext> context;
} HandleMapAsyncData;

Value BufferHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "getMappedRange") {
    return WGPU_FUNC_FROM_HOST_FUNC(getMappedRange, 2, [this]) {
      WGPU_LOG_FUNC_ARGS(getMappedRange);
      auto offset = (size_t)count > 0 ? arguments[0].asNumber() : 0;
      auto size = (size_t)count > 1 ? arguments[1].asNumber() : wgpuBufferGetSize(_value) - offset;
      auto range = wgpuBufferGetMappedRange(_value, offset, size);
      _context->getErrorHandler()->throwPendingJSIError();
      return createUnownedArrayBuffer(runtime, range, size);
    });
  }

  if (name == "unmap") {
    return WGPU_FUNC_FROM_HOST_FUNC(unmap, 0, [this]) {
      WGPU_LOG_FUNC_ARGS(unmap);
      wgpuBufferUnmap(_value);
      return Value::undefined();
    });
  }

  if (name == "mapAsync") {
    return WGPU_FUNC_FROM_HOST_FUNC(mapAsync, 3, [this]) {
      WGPU_LOG_FUNC_ARGS(mapAsync);
      auto mode = (WGPUMapModeFlags)arguments[0].asNumber();
      uint64_t offset = count > 1 ? (uint64_t)arguments[1].asNumber() : 0;
      uint64_t size = count > 2 ? (uint64_t)arguments[2].asNumber() : (wgpuBufferGetSize(_value) - offset);

      return Promise::makeJSPromise(_context->getJSIInstance(),
                                    [this, mode, offset, size](auto &runtime, auto &promise) {
        volatile bool isReady = false;
        const HandleMapAsyncData data = {
          .isReady = &isReady,
          .context = _context,
        };
        wgpuBufferMapAsync(_value, mode, offset, size, wgpuHandleMapAsync, promise->toCDataWithExtras(data));
        auto thread = std::thread([this, &isReady]() {
          while (!isReady) {
            _context->poll(true);
          }
        });
        thread.detach();
      });
    });
  }

  if (name == "label") {
    return String::createFromUtf8(runtime, _label);
  }

  if (name == "size") {
    return Value((int)wgpuBufferGetSize(_value));
  }

  if (name == "usage") {
    return Value((int)wgpuBufferGetUsage(_value));
  }

  // Not implemented
  //  if (name == "mapState") {
  //    return String::createFromUtf8(runtime, WGPUBufferMapStateToString(wgpuBufferGetMapState(_value)));
  //  }

  if (name == "destroy") {
    return WGPU_FUNC_FROM_HOST_FUNC(destroy, 0, [this]) {
      wgpuBufferDestroy(_value);
      return Value::undefined();
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> BufferHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "getMappedRange", "unmap", "mapAsync", "label", "size", "usage", "destroy");
}

static std::string mapAsyncStatusToString(WGPUBufferMapAsyncStatus status) {
  switch (status) {
    case WGPUBufferMapAsyncStatus_DeviceLost:
      return "DeviceLost";
    case WGPUBufferMapAsyncStatus_ValidationError:
      return "ValidationError";
    case WGPUBufferMapAsyncStatus_SizeOutOfRange:
      return "SizeOutOfRange";
    case WGPUBufferMapAsyncStatus_OffsetOutOfRange:
      return "OffsetOutOfRange";
    case WGPUBufferMapAsyncStatus_MappingAlreadyPending:
      return "MappingAlreadyPending";
    case WGPUBufferMapAsyncStatus_UnmappedBeforeCallback:
      return "UnmappedBeforeCallback";
    case WGPUBufferMapAsyncStatus_DestroyedBeforeCallback:
      return "DestroyedBeforeCallback";
    case WGPUBufferMapAsyncStatus_Success:
      return "Success";
    case WGPUBufferMapAsyncStatus_Unknown:
    default:
      return "unknown";
  }
}

static void wgpuHandleMapAsync(WGPUBufferMapAsyncStatus status, void *userdata) {
  Promise::fromCDataWithExtras<HandleMapAsyncData>(userdata, [status](auto &promise, auto data) {
    *data.isReady = true;
    if (status == WGPUBufferMapAsyncStatus_Success) {
      promise->resolve([](auto &runtime) { return Value::undefined(); });
    } else {
      auto message = "GPUBuffer.mapAsync error: " + mapAsyncStatusToString(status);
      promise->reject([message = std::move(message)](auto &runtime) { return makeJSError(runtime, message); });
    }
  });
}
