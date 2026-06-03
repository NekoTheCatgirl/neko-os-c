#include "panic.h"

#include "klogf.h"
#include "macros.h"

NORETURN void kpanic(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	klog(LOG_FATAL, "KERNEL PANIC:");
	kvlog(LOG_FATAL, fmt, args);

	va_end(args);

	__asm__ volatile(
		"cli\n"
		"hlt"
	);

	__builtin_unreachable();
}
