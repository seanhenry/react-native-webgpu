package com.webgpu.experimental

import com.facebook.react.bridge.ReactApplicationContext

class WebgpuExperimentalModule(reactContext: ReactApplicationContext): NativeWebgpuExperimentalModuleSpec(reactContext) {
  override fun getTypedExportedConstants(): MutableMap<String, Any> {
    TODO("Implemented in c++")
  }

  override fun installWithThreadId(threadId: String?): Boolean {
    TODO("Implemented in c++")
  }
}
