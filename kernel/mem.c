#include "mem.h"

#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
	const auto d = (uint8_t*)dest;
	const auto s = (const uint8_t*)src;
	for (size_t i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
	const auto d = (uint8_t*)dest;
	const auto s = (const uint8_t*)src;
	if (d < s) {
		for (size_t i = 0; i < n; i++)
			d[i] = s[i];
	} else {
		for (size_t i = n; i > 0; i--)
			d[i - 1] = s[i - 1];
	}
	return dest;
}

void* memset(void* dest, int val, size_t n) {
	const auto d = (uint8_t*)dest;
	for (size_t i = 0; i < n; i++)
		d[i] = val;
	return dest;
}

int memcmp(const void* a, const void* b, size_t n) {
	const auto pa = (const uint8_t*)a;
	const auto pb = (const uint8_t*)b;
	for (size_t i = 0; i < n; i++) {
		if (pa[i] < pb[i]) return -1;
		if (pa[i] > pb[i]) return  1;
	}
	return 0;
}
