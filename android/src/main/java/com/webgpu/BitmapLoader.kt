package com.webgpu

import android.graphics.BitmapFactory
import android.os.SharedMemory
import android.system.OsConstants
import com.facebook.proguard.annotations.DoNotStrip
import java.io.BufferedInputStream
import java.net.HttpURLConnection
import java.net.URL

@DoNotStrip
class BitmapLoader(private val url: String) {
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
    val connection = URL(url).openConnection() as HttpURLConnection
    try {
      val inStream = BufferedInputStream(connection.inputStream)
      val bitmap = BitmapFactory.decodeStream(inStream)
      this.sharedMemory = SharedMemory.create(url, bitmap.width * bitmap.height * 4).apply {
        bitmap.copyPixelsToBuffer(mapReadWrite())
        setProtect(OsConstants.PROT_READ)
      }
      width = bitmap.width
      height = bitmap.height
    } catch (exception: Exception) {
      errorMessage = exception.localizedMessage
    } finally {
      connection.disconnect()
    }
  }
}