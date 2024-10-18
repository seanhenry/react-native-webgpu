package com.webgpu

import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.module.annotations.ReactModule
import com.facebook.react.uimanager.SimpleViewManager
import com.facebook.react.uimanager.ThemedReactContext
import com.facebook.react.uimanager.ViewManagerDelegate
import com.facebook.react.viewmanagers.WGPUWebGPUViewManagerDelegate
import com.facebook.react.viewmanagers.WGPUWebGPUViewManagerInterface

@ReactModule(name = WebGPUViewManagerImpl.NAME)
class WebGPUViewManager(
  private val callerContext: ReactApplicationContext
) : SimpleViewManager<WebGPUView>(), WGPUWebGPUViewManagerInterface<WebGPUView> {
  private val delegate: ViewManagerDelegate<WebGPUView> = WGPUWebGPUViewManagerDelegate(this)
  override fun getDelegate(): ViewManagerDelegate<WebGPUView> = delegate
  override fun getName(): String = WebGPUViewManagerImpl.NAME
  override fun createViewInstance(context: ThemedReactContext): WebGPUView =
    WebGPUViewManagerImpl.createViewInstance(context)
  override fun getExportedCustomBubblingEventTypeConstants(): Map<String, Any> {
    return WebGPUViewManagerImpl.getExportedCustomBubblingEventTypeConstants()
  }
}
