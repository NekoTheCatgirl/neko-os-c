#include "smp.h"

#include <hpet.h>
#include <klogf.h>
#include <lapic.h>
#include <madt.h>
#include <scheduler.h>

#include "macros.h"
#include "mem.h"

#define AP_STACK_SIZE 0x4000
#define MAX_APS 16

static uint32_t aps_started = 0;
static uint32_t aps_expected = 0;
static uint8_t ap_stacks[MAX_APS][AP_STACK_SIZE] ALIGNED(16);

static uint64_t get_ap_stack(uint8_t cpu_id) {
	return (uint64_t)ap_stacks[cpu_id] + AP_STACK_SIZE; // Top of the stack
}

extern uint8_t trampoline_start[];
extern uint8_t trampoline_end[];
extern uint8_t trampoline_pm[];
extern uint8_t trampoline_pm_jmp[];
extern uint8_t trampoline_gdt[];
extern uint8_t trampoline_gdt_ptr[];
extern uint8_t trampoline_gdt64[];
extern uint8_t trampoline_gdt64_ptr[];
extern uint8_t trampoline_pml4[];
extern uint8_t trampoline_stack[];
extern uint8_t trampoline_entry[];

void ap_main() {
	lapic_init();

	lapic_write(LAPIC_TIMER, LVT_MASK);

	uint32_t id = lapic_id();
	klog(LOG_INFO, "AP %d online", id);

	__sync_fetch_and_add(&aps_started, 1);
	__sync_synchronize();

	while (aps_started < aps_expected) __asm__ volatile("pause");

	cpu_enter_worker();
}

void smp_init() {
	size_t size = trampoline_end - trampoline_start;
	// Copy the trampoline to the correct memory address.
	uint8_t* src = trampoline_start;
	uint8_t* dst = (uint8_t*)SMP_TRAMPOLINE_ADDR;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];

	// Patch GDT pointers
	uint32_t gdt_phys = SMP_TRAMPOLINE_ADDR + (trampoline_gdt - trampoline_start);
	uint32_t gdt64_phys = SMP_TRAMPOLINE_ADDR + (trampoline_gdt64 - trampoline_start);

	*(uint32_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_gdt_ptr - trampoline_start) + 2) = gdt_phys;
	*(uint32_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_gdt64_ptr - trampoline_start) + 2) = gdt64_phys;

	// Patch far jump — write offset and segment separately
	uint32_t pm_target = SMP_TRAMPOLINE_ADDR + (trampoline_pm - trampoline_start);
	uint8_t* jmp_patch = (uint8_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_pm_jmp - trampoline_start));
	jmp_patch[0] = 0xEA;                          // far jmp opcode
	jmp_patch[1] = (pm_target) & 0xFF;            // offset low
	jmp_patch[2] = (pm_target >> 8) & 0xFF;       // offset high
	jmp_patch[3] = 0x08;                          // segment low (code segment)
	jmp_patch[4] = 0x00;                          // segment high

	// Patch data area
	uint64_t cr3;
	__asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));

	uint32_t bsp_id = lapic_id();
	uint32_t cpu_count = madt_get_cpu_count();
	aps_expected = cpu_count - 1;

	klog(LOG_INFO, "BSP LAPIC id=%d starting %d APs", bsp_id, cpu_count - 1);

	for (uint8_t cpu = 0; cpu < (uint8_t)cpu_count; cpu++) {
		uint8_t apic_id = madt_get_lapic_id(cpu);
		if (apic_id == bsp_id) continue;
		uint32_t expected = aps_started + 1;

		klog(LOG_INFO, "Starting AP cpu=%d apic_id=%d", cpu, apic_id);

		// Patch per-AP data
		*(uint32_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_pml4 - trampoline_start)) = (uint32_t)cr3;
		*(uint64_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_stack - trampoline_start)) = get_ap_stack(cpu);
		*(uint64_t*)(SMP_TRAMPOLINE_ADDR + (trampoline_entry - trampoline_start)) = (uint64_t)ap_main;

		// INIT IPI
		lapic_send_ipi(apic_id, 0x00004500);
		hpet_sleep_ms(10);

		// SIPI (vector = trampoline page = 0x8000 >> 12 = 0x08)
		lapic_send_ipi(apic_id, 0x00004608);
		hpet_sleep_ms(1);

		// Second SIPI
		lapic_send_ipi(apic_id, 0x00004608);
		hpet_sleep_ms(1);

		// Wait for this AP to check in before starting the next one
		uint64_t timeout = 5000;  // 5-second timeout
		while (aps_started < expected) {
			hpet_sleep_ms(1);
			if (--timeout == 0) {
				klog(LOG_WARN, "AP cpu=%d timed out", cpu);
				break;
			}
			__asm__ volatile("pause");
		}
	}

	while (aps_started < cpu_count - 1)
		hpet_sleep_ms(1);

	klog(LOG_INFO, "All %d APs online", aps_started);
}