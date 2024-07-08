#include "QueueHostObject.h"
#include "CommandBufferHostObject.h"
#include "WGPUJsiUtils.h"
#include "WGPUContext.h"
#include "BufferHostObject.h"
#include "WGPUConversions.h"
#include "TextureHostObject.h"
#include "ImageBitmapHostObject.h"
#include "WGPUDefaults.h"

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
            auto data = getArrayBufferFromArrayBufferLike(runtime, arguments[2].asObject(runtime));
            auto dataOffset = count > 3 ? (size_t)arguments[3].asNumber() : 0;
            auto size = count > 4 ? (size_t)arguments[4].asNumber() : (data.size(runtime) - dataOffset);
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

    if (name == "copyExternalImageToTexture") {
        return WGPU_FUNC_FROM_HOST_FUNC(copyExternalImageToTexture, 3, [this]) {
            auto sourceParam = arguments[0].asObject(runtime);
            
            auto source = sourceParam.getPropertyAsObject(runtime, "source");

            WGPUTextureDataLayout dataLayout = {0};
            void *data = NULL;
            size_t dataSize = 0;
            // TODO: Support other sources
            if (sourceParam.getPropertyAsObject(runtime, "source").isHostObject<ImageBitmapHostObject>(runtime)) {
                auto imageBitmap = WGPU_HOST_OBJ(sourceParam, source, ImageBitmapHostObject);
                data = imageBitmap->_data;
                dataSize = imageBitmap->_size;

                auto width = imageBitmap->_width;
                auto height = imageBitmap->_height;
                uint32_t bytesPerPixel = dataSize / (width * height);
                dataLayout.rowsPerImage = height;
                dataLayout.bytesPerRow = width * bytesPerPixel;
            } else {
                throw JSError(runtime, "Only supports ImageBitmap");
            }

            auto destination = arguments[1].asObject(runtime);
            auto copyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));

            auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

            wgpuQueueWriteTexture(_value, &copyTexture, data, dataSize, &dataLayout, &copySize);
            return Value::undefined();
        });
    }

    return Value::undefined();
}

std::vector<PropNameID> QueueHostObject::getPropertyNames(Runtime& runtime) {
    return PropNameID::names(runtime, "submit", "writeBuffer", "copyExternalImageToTexture");
}
