#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "armDefines.h"
#include "arm7_support.h"
#include "rxApi.h"
#include "tweetnacl.h"
#include "sha256.h"


#define ENTER_SECURE_USING_MONITOR() asm("mov r2, #1\n" "SMC #0\n" )

#define ENTER_NON_SECURE_USING_MONITOR() asm("mov r2, #2\n" "SMC #0\n" "CPS #0x13\n")

#define TZPC_BASE         ((volatile unsigned char *) 0x10001000) 

#define TZPC_DECPROT0_SET ((volatile unsigned char *) (TZPC_BASE + 0x804)) 

#define TZPC_DECPROT0_CLR ((volatile unsigned char *) (TZPC_BASE + 0x808))

#define DISABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_CLR = 1

#define ENABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_SET = 1;

register r2 asm("r2");


#define  NB_ITERATIONS 3

int received=0;
int iteration=0;
int previous=0;

// Define a struct that stores each flit and the time that it arrive in the router
typedef struct{
    int sender;
    int messageSize; 
} messageHeader;


#define HEADER_SIZE 8


// KEY DECLARATIONS
typedef struct {
    unsigned char sk[PRIV_KEY_LEN];
    unsigned char pk[PUB_KEY_LEN];
} ec_keys;

ec_keys EC_keys; 

unsigned char sharedSecret[PUB_KEY_LEN];
unsigned char sessionKey[PUB_KEY_LEN];
unsigned char dataFromMemory[PUB_KEY_LEN];
// END

unsigned char messageSender = 0;

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

// print hex strings
void hexdump(char * data, int len) 
{
    int i;
    //data = &shared_Status;
    //len = 0x080;
    for (i = 0; i < len; i++) {
        printf("%02X", (unsigned char)data[i]);
    }
    printf("\n");
}


// IRQ handler - called when any sort peripheral signals completion
static void irq_handler(void *ud)
{

    // Saving processor state  
    asm(
         "MRS R0, SPSR \n" 
         "STMFD   sp!, {R0-R12, LR}  \n"     // não ajudou em nada
       );

    int i , buffer[HEADER_SIZE]; // loop counter
    messageHeader header; //message header


    printf("------------------------------------------------------------------------------------->  Received interrupt. Starting handle\n");
    DISABLE_INTERRUPTS();
    ackInterruptHandler();
    
    ENTER_SECURE_USING_MONITOR();
    printf("------------------------------------------------------------------------------------->  INTERRUPT Entered in secure world\n");

    for(i = 0; i< HEADER_SIZE; i++)
        { buffer[i] = *(RG_READ_HEADER_DATA);
          printf(" . %d ", buffer[i]);
        }

    header.sender =  buffer[0];        // transmissor
    header.messageSize =  buffer[4];  
    messageSender = header.sender;
    if(header.sender == 0 || header.sender == 1)
    {
        ENABLE_BRIDGE_COMMUNICATION();
        receiveMessage(header.messageSize,0,header.sender);
        DISABLE_BRIDGE_COMMUNICATION();
    }
    else
    {
        printf("Invalid Message\n");
    }
    
    ENTER_NON_SECURE_USING_MONITOR();
    printf("------------------------------------------------------------------------------------->  INTERRUPT Leaving in secure world\n");
   

    received =  buffer[4];    /// received recebe o tamanho da MSG

    ENABLE_INTERRUPTS(); 

    printf("------------------------------------------------------------------------------------->  Ending Interruption \n\n\n\n\n");

    // restore context
    asm(
          "LDMFD   sp!, {R0-R12, LR}  \n"     // não ajudou em nada
          "MSR    SPSR, R0  \n"  
          "LDR     PC, =run\n\t"
    );
   
}



void SMC_Handler_Main()
{
    
    int smc_number = r2;

    printf("NS - Entered in Super Monitor Call Handler!!! Operation is %d\n", smc_number);

    switch (smc_number)
    {
    case TO_SECURE:
        printf("Writing 0x0 to ns bit  \n");
        WR_SCR(0);
        break;
    case TO_UNSECURE:
        printf("Writing 1 to ns bit\n");
        WR_SCR(1);
        break;
    default:
     asm(
         "MSR SPSR_cxsf, #0x13\n"
         "LDR PC, =main\n" // volta para o main 
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



void sendProcess(int target, int command){
    printf("RS is sending to %d",target);
    char messageOut[100];

    sprintf(messageOut,"%d Message from Secure Processor", command);

    requireToSend();
    sendMessage(target, messageOut);

}

void sendKey(int target){

    char messageOut[100];

    memcpy(messageOut,EC_keys.pk,PUB_KEY_LEN);
    requireToSend();
    sendMessage(target, messageOut);

}


// READ KEY FROM SECURE MEMORY
void memoryKeyRead(int keyid){
    
    int memoffset = (keyid+1) * PUB_KEY_LEN;
    int offset;
    ENTER_SECURE_USING_MONITOR();
    printf("Reading key %d from memory\n",memoffset);
    for(offset = 0; offset < PUB_KEY_LEN ; offset++)
       dataFromMemory[offset] = *(SECURE_KEY_REGION+(memoffset) + offset);
    printf("\n");
      
   ENTER_NON_SECURE_USING_MONITOR();

}













//////////  le os dados da região segura ////////////////////////////// 
void memoryAcessTest(int size){


   int offset = 0;
  

   ENTER_SECURE_USING_MONITOR();
   printf("------------------------------------------------------------------------------------->  memoryAcessTest Entered in secure world\n");
 
   printf("------Memory acess test--------------#########>> ||");
   for(offset = 0; offset < size ; offset++){
       printf("%c", *(SECURE_MEMORY_REGION + offset));
       dataFromMemory[offset] = *(SECURE_MEMORY_REGION + offset);
   }

   printf("||\n\n");
      
   ENTER_NON_SECURE_USING_MONITOR();
   printf("---------------------------------->  memoryAcessTest Leaving in secure world\n\n\n\n");
  
}


void generateSecretKey(){

    int i;
	srand(0xcba98cf96c2e603ffbcf08785cf17073);
	for( i = 0; i < 32; i++){
    	EC_keys.sk[i] = (unsigned char)rand()%256; 
	}
}


void computeSessionKey(){

    // SHARED SECRET COMPUTING
    // printf("RS received Public Key: ");
    // hexdump((char *)&dataFromMemory,  PUB_KEY_LEN);
    // fflush(stdout);

    // printf("RS Secret Key: ");
    // hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    // fflush(stdout);

    printf("####### RS is now processing the Session Key\n");

    crypto_box_beforenm(sharedSecret,dataFromMemory,EC_keys.sk);
    // printf("RS Shared Secret: ");
    // hexdump((char*)sharedSecret, PUB_KEY_LEN); 

    // SHA-256 COMPUTING
    SHA256_CTX ctx;
    sha256_init(&ctx);
    //sha256_update(&ctx,(unsigned char*)"abc",3);
    //sha256_update(&ctx,(unsigned char*)"9CCD5020D72C2525EC178C7C48758156EC831465CE151860BCAAC3A402DA7722",64);
    sha256_update(&ctx,(unsigned char*)sharedSecret,PUB_KEY_LEN);
    sha256_final(&ctx,sessionKey);
    printf("RS and RNS%d Session Key: ", messageSender+1);
    hexdump((char*)sessionKey, PUB_KEY_LEN);     
    fflush(stdout);

}

//unsigned char skey[64] = {0x1B,0xFE,0xD7,0xCF,0xEE,0x6A,0xEF,0x3D,0x98,0x33,0x61,0x75,0x6F,0xDF,0x4C,0x1C,0x18,0x34,0xAA,0x2E,0x34,0xB1,0x39,0xDF,0xC4,0x56,0x63,0xE3,0xF9,0xA8,0xB3,0x8A};
//unsigned char pkey[64] = {0x9F,0xDC,0x50,0x9B,0xFF,0x42,0x45,0xBD,0x12,0xB8,0x81,0x83,0xBD,0xAF,0x42,0x7A,0xBC,0x1B,0xC9,0xE8,0x14,0x8A,0xE7,0x24,0xA9,0x0C,0x04,0xC8,0x56,0x08,0xB0,0x75};


void computeKeys(){
    generateSecretKey();
   
    crypto_box_keypair(EC_keys.pk, EC_keys.sk);
    printf("RS Private Key: ");
    hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("RS Public Key: ");
    hexdump((char *)&EC_keys.pk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("\n");
}


unsigned char sendKey_lock = 0;
unsigned char computeKey_lock = 0;
unsigned char sessionKeyState[2] = {0,0};



void run (){

    // Envia a chave publica para os processadores não seguros, essencial rodar apenas uma vez.
    if (!sendKey_lock){
        sendKey(0);
        sendKey(1);
        sendKey_lock = 1;
    }
    for(  ; iteration < NB_ITERATIONS ; )  {        
          
          // envia pacote para um processador não seguro - o !received evita dois envios!
          if(!received && sessionKeyState[0] == 1 && sessionKeyState[1] == 1 )
             {  int cmd;

                // avisa os dois periféricos que é última iteração
                if( iteration >= NB_ITERATIONS-2 )  cmd=1;  else cmd=0;

                if  (!previous)
                     { sendProcess( 0, cmd );  previous=1;}
                else
                     { sendProcess( 1, cmd );  previous=0;}
             }
          
           while( received==0 );  // espera resposta da interrupção

           printf("--------------------------------#########>> Data from NONSEC %d \n\n", iteration);

           DISABLE_INTERRUPTS();
           memoryAcessTest(received);

           if (sessionKeyState[0] == 0 || sessionKeyState[1] == 0 )
           {
            printf("RS received Public Key from RNS%d\n",messageSender+1);
            computeSessionKey();
            sessionKeyState[messageSender] = 1;
            }

           ;
           fflush(stdout);
           printf("\n");
           ENABLE_INTERRUPTS();

           received = 0;
           ++iteration;
    }
    puts("###### R U N      E N D E D");

   enterNonSecure();   // non secure
}



////////////////////////////////////////////////////////////////////////////////////////////////////
int main() 
{ 
    received = 0;     

    printf("---------------------> Hello World from secure-processor!!!\n");
   
    // Boot for secure world
    CPU_INIT();
    setupTranslationTable();
    enableTLB();
    setSVCHandler();
    computeKeys();
    ENTER_NON_SECURE_USING_MONITOR();
    printf("---------------------------------->  Leaving secure world\n");
    

    REGISTER_ISR(irq, irq_handler, (void *)NULL);
    

    if( iteration != NB_ITERATIONS) {
          ENABLE_INTERRUPTS(); 
          printf("-----------------------------------------> main com interrupções ativas\n");
          run();  
    }
    

    puts("###### E X E C U T O U     C O M     S U C E S S O  (mas com gambiarra) ");

    exit(0);
}
