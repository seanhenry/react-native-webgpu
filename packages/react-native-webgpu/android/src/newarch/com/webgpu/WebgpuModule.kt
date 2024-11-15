package com.webgpu

import com.facebook.fbreact.specs.NativeWebgpuModuleSpec
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.modules.blob.BlobModule

class WebgpuModule(reactContext: ReactApplicationContext) :
  NativeWebgpuModuleSpec(reactContext) {
  private val exceptionHandler = ExceptionHandler(reactContext)

  init {
    val blobModule = reactContext.getNativeModule(BlobModule::class.java)
    val factory = BitmapLoaderFactory(blobModule, reactContext)
    CxxBridge.setJavaModules(factory, exceptionHandler)
  }

  override fun installWithThreadId(threadId: String): Boolean {
    // Turbo module is run from c++
    return false
  }
}
