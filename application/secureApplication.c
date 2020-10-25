#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "armDefines.h"
#include "arm7_support.h"
#include "rxApi.h"


#define ENTER_SECURE_USING_MONITOR() asm("mov r2, #1\n" "SMC #0\n" )

#define ENTER_NON_SECURE_USING_MONITOR() asm("mov r2, #2\n" "SMC #0\n" "CPS #0x13\n")

#define TZPC_BASE         ((volatile unsigned char *) 0x10001000) 

#define TZPC_DECPROT0_SET ((volatile unsigned char *) (TZPC_BASE + 0x804)) 

#define TZPC_DECPROT0_CLR ((volatile unsigned char *) (TZPC_BASE + 0x808))

#define DISABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_CLR = 1

#define ENABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_SET = 1;

register r2 asm("r2");


bool messageReceivedRNS1 = false;
int received=0;
int enterLock = 0;


// Define a struct that stores each flit and the time that it arrive in the router
typedef struct{
    int sender;
    int messageSize; 
} messageHeader;


#define HEADER_SIZE 8


enum
{
    NONE,
    TO_SECURE,
    TO_UNSECURE
};

// Enter non-secure mode by setting SCR.NS (bit 0)
void enterNonSecure() 
{
    unsigned int scr;
    RD_SCR(scr);
    WR_SCR(scr | 1);
}


//
// SMC handler - 
//
static void smc_handler(void *ud)
{
    asm(
    ".global SMC_Handler_Main\n"
    "TST lr, #4\n"
    "ITE EQ\n"
    "B SMC_Handler_Main\n"
    );
}

// IRQ handler - called when any sort peripheral signals completion

static void irq_handler(void *ud)
{
    //   https://stackoverflow.com/questions/32865407/arm-interrupts-and-context-saving
    //asm(
       // "MRS R0, CPSR \n" 
       // "STMFD   sp!, {R0-R12, lr}  \n" 

        //"STMFD   r13!, {r0-r12,r14} \n"
    //  );

    int i , buffer[HEADER_SIZE]; // loop counter
    messageHeader header; //message header


    printf("--------------------------------->  Received interrupt. Starting handle\n");
    DISABLE_INTERRUPTS();
    ackInterruptHandler();
    
    ENTER_SECURE_USING_MONITOR();
    
    printf("---------------------------------->  Entered in secure world\n");

    for(i = 0; i< HEADER_SIZE; i++)
        { buffer[i] = *(RG_READ_HEADER_DATA);
          /////rintf(" %d ", buffer[i]);
        }

    header.sender =  buffer[0];        // transmissor
    header.messageSize =  buffer[4];  

     enterLock = 0;

    //printf("           size %d   %d ", HEADER_SIZE,header.messageSize);

    if(header.sender == 0 || header.sender == 1)
    {
        ENABLE_BRIDGE_COMMUNICATION();
        receiveMessage(header.messageSize);
        DISABLE_BRIDGE_COMMUNICATION();
    }
    else
    {
        printf("Invalid Message\n");
    }
    
    //ENTER_NON_SECURE_USING_MONITOR();
    //enterNonSecure();   


    messageReceivedRNS1 = true;

    received=1;

    ENABLE_INTERRUPTS(); 

    printf("---------------------------------->  Ending Interruption \n");

    asm(
     
  // "LDMFD   sp!, {R0-R12, pc} \n"

  // "MOV     PC, R0\n"

    // "MRS     R0, CPSR\n\t"
    // "BIC     R0, R0, #0xC0\n\t" 
    // "MSR     CPSR, R0\n"
    // "CPS     #0x13\n" // Set execution Level

    //"LDMFD   r13!, {r0-r12,r14} \n"
     "LDR     PC, =run\n"

    );
   
}




// // HELIO IRQ HANDLER
// static void irq_handler(void *ud)
// {
//     char buffer[HEADER_SIZE]; // buffer to retrieve header information
//     int i; // loop counter
//     messageHeader *header; //message header

//     printf("Received interrupt. Starting handle\n\n############\n\n");
//     DISABLE_INTERRUPTS();
//     ackInterruptHandler();

//     printf("Entering in secure world\n");
    
//     ENTER_SECURE_USING_MONITOR();
    
//     printf("Entered in secure world\n");

//     for(i = 0; i< HEADER_SIZE; i++)
//         buffer[i] = *(RG_READ_HEADER_DATA);

//     memcpy(header,buffer,HEADER_SIZE);
    
//     if(header->sender == 0 || header->sender == 1)
//     {
//         ENABLE_BRIDGE_COMMUNICATION();
//         receiveMessage(header->messageSize);
//         DISABLE_BRIDGE_COMMUNICATION();
//     }
//     else
//     {
//         printf("Invalid Message\n");
//     }
//      messageReceivedRNS1 = true;
//     received=1;
//     ENTER_NON_SECURE_USING_MONITOR();
    
//     asm(
//     // Enable interrupt
//     "MRS     R0, CPSR\n\t"
//     "BIC     R0, R0, #0xC0\n\t" 
//     "MSR     CPSR, R0\n"
//     "CPS     #0x13\n" // Set execution Level
//     "LDR PC, =run\n"
//     );
   
// }



void SMC_Handler_Main()
{
    
    int smc_number = r2;
    printf("NS - Entered in Super Monitor Call Handler!!! Operation is %d\n", smc_number);
    switch (smc_number)
    {
    case TO_SECURE:
        printf("Writing 0x0 to ns bit\n");
        WR_SCR(0);
        break;
    case TO_UNSECURE:
        printf("Writing 1 to ns bit\n");
        WR_SCR(1);
        break;
    default:
        asm(
            "MSR SPSR_cxsf, #0x13\n"
            //"LDR PC, =main\n" // SVC_entry points to the first 
            "MOV PC, lr"
        );
        break;
    }
}

/// ############# Config methods #############

// Add a section entry to the translation table mapping 1MB
// starting at VA to physical address PA
void addSection(unsigned int VA, unsigned int PA) {

    unsigned int index  = (VA >> 20);
    unsigned int entry  = (PA & 0xfff00000)             |  // Section base address
                          0x3 << 10                     |  // AP = read/write
                          0x2;                             // Section entry, PXN=0

    // Set the entry in the table to be a section
    translationTable[index] = entry;

}

// Setup translation table entries for first 2MB of memory and UART addresses
void setupTranslationTable() {
    
    addSection(0x00000000, 0x00000000);
    addSection(0x00100000, 0x00100000);
    addSection(0xfffffff0,0x001f0000);
    addSection(0xfffffff8,0x001f0008);
    addSection(0x200000,0x001ff000);
    
    addSection(0x300000,0x001fff00);
    addSection(TZPC_BASE,TZPC_BASE);
    addSection(RG_ACK_INTERRUPT,RG_ACK_INTERRUPT);
    addSection(0x01000000,0x01000000);
    addSection(0xea0020c2, 0x001fff00);
    
}

// Enable the TLB
void enableTLB() {

    // Write the translation table base register
    WR_TTBR0((unsigned int)translationTable);

    // Set DACR to 1 to enable client permissions for domain 0
    WR_DACR(1);

    // Set SCTLR.M (bit 0) to enable TLB
    unsigned int sctlr;
    RD_SCTLR(sctlr);
    WR_SCTLR(sctlr | 1);

}


void setSVCHandler()
{
    // reset vector
    unsigned int *reset = (void *) 0x8;
    // branch instruction encoding
    unsigned int instr = 0xea000000;
    unsigned int start = (unsigned int)(void*)&smc_handler;

    start = ((start-8)>>2) & 0x00ffffff;
    // write svc handle vector
    *reset = (instr | start);
}



void sendProcess(int target){

    char messageOut[PACKET_SIZE] = "Message from Secure Processor";

    //DISABLE_INTERRUPTS();
    requireToSend();
    sendMessage(target, messageOut);
    //ENABLE_INTERRUPTS();

}



/////char* readSecureMemory(int dataOffset){
/////
/////    int offset;
/////    
/////    r2 = TO_SECURE;
/////    asm("SMC #0\n");
/////    printf("MEMORY:");
/////    for(offset = 0; offset < PACKET_SIZE ; offset++){
/////        printf("%c.", readMemoryBuffer[offset]);
/////    }
/////    printf("\n\n");
/////
/////    r2 = TO_UNSECURE;
/////    asm("SMC #0\n");
/////    return readMemoryBuffer;
/////
/////}

//////////  le os dados da região segura ////////////////////////////// 
void memoryAcessTest(){

   int offset = 0;
   printf("--------------------------> Memory acess test \n");

   //DISABLE_INTERRUPTS();
  // asm("CPS #0x13\n");
   //ENTER_SECURE_USING_MONITOR();  // ESSA LINHA ESTÁ CAUSANDO DEADLOCK CASO O PROCESSADORE SEJA INTERROMPIDO DEPOIS.

  
   printf("data: |");
   for(offset = 0; offset < PACKET_SIZE ; offset++)
       printf("%c", *(SECURE_MEMORY_REGION + offset));

   printf("\n|\n----------------------> End\n");
       
   //ENTER_NON_SECURE_USING_MONITOR();
   //enterNonSecure();
   //ENABLE_INTERRUPTS();
}


//////////  fibonacci de 0 a 19  ////////////////////////////// 
static int fib(int i)
{
    return (i > 1) ? fib(i - 1) + fib(i - 2) : i;
}

void fibPrint()
{    
    int i, j;
    for (j = 0; j < 1; j++) {

            for (i = 0; i < 20; i++) {
                printf("fib(%d) = %d\n", i, fib(i));
            }

        }
}

bool cpuProcess = true;
int receivedCount = 0;

////////// executa qq coisa - um fibonacci de 0 a 19 ////////// 
void run() 
{  
    asm("CPS #0x13\n");  // no secure

    while(cpuProcess){

       fibPrint();
       

       // testa se algo chegou
       if(received) {
           memoryAcessTest();
           if (receivedCount == 0) sendProcess(1);
           received=0;
           receivedCount++;
           if (receivedCount == 2){
              
               sendProcess(0);
              
           }  
           if (receivedCount == 3){
               cpuProcess = false;
               printf("Shutdown command \n");
           }  
         }
   }

     //fibPrint();
     //fibPrint();
     printf("\n\n\n-----------> An Exception will be generated to shutdown the processor.\n");
    printf("%c ", *(SECURE_MEMORY_REGION + 0xffffffffffffff)); 
       
        // Está aqui para impedir que o processador desligue logo após enviar a mensagem.
                 // Pode ser retirado caso necessario.
}



////////////////////////////////////////////////////////////////////////////////////////////////////
int main() 
{ 
    received = 0;     

     printf("---------------------> Hello World from secure-processor!!!\n");
        REGISTER_ISR(irq, irq_handler, (void *)NULL);

    asm("CPS #0x13\n");

    // Boot for secure world
    CPU_INIT();
    setupTranslationTable();
    enableTLB();
    setSVCHandler();
    
    // Boot for unsecure world
    enterNonSecure();   
    REGISTER_ISR(irq, irq_handler, (void *)NULL);
    ENABLE_INTERRUPTS(); 

    printf("-----------------------------------------> main com interrupções ativas\n");

    run();

    return 0;
}
