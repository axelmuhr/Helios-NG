#--------------------------------------------------------------------> makefile
# Helios generic make system
#------------------------------------------------------------------------------
# 'C40 processor specific make rules for the KERNEL and executive.
#
# File: /HSRC/kernel/C40/makefile
#
# This file contains the extra definitions of variables and rules that are
# required to make a processor specific version of this component.
#
# If you wish to overide a targets rule that has been defined in the generic
# part of the makefile, you should bracket it with "ifneq ($(HPROC),YYY)" and
# supply the new rule in this makefile.
#
# This makefile is always executed within this processor specific subdir.
# All processor specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.19 1993/08/24 10:33:50 tony Exp $: makefile,v 1.1 90/09/25 19:14:37 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# Add processor specific targets:
# 	/tmp/kdebugmap - complete kernel debug objdump disassembly
XTARGETS = c40hboot.i c40ah1boot.i c40ah2boot.i c40sboot.i c40rombt.i c40flashbt.i \
	   c40test.i

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(CP) $(XTARGETS) $(LIB)
	$(TOUCH) Xinstall

# Remove processor specific files other than targets and objects here:
Xclean:
		-$(RM) *.o *.p \
			mkout core emacs.bak install Xinstall

#-----------------------------------------------------------------------------
# All extra processor specific sources

XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p) $(XSOURCES.a:.a=.s)

XOBJECTS.a	= $(XSOURCES.a:.a=.p)

XOBJECTS.c	= $(XSOURCES.c:.c=.p)

# Fill in processor specific sources here:
XSOURCES.c	= c40dma.c stack_c.c c40specials.c

XSOURCES.a	= c40exec.a c40intr.a c40slice.a c40nccsupp.a c40boot.a \
		  c40hboot.a c40ah1boot.a c40ah2boot.a c40sboot.a \
		  c40linkio.a stack_a.a c40rombt.a c40flashbt.a \
		  c40test.a

XHEADERS.c	= c40exec.h

XHEADERS.m	= c40exec.m c40config.m c40idrom.m

XSOURCES.odd	= makefile c40exec.doc csliceintr.c \
		kernel.d porting.tex	# last is tmp


#-----------------------------------------------------------------------------
# Processor specific rules and generic rule overides go here:

# Start and completion messages:
inmess:		; @echo "                                 [[[ Making the Helios 'C40 Executive images ]]]"
outmess:	; @echo "                                 [[[ Executive images constructed ]]]"


# Overide normal -zl library
%.p : %.c
	$(NC) -zr $(NCFLAGS) -o $*.p -c $<


$(XOBJECTS.a): $(XHEADERS.m)

$(XOBJECTS.c): $(XHEADERS.c)

# Board specific bootstraps:

# Hema bootstrap.
c40hboot.i: c40hboot.p
	$(LINK) $< -o $@

# Ariel Hydra I VME bootstrap.
c40ah1boot.i: c40ah1boot.p
	$(LINK) $< -o $@

# Ariel Hydra II VME bootstrap.
c40ah2boot.i: c40ah2boot.p
	$(LINK) $< -o $@

# Sonitec PC bootstrap.
c40sboot.i: c40sboot.p
	$(LINK) c40sboot.p -o $@

# ROM bootstrap.
# -i disables the generation of an image file header
c40rombt.i: c40rombt.p
	$(LINK) -i $< -o $@

c40flashbt.i: c40flashbt.p
	$(LINK) -i $< -o $@

c40test.i: c40test.p
	$(LINK) -i $< -o $@

# end of kernel/C40/makefile
