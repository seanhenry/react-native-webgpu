#include "QueueHostObject.h"

#include "ArrayBufferUtils.h"
#include "BufferHostObject.h"
#include "CommandBufferHostObject.h"
#include "ImageBitmapHostObject.h"
#include "Mixins.h"
#include "TextureHostObject.h"
#include "WGPUContext.h"
#include "WGPUConversions.h"
#include "WGPUDefaults.h"
#include "WGPUJsiUtils.h"
#include "WGPUPromise.h"

using namespace facebook::jsi;
using namespace wgpu;

namespace wgpu {
void wgpuOnSubmittedWorkDone(WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void *userdata);
}

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
      WGPUOrigin3D origin{.x = 0, .y = 0, .z = 0};
      if (WGPU_HAS_PROP(sourceParam, origin)) {
        auto originIn = WGPU_OBJ(sourceParam, origin);
        origin = makeWGPUOrigin3D(runtime, originIn);
      }

      auto destination = arguments[1].asObject(runtime);
      auto copyTexture = makeWGPUImageCopyTexture(runtime, std::move(destination));

      auto copySize = makeGPUExtent3D(runtime, arguments[2].asObject(runtime));

      uint8_t *sourceData = nullptr;
      size_t sourceDataSize = 0;
      uint32_t sourceWidth = 0;
      uint32_t sourceHeight = 0;
      // TODO: Support other sources
      if (source.isHostObject<ImageBitmapHostObject>(runtime)) {
        auto imageBitmap = source.asHostObject<ImageBitmapHostObject>(runtime);
        sourceData = imageBitmap->_data;
        sourceDataSize = imageBitmap->_size;
        sourceWidth = imageBitmap->_width;
        sourceHeight = imageBitmap->_height;
      } else if (WGPU_HAS_PROP(source, _threeImageBitmap) &&
                 WGPU_OBJ(source, _threeImageBitmap).isHostObject<ImageBitmapHostObject>(runtime)) {
        auto imageBitmap = WGPU_HOST_OBJ(source, _threeImageBitmap, ImageBitmapHostObject);
        sourceData = imageBitmap->_data;
        sourceDataSize = imageBitmap->_size;
        sourceWidth = imageBitmap->_width;
        sourceHeight = imageBitmap->_height;
      } else if (WGPU_HAS_PROP(source, data)) {  // ImageData
        auto rgbaArray = WGPU_OBJ(source, data);
        auto arrayBuffer = rgbaArray.getPropertyAsObject(runtime, "buffer").getArrayBuffer(runtime);
        sourceData = arrayBuffer.data(runtime);
        sourceDataSize = arrayBuffer.size(runtime);
        sourceWidth = WGPU_NUMBER(source, width, uint32_t);
        sourceHeight = WGPU_NUMBER(source, height, uint32_t);
      } else if ((WGPU_BOOL_OPT(source, __isCopyExternalImageToTextureCompatible, false))) {
        auto arrayBuffer = WGPU_OBJ(source, arrayBuffer).getArrayBuffer(runtime);
        sourceData = arrayBuffer.data(runtime);
        sourceDataSize = arrayBuffer.size(runtime);
        sourceWidth = WGPU_NUMBER(source, width, uint32_t);
        sourceHeight = WGPU_NUMBER(source, height, uint32_t);
      }
      if (sourceData == nullptr) {
        throw JSINativeException("Only supports ImageBitmap and ImageData");
      }

      uint32_t bytesPerPixel = sourceDataSize / (sourceWidth * sourceHeight);
      uint32_t sourceBytesPerRow = sourceWidth * bytesPerPixel;
      WGPUTextureDataLayout dataLayout = {
        .nextInChain = nullptr,
        .offset = 0,
        .bytesPerRow = sourceBytesPerRow,
        .rowsPerImage = sourceHeight,
      };

      if (!flipY && origin.x == 0 && origin.y == 0 && copySize.width == sourceWidth) {
        wgpuQueueWriteTexture(_value, &copyTexture, sourceData, sourceDataSize, &dataLayout, &copySize);
        _context->getErrorHandler()->throwPendingJSIError();
        return Value::undefined();
      }

      if (origin.x < 0 || origin.y < 0) {
        throw JSError(runtime, "Origin was negative");
      }
      if (copySize.width < 0 || copySize.height < 0) {
        throw JSError(runtime, "Copy image size was negative");
      }
      if (sourceWidth < origin.x + copySize.width || sourceHeight < origin.y + copySize.height) {
        throw JSError(runtime, "Copy image extends beyond the source image bounds");
      }

      auto copyBytesPerRow = copySize.width * bytesPerPixel;
      auto copyDataSize = copyBytesPerRow * copySize.height;
      auto copyData = (uint8_t *)malloc(copyDataSize);
      for (auto y = 0; y < copySize.height; y++) {
        auto sourceXOffset = origin.x * bytesPerPixel;
        auto sourceYOffset = (y + origin.y) * sourceBytesPerRow;
        auto yOffset = (flipY ? copySize.height - y - 1 : y) * copyBytesPerRow;
        memcpy(copyData + yOffset, sourceData + sourceXOffset + sourceYOffset, copyBytesPerRow);
      }
      dataLayout.bytesPerRow = copyBytesPerRow;
      dataLayout.rowsPerImage = copySize.height;

      wgpuQueueWriteTexture(_value, &copyTexture, copyData, copyDataSize, &dataLayout, &copySize);
      _context->getErrorHandler()->throwPendingJSIError();

      free(copyData);
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

  if (name == "onSubmittedWorkDone") {
    return WGPU_FUNC_FROM_HOST_FUNC(onSubmittedWorkDone, 0, [this]) {
      return Promise::makeJSPromise(_context->getJSIInstance(), [this](auto &runtime, auto &promise) {
        wgpuQueueOnSubmittedWorkDone(_value, wgpuOnSubmittedWorkDone, promise->toCData());
      });
    });
  }

  WGPU_GET_BRAND(GPUQueue)

  WGPU_LOG_UNIMPLEMENTED_GET_PROP;

  return Value::undefined();
}

std::vector<PropNameID> QueueHostObject::getPropertyNames(Runtime &runtime) {
  return PropNameID::names(runtime, "submit", "writeBuffer", "copyExternalImageToTexture", "writeTexture",
                           "onSubmittedWorkDone", "__brand");
}

void wgpu::wgpuOnSubmittedWorkDone(WGPUQueueWorkDoneStatus status, WGPU_NULLABLE void *userdata) {
  Promise::fromCData(userdata, [status](auto &promise) {
    if (status == WGPUQueueWorkDoneStatus_Success) {
      promise->resolve([](auto &runtime) { return Value::undefined(); });
    } else {
      promise->reject([](auto &runtime) { return makeJSError(runtime, "GPUQueue.onSubmittedWorkDone failed"); });
    }
  });
}
