/*
 *
 * Copyright (c) 2005-2017 Imperas Software Ltd., www.imperas.com
 *
 * The contents of this file are provided under the Software License
 * Agreement that you accepted before downloading this file.
 *
 * This source forms part of the Software and can be used for educational,
 * training, and demonstration purposes but cannot be used for derivative
 * works except in cases where the derivative works require OVP technology
 * to run.
 *
 * For open source models released under licenses that you can use for
 * derivative works, please visit www.OVPworld.org or www.imperas.com
 * for the location of the open source models.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
//                W R I T T E N   B Y   I M P E R A S   I G E N
//
//                              Version 99999999
//
////////////////////////////////////////////////////////////////////////////////

// This file constructs the platform.
// This file should NOT need to be edited.

#include "ovpworld.org/modelSupport/tlmDecoder/1.0/tlm2.0/tlmDecoder.hpp"
#include "ovpworld.org/memory/ram/1.0/tlm2.0/tlmMemory.hpp"
#include "arm.ovpworld.org/processor/arm/1.0/tlm2.0/arm_Cortex-A9UP.igen.hpp"
#include "arm.ovpworld.org/peripheral/UartPL011/1.0/tlm2.0/pse.igen.hpp"
#include "ovpworld.org/peripheral/DynamicBridge/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/TzpcBP147/1.0/tlm2.0/pse.igen.hpp"


////////////////////////////////////////////////////////////////////////////////
//                            programControl Class
////////////////////////////////////////////////////////////////////////////////

class programControl : public sc_core::sc_module {

  public:
    programControl (sc_core::sc_module_name name);

    icmTLMPlatform        Platform;
    decoder             <2,4> tzBus;
    decoder             <4,3> pBus;
    ram                   ram0;
    arm_Cortex_A9UP       cpu;
    UartPL011             uart0;
    DynamicBridge         uart0NS;
    TzpcBP147             tzpc;


    icmAttrListObject *attrsForcpu() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("variant", "Cortex-A9UP");
        userAttrs->addAttr("compatibility", "nopSVC");
        userAttrs->addAttr("endian", "little");
        return userAttrs;
    }

    icmAttrListObject *attrsForuart0() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("outfile", "uart0.log");
        return userAttrs;
    }

    icmAttrListObject *attrsForuart0NS() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("mpLoAddress", (int)0x10000000);
        userAttrs->addAttr("spLoAddress", (Uns64)0x10010000000ULL);
        userAttrs->addAttr("portSize", (int)0x1000);
        return userAttrs;
    }
}; /* programControl */

////////////////////////////////////////////////////////////////////////////////
//                         programControl Constructor
////////////////////////////////////////////////////////////////////////////////

programControl::programControl ( sc_core::sc_module_name name)
    : sc_core::sc_module (name)
    , Platform ("icm", ICM_INIT_DEFAULT)
    , tzBus("tzBus")
    , pBus("pBus")
    , ram0 ("ram0", "sp1", 0x1fffff)
    , cpu ( "cpu", 0, ICM_ATTR_DEFAULT, attrsForcpu(),  icmGetVlnvString (NULL, "arm.ovpworld.org", "semihosting", "armNewlib", "1.0", "model") )
    , uart0 ("uart0", attrsForuart0())
    , uart0NS ("uart0NS", attrsForuart0NS())
    , tzpc ("tzpc")
{

    // tzBus masters
    cpu.INSTRUCTION.socket(tzBus.target_socket[0]);
    cpu.DATA.socket(tzBus.target_socket[1]);

    // tzBus slaves
    tzBus.initiator_socket[0](uart0NS.sp.socket); // Peripheral
    tzBus.setDecode(0, 0x0, 0x0);

    tzBus.initiator_socket[1](pBus.target_socket[1]); // Bridge
    tzBus.setDecode(1, 0x0, 0xffffffff);

    tzBus.initiator_socket[2](pBus.target_socket[2]); // Bridge
    tzBus.setDecode(2, 0x10000000000ULL, 0x100001fffffULL);

    tzBus.initiator_socket[3](pBus.target_socket[3]); // Bridge
    tzBus.setDecode(3, 0x10010001000ULL, 0x10010001fffULL);


    // pBus masters
    uart0NS.mp.socket(pBus.target_socket[0]);

    // pBus slaves
    pBus.initiator_socket[0](uart0.bport1.socket); // Peripheral
    pBus.setDecode(0, 0x10000000, 0x10000fff);

    pBus.initiator_socket[1](tzpc.bport1.socket); // Peripheral
    pBus.setDecode(1, 0x10001000, 0x10001fff);

    pBus.initiator_socket[2](ram0.sp1); // Memory
    pBus.setDecode(2, 0x0, 0x1fffff);


    // Net connections
    tzpc.TZPCDECPROT0_0(uart0NS.enable);
}
