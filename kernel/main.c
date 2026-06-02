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
#include "printf.h"
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

	// Parse mb2 first so fb_init gets called
	if (mb2_magic != 0x36D76289) {
		// Can't log yet, write to serial at least
		kpanic("Invalid Multiboot2 magic");
		return;
	}

	mb2_parse(mb2_info);   // fb_init happens in here
	console_init();        // font renderer on top of framebuffer
	print_clear();         // now safe to use


	// Now we can log
	klog(LOG_INFO, "Neko OS Booting...");
	klog(LOG_INFO, "Kernel: 0x%x - 0x%x (%u bytes)",
		(uint64_t)ld_kernel_start,
		(uint64_t)ld_kernel_end,
		(uint64_t)(ld_kernel_end - ld_kernel_start));

	pic_disable();
	acpi_init(mb2_info);
	klog(LOG_INFO, "UwU");

	lapic_init();

	fb_draw_cpu_logos(madt_get_cpu_count());
}