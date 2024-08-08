package com.webgpu

import android.util.Log
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import com.facebook.react.modules.blob.BlobModule
import java.lang.ref.WeakReference


class WebgpuModule(reactContext: ReactApplicationContext) :
  ReactContextBaseJavaModule(reactContext) {
  private val weakReactContext = WeakReference(reactContext)

  override fun getName(): String {
    return NAME
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  fun install(): Boolean {
    try {
      val context = weakReactContext.get()
      if (context == null) {
        Log.e(NAME, "ReactApplicationContext was null")
        return false
      }

      val jsiRuntimeRef = context.javaScriptContextHolder!!.get()
      val jsCallInvokerHolder = context.catalystInstance.jsCallInvokerHolder
      val blobModule = context.getNativeModule(BlobModule::class.java)
      val factory = BitmapLoaderFactory(blobModule)
      return CxxBridge.installJsi(jsiRuntimeRef, jsCallInvokerHolder, factory)
    } catch (exception: Exception) {
      Log.e(NAME, "Failed to initialize react-native-webgpu", exception)
      return false
    }
  }

  companion object {
    const val NAME = "WGPUJsi"
  }
}
