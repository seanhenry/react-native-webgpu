package com.webgpu

import com.facebook.proguard.annotations.DoNotStrip
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.common.JavascriptException

@DoNotStrip
class ExceptionHandler(val context: ReactApplicationContext) {
  @DoNotStrip
  fun handleException(message: String) {
    context.handleException(JavascriptException(message))
  }
}