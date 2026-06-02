#include <console.h>
#include <lapic.h>
#include <pic.h>
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

	fb_draw_cpu_logos(madt_get_cpu_count());
	klog(LOG_INFO, "UwU");

	while (1) {
		__asm__ __volatile__("hlt");
	}
}