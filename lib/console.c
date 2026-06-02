#include "console.h"

#include "alloc.h"
#include "font.h"
#include "framebuffer.h"
#include "mem.h"

typedef struct {
	char c;
	uint32_t fg;
	uint32_t bg;
} console_char_t;

static volatile bool initialized = false;

static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint32_t fg_color;
static uint32_t bg_color;

static uint32_t cols;
static uint32_t rows;
static uint32_t char_w;
static uint32_t char_h;

static console_char_t* buffer = nullptr;

bool console_ready() {
	return initialized;
}

void console_init() {
	font_init();
	char_w = font_get_width();
	char_h = font_get_height();
	cols = fb_get_width() / char_w;
	rows = fb_get_height() / char_h;
	fg_color = fb_color(255, 255, 255);
	bg_color = fb_color(0, 0, 0);

	buffer = (console_char_t*)kmalloc(cols * rows * sizeof(console_char_t));

	console_clear();
	initialized = true;
}

void console_clear() {
	fb_clear(bg_color);
	if (buffer) {
		for (uint32_t i = 0; i < cols * rows; i++) {
			buffer[i] = (console_char_t){' ', fg_color, bg_color};
		}
	}
	cursor_x = 0;
	cursor_y = 0;
}

void console_set_color(uint32_t fg, uint32_t bg) {
	fg_color = fg;
	bg_color = bg;
}

static void redraw_all() {
	for (uint32_t y = 0; y < rows; y++) {
		for (uint32_t x = 0; x < cols; x++) {
			console_char_t cc = buffer[y * cols + x];
			font_draw_char(cc.c, x * char_w, y * char_h, cc.fg, cc.bg);
		}
	}
}

static void scroll() {
	memmove(buffer, buffer + cols, (rows - 1) * cols * sizeof(console_char_t));
	for (uint32_t x = 0; x < cols; x++) {
		buffer[(rows - 1) * cols + x] = (console_char_t){' ', fg_color, bg_color};
	}
	redraw_all();
}

void console_putchar(char c) {
	if (c == '\n') {
		cursor_x = 0;
		cursor_y++;
	} else if (c == '\r') {
		cursor_x = 0;
	} else if (c == '\b') {
		if (cursor_x > 0) cursor_x--;
	} else {
		if (buffer) {
			buffer[cursor_y * cols + cursor_x] = (console_char_t){c, fg_color, bg_color};
		}
		font_draw_char(c, cursor_x * char_w, cursor_y * char_h, fg_color, bg_color);
		cursor_x++;
		if (cursor_x >= cols) {
			cursor_x = 0;
			cursor_y++;
		}
	}

	if (cursor_y >= rows) {
		scroll();
		cursor_y = rows - 1;
	}
}

void console_print(const char* str) {
	while (*str) console_putchar(*str++);
}

static void cprint_uint(uint64_t value, int base, int width, int precision, int flags, bool upper) {
	static constexpr char digits_lower[] = "0123456789abcdef";
	static constexpr char digits_upper[] = "0123456789ABCDEF";
	const char* digits = upper ? digits_upper : digits_lower;
	char buf[64];
	int i = 0;

	if (value == 0 && precision != 0) {
		buf[i++] = '0';
	} else {
		while (value > 0) {
			buf[i++] = digits[value % base];
			value /= base;
		}
	}

	int actual_precision = (precision > i) ? precision : i;
	int count = actual_precision;
	if (flags & 1) { // '#' flag
		if (base == 16) count += 2;
		else if (base == 8) count += 1;
	}

	if (!(flags & 2)) { // NOT '-' flag
		char pad = (flags & 4) ? '0' : ' '; // '0' flag
		while (width > count) {
			console_putchar(pad);
			width--;
		}
	}

	if (flags & 1) {
		if (base == 16) {
			console_print(upper ? "0X" : "0x");
		} else if (base == 8) {
			console_putchar('0');
		}
	}

	while (actual_precision > i) {
		console_putchar('0');
		actual_precision--;
	}

	for (int j = i - 1; j >= 0; j--)
		console_putchar(buf[j]);

	if (flags & 2) { // '-' flag
		while (width > count) {
			console_putchar(' ');
			width--;
		}
	}
}

static void cprint_int(int64_t value, int width, int precision, int flags) {
	bool neg = false;
	if (value < 0) {
		neg = true;
		value = -value;
	}

	int count = 0;
	uint64_t temp = (uint64_t)value;
	if (temp == 0 && precision != 0) count = 1;
	while (temp > 0) {
		count++;
		temp /= 10;
	}
	if (precision > count) count = precision;

	if (neg || (flags & 8) || (flags & 16)) count++; // '+', ' '

	if (!(flags & 2) && !(flags & 4)) { // No '-' and no '0'
		while (width > count) {
			console_putchar(' ');
			width--;
		}
	}

	if (neg) console_putchar('-');
	else if (flags & 8) console_putchar('+');
	else if (flags & 16) console_putchar(' ');

	if (!(flags & 2) && (flags & 4)) { // '0' flag
		while (width > count) {
			console_putchar('0');
			width--;
		}
	}

	cprint_uint((uint64_t)value, 10, 0, precision, 0, false);

	if (flags & 2) {
		while (width > count) {
			console_putchar(' ');
			width--;
		}
	}
}

void cvprintf(const char* fmt, va_list args) {
	for (const char* p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			console_putchar(*p);
			continue;
		}

		p++; // move past '%'

		int flags = 0;
		// 1:#, 2:-, 4:0, 8:+, 16:space
		bool parsing_flags = true;
		while (parsing_flags) {
			switch (*p) {
				case '#': flags |= 1; p++; break;
				case '-': flags |= 2; p++; break;
				case '0': flags |= 4; p++; break;
				case '+': flags |= 8; p++; break;
				case ' ': flags |= 16; p++; break;
				default: parsing_flags = false; break;
			}
		}

		int width = 0;
		if (*p == '*') {
			width = va_arg(args, int);
			if (width < 0) {
				flags |= 2;
				width = -width;
			}
			p++;
		} else {
			while (*p >= '0' && *p <= '9') {
				width = width * 10 + (*p - '0');
				p++;
			}
		}

		int precision = -1;
		if (*p == '.') {
			p++;
			if (*p == '*') {
				precision = va_arg(args, int);
				p++;
			} else {
				precision = 0;
				while (*p >= '0' && *p <= '9') {
					precision = precision * 10 + (*p - '0');
					p++;
				}
			}
		}

		int length = 0; // 0:default, 1:h, 2:hh, 3:l, 4:ll, 5:z, 6:j, 7:t
		if (*p == 'h') {
			p++;
			if (*p == 'h') {
				length = 2;
				p++;
			} else {
				length = 1;
			}
		} else if (*p == 'l') {
			p++;
			if (*p == 'l') {
				length = 4;
				p++;
			} else {
				length = 3;
			}
		} else if (*p == 'z') {
			length = 5;
			p++;
		} else if (*p == 'j') {
			length = 6;
			p++;
		} else if (*p == 't') {
			length = 7;
			p++;
		}

		switch (*p) {
			case 'c': {
				char c = (char)va_arg(args, int);
				if (!(flags & 2)) while (--width > 0) console_putchar(' ');
				console_putchar(c);
				if (flags & 2) while (--width > 0) console_putchar(' ');
				break;
			}
			case 's': {
				const char* str = va_arg(args, const char*);
				if (!str) str = "(null)";
				int len = 0;
				while (str[len] && (precision < 0 || len < precision)) len++;
				if (!(flags & 2)) while (width > len) { console_putchar(' '); width--; }
				for (int i = 0; i < len; i++) console_putchar(str[i]);
				if (flags & 2) while (width > len) { console_putchar(' '); width--; }
				break;
			}
			case 'd':
			case 'i': {
				int64_t val;
				if (length == 4) val = va_arg(args, long long);
				else if (length == 3) val = va_arg(args, long);
				else if (length == 2) val = (signed char)va_arg(args, int);
				else if (length == 1) val = (short)va_arg(args, int);
				else if (length == 5) val = (int64_t)va_arg(args, size_t);
				else if (length == 6) val = va_arg(args, intmax_t);
				else if (length == 7) val = va_arg(args, ptrdiff_t);
				else val = va_arg(args, int);
				cprint_int(val, width, precision, flags);
				break;
			}
			case 'u':
			case 'o':
			case 'x':
			case 'X': {
				uint64_t val;
				if (length == 4) val = va_arg(args, unsigned long long);
				else if (length == 3) val = va_arg(args, unsigned long);
				else if (length == 2) val = (unsigned char)va_arg(args, int);
				else if (length == 1) val = (unsigned short)va_arg(args, int);
				else if (length == 5) val = va_arg(args, size_t);
				else if (length == 6) val = va_arg(args, uintmax_t);
				else if (length == 7) val = va_arg(args, ptrdiff_t);
				else val = va_arg(args, unsigned int);
				
				int base = 10;
				if (*p == 'o') base = 8;
				else if (*p == 'x' || *p == 'X') base = 16;
				cprint_uint(val, base, width, precision, flags, (*p == 'X'));
				break;
			}
			case 'p': {
				cprint_uint((uint64_t)va_arg(args, void*), 16, width, precision, flags | 1, false);
				break;
			}
			case '%':
				console_putchar('%');
				break;
			default:
				console_putchar('%');
				if (*p) console_putchar(*p);
				break;
		}
	}
}

void cprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	cvprintf(fmt, args);
	va_end(args);
}
