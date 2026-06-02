#pragma once
#include <stdarg.h>

typedef enum {
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_FATAL,
} log_level_t;

void kvlog(log_level_t level, const char* fmt, va_list args);
void klog(log_level_t level, const char* fmt, ...);