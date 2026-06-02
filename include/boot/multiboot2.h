#pragma once
#include "../macros.h"

#include <stdint.h>

#define MB2_MAGIC 0x36D76289

typedef struct {
	uint32_t type;
	uint32_t size;
} PACKED mb2_tag_t;

typedef struct {
	uint32_t total_size;
	uint32_t reserved;
	mb2_tag_t tags[];
} PACKED mb2_info_t;

#define MB2_TAG_END         0
#define MB2_TAG_CMDLINE     1
#define MB2_TAG_BOOTLOADER  2
#define MB2_TAG_MMAP      6

typedef struct {
	mb2_tag_t tag;
	char string[];
} PACKED mb2_tag_string_t;

typedef struct {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;
} PACKED mb2_mmap_entry_t;

typedef struct {
	mb2_tag_t tag;
	uint32_t entry_size;
	uint32_t entry_version;
	mb2_mmap_entry_t entries[];
} PACKED mb2_tag_mmap_t;

#define MB2_TAG_ACPI_V1 14
#define MB2_TAG_ACPI_V2 15

typedef struct {
	mb2_tag_t tag;
	uint8_t rsdp[];
} PACKED mb2_tag_acpi_t;

// ACPI 1.0 RSDP
typedef struct {
	char signature[8];      // "RSD PTR "
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;       // 0 = ACPI 1.0, 2 = ACPI 2.0+
	uint32_t rsdt_address;
} PACKED rsdp_v1_t;

// ACPI 2.0+ XSDP (extended)
typedef struct {
	rsdp_v1_t v1;
	uint32_t length;
	uint64_t xsdt_address;  // 64-bit address, prefer this over rsdt_address
	uint8_t extended_checksum;
	uint8_t reserved[3];
} PACKED rsdp_v2_t;

#define MB2_TAG_FRAMEBUFFER 8

typedef struct {
	mb2_tag_t tag;
	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;  // Bytes per row
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t  framebuffer_bpp;    // Bits per pixel
	uint8_t  framebuffer_type;   // 1 = RGB
	uint16_t reserved;
} PACKED mb2_tag_framebuffer_t;

void mb2_parse(mb2_info_t* info);

rsdp_v1_t* get_acpi_rsdp(mb2_info_t* info);