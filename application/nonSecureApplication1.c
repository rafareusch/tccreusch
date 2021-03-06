#include <stdio.h>
#include "armDefines.h"
#include "txApi.h"
#include "tweetnacl.h"
#include "sha256.h"
#include "time.h"
#include "aes.h"

#define RNS_ID 1
#define RAND_SEED1 0x921797a172
#define RAND_SEED2 0xbca6947567


typedef struct {
    unsigned char sk[PRIV_KEY_LEN];
    unsigned char pk[PUB_KEY_LEN];
} ec_keys;

ec_keys EC_keys; 

struct AES_ctx ctx;

unsigned char sharedSecret[PUB_KEY_LEN];
unsigned char sessionKey[PUB_KEY_LEN];
unsigned char hisPublicKey[PUB_KEY_LEN];
unsigned char nounce[PUB_KEY_LEN/2];
unsigned char AESkey[PUB_KEY_LEN/2];
unsigned char wd[100];


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

// Enter non-secure mode by setting SCR.NS (bit 0)
void enterNonSecure() 
{
    unsigned int scr;
    RD_SCR(scr);
    WR_SCR(scr | 1);
}

int readKey_lock = 0;




void decryptMessage(){
    printf("\n\n################### CLI IS DECRYPTING #################\n");
    printf("\nCLI CIPHERED MESSAGE: ");
    hexdump((char *)&wd,  PUB_KEY_LEN);
    fflush(stdout);
    AES_init_ctx_iv(&ctx, AESkey, nounce); 
    AES_CBC_decrypt_buffer(&ctx, wd, 32);
    printf("\nDECRYPTED TEXT: %s ", wd);
     printf("\n\n################### END OF DECRYPT #################\n\n");

}


void example1(){
    int fim, i=0;
   
    
    // requireToRead(wd);
    // printf("\n\n################### KEY Received: [%s]\n ", wd);
        
    
    for(;;) {
        requireToRead(wd);
        decryptMessage();
        
        
        //printf("\n\n---> RNS Received: [%s]\n\n ", wd);
        
        if (wd[0]=='1')  fim=1;  else fim=0;
        
        sprintf(wd,"{from CLIENT[%d] to SEC - packet %d}", RNS_ID, i++);


        printf("\nCLI AES KEY: ") ;
        hexdump((char *)&AESkey,  PUB_KEY_LEN/2);
        fflush(stdout);
        printf("\nCLI NOUNCE: ") ;
        hexdump((char *)&nounce,  PUB_KEY_LEN/2);
        fflush(stdout);
      
        printf("\nCLI PLAIN TEXT: %s\n",wd) ;

        AES_init_ctx_iv(&ctx, AESkey, nounce);
        AES_CBC_encrypt_buffer(&ctx, wd, 32);

        printf("\nCLI CIPHERED TEXT: ") ;
        hexdump((char *)&wd, 32);
        fflush(stdout);

       

        sendMessage((RNS_ID-1), wd);

        if(fim) break;
    }
    
    printf("######### NONSEC %d ENDED\n",RNS_ID);
}

//unsigned char pkey[64] = {0xD6,0x79,0x9F,0xB7,0x3A,0xF9,0x68,0xBE,0xF8,0x60,0x1D,0x94,0x67,0x07,0x76,0x1B,0x8C,0xD0,0xED,0xDE,0xBD,0x34,0x8F,0xAF,0xE4,0x64,0xE4,0x48,0x86,0x5F,0x7B,0x60};
//unsigned char skey[64] = {0x01,0x03,0xAD,0x86,0xF1,0x9F,0xCF,0xE6,0x7B,0xE3,0x91,0x0C,0x52,0x89,0xC6,0xB9,0xAB,0xCD,0x63,0xF7,0x3C,0xF2,0x19,0x6E,0x36,0x9C,0x99,0xF8,0x6D,0x81,0xED,0x3A};


void generateSecretParameters(){

    int i;
    if(RNS_ID == 1){
        srand(RAND_SEED1);
    } else {
        srand(RAND_SEED2);
    }

	for( i = 0; i < 32; i++){
    	EC_keys.sk[i] = (unsigned char)rand()%256; 
	}

}

void computeKeys(){
    printf("\n############## CLI%d is initializing cryptographic variables...\n\n",RNS_ID);

    generateSecretParameters();

    crypto_box_keypair(EC_keys.pk, EC_keys.sk); 
    printf("CLI %d Private Key: HEX ",RNS_ID);
    hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("CLI %d Public Key: ",RNS_ID);
    hexdump((char *)&EC_keys.pk,  PUB_KEY_LEN);
    fflush(stdout);
}

void computeSessionKey(){


    printf("Sever Public Key: ");
    hexdump((char*)hisPublicKey, PUB_KEY_LEN);
    printf("Client secret Key: ");
    hexdump((char*)EC_keys.sk, PUB_KEY_LEN); 


    printf("\n#################### CLI%d is now processing the Shared Secret ####################\n\n",RNS_ID);
    crypto_box_beforenm(sharedSecret,hisPublicKey,EC_keys.sk);
    printf("CLI%d Shared Secret: ",RNS_ID);
    hexdump((char*)sharedSecret, PUB_KEY_LEN); 

    printf("Performing hash operation\n");

    //SHA-256 COMPUTING
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx,(unsigned char*)sharedSecret,PUB_KEY_LEN);
    sha256_final(&ctx,sessionKey);
    printf(" ------------------------------------------------------------------->  CLI%d Session Key: ",RNS_ID);
    hexdump((char*)sessionKey, PUB_KEY_LEN);        
    fflush(stdout);
}

void ECDH(){
    unsigned char wd[100];
    requireToRead(wd);
    memcpy(hisPublicKey,wd,64);
    computeKeys();  
    computeSessionKey(); 
    memcpy(wd,EC_keys.pk,PRIV_KEY_LEN);
    sendMessage((RNS_ID-1), wd);
}


int main() 
{
  
    enterNonSecure();
    printf("Hello from client %d!\n",RNS_ID);


    ECDH();

    memcpy(AESkey,sessionKey,PUB_KEY_LEN/2);
    memcpy(nounce,sessionKey+16,PUB_KEY_LEN/2);
    printf("\n");

    example1();
    return(0);
}