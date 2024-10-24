#include "QueueHostObject.h"

#include "ArrayBufferUtils.h"
#include "BufferHostObject.h"
#include "CommandBufferHostObject.h"
#include "ImageBitmapHostObject.h"
#include "TextureHostObject.h"
#include "VideoPlayer.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"

using namespace facebook::jsi;
using namespace wgpu;

Value QueueHostObject::get(Runtime &runtime, const PropNameID &propName) {
  auto name = propName.utf8(runtime);

  WGPU_LOG_GET_PROP;

  if (name == "submit") {
    return WGPU_FUNC_FROM_HOST_FUNC(submit, 1, [this]) {
      WGPU_LOG_FUNC_ARGS(submit);
      auto commandsJsi = arguments[0].asObject(runtime).asArray(runtime);
      auto commands =
        jsiArrayToVector<WGPUCommandBuffer>(runtime, std::move(commandsJsi), [](Runtime &runtime, Value item) {
        return item.asObject(runtime).asHostObject<CommandBufferHostObject>(runtime)->getValue();
      });
      wgpuQueueSubmit(_value, commands.size(), commands.data());
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "writeBuffer") {
    return WGPU_FUNC_FROM_HOST_FUNC(writeBuffer, 5, [this]) {
      WGPU_LOG_FUNC_ARGS(writeBuffer);
      auto buffer = arguments[0].asObject(runtime).asHostObject<BufferHostObject>(runtime)->getValue();
      auto bufferOffset = (uint64_t)arguments[1].asNumber();
      auto data = getArrayBufferViewFromArrayBufferLike(runtime, arguments[2].asObject(runtime));
      auto dataOffset = count > 3 ? (size_t)arguments[3].asNumber() : 0;
      auto size = count > 4 ? (size_t)arguments[4].asNumber() : (data.byteLength - dataOffset);

      auto dataPtr = data.arrayBuffer.data(runtime) + data.byteOffset + dataOffset;
      wgpuQueueWriteBuffer(_value, buffer, bufferOffset, dataPtr, size);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  if (name == "copyExternalImageToTexture") {
    return WGPU_FUNC_FROM_HOST_FUNC(copyExternalImageToTexture, 3, [this]) {
      WGPU_LOG_FUNC_ARGS(copyExternalImageToTexture);
      auto sourceParam = arguments[0].asObject(runtime);
      auto source = sourceParam.getPropertyAsObject(runtime, "source");
      auto flipY = WGPU_BOOL_OPT(sourceParam, flipY, false);

      auto destination = arguments[1].asObject(runtime);
      auto copyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));

      auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

      uint8_t *data = nullptr;
      size_t dataSize = 0;
      uint32_t width = 0;
      uint32_t height = 0;
      // TODO: Support other sources
      if (source.isHostObject<ImageBitmapHostObject>(runtime)) {
        auto imageBitmap = source.asHostObject<ImageBitmapHostObject>(runtime);
        data = imageBitmap->_data;
        dataSize = imageBitmap->_size;
        width = imageBitmap->_width;
        height = imageBitmap->_height;
      } else if (WGPU_HAS_PROP(source, _threeImageBitmap) &&
                 WGPU_OBJ(source, _threeImageBitmap).isHostObject<ImageBitmapHostObject>(runtime)) {
        auto imageBitmap = WGPU_HOST_OBJ(source, _threeImageBitmap, ImageBitmapHostObject);
        data = imageBitmap->_data;
        dataSize = imageBitmap->_size;
        width = imageBitmap->_width;
        height = imageBitmap->_height;
      } else if (WGPU_HAS_PROP(source, data)) {  // ImageData
        auto rgbaArray = WGPU_OBJ(source, data);
        auto arrayBuffer = rgbaArray.getPropertyAsObject(runtime, "buffer").getArrayBuffer(runtime);
        data = arrayBuffer.data(runtime);
        dataSize = arrayBuffer.size(runtime);
        width = WGPU_NUMBER(source, width, uint32_t);
        height = WGPU_NUMBER(source, height, uint32_t);
      } else if ((WGPU_BOOL_OPT(source, __isCopyExternalImageToTextureCompatible, false))) {
        auto arrayBuffer = WGPU_OBJ(source, arrayBuffer).getArrayBuffer(runtime);
        data = arrayBuffer.data(runtime);
        dataSize = arrayBuffer.size(runtime);
        width = WGPU_NUMBER(source, width, uint32_t);
        height = WGPU_NUMBER(source, height, uint32_t);
      }
      if (data == nullptr) {
        throw JSINativeException("Only supports ImageBitmap and ImageData");
      }
      uint32_t bytesPerPixel = dataSize / (width * height);
      uint32_t bytesPerRow = width * bytesPerPixel;
      WGPUTextureDataLayout dataLayout = {
        .nextInChain = nullptr,
        .offset = 0,
        .bytesPerRow = bytesPerRow,
        .rowsPerImage = height,
      };

      uint8_t *flipped = nullptr;
      if (flipY) {
        flipped = (uint8_t *)malloc(dataSize);
        for (auto y = 0; y < height; y++) {
          auto flippedIndex = (height - y - 1) * bytesPerRow;
          auto originalIndex = y * bytesPerRow;
          memcpy(flipped + flippedIndex, data + originalIndex, bytesPerRow);
        }
      }

      wgpuQueueWriteTexture(_value, &copyTexture, flipped ?: data, dataSize, &dataLayout, &copySize);
      _context->getErrorHandler()->throwPendingJSIError();

      if (flipped) {
        free(flipped);
      }
      return Value::undefined();
    });
  }

  if (name == "writeTexture") {
    return WGPU_FUNC_FROM_HOST_FUNC(writeTexture, 4, [this]) {
      WGPU_LOG_FUNC_ARGS(writeTexture);
      auto destination = arguments[0].asObject(runtime);
      auto copyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));
      auto data = getArrayBufferViewFromArrayBufferLike(runtime, arguments[1].asObject(runtime));
      auto dataLayoutIn = arguments[2].asObject(runtime);
      auto writeSize = makeGPUExtent3D(runtime, arguments[3].asObject(runtime));
      auto dataLayout = makeWGPUTextureDataLayout(runtime, dataLayoutIn, writeSize);

      auto dataPtr = data.arrayBuffer.data(runtime) + data.byteOffset;
      wgpuQueueWriteTexture(_value, &copyTexture, dataPtr, data.byteLength, &dataLayout, &writeSize);
      _context->getErrorHandler()->throwPendingJSIError();
      return Value::undefined();
    });
  }

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> QueueHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "submit", "writeBuffer", "copyExternalImageToTexture");
}
