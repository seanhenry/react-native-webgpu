package com.webgpu.experimental

import com.facebook.react.turbomodule.core.interfaces.CallInvokerHolder

class CxxBridge {
  companion object {
    init {
      System.loadLibrary("react-native-webgpu-experimental")
    }

    external fun installJsi(threadId: String, jsiRuntimeRef: Long, jsCallInvokerHolder: CallInvokerHolder): Boolean
  }
}
