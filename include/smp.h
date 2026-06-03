#pragma once
#include <stdint.h>

extern uint64_t trampoline_load_addr;
#define SMP_TRAMPOLINE_ADDR ((uint64_t)&trampoline_load_addr)

void smp_init();