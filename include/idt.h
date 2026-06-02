#pragma once
#include <stdint.h>

#include "addr.h"
#include "macros.h"

typedef struct {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
} PACKED idt_entry_t;

typedef struct {
	virt_addr_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
} PACKED interrupt_frame_t;

void idt_init();