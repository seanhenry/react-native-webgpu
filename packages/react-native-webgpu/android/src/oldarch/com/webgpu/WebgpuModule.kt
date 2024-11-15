package com.webgpu

import android.util.Log
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.modules.blob.BlobModule
import java.lang.ref.WeakReference


class WebgpuModule(val reactContext: ReactApplicationContext) :
  ReactContextBaseJavaModule(reactContext) {
  private val exceptionHandler = ExceptionHandler(reactContext)

  override fun getName(): String {
    return NAME
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  fun installWithThreadId(threadId: String): Boolean {
    try {
      val blobModule = reactContext.getNativeModule(BlobModule::class.java)
      val factory = BitmapLoaderFactory(blobModule, reactContext)
      CxxBridge.setJavaModules(factory, exceptionHandler)

      val jsiRuntimeRef = reactContext.javaScriptContextHolder!!.get()
      val jsCallInvokerHolder = reactContext.catalystInstance.jsCallInvokerHolder
      return CxxBridge.installJsi(threadId, jsiRuntimeRef, jsCallInvokerHolder)
    } catch (exception: Exception) {
      Log.e(NAME, "Failed to initialize react-native-webgpu", exception)
      return false
    }
  }

  companion object {
    const val NAME = "WGPUJsi"
  }
}
