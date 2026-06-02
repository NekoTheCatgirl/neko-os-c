#include "serial.h"

#include <stddef.h>

#include "x86.h"

void serial_init() {
	x86_outb(COM1 + 1, 0x00);  // disable interrupts
	x86_outb(COM1 + 3, 0x80);  // enable DLAB to set baud rate
	x86_outb(COM1 + 0, 0x03);  // baud rate low byte (38400)
	x86_outb(COM1 + 1, 0x00);  // baud rate high byte
	x86_outb(COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit
	x86_outb(COM1 + 2, 0xC7);  // enable FIFO, clear, 14-byte threshold
	x86_outb(COM1 + 4, 0x0B);  // IRQs enabled, RTS/DSR set
}

static bool serial_is_transmit_empty() {
	return x86_inb(COM1 + 5) & 0x20;
}

void serial_putchar(char c) {
	while (!serial_is_transmit_empty());
	x86_outb(COM1, c);
}

void serial_print(const char* str) {
	for (size_t i = 0; str[i] != '\0'; i++)
		serial_putchar(str[i]);
}
