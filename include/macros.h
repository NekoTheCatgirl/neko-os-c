#pragma once

#define PACKED		   __attribute__((packed))
#define ALIGNED(x)     __attribute__((aligned(x)))
#define NORETURN       __attribute__((noreturn))
#define UNUSED         __attribute__((unused))
#define INLINE         __attribute__((always_inline)) inline
#define NAKED          __attribute__((naked))
#define SECTION(x)     __attribute__((section(x)))

#define INTERRUPT	   __attribute__((interrupt))
#define FASTCALL	   __attribute__((fastcall))
#define NO_CALLER_REGS __attribute__((no_caller_saved_registers))

#define fence() __asm__ volatile ("":::"memory")