/*
 * Copyright (c) 2005-2017 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                             Version 20170201.0
//
////////////////////////////////////////////////////////////////////////////////

#ifndef PSE_IGEN_H
#define PSE_IGEN_H

#ifdef _PSE_
#    include "peripheral/impTypes.h"
#    include "peripheral/bhm.h"
#    include "peripheral/bhmHttp.h"
#    include "peripheral/ppm.h"
#else
#    include "hostapi/impTypes.h"
#endif

//////////////////////////////////// Externs ///////////////////////////////////

extern Uns32 diagnosticLevel;


/////////////////////////// Dynamic Diagnostic Macros //////////////////////////

// Bottom two bits of word used for PSE diagnostics
#define PSE_DIAG_LOW      (BHM_DIAG_MASK_LOW(diagnosticLevel))
#define PSE_DIAG_MEDIUM   (BHM_DIAG_MASK_MEDIUM(diagnosticLevel))
#define PSE_DIAG_HIGH     (BHM_DIAG_MASK_HIGH(diagnosticLevel))
// Next two bits of word used for PSE semihost/intercept library diagnostics
#define PSE_DIAG_SEMIHOST (BHM_DIAG_MASK_SEMIHOST(diagnosticLevel))

/////////////////////////// Register data declaration //////////////////////////

typedef struct readSecurePort_regs_dataS { 
    union { 
        Uns16 value;
    } ns_interrupt_ack;
} readSecurePort_regs_dataT, *readSecurePort_regs_dataTP;

typedef struct rxNonSecurePort1_rxRegs_dataS { 
    union { 
        Uns8 value;
    } rx_req;
    union { 
        Uns8 value;
    } rx_ack;
    union { 
        Uns8 value;
    } dataTxLocalHeader;
    union { 
        Uns8 value;
    } dataTxLocal;
} rxNonSecurePort1_rxRegs_dataT, *rxNonSecurePort1_rxRegs_dataTP;

typedef struct rxNonSecurePort2_rxRegs_dataS { 
    union { 
        Uns8 value;
    } rx_req;
    union { 
        Uns8 value;
    } rx_ack;
    union { 
        Uns8 value;
    } dataTxLocalHeader;
    union { 
        Uns8 value;
    } dataTxLocal;
} rxNonSecurePort2_rxRegs_dataT, *rxNonSecurePort2_rxRegs_dataTP;

/////////////////////////////// Port Declarations //////////////////////////////

extern readSecurePort_regs_dataT readSecurePort_regs_data;

extern rxNonSecurePort1_rxRegs_dataT rxNonSecurePort1_rxRegs_data;

extern rxNonSecurePort2_rxRegs_dataT rxNonSecurePort2_rxRegs_data;

#ifdef _PSE_
///////////////////////////////// Port handles /////////////////////////////////

typedef struct handlesS {
    void                 *readSecurePort;
    void                 *rxNonSecurePort1;
    void                 *rxNonSecurePort2;
    ppmNetHandle          enable;
    ppmNetHandle          newMessageAvailable;
} handlesT, *handlesTP;

extern handlesT handles;

////////////////////////////// Callback prototypes /////////////////////////////

PPM_REG_READ_CB(interruptRead);
PPM_REG_WRITE_CB(interruptWrite);
PPM_REG_READ_CB(readTxAck);
PPM_REG_READ_CB(readTxAckRNS2);
PPM_REG_WRITE_CB(txWrite);
PPM_REG_WRITE_CB(txWriteHeader);
PPM_NET_CB(updateEnable);
PPM_REG_WRITE_CB(writeTxReq);
PPM_REG_WRITE_CB(writeTxReqRNS2);
PPM_READ_CB(dataHeaderRead);
PPM_READ_CB(dataRead);
PPM_CONSTRUCTOR_CB(periphConstructor);
PPM_CONSTRUCTOR_CB(constructor);
PPM_SAVE_STATE_FN(peripheralSaveState);
PPM_RESTORE_STATE_FN(peripheralRestoreState);

///////////////////////////////// Local memory /////////////////////////////////

// To access local memory readSecurePort/regs/ns_data_header : (for example)
// char value = ((char*)handles.readSecurePort)[READSECUREPORT_REGS_NS_DATA_HEADER_OFFSET + myAddress];

#define READSECUREPORT_REGS_NS_DATA_HEADER_OFFSET  1
#define READSECUREPORT_REGS_NS_DATA_HEADER_SIZE    2

// To access local memory readSecurePort/regs/ns_data : (for example)
// char value = ((char*)handles.readSecurePort)[READSECUREPORT_REGS_NS_DATA_OFFSET + myAddress];

#define READSECUREPORT_REGS_NS_DATA_OFFSET  3
#define READSECUREPORT_REGS_NS_DATA_SIZE    1

#endif

#endif
