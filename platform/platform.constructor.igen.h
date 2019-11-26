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


// This file constructs the platform according to TCL script.
// This file should NOT need to be edited.

////////////////////////////////////////////////////////////////////////////////

void platformConstructor(void) {

    icmInitAttrs programControl_attrs = ICM_INIT_DEFAULT;

    icmInitPlatform(ICM_VERSION, programControl_attrs, 0, 0, "programControl");
    icmSetPlatformStatus(ICM_UNSET,ICM_BAREMETAL,ICM_VISIBLE);

////////////////////////////////////////////////////////////////////////////////
//                                 Bus pBusRS
////////////////////////////////////////////////////////////////////////////////


    handles.pBusRS_b = icmNewBus( "pBusRS", 32);

////////////////////////////////////////////////////////////////////////////////
//                                  Bus tzBus
////////////////////////////////////////////////////////////////////////////////


    handles.tzBus_b = icmNewBus( "tzBus", 41);

////////////////////////////////////////////////////////////////////////////////
//                                  Bus pBus
////////////////////////////////////////////////////////////////////////////////


    handles.pBus_b = icmNewBus( "pBus", 32);

////////////////////////////////////////////////////////////////////////////////
//                                Bus pBusRNS2
////////////////////////////////////////////////////////////////////////////////


    handles.pBusRNS2_b = icmNewBus( "pBusRNS2", 32);

////////////////////////////////////////////////////////////////////////////////
//                               Processor RScpu
////////////////////////////////////////////////////////////////////////////////


    const char *RScpu_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "arm"               ,    // name
        0                   ,    // version
        "model"                  // model
    );

    icmAttrListP RScpu_attrList = icmNewAttrList();
    icmAddStringAttr(RScpu_attrList, "variant", "Cortex-A8");
    icmAddDoubleAttr(RScpu_attrList, "mips", 0.000000);
    icmAddStringAttr(RScpu_attrList, "endian", "little");

    const char *armNewlib_0_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "armNewlib"         ,    // name
        0                   ,    // version
        "model"                  // model
    );
    icmNewProcAttrs RScpu_attrs = ICM_ATTR_DEFAULT;


    handles.RScpu_c = icmNewProcessor(
        "RScpu"             ,   // name
        0,
        0                   ,   // cpuId
        0x0000              ,  // flags
        41                  ,   // address bits
        RScpu_path          ,   // model
        0, 
        RScpu_attrs         ,   // procAttrs
        RScpu_attrList      ,   // attrlist
        armNewlib_0_path    ,   // semihost file
        0
    );


////////////////////////////////////////////////////////////////////////////////
//                              Processor cpuRNS1
////////////////////////////////////////////////////////////////////////////////


    const char *cpuRNS1_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "arm"               ,    // name
        0                   ,    // version
        "model"                  // model
    );

    icmAttrListP cpuRNS1_attrList = icmNewAttrList();
    icmAddStringAttr(cpuRNS1_attrList, "variant", "Cortex-A8");
    icmAddDoubleAttr(cpuRNS1_attrList, "mips", 0.000000);
    icmAddStringAttr(cpuRNS1_attrList, "endian", "little");

    const char *armNewlib_1_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "armNewlib"         ,    // name
        0                   ,    // version
        "model"                  // model
    );
    icmNewProcAttrs cpuRNS1_attrs = ICM_ATTR_DEFAULT;


    handles.cpuRNS1_c = icmNewProcessor(
        "cpuRNS1"           ,   // name
        0,
        1                   ,   // cpuId
        0x0000              ,  // flags
        32                  ,   // address bits
        cpuRNS1_path        ,   // model
        0, 
        cpuRNS1_attrs       ,   // procAttrs
        cpuRNS1_attrList    ,   // attrlist
        armNewlib_1_path    ,   // semihost file
        0
    );


////////////////////////////////////////////////////////////////////////////////
//                              Processor cpuRNS2
////////////////////////////////////////////////////////////////////////////////


    const char *cpuRNS2_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "arm"               ,    // name
        0                   ,    // version
        "model"                  // model
    );

    icmAttrListP cpuRNS2_attrList = icmNewAttrList();
    icmAddStringAttr(cpuRNS2_attrList, "variant", "Cortex-A8");
    icmAddDoubleAttr(cpuRNS2_attrList, "mips", 0.000000);
    icmAddStringAttr(cpuRNS2_attrList, "endian", "little");

    const char *armNewlib_2_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "armNewlib"         ,    // name
        0                   ,    // version
        "model"                  // model
    );
    icmNewProcAttrs cpuRNS2_attrs = ICM_ATTR_DEFAULT;


    handles.cpuRNS2_c = icmNewProcessor(
        "cpuRNS2"           ,   // name
        0,
        2                   ,   // cpuId
        0x0000              ,  // flags
        32                  ,   // address bits
        cpuRNS2_path        ,   // model
        0, 
        cpuRNS2_attrs       ,   // procAttrs
        cpuRNS2_attrList    ,   // attrlist
        armNewlib_2_path    ,   // semihost file
        0
    );


////////////////////////////////////////////////////////////////////////////////
//                                  PSE tzpc
////////////////////////////////////////////////////////////////////////////////


    const char *tzpc_path = icmGetVlnvString(
        0                   ,    // path (0 if from the product directory)
        "arm.ovpworld.org"  ,    // vendor
        0                   ,    // library
        "TzpcBP147"         ,    // name
        0                   ,    // version
        "pse"                    // model
    );

    icmAttrListP tzpc_attrList = icmNewAttrList();

    handles.tzpc_p = icmNewPSE(
        "tzpc"              ,   // name
        tzpc_path           ,   // model
        tzpc_attrList       ,   // attrlist
        0,       // unused
        0        // unused
    );

    icmConnectPSEBus( handles.tzpc_p, handles.pBusRS_b, "bport1", 0, 0x10001000, 0x10001fff);

////////////////////////////////////////////////////////////////////////////////
//                                 PSE hBridge
////////////////////////////////////////////////////////////////////////////////


    const char *hBridge_path = "peripheral/pse";

    icmAttrListP hBridge_attrList = icmNewAttrList();
    icmAddUns64Attr(hBridge_attrList, "portSize", 0x1000LL);

    handles.hBridge_p = icmNewPSE(
        "hBridge"           ,   // name
        hBridge_path        ,   // model
        hBridge_attrList    ,   // attrlist
        0,       // unused
        0        // unused
    );

    icmConnectPSEBus( handles.hBridge_p, handles.pBusRS_b, "readSecurePort", 0, 0x11001000, 0x11001007);

    icmConnectPSEBus( handles.hBridge_p, handles.pBus_b, "rxNonSecurePort1", 0, 0x11001000, 0x1100100b);

    icmConnectPSEBus( handles.hBridge_p, handles.pBusRNS2_b, "rxNonSecurePort2", 0, 0x11001000, 0x1100100b);

    icmConnectProcessorBusByName( handles.RScpu_c, "INSTRUCTION", handles.tzBus_b );

    icmConnectProcessorBusByName( handles.RScpu_c, "DATA", handles.tzBus_b );

    icmConnectProcessorBusByName( handles.cpuRNS1_c, "INSTRUCTION", handles.pBus_b );

    icmConnectProcessorBusByName( handles.cpuRNS1_c, "DATA", handles.pBus_b );

    icmConnectProcessorBusByName( handles.cpuRNS2_c, "INSTRUCTION", handles.pBusRNS2_b );

    icmConnectProcessorBusByName( handles.cpuRNS2_c, "DATA", handles.pBusRNS2_b );

////////////////////////////////////////////////////////////////////////////////
//                                Memory ramRS
////////////////////////////////////////////////////////////////////////////////

    handles.ramRS_m = icmNewMemory("ramRS", 0x7, 0x1fffff);


    icmConnectMemoryToBus( handles.pBusRS_b, "sp1", handles.ramRS_m, 0x0);

////////////////////////////////////////////////////////////////////////////////
//                              Memory secureRam
////////////////////////////////////////////////////////////////////////////////

    handles.secureRam_m = icmNewMemory("secureRam", 0x7, 0xffffff);


    icmConnectMemoryToBus( handles.pBusRS_b, "sp1", handles.secureRam_m, 0x1000000);

////////////////////////////////////////////////////////////////////////////////
//                               Memory ramRNS1
////////////////////////////////////////////////////////////////////////////////

    handles.ramRNS1_m = icmNewMemory("ramRNS1", 0x7, 0x1fffff);


    icmConnectMemoryToBus( handles.pBus_b, "sp1", handles.ramRNS1_m, 0x0);

////////////////////////////////////////////////////////////////////////////////
//                               Memory ramRNS2
////////////////////////////////////////////////////////////////////////////////

    handles.ramRNS2_m = icmNewMemory("ramRNS2", 0x7, 0x1fffff);


    icmConnectMemoryToBus( handles.pBusRNS2_b, "sp1", handles.ramRNS2_m, 0x0);

////////////////////////////////////////////////////////////////////////////////
//                               Bridge secure2
////////////////////////////////////////////////////////////////////////////////


    icmNewBusBridge(handles.tzBus_b, handles.pBusRS_b, "secure2", "sp", "mp", 0x0, 0xffffffff, 0x0);

////////////////////////////////////////////////////////////////////////////////
//                                Bridge ram0NS
////////////////////////////////////////////////////////////////////////////////


    icmNewBusBridge(handles.tzBus_b, handles.pBusRS_b, "ram0NS", "sp", "mp", 0x0, 0x1fffff, 0x10000000000ULL);

////////////////////////////////////////////////////////////////////////////////
//                                Bridge hBNSRS
////////////////////////////////////////////////////////////////////////////////


    icmNewBusBridge(handles.tzBus_b, handles.pBusRS_b, "hBNSRS", "sp", "mp", 0x11001000, 0x11001002, 0x10011001000ULL);


////////////////////////////////////////////////////////////////////////////////
//                                 CONNECTIONS
////////////////////////////////////////////////////////////////////////////////

    handles.fiq__n = icmNewNet("fiq_" );

    icmConnectProcessorNet( handles.RScpu_c, handles.fiq__n, "irq", 0);

    icmConnectPSENet( handles.hBridge_p, handles.fiq__n, "newMessageAvailable", 0);

////////////////////////////////////////////////////////////////////////////////
    handles.tzpcdecprot0_0_n = icmNewNet("tzpcdecprot0_0" );

    icmConnectPSENet( handles.tzpc_p, handles.tzpcdecprot0_0_n, "TZPCDECPROT0_0", 0);

    icmConnectPSENet( handles.hBridge_p, handles.tzpcdecprot0_0_n, "enable", 0);
}
