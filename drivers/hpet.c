#include "hpet.h"

#include "klogf.h"
#include "paging.h"
#include "panic.h"

static hpet_t* hpet_table = nullptr;
static uint64_t hpet_base = 0;

static uint64_t hpet_read(uint32_t reg) {
	return *(volatile uint64_t*)(hpet_base + reg);
}

static void hpet_write(uint32_t reg, uint64_t val) {
	*(volatile uint64_t*)(hpet_base + reg) = val;
}

void hpet_parse(hpet_t* hpet) {
	hpet_table = hpet;
}

void hpet_init() {
	if (!hpet_table)
		kpanic("HPET table not found");

	hpet_base = hpet_table->address.address;
	paging_map_mmio_region(hpet_base, 0x1000);

	// Read capabilities
	uint64_t cap = hpet_read(HPET_CAP);
	hpet_period = cap >> 32;
	hpet_freq = 1000000000000000ULL / hpet_period;

	klog(LOG_INFO, "HPET base=0x%x period=%u fs freq=%u Hz",
		hpet_base, hpet_period, hpet_freq);

	// Disable HPET, reset counter, then enable
	hpet_write(HPET_CONFIG, 0);
	hpet_write(HPET_COUNTER, 0);
	hpet_write(HPET_CONFIG, HPET_CFG_ENABLE);
}

void hpet_sleep_ms(uint64_t ms) {
    hpet_sleep_us(ms * 1000);
}

void hpet_sleep_us(uint64_t us) {
	uint64_t ticks = (us * hpet_freq) / 1000000ULL;
	uint64_t target = hpet_ticks() + ticks;
	while (hpet_ticks() < target)
		__asm__ volatile("pause");
}

uint64_t hpet_ticks() {
	return hpet_read(HPET_COUNTER);
}
