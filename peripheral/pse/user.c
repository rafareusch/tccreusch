#include "pse.igen.h"
#define RXNONSECUREPORT2
#define READSECUREPORT
#define RXNONSECUREPORT1
#define WRITESECUREPORT
#include "pse.macros.igen.h"
#include <string.h>


#define RNSID1 0
#define RNSID2 1

#define HEADER_SIZE 8

SimAddr mpNSLoAddress;
SimAddr spSLoAddress;




SimAddr mpSLoAddress;
SimAddr spNSLoAddress;

Uns64   portSize;
Bool    transmissionEnabled = False;
Bool    newMessage = False;

enum
{
    FREE,
    LOCKED,
    TRANSMITTING
};


int rnsRequest[2] = {0};

// Define a struct that stores each flit and the time that it arrive in the router
typedef struct{

    int sender; // @0 - 4

    int messageSize; // @4 - 8

}messageHeader;

messageHeader header;

void requireToSend(int rnsRequiring, int data)
{
    Bool canTransmit = True;
    if(data == 1)
    {
        int i;
        for(i=0; i<2;i++)
        {
            if(rnsRequest[i] != FREE)
            {
                canTransmit = False;
            }
        }
        if(canTransmit)
        {
            rnsRequest[rnsRequiring] = LOCKED;
            bhmMessage("I","RNS Tx require ","Locking buffer to RNS%d", rnsRequiring + 1);
        }
        else
        {
            bhmMessage("I","RNS Tx require ","RNS%d is not allowed to send", rnsRequiring + 1);
        }
        
    }

}

//////////////////////////////// Callback stubs ////////////////////////////////

PPM_NET_CB(updateEnable) {
    if(value == 1)
    {
        bhmMessage("I","RX Enable register","RS allowed transmission!");
    }
    else
    {
        bhmMessage("I","RX Enable register","RS disabled transmission!");
    }
    
    transmissionEnabled = value;
}

PPM_REG_WRITE_CB(interruptWrite) 
{
    bhmMessage("I","Interrupt","Received Interrupt ACK");
    ppmWriteNet(handles.newMessageAvailable, 0);
}

PPM_REG_READ_CB(interruptRead) 
{
    bhmMessage("I","Interrupt","Read");
    return 0;
}






// RS to RNS CALLBACKS


PPM_REG_WRITE_CB(ReqRSWrite) 
{
    bhmMessage("I"," ","Received Request from Secure Processor");
    //ppmWriteNet(handles.newMessageAvailable, 0);
}

PPM_REG_READ_CB(ReqRSRead) 
{
    bhmMessage("I"," ","Reading ACK");
    return 1;
}

PPM_REG_READ_CB(AckRSRead) 
{
    bhmMessage("I","Interrupt","Read");
    return 1;
}

PPM_REG_WRITE_CB(headerRSWrite) 
{
    static int headerRS[2];
    static int headerOffset = 0;

    bhmMessage("I","Request","Received HEADER from Secure Processor %d", data);
    headerRS[headerOffset] = data;
    if (headerOffset == 1){
        headerOffset = 0;
    } else {
        headerOffset++;
    }
    //ppmWriteNet(handles.newMessageAvailable, 0);
}

PPM_REG_WRITE_CB(dataRSWrite) 
{
    bhmMessage("I","Request","Received DATA from Secure Processor %c", data);
    //ppmWriteNet(handles.newMessageAvailable, 0);
}































PPM_READ_CB(dataHeaderRead)
{
    static char offset = 0;
    char *ret = (char*)(&header);
    ret = ret + offset;
    offset++;

    if(offset == HEADER_SIZE)
    {
        bhmMessage("I","Secure Register Header"," Header reset");
        offset = 0;
    }
    return *ret;
}


PPM_READ_CB(dataRead)
{
    static int dataReadCount = 0;
    char rtValue = 0;
    if(transmissionEnabled)
    {
        if(dataReadCount < header.messageSize)
        {
            bhmMessage("I","Secure buffer data","Reading");

            char* prtValue = 0;
            prtValue = &((char*)handles.readSecurePort)[READSECUREPORT_REGS_NS_DATA_OFFSET];
            prtValue = prtValue + dataReadCount;
            rtValue = *prtValue;
            dataReadCount++;
        }
        
        if(dataReadCount >= header.messageSize)
        {
            bhmMessage("I","Secure buffer data","No message in buffer, releasing to any port write");
            rnsRequest[0] = FREE;
            rnsRequest[1] = FREE;
            dataReadCount = 0;
        }
    }
    else
    {
        bhmMessage("I","Secure buffer data","data transfer is disabled");
    }

    return rtValue;
}


Bool isSomeRNSTransmitingData()
{
    Bool ret = False;
    int i = 0;

    for(i = 0; i < 2; i++)
    {
        if(rnsRequest[i] == TRANSMITTING)
        {
            ret = True;
        }
    }
    return ret;
}

// Callbacks to port RNS1 

// !@ brief: Callback to when some proccesor write to require to write.
PPM_REG_WRITE_CB(writeTxReq)
{
    requireToSend(data, RNSID1);
}


int offsetHeader = 0;
// Write data header
PPM_REG_WRITE_CB(txWriteHeader)
{
    int rnsWriting = -1;
    
    bhmMessage("I","TX Header Write","Write to header data was %d!", data);
    if(offsetHeader ==  0)
    {
        rnsWriting = data;
    }
    else
    {
        rnsWriting = header.sender;
    }
    

    if(rnsRequest[rnsWriting] == TRANSMITTING)
    {
        if(offsetHeader ==  0)
        {
            header.sender = data;
            offsetHeader++;
        }
        else
        {
            header.messageSize = data;
            offsetHeader = 0;
        }
    }
    else
    {
       
        bhmMessage("I","TX Header Write","RNS%d is not transmitting!", rnsWriting +1);
    }
}

//write datare
PPM_REG_WRITE_CB(txWrite)
{
    static int rnsWriteOffset = 0;
    // only allow data if this port is transmitting
    if(rnsRequest[header.sender] == TRANSMITTING)
    {
        if(rnsWriteOffset < header.messageSize)
        {
            ((char*)handles.readSecurePort)[READSECUREPORT_REGS_NS_DATA_OFFSET + rnsWriteOffset] = data;
            rnsWriteOffset++;
        }
        if(rnsWriteOffset == header.messageSize)
        {
            bhmMessage("I","TX Write","End of transmission, generating interrupt to secure processor!");
            ppmWriteNet(handles.newMessageAvailable, 1);
            rnsWriteOffset = 0;
        }
    }
    else
    {
        bhmMessage("I","TX Write","It is not transmitting!");
    }
    
}


Bool canGivenRNSTransmit(int cpuRNSId)
{
    Bool ret = False;
    if(isSomeRNSTransmitingData() == False)
    {
        
        ret = True;
        rnsRequest[cpuRNSId] = TRANSMITTING;
        bhmMessage("I","RNS TX Register","RNS%d is now transmitting!", cpuRNSId+1 );

    }
    return ret;
}

// !@ brief: Register that return to someone if it can write
PPM_REG_READ_CB(readTxAck)
{
    return canGivenRNSTransmit(RNSID1);
}


//callback to RNS port 2
PPM_REG_READ_CB(readTxAckRNS2) {
    
    return canGivenRNSTransmit(RNSID2);
}


PPM_REG_WRITE_CB(writeTxReqRNS2) 
{
    requireToSend(1, data);
    // YOUR CODE HERE (writeTxReqRNS2)
    *(Uns8*)user = data;
}

PPM_CONSTRUCTOR_CB(constructor) {

    periphConstructor();
    Bool enableBridge;
    bhmBoolParamValue("enableBridge", &enableBridge);

    //memset(data,0, 120);        
    if (handles.enable || enableBridge) {

        if(enableBridge)
        	updateEnable(1, 0);
    }
}


PPM_SAVE_STATE_FN(peripheralSaveState) {
    // YOUR CODE HERE (peripheralSaveState)
}

PPM_RESTORE_STATE_FN(peripheralRestoreState) {
    // YOUR CODE HERE (peripheralRestoreState)
}

