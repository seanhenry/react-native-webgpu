#pragma once

#include <android/log.h>

#define WGPU_LOG_INFO(...) __android_log_buf_print(LOG_ID_DEFAULT, ANDROID_LOG_INFO, "WebGPU_JNI", __VA_ARGS__)
#define WGPU_LOG_ERROR(...) __android_log_buf_print(LOG_ID_DEFAULT, ANDROID_LOG_ERROR, "WebGPU_JNI", __VA_ARGS__)
