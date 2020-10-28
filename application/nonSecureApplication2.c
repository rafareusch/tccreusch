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
    int fim, i=0;
    char wd[100];

    enterNonSecure();
    printf("Hello from non-secure processor 2!\n");

    // infite loop reacting to the secure processor
    for(;;) {
        requireToRead(wd);
        printf("\n\n###################Received: [%s] ", wd);

        if (wd[0]=='1')  fim=1;  else fim=0;

        sprintf(wd,"{from NONSEC[2] to SEC - packet %d}", i++);
        printf(" ### Sending [%s] \n", wd);
        sendMessage(1, wd);

        if(fim) break;
     }

     puts("#### NONSEC 2 ENDED");
     return(0);
}