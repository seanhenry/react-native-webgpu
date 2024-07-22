package com.webgpu

import com.facebook.proguard.annotations.DoNotStrip

@DoNotStrip
class BitmapLoaderFactory {
  companion object {
    @JvmStatic
    @DoNotStrip
    fun makeHttpBitmapLoader(uri: String): BitmapLoader {
      return BitmapLoader(uri)
    }
  }
}