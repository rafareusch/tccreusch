#include <stdio.h>
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
    sendMessage(1,"Test") ;   

    requireToRead();

    requireToSend();
    sendMessage(1,"Message") ;  
    
    // requireToSend();
    // sendMessage(1,"Message2") ;  
}
