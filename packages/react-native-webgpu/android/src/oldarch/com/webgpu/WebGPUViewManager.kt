package com.webgpu

import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.uimanager.SimpleViewManager
import com.facebook.react.uimanager.ThemedReactContext
import com.facebook.react.uimanager.annotations.ReactProp

class WebGPUViewManager(
  private val callerContext: ReactApplicationContext
) : SimpleViewManager<WebGPUView>() {
  override fun getName() = WebGPUViewManagerImpl.NAME
  override fun createViewInstance(context: ThemedReactContext) = WebGPUViewManagerImpl.createViewInstance(context)
  override fun getExportedCustomBubblingEventTypeConstants(): Map<String, Any> {
    return WebGPUViewManagerImpl.getExportedCustomBubblingEventTypeConstants()
  }
  @ReactProp(name = "pollSize")
  fun setPollSize(view: WebGPUView, pollSize: Boolean) {
    // no-op for Android
  }
  @ReactProp(name = "backends")
  fun setBackends(view: WebGPUView, backends: Int?) {
    view.backends = backends
  }
}
