/*
 *
 * Copyright (c) 2005-2017 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */

#ifndef ARM7_SUPPORT_H_
#define ARM7_SUPPORT_H_

#include "impTypes.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */


#define IO32_WR(BASE, OFFSET, VALUE) arm7_io_write32(BASE, OFFSET, VALUE)
#define IO32_RD(BASE, OFFSET) arm7_io_read32(BASE, OFFSET)

//
// Treat offset as a word offset (for word addressed peripherals)
//
#define SYSTEM_WIDTH 32
#define IO32_WR_WORD(BASE, OFFSET, VALUE) IO32_WR(BASE, (OFFSET*(SYSTEM_WIDTH/8)), VALUE)
#define IO32_RD_WORD(BASE, OFFSET) IO32_RD(BASE,(OFFSET*(SYSTEM_WIDTH/8)))

//
// Treat offset as a byte offset (for byte addressed peripherals)
//
#define IO32_WR_BYTE(BASE, OFFSET, VALUE) IO32_WR(BASE, OFFSET, VALUE)
#define IO32_RD_BYTE(BASE, OFFSET) IO32_RD(BASE, OFFSET)


#define ENABLE_INTERRUPTS()                             \
        asm volatile ( "MRS     R0, CPSR\n\t"           \
                       "BIC     R0, R0, #0xC0\n\t"      \
                       "MSR     CPSR, R0"               \
                       : /* output */                   \
                       : /* input */                    \
                       : "r0" /* clobber */             \
                      )

#define DISABLE_INTERRUPTS()                            \
        asm volatile ( "MRS     R0, CPSR\n\t"           \
                       "ORR     R0, R0, #0xC0\n\t"      \
                       "MSR     CPSR, R0"               \
                       : /* output */                   \
                       : /* input */                    \
                       : "r0" /* clobber */             \
                      )



#define CPU_INIT() arm7_cpu_init()

#define REGISTER_ISR(_IRQ, _HANDLER, _ARG)  \
    arm7_register_interrupt_handler(_IRQ, _HANDLER, _ARG)

// ========================================================

typedef enum {
    reset = 0,
    und   = 1,
    swi   = 2, // SVC
    pabt  = 3,
    dabt  = 4,
    rsvd  = 5,
    irq   = 6,
    fiq   = 7,
} arm7_interrupt_vector;

void arm7_cpu_init(void);

void arm7_register_interrupt_handler(
    arm7_interrupt_vector vector,
    void (*handler)(void *),
    void *arg
);

void arm7_io_write32(Uns32 address, Uns32  offset, Uns32 value);

Uns32 arm7_io_read32(Uns32 address, Uns32 offset);

#ifdef __cplusplus
}
#endif

#endif /*ARM7_SUPPORT_H_*/
