#include <stdio.h>
#include "transferMsg.h"
#include "armDefines.h"
#include "txApi.h"

// Enter non-secure mode by setting SCR.NS (bit 0)
void enterNonSecure() 
{
    unsigned int scr;
    RD_SCR(scr);
    WR_SCR(scr | 1);
}

int main() 
{
    enterNonSecure();
    printf("Hello from non-secure processor 1!\n");
    requireToSend();
    
    // sender is 1
    *RG_TX_WRITE_HEADER = 0;
    
    // size is 5
    *RG_TX_WRITE_HEADER = 5;
    
    *RG_TX_WRITE_DATA = 'H';
    *RG_TX_WRITE_DATA = 'E';
    *RG_TX_WRITE_DATA = 'L';
    *RG_TX_WRITE_DATA = 'I';
    *RG_TX_WRITE_DATA = 'O';
    


    requireToSend();
    
    // sender is 1
    *RG_TX_WRITE_HEADER = 0;
    
    // size is 5
    *RG_TX_WRITE_HEADER = 4;
    
    *RG_TX_WRITE_DATA = 'b';
    *RG_TX_WRITE_DATA = 'i';
    *RG_TX_WRITE_DATA = 'b';
    *RG_TX_WRITE_DATA = 'i';

        requireToSend();
    
    // sender is 1
    *RG_TX_WRITE_HEADER = 0;
    
    // size is 5
    *RG_TX_WRITE_HEADER = 4;
    
    *RG_TX_WRITE_DATA = 'c';
    *RG_TX_WRITE_DATA = 'o';
    *RG_TX_WRITE_DATA = 'b';
    *RG_TX_WRITE_DATA = 'i';
}
