#pragma once
#include <stddef.h>
#include <stdint.h>

#include "macros.h"

typedef struct {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t granularity;
	uint8_t base_high;
} PACKED gdt_entry_t;

typedef struct {
	uint32_t base_high;
	uint32_t reserved;
} PACKED gdt_entry_high_t;

typedef struct {
	gdt_entry_t null;
	gdt_entry_t kernel_code;
	gdt_entry_t kernel_data;
	gdt_entry_t tss_low;
	gdt_entry_high_t tss_high;
} PACKED ALIGNED(8) gdt_t;

typedef struct {
	uint32_t reserved0;
	uint64_t rsp0; // Stack pointer for ring 0
	uint64_t rsp1; // Stack pointer for ring 1
	uint64_t rsp2; // Stack pointer for ring 2
	uint64_t reserved1;
	uint64_t ist[7]; // Interrupt stack table
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t iopb_offset;
} PACKED tss_t;

void gdt_init();