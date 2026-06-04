#include "idt.h"

#include <lapic.h>
#include <scheduler.h>

#include "klogf.h"
#include "panic.h"

static idt_entry_t idt[256];

void idt_set_entry(uint8_t vector, void* handler, uint8_t flags) {
	auto addr = (uint64_t)handler;
	idt_entry_t* entry = &idt[vector];
	entry->offset_low = addr & 0xFFFF;
	entry->offset_mid = (addr >> 16) & 0xFFFF;
	entry->offset_high = (addr >> 32) & 0xFFFFFFFF;
	entry->selector = 0x08;
	entry->ist = 0;
	entry->flags = flags;
	entry->reserved = 0;
}

static const char* exception_names[] = {
	"Division By Zero", "Debug", "NMI", "Breakpoint",
	"Overflow", "Bound Range Exceeded", "Invalid Opcode",
	"Device Not Available", "Double Fault", "Coprocessor Segment Overrun",
	"Invalid TSS", "Segment Not Present", "Stack Fault",
	"General Protection Fault", "Page Fault", "Reserved",
	"x87 Floating Point", "Alignment Check", "Machine Check",
	"SIMD Floating Point", "Virtualization", "Control Protection",
	"Reserved", "Reserved", "Reserved", "Reserved", "Reserved",
	"Reserved", "Hypervisor Injection", "VMM Communication", "Security"
};

void exception_handler(uint64_t vector, uint64_t error_code) {
	uint64_t rip;
	__asm__ volatile("mov 16(%%rbp), %0" : "=r"(rip));
	kpanic("Exception %u: %s (error code: 0x%x) at rip=0x%x",
		vector, exception_names[vector], error_code, rip);
}

extern void isr0(), isr1(), isr2(), isr3(), isr4(), isr5(),
			isr6(), isr7(), isr8(), isr9(), isr10(), isr11(),
			isr12(), isr13(), isr14(), isr15(), isr16(), isr17(),
			isr18(), isr19(), isr20(), isr21(), isr22(), isr23(),
			isr24(), isr25(), isr26(), isr27(), isr28(), isr29(),
			isr30(), isr31();

static volatile uint64_t timer_count = 0;

void timer_handler() {
	lapic_eoi();
	scheduler_tick();
}

void spurious_handler() {
	klog(LOG_WARN, "Spurious interrupt received");
}

extern void isr_spurious(), isr_timer();

void idt_init() {

	void* handlers[] = {
		isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
		isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
		isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
		isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
	};

	for (int i = 0; i < 32; i++)
		idt_set_entry(i, handlers[i], 0x8E); // 0x8E = present, ring 0, interrupt gate

	idt_set_entry(0x20, isr_timer, 0x8E);
	idt_set_entry(0xFF, isr_spurious, 0x8E);

	struct {
		uint16_t limit;
		uint64_t base;
	} PACKED idtr = {
		.limit = sizeof(idt) - 1,
		.base  = (uint64_t)idt
	};

	__asm__ volatile ("lidt %0" : : "m"(idtr));
	klog(LOG_INFO, "IDTR base: 0x%x limit: %u", idtr.base, idtr.limit);
}
