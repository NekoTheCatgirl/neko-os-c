#include "madt.h"

#include "klogf.h"

static madt_t* madt_table = nullptr;
static uint32_t cpu_count = 0;

void madt_parse(madt_t* madt) {
	madt_table = madt;

	uint8_t* ptr = madt->entries;
	const uint8_t* end = (uint8_t*)madt + madt->header.length;

	while (ptr < end) {
		auto const h = (madt_ics_header_t*)ptr;

		switch (h->type) {
			case MADT_TYPE_LAPIC:
				auto const lapic = (madt_lapic_t*)ptr;
				if (lapic->flags & 1)  // only count enabled CPUs
					cpu_count++;
				klog(LOG_INFO, "LAPIC: cpu=%d apic_id=%d flags=%d",
					lapic->acpi_processor_uid, lapic->apic_id, lapic->flags);
				break;
			case MADT_TYPE_IOAPIC:
				auto const ioapic = (madt_ioapic_t*)ptr;
				klog(LOG_INFO, "IOAPIC: id=%d addr=0x%x gsi_base=%d",
					ioapic->ioapic_id, ioapic->ioapic_addr, ioapic->gsi_base);
				break;
			case MADT_TYPE_ISO:
				auto const iso = (madt_iso_t*)ptr;
				klog(LOG_INFO, "ISO: irq=%d -> gsi=%d flags=0x%x",
					iso->source, iso->gsi, iso->flags);
				break;
			case MADT_TYPE_LAPIC_NMI:
				auto const nmi = (madt_lapic_nmi_t*)ptr;
				klog(LOG_INFO, "LAPIC NMI: cpu=%d lint=%d",
					nmi->acpi_processor_uid, nmi->lint);
				break;
			default:
				break;
		}

		ptr += h->length;
	}
}

uint64_t madt_get_lapic_base() {
	uint8_t* ptr = madt_table->entries;
	uint8_t* end = (uint8_t*)madt_table + madt_table->header.length;

	while (ptr < end) {
		madt_ics_header_t* h = (madt_ics_header_t*)ptr;
		if (h->type == MADT_TYPE_LAPIC_OVERRIDE) {
			// Local APIC Address Override — 64-bit address
			typedef struct {
				madt_ics_header_t header;
				uint16_t reserved;
				uint64_t addr;
			} PACKED lapic_override_t;
			return ((lapic_override_t*)ptr)->addr;
		}
		ptr += h->length;
	}

	// Fall back to 32-bit address in MADT header
	return madt_table->lapic_addr;
}

uint8_t  madt_get_lapic_id(uint8_t cpu_id) {
	uint8_t* ptr = madt_table->entries;
	uint8_t* end = (uint8_t*)madt_table + madt_table->header.length;

	while (ptr < end) {
		madt_ics_header_t* h = (madt_ics_header_t*)ptr;
		if (h->type == MADT_TYPE_LAPIC) {
			madt_lapic_t* lapic = (madt_lapic_t*)ptr;
			if (lapic->acpi_processor_uid == cpu_id && (lapic->flags & 1))
				return lapic->apic_id;
		}
		ptr += h->length;
	}

	return 0xFF; // not found
}

uint32_t madt_get_cpu_count() { return cpu_count; }

uint32_t madt_get_ioapic_addr(uint8_t ioapic_id) {
	uint8_t* ptr = madt_table->entries;
	uint8_t* end = (uint8_t*)madt_table + madt_table->header.length;

	while (ptr < end) {
		madt_ics_header_t* h = (madt_ics_header_t*)ptr;
		if (h->type == MADT_TYPE_IOAPIC) {
			madt_ioapic_t* ioapic = (madt_ioapic_t*)ptr;
			if (ioapic->ioapic_id == ioapic_id)
				return ioapic->ioapic_addr;
		}
		ptr += h->length;
	}

	return 0; // not found
}
