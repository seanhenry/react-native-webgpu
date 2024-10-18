package com.webgpu

import com.facebook.react.uimanager.ThemedReactContext

object WebGPUViewManagerImpl {
  const val NAME = "WGPUWebGPUView"
  fun createViewInstance(context: ThemedReactContext) = WebGPUView(context)
  fun getExportedCustomBubblingEventTypeConstants(): Map<String, Any> {
    return mapOf(
      "onCreateSurface" to mapOf(
        "phasedRegistrationNames" to mapOf(
          "bubbled" to "onCreateSurface"
        )
      )
    )
  }
}
