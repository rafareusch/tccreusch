set desc "DynamicBridge - Dynamically enable/disable a bus bridge from the input slave 
port to the output master port.
The bridge is enabled when the input net is high, disabled when it is low.
The size of the port is defined with the portSize parameter.
The address on the input slave port is defined by the spLoAddress parameter.
The address on the output master port is defined by the mpLoAddress parameter.
All three parameters must be specified.
The input and output ports may be connected to the same bus."

set limitations "The range of the input slave port must not conflict with any exiting port 
connected to the bus.
The output bus width is hard coded to be 32 bits." 
#

imodelnewperipheral  \
    -name        HelioDynamicBridge \
    -library     peripheral \
    -version     0.69 \
    -constructor constructor\
    -releasestatus ovp        \
    -visibility    visible 

# Output bus size is 32 bits
set outputBusWidth  32




#GENERAL
# Define enable net input port
imodeladdnetport -name enable -type input -updatefunction updateEnable

# Define formals for specifying the output master and slave bus port location and size
imodeladdformal -name portSize    -type integer
imodeladdformal -name enableBridge -type bool



## Secure registers
imodeladdbusslaveport -name readSecurePort \
                      -mustbeconnected

imodeladdaddressblock -name regs   \
                       -port readSecurePort \
                       -offset 0x0  \
                       -width 16    \
                       -size 8

imodeladdmmregister  -name ns_interrupt_ack \
                     -addressblock readSecurePort/regs \
                     -readfunction   interruptRead \
                     -writefunction  interruptWrite             

imodeladdlocalmemory -name ns_data_header \
                     -addressblock readSecurePort/regs \
                     -readfunction dataHeaderRead\
                     -offset 1 \
                     -size 2

imodeladdlocalmemory -name ns_data \
                     -addressblock readSecurePort/regs \
                     -readfunction dataRead\
                     -offset 3 \
                     -size 1

### 
imodeladdnetport -name interruptRequest -type output 


# Non secure registers - RNS1

imodeladdbusslaveport -name txNonSecurePort 

imodeladdaddressblock -name txRegs   \
                       -port txNonSecurePort \
                       -offset 0x0  \
                       -width 8    \
                       -size 12

imodeladdmmregister  -name tx_req \
                     -addressblock txNonSecurePort/txRegs \
                     -writefunction writeTxReq \
                     -offset 0x0 \
                     -access w 

imodeladdmmregister  -name tx_ack \
                     -addressblock txNonSecurePort/txRegs \
                     -readfunction  readTxAck \
                     -offset 0x1 \
                     -access r


imodeladdmmregister  -name dataTxLocalHeader \
                     -addressblock txNonSecurePort/txRegs \
                     -writefunction  txWriteHeader \
                     -offset 0x2 \
                     -width 2 \
                     -access w

imodeladdmmregister  -name dataTxLocal \
                     -addressblock txNonSecurePort/txRegs \
                     -writefunction  txWrite \
                     -offset 0x4 \
                     -access w

                     
# Non secure registers - RNS2

imodeladdbusslaveport -name txNonSecurePort2

imodeladdaddressblock -name txRegs   \
                       -port txNonSecurePort2 \
                       -offset 0x0  \
                       -width 8    \
                       -size 12

imodeladdmmregister  -name tx_req \
                     -addressblock txNonSecurePort2/txRegs \
                     -writefunction writeTxReqRNS2 \
                     -offset 0x0 \
                     -access w 

imodeladdmmregister  -name tx_ack \
                     -addressblock txNonSecurePort2/txRegs \
                     -readfunction  readTxAckRNS2 \
                     -offset 0x1 \
                     -access r


imodeladdmmregister  -name dataTxLocalHeader \
                     -addressblock txNonSecurePort2/txRegs \
                     -writefunction  txWriteHeader \
                     -offset 0x2 \
                     -width 2 \
                     -access w

imodeladdmmregister  -name dataTxLocal \
                     -addressblock txNonSecurePort2/txRegs \
                     -writefunction  txWrite \
                     -offset 0x4 \
                     -access w
