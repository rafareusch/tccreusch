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

// This file declares handles to platform objects.
// Access to these are required by parts of the ICM API.
// This file should NOT need to be edited.

struct handlesS {
    icmBusP              pBusRS_b;
    icmBusP              tzBus_b;
    icmBusP              pBus_b;
    icmBusP              pBusRNS2_b;
    icmProcessorP        RScpu_c;
    icmProcessorP        cpuRNS1_c;
    icmProcessorP        cpuRNS2_c;
    icmMemoryP           ramRS_m;
    icmMemoryP           secureRam_m;
    icmMemoryP           ramRNS1_m;
    icmMemoryP           ramRNS2_m;
    icmPseP              tzpc_p;
    icmPseP              hBridge_p;
    icmNetP              fiq__n;
    icmNetP              tzpcdecprot0_0_n;
} handles;
