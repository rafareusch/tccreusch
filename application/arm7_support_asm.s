
	.section .text
	.align 4

	.global arm7_irq_handler_asm
	.type arm7_irq_handler_asm, function
    .global arm7_fiq_handler_asm
    .type arm7_fiq_handler_asm, function
    

arm7_irq_handler_asm:

	stmdb	r13!, {r0-r12,r14}
	bl		arm7_irqDispatcher
	ldmia	r13!, {r0-r12,r14}
	subs	pc, r14, #4

arm7_fiq_handler_asm:

	stmdb	r13!, {r0-r12,r14}
	bl		arm7_fiqDispatcher
	ldmia	r13!, {r0-r12,r14}
	subs	pc, r14, #4



