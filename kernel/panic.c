#include "panic.h"

#include "macros.h"
#include "sprintf.h"
#include "x86.h"

NORETURN void kpanic(const char* fmt, ...) {
	x86_disable_interrupts();
	va_list args;
	va_start(args, fmt);

	sprintf("KERNEL PANIC:\n");
	svprintf(fmt, args);
	sprintf("\n");

	va_end(args);

	__asm__ volatile("hlt");

	__builtin_unreachable();
}
