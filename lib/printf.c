#include "printf.h"

#include <stddef.h>

static constexpr size_t NUM_COLS = 80;
static constexpr size_t NUM_ROWS = 25;

struct [[maybe_unused]] Char {
	[[maybe_unused]] uint8_t character;
	[[maybe_unused]] uint8_t color;
};

auto buffer = (struct Char*)0xB8000;
size_t g_col = 0;
size_t g_row = 0;
uint8_t g_color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

void clear_row(const size_t row) {
	const struct Char empty = {' ', g_color};

	for (size_t col = 0; col < NUM_COLS; col++)
		buffer[col + NUM_COLS * row] = empty;
}

void print_clear() {
	for (size_t i = 0; i < NUM_ROWS; i++)
		clear_row(i);
}

void print_newline() {
	g_col = 0;

	if (g_row < NUM_ROWS - 1) {
		g_row++;
		return;
	}

	for (size_t row = 1; row < NUM_ROWS; row++) {
		for (size_t col = 0; col < NUM_COLS; col++) {
			const struct Char character = buffer[col + NUM_COLS * row];
			buffer[col + NUM_COLS * (row - 1)] = character;
		}
	}
	clear_row(NUM_ROWS - 1);
}


void print_char(const char character) {
	if (character == '\n') {
		print_newline();
		return;
	}

	if (g_col > NUM_COLS) {
		print_newline();
	}

	buffer[g_col + NUM_COLS * g_row] = (struct Char) { (uint8_t) character, g_color };

	g_col++;
}

void print_str(const char* string) {
	for (size_t i = 0; 1; i++) {
		const char character = string[i];

		if (character == '\0') {
			return;
		}

		print_char(character);
	}
}

void print_set_color(const uint8_t foreground, const uint8_t background) {
	g_color = foreground + (background << 4);
}

static void print_uint(uint64_t value, int base, int width, int precision, int flags, bool upper) {
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
			print_char(pad);
			width--;
		}
	}

	if (flags & 1) {
		if (base == 16) {
			print_str(upper ? "0X" : "0x");
		} else if (base == 8) {
			print_char('0');
		}
	}

	while (actual_precision > i) {
		print_char('0');
		actual_precision--;
	}

	for (int j = i - 1; j >= 0; j--)
		print_char(buf[j]);

	if (flags & 2) { // '-' flag
		while (width > count) {
			print_char(' ');
			width--;
		}
	}
}

static void print_int(int64_t value, int width, int precision, int flags) {
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
			print_char(' ');
			width--;
		}
	}

	if (neg) print_char('-');
	else if (flags & 8) print_char('+');
	else if (flags & 16) print_char(' ');

	if (!(flags & 2) && (flags & 4)) { // '0' flag
		while (width > count) {
			print_char('0');
			width--;
		}
	}

	print_uint((uint64_t)value, 10, 0, precision, 0, false);

	if (flags & 2) {
		while (width > count) {
			print_char(' ');
			width--;
		}
	}
}

void vprintf(const char* fmt, va_list args) {
	for (const char* p = fmt; *p != '\0'; p++) {
		if (*p != '%') {
			print_char(*p);
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
				if (!(flags & 2)) while (--width > 0) print_char(' ');
				print_char(c);
				if (flags & 2) while (--width > 0) print_char(' ');
				break;
			}
			case 's': {
				const char* str = va_arg(args, const char*);
				if (!str) str = "(null)";
				int len = 0;
				while (str[len] && (precision < 0 || len < precision)) len++;
				if (!(flags & 2)) while (width > len) { print_char(' '); width--; }
				for (int i = 0; i < len; i++) print_char(str[i]);
				if (flags & 2) while (width > len) { print_char(' '); width--; }
				break;
			}
			case 'd':
			case 'i': {
				int64_t val;
				if (length == 4) val = va_arg(args, long long);
				else if (length == 3) val = va_arg(args, long);
				else if (length == 2) val = (signed char)va_arg(args, int);
				else if (length == 1) val = (short)va_arg(args, int);
				else if (length == 5) val = va_arg(args, size_t);
				else if (length == 6) val = va_arg(args, intmax_t);
				else if (length == 7) val = va_arg(args, ptrdiff_t);
				else val = va_arg(args, int);
				print_int(val, width, precision, flags);
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
				print_uint(val, base, width, precision, flags, (*p == 'X'));
				break;
			}
			case 'p': {
				print_uint((uint64_t)va_arg(args, void*), 16, width, precision, flags | 1, false);
				break;
			}
			case '%':
				print_char('%');
				break;
			default:
				print_char('%');
				if (*p) print_char(*p);
				break;
		}
	}
}

void printf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
