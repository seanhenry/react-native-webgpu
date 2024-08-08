#pragma once

#include <os/log.h>

#define WGPU_LOG_INFO(...) os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_INFO, __VA_ARGS__)
#define WGPU_LOG_ERROR(...) os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_ERROR, __VA_ARGS__)
