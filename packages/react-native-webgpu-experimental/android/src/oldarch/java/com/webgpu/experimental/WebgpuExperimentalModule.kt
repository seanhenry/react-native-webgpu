package com.webgpu.experimental

import android.util.Log
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.bridge.ReactContextBaseJavaModule
import com.facebook.react.bridge.ReactMethod
import java.lang.ref.WeakReference


class WebgpuExperimentalModule(reactContext: ReactApplicationContext) :
  ReactContextBaseJavaModule(reactContext) {
  private val weakReactContext = WeakReference(reactContext)

  override fun getName(): String {
    return NAME
  }

  @ReactMethod(isBlockingSynchronousMethod = true)
  fun installWithThreadId(threadId: String): Boolean {
    try {
      val context = weakReactContext.get()
      if (context == null) {
        Log.e(NAME, "ReactApplicationContext was null")
        return false
      }

      val jsiRuntimeRef = context.javaScriptContextHolder!!.get()
      val jsCallInvokerHolder = context.catalystInstance.jsCallInvokerHolder
      return CxxBridge.installJsi(threadId, jsiRuntimeRef, jsCallInvokerHolder)
    } catch (exception: Exception) {
      Log.e(NAME, "Failed to initialize react-native-webgpu-experimental", exception)
      return false
    }
  }

  override fun getConstants(): MutableMap<String, Any> {
    return mutableMapOf(
      "ENABLE_THREADS" to false
    )
  }

  companion object {
    const val NAME = "WGPUExperimentalJsi"
  }
}
