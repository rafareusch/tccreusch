#define RG_TX_REQ ((volatile unsigned char *) (0x11001000))
#define RG_TX_REQ_ACK ((volatile unsigned char *) (0x11001001))
#define RG_TX_WRITE_HEADER ((volatile unsigned char *) (0x11001002))
#define RG_TX_WRITE_DATA ((volatile unsigned char *) (0x11001004))

// @brief: TBD
void requireToSend()
{
    printf("RNS: Requiring write permission\n");
    (*(char*) RG_TX_REQ) = 1;
    while(*RG_TX_REQ_ACK != 1);
    printf("Ready to send!\n");

}