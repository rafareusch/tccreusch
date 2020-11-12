#define RG_ACK_INTERRUPT ((volatile unsigned char *) (0x11001000))
#define RG_READ_HEADER_DATA ((volatile unsigned char *) (0x11001002))
#define RG_READ_DATA ((volatile unsigned char *) (0x11001003))

#define RG_REQ ((volatile unsigned char *) (0x11002000))
#define RG_REQ_ACK ((volatile unsigned char *) (0x11002002))
#define RG_TX_WRITE_HEADER ((volatile unsigned char *) (0x11002004))
#define RG_TX_WRITE_DATA  ((volatile unsigned char *) (0x11002006))


#define SECURE_MEMORY_REGION ((volatile unsigned char *) (0x01000000))
#define SECURE_KEY_REGION ((volatile unsigned char *) (0x01000100))
#include <string.h>
#include "../peripheral/pse/rules.h"


//

//  RS to RNS
void requireToSend()
{
    printf("RS: Requiring write permission\n");
    (*(char*) RG_REQ) = 1;
    while(*RG_REQ_ACK != 1);
    printf("RS: Ready to send!\n");
   
}

void sendMessage(int target,char* data)
{
    int i = 0;
    int size = 0;
    // Write header
    

    if (strlen(data) > PACKET_SIZE){
        size = PACKET_SIZE;
        printf("RS: PACKET_SIZE OVERFLOW. Message Size must be under %d bits\n", PACKET_SIZE);
    } else {
        size = strlen(data);
    }

    *RG_TX_WRITE_HEADER = target;
    *RG_TX_WRITE_HEADER = size;
    
    for(i = 0; i < size; i++ )
    {
       *RG_TX_WRITE_DATA = data[i]; 
    }


    printf("RS: Sent a message!\n");
}









// RNS to RS

// @brief: Answer peripheral interrupt
void ackInterruptHandler()
{
    (*(char*) RG_ACK_INTERRUPT) = 0x1;
}

// @brief: Receive a message with given size from peripheral
// and save it to secure memory.
void receiveMessage(int size)
{
    
    int i;
    char* nonSecureMemoryBuffer = malloc(size);
    printf("Reading message of %d\n", size);
    

    for(i = 0; i< size; i++)
    {
        nonSecureMemoryBuffer[i] = *(RG_READ_DATA);
        //printf("%c",nonSecureMemoryBuffer[i]);
    }
    printf("####\nRECEIVED WAS:\n");
    hexdump(nonSecureMemoryBuffer,  size);
    fflush(stdout);

    

    
    printf("\n######\n");
    memcpy(SECURE_MEMORY_REGION,0, PACKET_SIZE);
    memcpy(SECURE_MEMORY_REGION,nonSecureMemoryBuffer, size);
    memset(nonSecureMemoryBuffer,0, size);
}
