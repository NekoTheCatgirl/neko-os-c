#pragma once
#include <stdint.h>

#include "acpi.h"
#include "macros.h"

typedef struct {
	uint8_t type;
	uint8_t length;
} PACKED madt_ics_header_t;

typedef struct {
	madt_ics_header_t header;
	uint8_t acpi_processor_uid;
	uint8_t apic_id;
	uint32_t flags;
} PACKED madt_lapic_t;

typedef struct {
	madt_ics_header_t header;
	uint8_t ioapic_id;
	uint8_t reserved;
	uint32_t ioapic_addr;
	uint32_t gsi_base;
} PACKED madt_ioapic_t;

typedef struct {
	madt_ics_header_t header;
	uint8_t bus;
	uint8_t source;
	uint32_t gsi;
	uint16_t flags;
} PACKED madt_iso_t;

typedef struct {
	madt_ics_header_t header;
	uint8_t acpi_processor_uid;
	uint16_t flags;
	uint8_t lint;
} PACKED madt_lapic_nmi_t;

typedef enum {
	MADT_TYPE_LAPIC      = 0,
	MADT_TYPE_IOAPIC     = 1,
	MADT_TYPE_ISO        = 2,
	MADT_TYPE_NMI        = 3,
	MADT_TYPE_LAPIC_NMI  = 4,
	MADT_TYPE_LAPIC_OVERRIDE = 5,
} madt_ics_type_t;

typedef struct {
	acpi_sdt_header_t header;
	uint32_t lapic_addr;
	uint32_t flags;
	uint8_t entries[];
} PACKED madt_t;

void madt_parse(madt_t* madt);
uint64_t madt_get_lapic_base();
uint8_t  madt_get_lapic_id(uint8_t cpu_id);
uint32_t madt_get_cpu_count();
uint32_t madt_get_ioapic_addr(uint8_t ioapic_id);