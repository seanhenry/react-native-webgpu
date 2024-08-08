package com.webgpu

import com.facebook.proguard.annotations.DoNotStrip
import com.facebook.react.modules.blob.BlobModule

@DoNotStrip
class BitmapLoaderFactory(val blobModule: BlobModule?) {
  @DoNotStrip
  fun makeHttpBitmapLoader(uri: String): HTTPBitmapLoader {
    return HTTPBitmapLoader(uri)
  }

  @DoNotStrip
  fun makeBlobBitmapLoader(): BlobBitmapLoader? {
    return blobModule?.let { BlobBitmapLoader(it) }
  }
}