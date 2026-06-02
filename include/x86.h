#pragma once

#include <stdbool.h>
#include <stdint.h>

static inline void x86_outb(uint16_t port, uint8_t value) {
	__asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

static inline void x86_outw(uint16_t port, uint16_t value) {
	__asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

static inline void x86_outl(uint16_t port, uint32_t value) {
	__asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port) : "memory");
}

static inline uint8_t x86_inb(uint16_t port) {
	uint8_t value;
	__asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port) : "memory");
	return value;
}

static inline uint16_t x86_inw(uint16_t port) {
	uint16_t value;
	__asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port) : "memory");
	return value;
}

static inline uint32_t x86_inl(uint16_t port) {
	uint32_t value;
	__asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port) : "memory");
	return value;
}

static inline uint8_t x86_farpeekb(uint16_t sel, void* off) {
	uint8_t ret;
	__asm__ volatile (
		"push %%fs\n"
		"mov %1, %%fs\n"
		"movb %%fs:(%2), %0\n"
		"pop %%fs"
		: "=r"(ret) : "g"(sel), "r"(off)
	);
	return ret;
}

static inline uint16_t x86_farpeekw(uint16_t sel, void* off) {
	uint16_t ret;
	__asm__ volatile (
		"push %%fs\n"
		"mov %1, %%fs\n"
		"movw %%fs:(%2), %0\n"
		"pop %%fs"
		: "=r"(ret) : "g"(sel), "r"(off)
	);
	return ret;
}


static inline uint32_t x86_farpeekl(uint16_t sel, void* off) {
	uint32_t ret;
	__asm__ volatile (
		"push %%fs\n"
		"mov %1, %%fs\n"
		"mov %%fs:(%2), %0\n"
		"pop %%fs"
		: "=r"(ret) : "g"(sel), "r"(off)
	);
	return ret;
}

static inline void x86_farpokel(uint16_t sel, void* off, uint32_t val) {
	__asm__ volatile (
		"push %%fs\n"
		"mov %0, %%fs\n"
		"mov %2, %%fs:(%1)\n"
		"pop %%fs"
		: : "g"(sel), "r"(off), "r"(val)
	);
}

static inline void x86_farpokew(uint16_t sel, void* off, uint16_t val) {
	__asm__ volatile (
		"push %%fs\n"
		"mov %0, %%fs\n"
		"movw %2, %%fs:(%1)\n"
		"pop %%fs"
		: : "g"(sel), "r"(off), "r"(val)
	);
}

static inline void x86_farpokeb(uint16_t sel, void* off, uint8_t val) {
	__asm__ volatile (
		"push %%fs\n"
		"mov %0, %%fs\n"
		"movb %2, %%fs:(%1)\n"
		"pop %%fs"
		: : "g"(sel), "r"(off), "r"(val)
	);
}

static inline void x86_io_wait(void) {
	x86_outb(0x80, 0);
}

static inline bool x86_are_interrupts_enabled(void) {
	unsigned long flags;
	__asm__ volatile (
		"pushf\n"
		"pop %0"
		: "=g"(flags)
	);
	return flags & (1 << 9);
}

static inline unsigned long x86_save_irq_disable(void) {
	unsigned long flags;
	__asm__ volatile (
		"pushf\n"
		"cli\n"
		"pop %0"
		: "=r"(flags) : : "memory"
	);
	return flags;
}

static inline void x86_irq_restore(unsigned long flags) {
	__asm__ volatile (
		"push %0\n"
		"popf"
		: : "rm"(flags) : "memory","cc"
	);
}

static inline void x86_enable_interrupts(void) {
	__asm__ volatile ("sti");
}

static inline void x86_disable_interrupts(void) {
	__asm__ volatile ("cli");
}

static inline void x86_hlt(void) {
	__asm__ volatile ("hlt");
}

static inline void x86_cpuid(int code, uint32_t* a, uint32_t* d) {
	__asm__ volatile ("cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx");
}

static inline uint64_t x86_rdtsc(void) {
#if defined(__x86_64__)
	// 64-bit
	uint32_t lo, hi;
	__asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
#elif defined(__i386__)
	// 32-bit
	uint64_t ret;
	__asm__ volatile ("rdtsc" : "=A"(ret));
	return ret;
#endif
}

#if defined(__x86_64__)
static inline uint64_t x86_rdmsr(uint64_t msr) {
	// 64-bit
	uint32_t low, high;
	asm volatile (
		"rdmsr"
		: "=a"(low), "=d"(high)
		: "c"(msr)
	);
	return ((uint64_t)high << 32) | low;
}
#elif defined(__i386__)
static inline uint64_t x86_rdmsr(uint32_t msr_id) {
	// 32-bit
	uint64_t msr_value;
	__asm__ volatile ( "rdmsr" : "=A" (msr_value) : "c" (msr_id) );
	return msr_value;
}
#endif


#if defined(__x86_64__)
static inline void x86_wrmsr(uint64_t msr, uint64_t value) {
	// 64-bit
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}
#elif defined(__i386__)
static inline void x86_wrmsr(uint32_t msr_id, uint64_t msr_value) {
	// 32-bit
	__asm__ volatile ("wrmsr" : : "c" (msr_id), "A" (msr_value));
}
#endif

