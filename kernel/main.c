#include <console.h>
#include <hpet.h>
#include <lapic.h>
#include <pic.h>
#include <scheduler.h>
#include <smp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "acpi.h"
#include "madt.h"
#include "framebuffer.h"
#include "idt.h"
#include "klogf.h"
#include "paging.h"
#include "panic.h"
#include "serial.h"
#include "x86.h"
#include "alloc.h"
#include "boot/multiboot2.h"

#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

extern const char ld_kernel_start[];
extern const char ld_kernel_end[];

extern const char ld_heap_start[];
extern const char ld_heap_end[];

static void log_task(const void* arg) {
	(void)arg;
	while (true) {
		const auto cpu = cpu_local();
		const auto id = cpu->current->cpu_id;
		klog(LOG_INFO, "Hello from task! Cpu ID = %d", id);
		scheduler_yield();
		hpet_sleep_ms(1000);
	}
}

static void test_task(void* arg) {
	uint32_t id = (uint32_t)(uint64_t)arg;
	klog(LOG_INFO, "Task %d running on cpu %d", id, lapic_id());
	for (volatile int i = 0; i < 1000000; i++);  // burn some cycles
	klog(LOG_INFO, "Task %d done", id);
}

void kernel_main(mb2_info_t* mb2_info, const uint32_t mb2_magic) {
	serial_init();
	idt_init();
	paging_init(mb2_info);
	kalloc_init((void*)ld_heap_start, (uint64_t)(ld_heap_end - ld_heap_start));

	if (mb2_magic != 0x36D76289) {
		kpanic("Invalid Multiboot2 magic");
		return;
	}

	mb2_parse(mb2_info);
	console_init();

	klog(LOG_INFO, "Neko OS Booting...");
	klog(LOG_INFO, "Kernel: 0x%x - 0x%x (%u bytes)",
		(uint64_t)ld_kernel_start,
		(uint64_t)ld_kernel_end,
		(uint64_t)(ld_kernel_end - ld_kernel_start));

	pic_disable();
	acpi_init(mb2_info);

	lapic_init();
	hpet_init();

	scheduler_init();

	x86_disable_interrupts();
	smp_init();
	x86_enable_interrupts();

	for (int i = 0; i < 8; i++) {
		task_t* t = task_create(test_task, (void*)(uint64_t)i);
		scheduler_enqueue(t);
	}

	cpu_enter_worker();
	klog(LOG_ERROR, "cpu_enter_worker returned, this should never happen");
}