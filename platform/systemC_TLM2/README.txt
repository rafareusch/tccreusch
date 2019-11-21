
  Copyright (c) 2005-2017 Imperas Software Ltd. All Rights Reserved.

  The contents of this file are provided under the Software License
  Agreement that you accepted before downloading this file.

  This source forms part of the Software and can be used for educational,
  training, and demonstration purposes but cannot be used for derivative
  works except in cases where the derivative works require OVP technology
  to run.

  For open source models released under licenses that you can use for
  derivative works, please visit www.ovpworld.org or www.imperas.com
  for the location of the open source models.


###############################################################################
# SystemC TLM2 Platform
###############################################################################

The platform.cpp file found in this directory was generated from the platform.tcl
file found in the parent directory. 

###############################################################################
# Re-Generating Platform
###############################################################################

> igen.exe --batch ../platform.tcl --writetlm platform.cpp

###############################################################################
# Building the platform
###############################################################################

The simulation platform is compiled using MinGW/MSYS under Windows, and GCC 
under Linux.

The compilation can be performed using the following command line

> make -f IMPERAS_HOME/ImperasLib/buildutils/Makefile.TLM.platform \
          PLATFORM=platform 

The common Makefiles include environment information for the location of the 
SYSTEMC installation.

These must be correctly modified for your installation

    For example:

    # MinGW WINDOWS Configuration
    
    # Set this to your System C installation
    SYSTEMC  = C:\SystemC\Windows\systemc-2.3.0\lib-mingw
    
    # Linux Configuration
    
    # Set this to your SystemC installation
    SYSTEMC  = /home/SystemC/Linux/systemc-2.3.0

