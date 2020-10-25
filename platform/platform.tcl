
ihwnew -name nonSecToSecValidation
ihwaddclp -allargs


ihwaddbus -instancename pBusRS  -addresswidth 32
# tzBus - 41 bits of address - implements secure and non-secure address spaces
ihwaddbus -instancename tzBus -addresswidth 41


## Region secure CPU
ihwaddprocessor -instancename RScpu -type arm -vendor arm.ovpworld.org -endian little \
                -semihostname armNewlib -semihostvendor arm.ovpworld.org

ihwconnect      -instancename RScpu -busmasterport INSTRUCTION -bus tzBus
ihwconnect      -instancename RScpu -busmasterport DATA        -bus tzBus
ihwconnect      -instancename RScpu -netport irq -net irq_

ihwsetparameter   -handle       RScpu -name variant       -value Cortex-A8   -type enum

ihwaddmemory -instancename ramRS -type ram
ihwconnect   -instancename ramRS -busslaveport sp1 -bus pBusRS \
             -loaddress 0x00000000 -hiaddress 0x001fffff


ihwaddmemory -instancename secureRam -type ram
ihwconnect   -instancename secureRam -busslaveport sp1 -bus pBusRS \
             -loaddress 0x01000000 -hiaddress 0x01ffffff

#
# Bridge - Secure space of tzBus always mapped to pBus
#
ihwaddbridge -instancename secure2
ihwconnect   -instancename secure2 -busslaveport  sp -bus tzBus \
             -loaddress 0x00000000 -hiaddress 0xffffffff
ihwconnect   -instancename secure2 -busmasterport mp -bus pBusRS  \
             -loaddress 0x00000000 -hiaddress 0xffffffff



#
# Bridge - Memory always accessible from non-secure mode
#
ihwaddbridge -instancename ram0NS
ihwconnect   -instancename ram0NS -busslaveport  sp -bus tzBus \
             -loaddress 0x10000000000 -hiaddress 0x100001fffff
ihwconnect   -instancename ram0NS -busmasterport mp -bus pBusRS \
             -loaddress 0x00000000 -hiaddress 0x001fffff


#
# Bridge helio perif to non secure space - Ack and header always accesible from non-secure mode
#-loaddress 0x11001000 -hiaddress 0x11001007
ihwaddbridge -instancename hBNSRS
ihwaddbridge -instancename 1_hBNSRS

ihwconnect   -instancename hBNSRS -busslaveport  sp -bus tzBus \
             -loaddress 0x10011001000 -hiaddress 0x10011001002
ihwconnect   -instancename hBNSRS -busmasterport mp -bus pBusRS \
             -loaddress 0x11001000 -hiaddress 0x11001002

ihwconnect   -instancename 1_hBNSRS -busslaveport  sp -bus tzBus \
             -loaddress 0x10011002000 -hiaddress 0x10011002007
ihwconnect   -instancename 1_hBNSRS -busmasterport mp -bus pBusRS \
             -loaddress 0x11002000 -hiaddress 0x11002007


#
# TZPC: 0x10001000    0x10001fff  BP147 TrustZone Protection Controller
#
ihwaddperipheral -instancename tzpc -type TzpcBP147  -vendor arm.ovpworld.org
ihwconnect       -instancename tzpc -busslaveport bport1 -bus pBusRS \
                 -loaddress 0x10001000 -hiaddress 0x10001fff
ihwconnect       -instancename tzpc -netport TZPCDECPROT0_0 -net tzpcdecprot0_0


# Non secure processor 1

ihwaddbus -instancename pBus  -addresswidth 32

ihwaddprocessor -instancename cpuRNS1 -type arm -vendor arm.ovpworld.org -endian little \
                -semihostname armNewlib -semihostvendor arm.ovpworld.org

ihwconnect      -instancename cpuRNS1 -busmasterport INSTRUCTION -bus pBus
ihwconnect      -instancename cpuRNS1 -busmasterport DATA        -bus pBus

ihwsetparameter   -handle       cpuRNS1 -name variant       -value Cortex-A8   -type enum

ihwaddmemory -instancename ramRNS1 -type ram
ihwconnect   -instancename ramRNS1 -busslaveport sp1 -bus pBus \
             -loaddress 0x00000000 -hiaddress 0x001fffff


# Non secure processor 2

ihwaddbus -instancename pBusRNS2  -addresswidth 32

ihwaddprocessor -instancename cpuRNS2 -type arm -vendor arm.ovpworld.org -endian little \
                -semihostname armNewlib -semihostvendor arm.ovpworld.org

ihwconnect      -instancename cpuRNS2 -busmasterport INSTRUCTION -bus pBusRNS2
ihwconnect      -instancename cpuRNS2 -busmasterport DATA        -bus pBusRNS2

ihwsetparameter   -handle       cpuRNS2 -name variant       -value Cortex-A8   -type enum

ihwaddmemory -instancename ramRNS2 -type ram
ihwconnect   -instancename ramRNS2 -busslaveport sp1 -bus pBusRNS2 \
             -loaddress 0x00000000 -hiaddress 0x001fffff



# NonSecToSec
ihwaddperipheral -instancename nonSecToSec -type nonSecToSec -modelfile peripheral/pse

ihwconnect       -instancename nonSecToSec -netport enable -net tzpcdecprot0_0
ihwconnect       -instancename nonSecToSec -netport newMessageAvailable -net irq_

ihwsetparameter    -handle       nonSecToSec -name portSize    -value        0x1000   \
-type uns64

ihwconnect      -instancename nonSecToSec -busslaveport readSecurePort -bus pBusRS \
                -loaddress 0x11001000 -hiaddress 0x1100107F

ihwconnect      -instancename nonSecToSec -busslaveport writeSecurePort -bus pBusRS \
                -loaddress 0x11002000 -hiaddress 0x1100207F

ihwconnect      -instancename nonSecToSec -busslaveport rxNonSecurePort1 \
                 -bus pBus -loaddress 0x11001000 -hiaddress 0x1100107f
                
ihwconnect      -instancename nonSecToSec -busslaveport rxNonSecurePort2 \
                 -bus pBusRNS2 -loaddress 0x11001000 -hiaddress 0x1100107f

                
