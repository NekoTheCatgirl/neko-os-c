#include "gdt.h"

#include "mem.h"

static gdt_t gdt;
static tss_t tss;

static void gdt_set_entry(gdt_entry_t* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
	entry->limit_low = limit & 0xFFFF;
	entry->base_low = base & 0xFFFF;
	entry->base_mid = (base >> 16) & 0xFF;
	entry->access = access;
	entry->granularity = (granularity & 0xF0) | ((limit >> 16) & 0x0F);
	entry->base_high = (base >> 24) & 0xFF;
}

static void gdt_set_tss(uint64_t base, uint32_t limit) {
	gdt_set_entry(&gdt.tss_low, (uint32_t)base, limit, 0x89, 0x00);
	gdt.tss_high.base_high = (uint32_t)(base >> 32);
	gdt.tss_high.reserved  = 0;
}

void gdt_init() {
	// Null descriptor
	gdt_set_entry(&gdt.null, 0, 0, 0, 0);

	// Kernel code segment - 0x9A = present, ring 0, code, executable, readable
	gdt_set_entry(&gdt.kernel_code, 0, 0xFFFFF, 0x9A, 0xA0);

	// Kernel data segment - 0x92 = present, ring 0, data, writable
	gdt_set_entry(&gdt.kernel_data, 0, 0xFFFFF, 0x92, 0xC0);

	// TSS
	memset(&tss, 0, sizeof(tss_t));
	tss.iopb_offset = sizeof(tss_t);
	gdt_set_tss((uint64_t)&tss, sizeof(tss_t) - 1);

	// Load GDT
	struct {
		uint16_t limit;
		uint64_t base;
	} PACKED gdtr = {
		.limit = sizeof(gdt) - 1,
		.base  = (uint64_t)&gdt
	};

	__asm__ volatile (
		"lgdt %0\n"
		"mov $0x10, %%ax\n"   // 0x10 = kernel data segment
		"mov %%ax, %%ds\n"
		"mov %%ax, %%es\n"
		"mov %%ax, %%fs\n"
		"mov %%ax, %%gs\n"
		"mov %%ax, %%ss\n"
		"mov $0x18, %%ax\n"   // 0x18 = TSS descriptor
		"ltr %%ax\n"
		: : "m"(gdtr) : "ax", "memory"
	);
}