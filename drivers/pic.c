#include "pic.h"

#include "klogf.h"
#include "x86.h"

void pic_disable() {
	x86_outb(PIC1_COMMAND, 0xFF);
	x86_outb(PIC2_COMMAND, 0xFF);
	klog(LOG_INFO, "PIC disabled");
}