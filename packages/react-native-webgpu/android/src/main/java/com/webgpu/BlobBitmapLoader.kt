package com.webgpu

import android.graphics.BitmapFactory
import android.os.SharedMemory
import android.system.OsConstants
import com.facebook.proguard.annotations.DoNotStrip
import com.facebook.react.modules.blob.BlobModule

@DoNotStrip
class BlobBitmapLoader(val blobModule: BlobModule) {
  @DoNotStrip
  var sharedMemory: SharedMemory? = null
    private set
  @DoNotStrip
  var width = 0
    private set

  @DoNotStrip
  var height = 0
    private set

  @DoNotStrip
  fun loadBitmap(blobId: String, offset: Int, size: Int): Int {
    val blob = blobModule.resolve(blobId, offset, size) ?: return 1
    val bitmap = BitmapFactory.decodeByteArray(blob, 0, blob.size)
    this.sharedMemory = SharedMemory.create(blobId, bitmap.width * bitmap.height * 4).apply {
      bitmap.copyPixelsToBuffer(mapReadWrite())
      setProtect(OsConstants.PROT_READ)
    }
    width = bitmap.width
    height = bitmap.height
    return 0
  }
}