package com.webgpu

import android.view.Surface
import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder

class CxxBridge {
  companion object {
    init {
      System.loadLibrary("react-native-webgpu")
    }

    external fun installJsi(jsiRuntimeRef: Long, jsCallInvokerHolder: CallInvokerHolder): Boolean
    external fun onSurfaceCreated(surface: Surface, uuid: String): Boolean
    external fun onSurfaceDestroyed(uuid: String)
  }
}