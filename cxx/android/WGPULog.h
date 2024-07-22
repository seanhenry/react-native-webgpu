#pragma once

#include <android/log.h>

#define LOG_TAG "WebGPU_JNI"
#define LOG_INFO(...) __android_log_buf_print(LOG_ID_DEFAULT, ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_buf_print(LOG_ID_DEFAULT, ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
