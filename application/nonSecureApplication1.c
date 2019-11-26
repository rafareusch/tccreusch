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
    sendMessage(7,0, "Example");

    
    requireToSend();
    sendMessage(12,0, "RNS2-Message");
}
