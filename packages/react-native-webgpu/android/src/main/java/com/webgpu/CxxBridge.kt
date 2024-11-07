package com.webgpu

import android.view.Surface
import com.facebook.react.bridge.queue.QueueThreadExceptionHandler
import com.facebook.react.modules.blob.BlobModule
import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder

class CxxBridge {
  companion object {
    init {
      System.loadLibrary("react-native-webgpu")
    }

    external fun installJsi(threadId: String, jsiRuntimeRef: Long, jsCallInvokerHolder: CallInvokerHolder, factory: BitmapLoaderFactory?, exceptionHandler: ExceptionHandler): Boolean
    external fun onSurfaceCreated(surface: Surface, uuid: String, density: Float): Boolean
    external fun onSurfaceDestroyed(uuid: String)
  }
}