#include "klogf.h"

#include <framebuffer.h>
#include <stdarg.h>

#include "console.h"
#include "printf.h"
#include "serial.h"
#include "sprintf.h"

void kvlog(log_level_t level, const char* fmt, va_list args) {
	switch (level) {
		case LOG_INFO:
			print_set_color(PRINT_COLOR_GREEN, PRINT_COLOR_BLACK);
			print_str("[INFO]  ");
			serial_print("[INFO]  ");
			if (console_ready()) {
				console_set_color(fb_color(0, 255, 0), fb_color(0, 0, 0));
				console_print("[INFO]  ");
			}
			break;
		case LOG_WARN:
			print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
			print_str("[WARN]  ");
			serial_print("[WARN]  ");
			if (console_ready()) {
				console_set_color(fb_color(255, 255, 0), fb_color(0, 0, 0));
				console_print("[WARN]  ");
			}
			break;
		case LOG_ERROR:
			print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
			print_str("[ERROR] ");
			serial_print("[ERROR] ");
			if (console_ready()) {
				console_set_color(fb_color(255, 0, 0), fb_color(0, 0, 0));
				console_print("[ERROR] ");
			}
			break;
		case LOG_FATAL:
			print_set_color(PRINT_COLOR_MAGENTA, PRINT_COLOR_BLACK);
			print_str("[FATAL] ");
			serial_print("[FATAL] ");
			if (console_ready()) {
				console_set_color(fb_color(255, 0, 255), fb_color(0, 0, 0));
				console_print("[FATAL] ");
			}
			break;
	}
	print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
	if (console_ready())
		console_set_color(fb_color(255, 255, 255), fb_color(0, 0, 0));

	va_list args1;
	va_list args2;

	va_copy(args1, args);
	va_copy(args2, args);

	if (console_ready())
		cvprintf(fmt, args1);
	else
		vprintf(fmt, args1);
	svprintf(fmt, args2);

	va_end(args1);
	va_end(args2);

	sprintf("\n");
	print_char('\n');
	if (console_ready())
		console_putchar('\n');
}

void klog(log_level_t level, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	kvlog(level, fmt, args);
	va_end(args);
}