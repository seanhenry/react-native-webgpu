#include "Compression.h"

#include <zlib.h>

#include "WGPUJsiUtils.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

int inf(uint8_t *data, size_t size, std::vector<uint8_t> &bufferOut) {
  int ret;
  z_stream strm;
  size_t lastOut = 0;

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = size;
  strm.next_in = data;
  ret = inflateInit2(&strm, 16 + MAX_WBITS);
  if (ret != Z_OK) {
    return ret;
  }

  do {
    auto currentSize = bufferOut.size();
    bufferOut.resize(currentSize + CHUNK);

    strm.avail_out = CHUNK;
    strm.next_out = bufferOut.data() + currentSize;

    ret = inflate(&strm, Z_NO_FLUSH);
    assert(ret != Z_STREAM_ERROR);
    switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return ret;
      default:
        break;
    }
    lastOut = strm.avail_out;
  } while (strm.avail_in > 0);

  (void)inflateEnd(&strm);
  if (ret == Z_STREAM_END) {
    bufferOut.resize(bufferOut.size() - lastOut);
    return Z_OK;
  }
  return Z_DATA_ERROR;
}

Value wgpu::inflate(Runtime &runtime) {
  return WGPU_FUNC_FROM_HOST_FUNC(unzip, 1, []) {
    auto arrayBuffer = arguments[0].asObject(runtime).getArrayBuffer(runtime);
    std::vector<uint8_t> out;
    auto result = inf(arrayBuffer.data(runtime), arrayBuffer.length(runtime), out);
    if (result == Z_OK) {
      return createOwnedVectorArrayBuffer(runtime, std::move(out));
    }
    throw JSINativeException("Failed to inflate array buffer");
  });
}
