#include "QueueHostObject.h"
#include "CommandBufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "BufferHostObject.h"

using namespace facebook::jsi;
using namespace wgpu;

Value QueueHostObject::get(Runtime &runtime, const PropNameID &propName) {
    auto name = propName.utf8(runtime);

    if (name == "submit") {
        return WGPU_FUNC_FROM_HOST_FUNC(submit, 1, [this]) {
            auto commandsJsi = arguments[0].asObject(runtime).asArray(runtime);
            auto commands = jsiArrayToVector<WGPUCommandBuffer>(runtime, std::move(commandsJsi), [](Runtime &runtime, Value item){
                return item.asObject(runtime).asHostObject<CommandBufferHostObject>(runtime)->_value;
            });
            wgpuQueueSubmit(_value, commands.size(), commands.data());
            return Value::undefined();
        });
    }

    if (name == "writeBuffer") {
        return WGPU_FUNC_FROM_HOST_FUNC(writeBuffer, 5, [this]) {
            auto buffer = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->_value;
            auto bufferOffset = (uint64_t)arguments[1].asNumber();
            auto data = arguments[2].asObject(runtime).getArrayBuffer(runtime);
            auto dataOffset = count > 3 ? (size_t)arguments[3].asNumber() : 0;
            auto size = count > 4 ? (size_t)arguments[4].asNumber() : data.size(runtime);
            wgpuQueueWriteBuffer(
                _value,
                buffer,
                bufferOffset,
                data.data(runtime) + dataOffset,
                size
            );
            return Value::undefined();
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> QueueHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "submit");
}
