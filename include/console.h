#pragma once
#include <stdarg.h>
#include <stdint.h>

bool console_ready();
void console_init();
void console_clear();
void console_set_color(uint32_t fg, uint32_t bg);
void console_putchar(char c);
void console_print(const char* str);

void cvprintf(const char* fmt, va_list args);
void cprintf(const char* fmt, ...);