#include "pic.h"

#include "klogf.h"
#include "x86.h"

void pic_disable() {
	x86_outb(PIC1_DATA, 0xFF);
	x86_io_wait();
	x86_outb(PIC2_DATA, 0xFF);
	x86_io_wait();

	klog(LOG_INFO, "PIC disabled");
}
