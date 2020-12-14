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
#include "aes.h"


#define ENTER_SECURE_USING_MONITOR() asm("mov r2, #1\n" "SMC #0\n" )

#define ENTER_NON_SECURE_USING_MONITOR() asm("mov r2, #2\n" "SMC #0\n" "CPS #0x13\n")

#define TZPC_BASE         ((volatile unsigned char *) 0x10001000) 

#define TZPC_DECPROT0_SET ((volatile unsigned char *) (TZPC_BASE + 0x804)) 

#define TZPC_DECPROT0_CLR ((volatile unsigned char *) (TZPC_BASE + 0x808))

#define DISABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_CLR = 1

#define ENABLE_BRIDGE_COMMUNICATION() *TZPC_DECPROT0_SET = 1;

register r2 asm("r2");


#define  NB_ITERATIONS 7
#define HEADER_SIZE 8

int received=0;
int iteration=0;
int previous=0;

// Define a struct that stores each flit and the time that it arrive in the router
typedef struct{
    int sender;
    int messageSize; 
} messageHeader;





// KEY DECLARATIONS
typedef struct {
    unsigned char sk[PRIV_KEY_LEN];
    unsigned char pk[PUB_KEY_LEN];
} ec_keys;

ec_keys EC_keys; 

unsigned char sharedSecret[PUB_KEY_LEN];
unsigned char sessionKey[PUB_KEY_LEN];
unsigned char dataFromMemory[PACKET_SIZE];
unsigned char keyFromMemory[PUB_KEY_LEN];
unsigned char nounce[PUB_KEY_LEN/2];
unsigned char AESkey[PUB_KEY_LEN/2];

struct AES_ctx ctx;
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


    printf("----------------------------------------------------------------------------->  Received interrupt. Starting handle\n");
    DISABLE_INTERRUPTS();
    ackInterruptHandler();
    
    ENTER_SECURE_USING_MONITOR();
    printf(" -------------------------> Entering Secure mode\n");

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
        receiveMessage(header.messageSize);
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

    //printf("NS - Entered in Super Monitor Call Handler!!! Operation is %d\n", smc_number);

    switch (smc_number)
    {
    case TO_SECURE:
        printf("SMC: Entering Secure mode  \n");
        WR_SCR(0);
        break;
    case TO_UNSECURE:
        printf("SMC: Entering Unsecure mode\n");
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


unsigned char msg0[64] = {0x5A,0xD6,0x54,0x79,0xFB,0xCE,0x9A,0x7B,0x6E,0x78,0x83,0xBB,0xDB,0x39,0x5A,0xAD,0xB9,0x6C,0x4C,0xDF,0xA7,0xA4,0x0C,0xE8,0xB5,0xA8,0x19,0x6A,0x8D,0xF9,0xF9,0xEC};
unsigned char msg1[64] = {0xCF,0xA0,0xA4,0x2C,0xC2,0xFB,0x92,0x1D,0x93,0xCE,0x2F,0xA2,0x29,0xA6,0xB6,0x8A,0x0F,0xE6,0x5A,0x1E,0x0C,0x79,0x46,0x17,0xCE,0x82,0xE6,0xCB,0xF3,0xDF,0x32,0xB3};


void sendProcess(int target, int command){
    printf("___________________________________________________________________________\n");
    printf("################# Server is starting send process to CLI%d ####################\n\n",target+1);
    
    char messageOut[100];

    // if (command == 1){
    //     memcpy(messageOut,msg1,PUB_KEY_LEN);
    // } else {
    //     memcpy(messageOut,msg0,PUB_KEY_LEN);
    // }

    sprintf(messageOut,"%d Message from Server", command);
    printf("String Message: %s\n",messageOut);
    printf("HEX Message: ");
    hexdump((char*)messageOut, PUB_KEY_LEN); 
    printf("\n");
    memoryKeyRead(target);

    printf("Encrypting message with AES...\n");
    memcpy(AESkey,keyFromMemory,PUB_KEY_LEN/2);
    memcpy(nounce,keyFromMemory+16,PUB_KEY_LEN/2);

    AES_init_ctx_iv(&ctx, AESkey, nounce);
    AES_CBC_encrypt_buffer(&ctx, messageOut, 32);

    printf("Encrypted message: %s");
    hexdump((char*)messageOut, PUB_KEY_LEN);  
    printf("\n");
    requireToSend();
    sendMessage(target, messageOut);
    printf("________________________________________________________________________\n");
    printf(    "######################## End of transmission ###########################\n\n",target+1);
    //printf("##################################################################################################\n");
    ENABLE_INTERRUPTS();

}

void sendKey(int target){

    char messageOut[100];

    memcpy(messageOut,EC_keys.pk,PUB_KEY_LEN);
    requireToSend();
    sendMessage(target, messageOut);

}


// READ KEY FROM SECURE MEMORY
void memoryKeyRead(int keyid){
    
    int memoffset = keyid * PUB_KEY_LEN ;
    int offset;
    ENTER_SECURE_USING_MONITOR();

    printf("Reading key %d from memory\n",keyid+1);
    
    for(offset = 0; offset < PUB_KEY_LEN ; offset++)
       keyFromMemory[offset] = *(SECURE_KEY_REGION+memoffset+offset);

    printf("Key on memory read: ");
    hexdump((char*)keyFromMemory, PUB_KEY_LEN);     
    
      
   ENTER_NON_SECURE_USING_MONITOR();
   printf("\n");

}

void memoryKeyWrite(int keyid){
    int memoffset = keyid * PUB_KEY_LEN;
    ENTER_SECURE_USING_MONITOR();
    printf("Saving session key to secure memory\n");
    memcpy(SECURE_KEY_REGION+memoffset,sessionKey, PUB_KEY_LEN);
    memcpy(keyFromMemory,sessionKey, PUB_KEY_LEN);

    ENTER_NON_SECURE_USING_MONITOR();
}



void memoryDataRead(int size){

   int offset = 0;
  
   memset(dataFromMemory,0,PACKET_SIZE);
   ENTER_SECURE_USING_MONITOR();
   printf("Reading DATA from secure memory\n");
   for(offset = 0; offset < size ; offset++){
       //printf("%c", *(SECURE_MEMORY_REGION + offset));
       dataFromMemory[offset] = *(SECURE_MEMORY_REGION + offset);
   }
    printf("Memory DATA read:  ");
    hexdump((char*)dataFromMemory, PUB_KEY_LEN);     
    
      
   ENTER_NON_SECURE_USING_MONITOR();
   printf("\n");
 

}



void decryptMessage(){

    printf( "\n\n#########################################################\n");
    printf(     "################ STARTING DECRYPT ALGORITHM #############\n");
    printf(     "#########################################################\n\n");
    memoryKeyRead(messageSender);
    memoryDataRead(received);
    memcpy(AESkey,keyFromMemory,PUB_KEY_LEN/2);
    memcpy(nounce,keyFromMemory+16,PUB_KEY_LEN/2);


    printf("Sender: CLI%d",messageSender+1);
    printf("\nSESSION KEY: ") ;
    hexdump((char *)&keyFromMemory,  PUB_KEY_LEN);
    fflush(stdout);
    printf("\nAES KEY: ") ;
    hexdump((char *)&AESkey,  PUB_KEY_LEN/2);
    fflush(stdout);
    printf("\nIV: ") ;
    hexdump((char *)&nounce,  PUB_KEY_LEN/2);
    fflush(stdout);
    printf("\nCIPHERED DATA: ") ;
    hexdump((char *)&dataFromMemory,  32);
    fflush(stdout);
    
    AES_init_ctx_iv(&ctx, AESkey, nounce); 
    AES_CBC_decrypt_buffer(&ctx, dataFromMemory, 32);
    printf("\nDECRYPTED TEXT: %s ", dataFromMemory);

    printf( "\n\n##########################################################\n");
    printf(     "##################  END OF DECRYPTION  ###################\n");
    printf(     "##########################################################\n\n");


}



void generateSecretKey(){

    int i;
	srand(0xcba98cf96c2e603ffbcf08785cf17073);
	for( i = 0; i < 32; i++){
    	EC_keys.sk[i] = (unsigned char)rand()%256; 
	}
}


void computeSessionKey(){


    printf("Client Public Key: ");
    hexdump((char*)dataFromMemory, PUB_KEY_LEN);
    printf("Server secret Key: ");
    hexdump((char*)EC_keys.sk, PUB_KEY_LEN); 

    printf("\n############ Server is now computing the Session Key\n");
    crypto_box_beforenm(sharedSecret,dataFromMemory,EC_keys.sk);

    // SHA-256 COMPUTING
    SHA256_CTX ctx;
    sha256_init(&ctx);

    sha256_update(&ctx,(unsigned char*)sharedSecret,PUB_KEY_LEN);
    sha256_final(&ctx,sessionKey);
    printf("############ Server and Client %d Session Key: ", messageSender+1);
    hexdump((char*)sessionKey, PUB_KEY_LEN);     
    fflush(stdout);
    printf("\n");

}

//unsigned char skey[64] = {0x1B,0xFE,0xD7,0xCF,0xEE,0x6A,0xEF,0x3D,0x98,0x33,0x61,0x75,0x6F,0xDF,0x4C,0x1C,0x18,0x34,0xAA,0x2E,0x34,0xB1,0x39,0xDF,0xC4,0x56,0x63,0xE3,0xF9,0xA8,0xB3,0x8A};
//unsigned char pkey[64] = {0x9F,0xDC,0x50,0x9B,0xFF,0x42,0x45,0xBD,0x12,0xB8,0x81,0x83,0xBD,0xAF,0x42,0x7A,0xBC,0x1B,0xC9,0xE8,0x14,0x8A,0xE7,0x24,0xA9,0x0C,0x04,0xC8,0x56,0x08,0xB0,0x75};


unsigned char skey[64] = {0x4B,0x65,0xE1,0xE6,0x2A,0x9B,0x32,0x3E,0x12,0xF4,0x00,0x39,0xAE,0x8C,0x5C,0xF2,0x5C,0xE7,0xBD,0x20,0x0E,0x4F,0xAE,0x02,0x89,0xD8,0x8B,0xB9,0xF3,0x86,0x5D,0x6C};
unsigned char pkey[64] = {0x3B,0x97,0x04,0x82,0xD2,0xB4,0x21,0x28,0x57,0x41,0xBA,0x65,0x8F,0x81,0x5E,0x0A,0x7A,0x73,0x03,0x58,0xFA,0xE4,0x12,0x3A,0xB0,0x15,0x4A,0xF0,0xCF,0xE4,0x50,0x58};
unsigned char keycli1[64] = {0xC0,0x2D,0x53,0x52,0x1C,0x74,0xC1,0xD8,0xEA,0x23,0xD4,0xE2,0xBF,0xD6,0xDC,0x72,0x71,0xE5,0x7A,0x71,0x0F,0x4A,0xBD,0x64,0x20,0xB4,0xB7,0xD0,0x14,0x0F,0x0F,0x03};
unsigned char keycli2[64] = {0xDF,0x67,0xE0,0x68,0xBB,0x60,0x65,0x65,0x00,0x3F,0xFF,0xC8,0x12,0xEB,0x95,0xA3,0x38,0xA8,0x86,0x25,0xA0,0x2A,0x0A,0xA5,0x0E,0x75,0x58,0x3B,0xBF,0x3D,0x2F,0xD6};

void computeKeys(){
    generateSecretKey();
    //memcpy(EC_keys.sk,skey,64);
    //memcpy(EC_keys.pk,pkey,64);
    crypto_box_keypair(EC_keys.pk, EC_keys.sk);
    printf("Server Private Key: ");
    hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("Server Public Key: ");
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
        printf("> Server will now send it's Public Key to Client 1 and Client 2\n\n");
        sendKey(0);
        sendKey(1);
        sendKey_lock = 1;
    }
    for(  ; iteration < NB_ITERATIONS ; )  {        
          
        // envia pacote para um processador não seguro - o !received evita dois envios!
        // sessionKeyState não permite iniciar a comunicação até que os dois processadores seguros estejam autenticados
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
        
        printf("\n############################################################################################################\n");
        printf(  "############################################# SERVER RECEIVED DATA #########################################\n");
        printf(  "#################################################  HANDLE START ############################################\n\n");
        //printf("--------------------------------#########>> Data from NONSEC %d \n\n", iteration);

        DISABLE_INTERRUPTS();
        


        // Esse if controla a autenticação dos dois processadores.
        if (sessionKeyState[0] == 0 || sessionKeyState[1] == 0 )
        {
            printf("> SERV reports it has received Public Key from CLI%d\nStarting Diffie-Hellman Key-Exchange...\n\n",messageSender+1);
            //printf("Reading key received from secure memory\n");
            memoryDataRead(received); // this step is essential to compute the session key
            computeSessionKey();
            // if (messageSender == 0){
            //     memcpy(sessionKey,keycli1,64);
            // } else{
            //     memcpy(sessionKey,keycli2,64);

            // }
            
            memoryKeyWrite(messageSender);
            //memoryKeyRead(messageSender);
            sessionKeyState[messageSender] = 1;
        } else {
            printf("> SERV reports it has received a ciphered message from CLI%d\nStarting AES decryption...\n\n",messageSender+1);
            decryptMessage();
            
        }

           
        fflush(stdout);
        printf("\n");
        printf(  "############################################ END HANDLE ###############################################\n\n\n");
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

    printf("---------------------> Starting Server Processor...!\n");
    printf("-----> Initializing EC keys....\n");
   
    // Boot for secure world
    CPU_INIT();
    setupTranslationTable();
    enableTLB();
    setSVCHandler();
    computeKeys();
    ENTER_NON_SECURE_USING_MONITOR();
    
    

    REGISTER_ISR(irq, irq_handler, (void *)NULL);
    

    if( iteration != NB_ITERATIONS) {
          ENABLE_INTERRUPTS(); 
          
          run();  
    }
    

    puts("#############  E X E C U T O U     C O M     S U C E S S O  ################### ");

    exit(0);
}