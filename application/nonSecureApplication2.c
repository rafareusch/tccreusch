#include <stdio.h>
#include "armDefines.h"
#include "txApi.h"
#include "tweetnacl.h"
#include "sha256.h"
#include "time.h"

#define RNS_ID 2
#define RAND_SEED1 0x921797a172461ee6cec24f9fb8c6442a
#define RAND_SEED2 0xbca694756748677a744aa9072dc9e666

typedef struct {
    unsigned char sk[PRIV_KEY_LEN];
    unsigned char pk[PUB_KEY_LEN];
} ec_keys;

ec_keys EC_keys; 
unsigned char sharedSecret[PUB_KEY_LEN];
unsigned char sessionKey[PUB_KEY_LEN];
unsigned char hisPublicKey[PUB_KEY_LEN];


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

void example1(){
    int fim, i=0;
    unsigned char wd[100];
    
    // requireToRead(wd);
    // printf("\n\n################### KEY Received: [%s]\n ", wd);
        
    
    for(;;) {
        requireToRead(wd);
        
        printf("\n\n###################Received: [%s]\n ", wd);
        
        if (wd[0]=='1')  fim=1;  else fim=0;
        //memcpy(wd,EC_keys.pk,PRIV_KEY_LEN);
        sprintf(wd,"{from NONSEC[%d] to SEC - packet %d}", RNS_ID, i++);
        printf(" ### Sending [%s] \n", wd);
        sendMessage((RNS_ID-1), wd);

        if(fim) break;
    }
    
    printf("#### NONSEC %d ENDED",RNS_ID);
}

//unsigned char pkey[64] = {0xD6,0x79,0x9F,0xB7,0x3A,0xF9,0x68,0xBE,0xF8,0x60,0x1D,0x94,0x67,0x07,0x76,0x1B,0x8C,0xD0,0xED,0xDE,0xBD,0x34,0x8F,0xAF,0xE4,0x64,0xE4,0x48,0x86,0x5F,0x7B,0x60};
//unsigned char skey[64] = {0x01,0x03,0xAD,0x86,0xF1,0x9F,0xCF,0xE6,0x7B,0xE3,0x91,0x0C,0x52,0x89,0xC6,0xB9,0xAB,0xCD,0x63,0xF7,0x3C,0xF2,0x19,0x6E,0x36,0x9C,0x99,0xF8,0x6D,0x81,0xED,0x3A};

void generateSecretKey(){

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
    printf("\n### RNS%d is initializing cryptographic variables...\n",RNS_ID);

    generateSecretKey();

    crypto_box_keypair(EC_keys.pk, EC_keys.sk); 
    printf("RNS %d Private Key: HEX ",RNS_ID);
    hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("RNS %d Public Key: ",RNS_ID);
    hexdump((char *)&EC_keys.pk,  PUB_KEY_LEN);
    fflush(stdout);
}

void computeSessionKey(){


    printf("###### RNS%d is now processing the Session Key\n",RNS_ID);
    crypto_box_beforenm(sharedSecret,hisPublicKey,EC_keys.sk);
    // printf("RNS%d Shared Secret: ",RNS_ID);
    // hexdump((char*)sharedSecret, PUB_KEY_LEN); 

    //SHA-256 COMPUTING
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx,(unsigned char*)sharedSecret,PUB_KEY_LEN);
    sha256_final(&ctx,sessionKey);
    printf(" ------------------------------------------------------------------->  RNS%d Session Key: ",RNS_ID);
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
    printf("Hello from non-secure processor %d!\n",RNS_ID);


    ECDH();
    
    
    example1();
    //while(1);
    //exit(1);
    return(0);
}