#include "BufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"

using namespace facebook::jsi;
using namespace wgpu;

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

    return Value::undefined();
}

std::vector<PropNameID> BufferHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "getMappedRange", "unmap");
}

