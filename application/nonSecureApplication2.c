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
    printf("Hello from non-secure processor 2!\n");
    requireToSend();
    
    // sender is 1
    *RG_TX_WRITE_HEADER = 1;
    
    // size is 5
    *RG_TX_WRITE_HEADER = 4;
    
    *RG_TX_WRITE_DATA = 'F';
    *RG_TX_WRITE_DATA = 'U';
    *RG_TX_WRITE_DATA = 'Q';
    *RG_TX_WRITE_DATA = 'S';

    requireToSend();
          // sender is 1
    *RG_TX_WRITE_HEADER = 1;
    
    // size is 5
    *RG_TX_WRITE_HEADER = 6;
    
    *RG_TX_WRITE_DATA = 'S';
    *RG_TX_WRITE_DATA = 'U';
    *RG_TX_WRITE_DATA = 'Q';
    *RG_TX_WRITE_DATA = 'S';
    *RG_TX_WRITE_DATA = 'S';
    *RG_TX_WRITE_DATA = 'S';

}
