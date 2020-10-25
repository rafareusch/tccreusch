#include <stdio.h>
#include "armDefines.h"
#include "txApi.h"
#include "../peripheral/pse/rules.h"

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

    
    sendMessage(1,"First Message");   
    
    //sendMessage(1,"RNS Message   ");
    
    char read[PACKET_SIZE];
    requireToRead(read);
    
    sendMessage(1,"Second Message");
    //sendMessage(1,"Third Message");
   //sendMessage(1,"Q Message");



     //requireToSend();
    // sendMessage(1,"Message") ;  
    
    // requireToSend();
    // sendMessage(1,"Message2") ;

}
