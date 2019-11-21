IMPERAS_HOME := $(shell getpath.exe "$(IMPERAS_HOME)")

# always generate locally
NOVLNV?=1

include $(IMPERAS_HOME)/ImperasLib/buildutils/Makefile.platform

realclean: clean
	rm -f platform.c
	rm -f platform.c.igen.stubs
	rm -f *.igen.h
