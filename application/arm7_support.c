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

#include <stdio.h>
#include "arm7_support.h"


void arm7_io_write32(Uns32 address, Uns32 offset, Uns32 value)
{
    volatile int *ptr = (volatile int *)(address+offset);
    *ptr = value;
}

Uns32 arm7_io_read32(Uns32 address, Uns32 offset)
{
    volatile int *ptr = (volatile int *)(address+offset);
    return *ptr;
}

extern void arm7_irq_handler_asm(void);
extern void arm7_fiq_handler_asm(void);

struct irq_handler {
    void    (*handler)(void *);
    void    *arg;
};

static struct irq_handler user_irqHandler;
static struct irq_handler user_fiqHandler;

void arm7_irqDispatcher(void)
{
    if (user_irqHandler.handler) {
        user_irqHandler.handler(user_irqHandler.arg);
    }
}

void arm7_fiqDispatcher(void)
{
    if (user_fiqHandler.handler) {
        user_fiqHandler.handler(user_fiqHandler.arg);
    }
}

static Uns32 makeBranchOp(Uns32 fromAddress, Uns32 toAddress)
{
    Uns32 offset = (Uns32)toAddress - (Uns32)fromAddress - 8 ;
    Uns32 instruction = 0xEA000000 | ( 0x00FFFFFF & (offset >> 2)) ;
    return instruction;
}

#define FIQ_STACK_SIZE 1024
#define IRQ_STACK_SIZE 1024
static Uns32 fiq_stack[FIQ_STACK_SIZE];
static Uns32 irq_stack[IRQ_STACK_SIZE];

#define ARM7_PSR_MODE_USR     0x10
#define ARM7_PSR_MODE_FIQ     0x11
#define ARM7_PSR_MODE_IRQ     0x12
#define ARM7_PSR_MODE_SVC     0x13
#define ARM7_PSR_MODE_ABT     0x17
#define ARM7_PSR_MODE_UND     0x1B
#define ARM7_PSR_MODE_SYS     0x1F
#define ARM7_PSR_MODE_MASK    0x1F

#define S_OF2(_X) #_X
#define S_OF(_X) S_OF2(_X)

void arm7_cpu_init(void)
{
    Uns32 vBase = 0x00000000;
    Uns32 fiqVector = (fiq<<2);
    Uns32 irqVector = (irq<<2);
    int i;

    asm volatile (
                   "MOV     R1, %0\n\t"
                   "MRS     R0, CPSR\n\t"
                   "BIC     R0, R0, #0x1F\n\t"
                   "ORR     R0, R0, #" S_OF(ARM7_PSR_MODE_FIQ) "\n\t"
                   "MSR     CPSR, R0\n\t"
                   "MOV     SP, R1\n\t"
                   "BIC     R0, R0, #0x1F\n\t"
                   "ORR     R0, R0, #" S_OF(ARM7_PSR_MODE_SVC) "\n\t"
                   "MSR     CPSR, R0"
                   : /* output */
                   : "r" (&fiq_stack[FIQ_STACK_SIZE-1])/* input */
                   : "r0", "r1" /* clobber */
                  );
    
    IO32_WR(vBase, fiqVector, makeBranchOp(fiqVector, (Uns32)&arm7_fiq_handler_asm));
    IO32_WR(vBase, irqVector, makeBranchOp(irqVector, (Uns32)&arm7_irq_handler_asm));
}

void arm7_register_interrupt_handler(
    arm7_interrupt_vector     vector,
    void                    (*handler)(void *),
    void                     *arg
) {
    struct irq_handler *slot = 0;
    switch (vector) {
        case fiq:   slot = &user_fiqHandler; break;
        case irq:   slot = &user_irqHandler; break;
    }
    if (slot) {
        slot->handler = handler;
        slot->arg = arg;
    }
}
