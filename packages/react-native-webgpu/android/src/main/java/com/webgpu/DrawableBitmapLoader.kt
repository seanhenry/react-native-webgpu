package com.webgpu

import android.os.SharedMemory
import android.system.OsConstants
import androidx.core.graphics.drawable.toBitmap
import com.facebook.proguard.annotations.DoNotStrip
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.views.imagehelper.ResourceDrawableIdHelper

@DoNotStrip
class DrawableBitmapLoader(private val name: String, private val context: ReactApplicationContext) {
  @DoNotStrip
  var sharedMemory: SharedMemory? = null
    private set
  @DoNotStrip
  var errorMessage: String? = null
    private set
  @DoNotStrip
  var width = 0
    private set
  @DoNotStrip
  var height = 0
    private set

  @DoNotStrip
  fun fetchBitmap() {
    val drawable =
      ResourceDrawableIdHelper.instance.getResourceDrawable(context, name)
    if (drawable == null) {
      errorMessage = "Could not find drawable $name"
      return;
    }
    val bitmap = drawable.toBitmap()
    this.sharedMemory = SharedMemory.create(name, bitmap.width * bitmap.height * 4).apply {
      bitmap.copyPixelsToBuffer(mapReadWrite())
      setProtect(OsConstants.PROT_READ)
    }
    width = bitmap.width
    height = bitmap.height
  }
}