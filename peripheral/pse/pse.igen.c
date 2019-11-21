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


#include "pse.igen.h"
/////////////////////////////// Port Declarations //////////////////////////////

readSecurePort_regs_dataT readSecurePort_regs_data;

txNonSecurePort_txRegs_dataT txNonSecurePort_txRegs_data;

txNonSecurePort2_txRegs_dataT txNonSecurePort2_txRegs_data;

handlesT handles;

/////////////////////////////// Diagnostic level ///////////////////////////////

// Test this variable to determine what diagnostics to output.
// eg. if (diagnosticLevel >= 1) bhmMessage("I", "HelioDynamicBridge", "Example");
//     Predefined macros PSE_DIAG_LOW, PSE_DIAG_MEDIUM and PSE_DIAG_HIGH may be used
Uns32 diagnosticLevel;

/////////////////////////// Diagnostic level callback //////////////////////////

static void setDiagLevel(Uns32 new) {
    diagnosticLevel = new;
}

///////////////////////////// MMR Generic callbacks ////////////////////////////

static PPM_VIEW_CB(view8) {  *(Uns8*)data = *(Uns8*)user; }

static PPM_VIEW_CB(view16) {  *(Uns16*)data = *(Uns16*)user; }

//////////////////////////////// Bus Slave Ports ///////////////////////////////

static void installSlavePorts(void) {
    handles.readSecurePort = ppmCreateSlaveBusPort("readSecurePort", 8);
    if (!handles.readSecurePort) {
        bhmMessage("E", "PPM_SPNC", "Could not connect port 'readSecurePort'");
    }

    ppmInstallReadCallback(dataHeaderRead, (void*)0x0 , handles.readSecurePort + 0x1, 0x2);
    ppmInstallReadCallback(dataRead, (void*)0x0 , handles.readSecurePort + 0x3, 0x1);
    handles.txNonSecurePort = ppmCreateSlaveBusPort("txNonSecurePort", 12);

    handles.txNonSecurePort2 = ppmCreateSlaveBusPort("txNonSecurePort2", 12);

}

//////////////////////////// Memory mapped registers ///////////////////////////

static void installRegisters(void) {

    ppmCreateRegister("regs_ns_interrupt_ack",
        0,
        handles.readSecurePort,
        0,
        2,
        interruptRead,
        interruptWrite,
        view16,
        &(readSecurePort_regs_data.ns_interrupt_ack.value),
        True
    );

    ppmCreateRegister("txRegs_tx_req",
        0,
        handles.txNonSecurePort,
        0,
        1,
        0,
        writeTxReq,
        view8,
        &(txNonSecurePort_txRegs_data.tx_req.value),
        True
    );
    ppmCreateRegister("txRegs_tx_ack",
        0,
        handles.txNonSecurePort,
        1,
        1,
        readTxAck,
        0,
        view8,
        &(txNonSecurePort_txRegs_data.tx_ack.value),
        True
    );
    ppmCreateRegister("txRegs_dataTxLocalHeader",
        0,
        handles.txNonSecurePort,
        2,
        1,
        0,
        txWriteHeader,
        view8,
        &(txNonSecurePort_txRegs_data.dataTxLocalHeader.value),
        True
    );
    ppmCreateRegister("txRegs_dataTxLocal",
        0,
        handles.txNonSecurePort,
        4,
        1,
        0,
        txWrite,
        view8,
        &(txNonSecurePort_txRegs_data.dataTxLocal.value),
        True
    );

    ppmCreateRegister("txRegs_tx_req",
        0,
        handles.txNonSecurePort2,
        0,
        1,
        0,
        writeTxReqRNS2,
        view8,
        &(txNonSecurePort2_txRegs_data.tx_req.value),
        True
    );
    ppmCreateRegister("txRegs_tx_ack",
        0,
        handles.txNonSecurePort2,
        1,
        1,
        readTxAckRNS2,
        0,
        view8,
        &(txNonSecurePort2_txRegs_data.tx_ack.value),
        True
    );
    ppmCreateRegister("txRegs_dataTxLocalHeader",
        0,
        handles.txNonSecurePort2,
        2,
        1,
        0,
        txWriteHeader,
        view8,
        &(txNonSecurePort2_txRegs_data.dataTxLocalHeader.value),
        True
    );
    ppmCreateRegister("txRegs_dataTxLocal",
        0,
        handles.txNonSecurePort2,
        4,
        1,
        0,
        txWrite,
        view8,
        &(txNonSecurePort2_txRegs_data.dataTxLocal.value),
        True
    );

}

/////////////////////////////////// Net Ports //////////////////////////////////

static void installNetPorts(void) {
    handles.enable = ppmOpenNetPort("enable");
    if (handles.enable) {
        ppmInstallNetCallback(handles.enable, updateEnable, (void*)0);
    }

// To write to this net, use ppmWriteNet(handles.interruptRequest, value);

    handles.interruptRequest = ppmOpenNetPort("interruptRequest");

}

////////////////////////////////// Constructor /////////////////////////////////

PPM_CONSTRUCTOR_CB(periphConstructor) {
    installSlavePorts();
    installRegisters();
    installNetPorts();
}

///////////////////////////////////// Main /////////////////////////////////////

int main(int argc, char *argv[]) {

    diagnosticLevel = 0;
    bhmInstallDiagCB(setDiagLevel);
    constructor();

    bhmWaitEvent(bhmGetSystemEvent(BHM_SE_END_OF_SIMULATION));
    return 0;
}

