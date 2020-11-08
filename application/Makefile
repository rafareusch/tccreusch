IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")
include $(IMPERAS_HOME)/bin/Makefile.include

ifndef IMPERAS_HOME
  IMPERAS_ERROR := $(error "IMPERAS_HOME not defined")
endif

CROSS?=ARM_CORTEX_A8
-include $(IMPERAS_HOME)/lib/$(IMPERAS_ARCH)/CrossCompiler/$(CROSS).makefile.include
ifeq ($($(CROSS)_CC),)
    IMPERAS_ERROR := $(error "Please install the $(CROSS)_CC toolchain")
endif


ELF=secureApplication.$(CROSS).elf  nonSecureApplication1.$(CROSS).elf  nonSecureApplication2.$(CROSS).elf
all: $(ELF)

OPTIMISATION?=-O0 -g 

## NS 1
secureApplication.$(CROSS).elf : secureApplication.o arm7_support.o arm7_support_asm.o sha256.o tweetnacl.o randombytes.o
	$(V) echo "Linking Application $@"
	$(V) $($(CROSS)_LINK) $(OPTIMISATION) -o $@  $^ $(IMPERAS_LDFLAGS)

secureApplication.o: secureApplication.c arm7_support.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<


arm7_support.o: arm7_support.c arm7_support.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<

tweetnacl.o: tweetnacl.c tweetnacl.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<

randombytes.o: randombytes.c randombytes.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<

sha256.o: sha256.c sha256.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<

arm7_support_asm.o: arm7_support_asm.s
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<


.INTERMEDIATE:secureApplication.o arm7_support.o arm7_support_asm.o 


## RNS 1
nonSecureApplication1.$(CROSS).elf: nonSecureApplication1.o sha256.o tweetnacl.o randombytes.o
	$(V) echo "Linking Application $@"
	$(V) $($(CROSS)_LINK) $(OPTIMISATION) -o $@  $^ $(IMPERAS_LDFLAGS)

nonSecureApplication1.o: nonSecureApplication1.c arm7_support.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<




## RNS 2
nonSecureApplication2.$(CROSS).elf: nonSecureApplication2.o sha256.o tweetnacl.o randombytes.o
	$(V) echo "Linking Application $@"
	$(V) $($(CROSS)_LINK) $(OPTIMISATION) -o $@  $^ $(IMPERAS_LDFLAGS)

nonSecureApplication2.o: nonSecureApplication2.c arm7_support.h
	$(V) echo "Compiling Application $@"
	$(V) $($(CROSS)_CC)  $(OPTIMISATION) -c -o $@  $<




clean:
	$(V) - rm -f $(ELF) *.o

