package com.webgpu

import com.facebook.proguard.annotations.DoNotStrip
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.modules.blob.BlobModule

@DoNotStrip
class BitmapLoaderFactory(val blobModule: BlobModule?, val context: ReactApplicationContext) {
  @DoNotStrip
  fun makeHttpBitmapLoader(uri: String): HTTPBitmapLoader {
    return HTTPBitmapLoader(uri)
  }

  @DoNotStrip
  fun makeDrawableBitmapLoader(uri: String): DrawableBitmapLoader {
    return DrawableBitmapLoader(uri, context)
  }

  @DoNotStrip
  fun makeBlobBitmapLoader(): BlobBitmapLoader? {
    return blobModule?.let { BlobBitmapLoader(it) }
  }
}