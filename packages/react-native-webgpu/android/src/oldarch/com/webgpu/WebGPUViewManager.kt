package com.webgpu

import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.uimanager.SimpleViewManager
import com.facebook.react.uimanager.ThemedReactContext

class WebGPUViewManager(
  private val callerContext: ReactApplicationContext
) : SimpleViewManager<WebGPUView>() {
  override fun getName() = WebGPUViewManagerImpl.NAME
  override fun createViewInstance(context: ThemedReactContext) = WebGPUViewManagerImpl.createViewInstance(context)
  override fun getExportedCustomBubblingEventTypeConstants(): Map<String, Any> {
    return WebGPUViewManagerImpl.getExportedCustomBubblingEventTypeConstants()
  }
}
