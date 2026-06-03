#pragma once

#include <stdint.h>

#include "acpi.h"
#include "macros.h"

// HPET register offsets
#define HPET_CAP         0x000  // General Capabilities and ID
#define HPET_CONFIG      0x010  // General Configuration
#define HPET_STATUS      0x020  // General Interrupt Status
#define HPET_COUNTER     0x0F0  // Main Counter Value

// Config register bits
#define HPET_CFG_ENABLE  (1 << 0)
#define HPET_CFG_LEGACY  (1 << 1)

static uint64_t hpet_period = 0;  // femtoseconds per tick
static uint64_t hpet_freq = 0;  // ticks per second

typedef struct {
	acpi_sdt_header_t header;
	uint8_t hardware_ref_id;
	uint8_t comparator_count:5;
	uint8_t counter_size:1;
	uint8_t reserved:1;
	uint8_t legacy_replacement:1;
	uint16_t pci_vendor_id;
	acpi_address_t address;
	uint8_t hpet_number;
	uint16_t minimum_tick;
	uint8_t page_protection;
} PACKED hpet_t;

void hpet_parse(hpet_t* hpet);
void hpet_init();
void hpet_sleep_ms(uint64_t ms);
void hpet_sleep_us(uint64_t us);
uint64_t hpet_ticks();