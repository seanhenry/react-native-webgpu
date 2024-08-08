package com.webgpu

import android.content.Context
import android.graphics.PixelFormat
import android.util.AttributeSet
import android.util.DisplayMetrics
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.WindowMetrics
import com.facebook.react.bridge.Arguments
import com.facebook.react.bridge.ReactContext
import com.facebook.react.bridge.WritableMap
import com.facebook.react.uimanager.events.RCTEventEmitter
import java.util.UUID

class WebGPUView : SurfaceView, SurfaceHolder.Callback2 {
  private val uuid = UUID.randomUUID().toString()

  constructor(context: Context) : super(context)
  constructor(context: Context, attrs: AttributeSet) : super(context, attrs)
  constructor(context: Context, attrs: AttributeSet, defStyle: Int) : super(context, attrs, defStyle)

  init {
    holder.addCallback(this)
    // Make background transparent
    this.setZOrderMediaOverlay(true)
    holder.setFormat(PixelFormat.TRANSPARENT)
  }

  override fun surfaceCreated(holder: SurfaceHolder) {
    holder.let { h ->

      val density = context.resources.displayMetrics.density
      val result = CxxBridge.onSurfaceCreated(h.surface, uuid, density);
      if (result) {
        setWillNotDraw(false)

        val event = Arguments.createMap().apply {
          putString("uuid", uuid)
        }
        sendOnCreateSurface(event)
      } else {
        val event = Arguments.createMap().apply {
          putString("error", "Failed to create surface")
        }
        sendOnCreateSurface(event)
      }
    }
  }

  private fun sendOnCreateSurface(event: WritableMap) {
    val reactContext = context as ReactContext
    reactContext
      .getJSModule(RCTEventEmitter::class.java)
      .receiveEvent(id, "onCreateSurface", event)
  }

  override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
    // TODO:
  }

  override fun surfaceDestroyed(holder: SurfaceHolder) {
    CxxBridge.onSurfaceDestroyed(uuid)
  }

  override fun surfaceRedrawNeeded(holder: SurfaceHolder) {}
}
