#include <stdio.h>
#include "armDefines.h"
#include "txApi.h"
#include "tweetnacl.h"
#include "sha256.h"
#define PRIV_KEY_LEN 32
#define PUB_KEY_LEN 32


typedef struct {
    unsigned char sk[PRIV_KEY_LEN];
    unsigned char pk[PUB_KEY_LEN];
} ec_keys;

ec_keys EC_keys; 
//unsigned char sharedSecret[PUB_KEY_LEN];
//unsigned char sessionKey[32];


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


void example1(){
    int fim, i=0;
    char wd[100];
    printf("On\n");
    for(;;) {
        requireToRead(wd);
        printf("\n\n###################Received: [%s] ", wd);
        
        if (wd[0]=='1')  fim=1;  else fim=0;

        sprintf(wd,"{from NONSEC[1] to SEC - packet %d}", i++);
        printf(" ### Sending [%s] \n", wd);
        sendMessage(0, wd);

        if(fim) break;
     }

     puts("#### NONSEC 1 ENDED");
}

void example2(){
    printf("Crypto\n");
    crypto_box_keypair(EC_keys.pk, EC_keys.sk);
    printf("RNS 1 Private Key: ");
    hexdump((char *)&EC_keys.sk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("RNS 1 Public Key: ");
    hexdump((char *)&EC_keys.pk,  PUB_KEY_LEN);
    fflush(stdout);
    printf("\n");
}


int main() 
{
  

    enterNonSecure();
    printf("Hello from non-secure processor 1!\n");
    // infite loop reacting to the secure processor
    
    // memcpy(EC_keys.pk,"5028ED1C62CF6920D529880C8E49AF696E140411C86FDA01E7FC5446E753C86B",sizeof(EC_keys.pk));;
    // memcpy(EC_keys.sk,"5028ED1C62CF6920D529880C8E49AF696E140411C86FDA01E7FC5446E753C86B",sizeof(EC_keys.pk));
    // EC_keys.pk = "5028ED1C62CF6920D529880C8E49AF696E140411C86FDA01E7FC5446E753C86B";
    // EC_keys.sk = "5028ED1C62CF6920D529880C8E49AF696E140411C86FDA01E7FC5446E753C86B";
    //example2();
    example1();
    //while(1);
    return(0);
}
