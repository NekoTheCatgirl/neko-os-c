#pragma once

#include <stdint.h>

#include "macros.h"
#include "boot/multiboot2.h"

typedef struct {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} PACKED acpi_sdt_header_t;

typedef struct {
	acpi_sdt_header_t header;
	uint32_t tables[];
} PACKED rsdt_t;

typedef struct {
	acpi_sdt_header_t header;
	uint64_t tables[];
} PACKED xsdt_t;

void acpi_init(mb2_info_t* info);