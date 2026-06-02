#include "acpi.h"

#include <madt.h>

#include "klogf.h"
#include "mem.h"
#include "panic.h"

void acpi_parse_rsdt(rsdt_t* rsdt);
void acpi_parse_xsdt(xsdt_t* xsdt);
void acpi_parse_tables(acpi_sdt_header_t* header, int entry_size);

void acpi_init(mb2_info_t* info) {
	auto rsdp = get_acpi_rsdp(info);

	if (!rsdp)
		kpanic("Failed to find ACPI RSDP");

	if (rsdp->revision >= 2) {
		auto rsdp_ext = (rsdp_v2_t*)rsdp;
		if (!rsdp_ext->xsdt_address)
			kpanic("ACPI 2.0 RSDP has no XSDT address");

		klog(LOG_INFO, "Using XSDT at 0x%x", rsdp_ext->xsdt_address);
		auto xsdt = (xsdt_t*)rsdp_ext->xsdt_address;
		acpi_parse_xsdt(xsdt);
	} else {
		if (!rsdp->rsdt_address)
			kpanic("ACPI 1.0 RSDP has no RSDT address");

		klog(LOG_INFO, "Using RSDT at 0x%x", rsdp->rsdt_address);
		auto rsdt = (rsdt_t*)(uint64_t)rsdp->rsdt_address;
		acpi_parse_rsdt(rsdt);
	}
}

void acpi_parse_rsdt(rsdt_t* rsdt) {
	klog(LOG_INFO, "Parsing RSDT");
	acpi_parse_tables((acpi_sdt_header_t*)rsdt, 4);
}

void acpi_parse_xsdt(xsdt_t* xsdt) {
	klog(LOG_INFO, "Parsing XSDT");
	acpi_parse_tables((acpi_sdt_header_t*)xsdt, 8);
}

void acpi_parse_tables(acpi_sdt_header_t* header, int entry_size) {
	uint32_t count = (header->length - sizeof(acpi_sdt_header_t)) / entry_size;
	auto entries = (uint8_t*)(header + 1);

	for (uint32_t i = 0; i < count; i++) {
		uint64_t addr = 0;
		if (entry_size == 4)
			addr = (uint64_t)*(uint32_t*)(entries + i * 4);
		else
			addr = *(uint64_t*)(entries + i * 8);

		acpi_sdt_header_t* table = (acpi_sdt_header_t*)addr;
		klog(LOG_INFO, "Found ACPI table: %.4s", table->signature);
		// handle the table...
		if (memcmp(table->signature, "APIC", 4) == 0) {
			klog(LOG_INFO, "Starting to parse APIC table");
			madt_parse((madt_t*)table);
		}
	}
}
