#pragma once
#include <stdint.h>

#include "macros.h"

// LAPIC registers (offsets from base)
#define LAPIC_ID            0x020
#define LAPIC_VERSION       0x030
#define LAPIC_TPR           0x080   // Task Priority
#define LAPIC_EOI           0x0B0   // End of Interrupt
#define LAPIC_SPURIOUS      0x0F0   // Spurious Interrupt Vector
#define LAPIC_ICR_LOW       0x300   // Interrupt Command (low)
#define LAPIC_ICR_HIGH      0x310   // Interrupt Command (high)
#define LAPIC_TIMER         0x320   // Timer LVT
#define LAPIC_TIMER_INIT    0x380   // Timer Initial Count
#define LAPIC_TIMER_CURRENT 0x390   // Timer Current Count
#define LAPIC_TIMER_DIV     0x3E0   // Timer Divide Config

#define LAPIC_ENABLE        (1 << 8)  // Spurious vector enable bit

// LVT Shenanegans
#define LVT_VECTOR_MASK			0xFF
#define LVT_DELIVRY_MODE		(0x7 << 8);
#define LVT_PENDING				(1 << 12);
#define LVT_POLARITY			(1 << 13) // LINT only
#define LVT_REMOTE_IRR			(1 <<  14) // LINT only
#define LVT_TRIGGER_MODE		(1 << 15) // LINT only
#define LVT_MASK				(1 << 16)
#define LVT_TIMER_PERIODIC		(1 << 17)
#define LVT_TIMER_TSC_DEADLINE	(1 << 18)

uint32_t lapic_read(uint32_t reg);
void lapic_write(uint32_t reg, uint32_t val);
void lapic_init();
void lapic_send_ipi(uint8_t apic_id, uint32_t flags);
void lapic_sleep_ms(uint32_t ms);
void lapic_eoi();
uint32_t lapic_id();