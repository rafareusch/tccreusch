// TrustZone Protection Controller Registers
#define TZPC_BASE         0x10001000
#define TZPC_DECPROT0_SET ((volatile unsigned char *) (TZPC_BASE + 0x804))
#define TZPC_DECPROT0_CLR ((volatile unsigned char *) (TZPC_BASE + 0x808))

// UART Registers
#define UART_BASE 0x00100000
#define UART_TX   ((volatile unsigned char *) (UART_BASE + 0x0 ))
#define UART_FR   ((volatile unsigned char *) (UART_BASE + 0x18))

// virtual memory translation table
unsigned int translationTable[4096] __attribute__ ((aligned (16384)));

// CP15 register access
#define WR_CP15(_OP1, _CRN, _CRM, _OP2, _VAL) \
    asm volatile("mcr p15, "#_OP1", %0, "#_CRN", "#_CRM", "#_OP2::"r"(_VAL))

#define RD_CP15(_OP1, _CRN, _CRM, _OP2, _VAL) \
    asm volatile("mrc p15, "#_OP1", %0, "#_CRN", "#_CRM", "#_OP2:"=r"(_VAL))

#define WR_SCTLR(_VAL) WR_CP15(0, c1, c0, 0, _VAL)
#define RD_SCTLR(_VAL) RD_CP15(0, c1, c0, 0, _VAL)
#define WR_SCR(_VAL)   WR_CP15(0, c1, c1, 0, _VAL)
#define RD_SCR(_VAL)   RD_CP15(0, c1, c1, 0, _VAL)
#define WR_TTBR0(_VAL) WR_CP15(0, c2, c0, 0, _VAL)
#define WR_DACR(_VAL)  WR_CP15(0, c3, c0, 0, _VAL)