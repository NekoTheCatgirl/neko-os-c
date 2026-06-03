#include "lapic.h"

#include "klogf.h"
#include "madt.h"
#include "paging.h"
#include "x86.h"

static uint64_t lapic_base = 0;

uint32_t lapic_read(const uint32_t reg) {
	return *(volatile uint32_t*)(lapic_base + reg);
}

void lapic_write(const uint32_t reg, const uint32_t val)  {
	*(volatile uint32_t*)(lapic_base + reg) = val;
}

void lapic_init() {
	lapic_base = madt_get_lapic_base();
	paging_map_mmio_region(lapic_base, 0x1000);

	// Enable LAPIC via IA32_APIC_BASE MSR
	uint64_t msr = x86_rdmsr(0x1B);
	msr |= (1 << 11); // Global enable bit
	x86_wrmsr(0x1B, msr);

	// Set spurious interrupt vector to 0xFF and enable
	lapic_write(LAPIC_SPURIOUS, LVT_VECTOR_MASK | LAPIC_ENABLE);

	// Set task priority to 0 - accept all interrupts
	lapic_write(LAPIC_TPR, 0);

	// Configure timer
	lapic_write(LAPIC_TIMER_DIV, 0x3);
	lapic_write(LAPIC_TIMER, 0x20 | LVT_TIMER_PERIODIC);
	lapic_write(LAPIC_TIMER_INIT, 0x100000);

	// Enable timer
	auto const timer = lapic_read(LAPIC_TIMER);
	lapic_write(LAPIC_TIMER, timer);

	klog(LOG_INFO, "LAPIC id=%d version=0x%x", lapic_read(LAPIC_ID) >> 24, lapic_read(LAPIC_VERSION) & 0xFF);
}

void lapic_send_ipi(const uint8_t apic_id, const uint32_t flags) {
	// Write destination first (high), then command (low) which triggers sending
	lapic_write(LAPIC_ICR_HIGH, (uint32_t)apic_id << 24);
	lapic_write(LAPIC_ICR_LOW, flags);

	// Spin until delivery status bit clears
	while (lapic_read(LAPIC_ICR_LOW) & (1 << 12))
		__asm__ volatile("pause");
}

void lapic_sleep_ms(uint32_t ms) {
	for (uint32_t i = 0; i < ms; i++)
		for (volatile uint32_t j = 0; j < 100000; j++)
			__asm__ volatile("pause");
}

void lapic_eoi() {
	*(volatile uint32_t*)(lapic_base + LAPIC_EOI) = 0;
}

uint32_t lapic_id() {
    return lapic_read(LAPIC_ID) >> 24;
}

