#ifndef __MMIO_H
#define __MMIO_H

#include <stddef.h>
#include <stdint.h>

#define MMIO_BASE 0x3F000000

// Memory-Mapped I/O output
inline void mmio_write(uint32_t reg, uint32_t data)
{
	*(volatile uint32_t*)(MMIO_BASE + reg) = data;
}

// Memory-Mapped I/O input
inline uint32_t mmio_read(uint32_t reg)
{
	return *(volatile uint32_t*)(MMIO_BASE + reg);
}

// Loop <delay> times in a way that the compiler won't optimize away
inline void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}

enum
{
    // The offsets for reach register.
    GPIO_BASE = 0x200000,

    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD = (GPIO_BASE + 0x94),

    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0 = (GPIO_BASE + 0x98),
};

#endif
