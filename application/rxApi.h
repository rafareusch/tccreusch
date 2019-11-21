#define RG_ACK_INTERRUPT ((volatile unsigned char *) (0x11001000))
#define RG_READ_HEADER_DATA ((volatile unsigned char *) (0x11001002))
#define RG_READ_DATA ((volatile unsigned char *) (0x11001003))

#define SECURE_MEMORY_REGION ((volatile unsigned char *) (0x01000000))

// @brief: TBD
void ackInterruptHandler()
{
    (*(char*) RG_ACK_INTERRUPT) = 0x1;
}



// @brief: TBD
static void receiveMessage(int size)
{
    int i;
    char* secureMemoryBuffer = malloc(size);
    printf("Reading message of %d\n", size);
    printf("####\nRECEIVED WAS:\n");
    for(i = 0; i< size; i++)
    {
        secureMemoryBuffer[i] = *(RG_READ_DATA);
        printf("%c",secureMemoryBuffer[i]);
    }
    
    printf("\n######\n");
    memcpy(SECURE_MEMORY_REGION,secureMemoryBuffer, size);
}
