package com.webgpu

import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.modules.blob.BlobModule
import com.facebook.react.uimanager.SimpleViewManager
import com.facebook.react.uimanager.ThemedReactContext

class WebGPUViewManager(
  private val callerContext: ReactApplicationContext
) : SimpleViewManager<WebGPUView>() {

  override fun getName() = REACT_CLASS

  companion object {
    const val REACT_CLASS = "WGPUWebGPUView"
  }

  override fun createViewInstance(context: ThemedReactContext) =
    WebGPUView(context)

  override fun getExportedCustomBubblingEventTypeConstants(): Map<String, Any> {
    return mapOf(
      "onCreateSurface" to mapOf(
        "phasedRegistrationNames" to mapOf(
          "bubbled" to "onCreateSurface"
        )
      )
    )
  }
}