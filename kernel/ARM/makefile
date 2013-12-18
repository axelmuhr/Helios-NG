#--------------------------------------------------------------------> makefile
# Helios generic make system
#------------------------------------------------------------------------------
# ARM processor specific make rules for the KERNEL and executive.
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
# RcsId: $Id: makefile,v 1.2 1993/08/24 08:41:11 paul Exp $
#
# (C) Copyright 1992 Perihelion Software
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

XTARGETS = #armboot.i #saboot.i

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(TOUCH) Xinstall

# Remove processor specific files other than targets and objects here:
Xclean:
		-$(RM) *.o *.p trapinit.a trapstubs.a trapdefs.m trapdefs.h \
			mkout core emacs.bak install Xinstall

#-----------------------------------------------------------------------------
# All extra processor specific sources

XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)

XOBJECTS.a	= $(XSOURCES.a:.a=.p)

XOBJECTS.c	= $(XSOURCES.c:.c=.p)

# Fill in processor specific sources here:
XSOURCES.c	= romsupp.c armcexec.c hw_archc.c hw_glapc.c

XSOURCES.a	= armexec.a armintr.a armslice.a armnccsupp.a armboot.a \
		  traphand.a armextra.a hw_archa.a hw_glapa.a

XHEADERS.c	= armexec.h hw_glap.h

XHEADERS.m	= armexec.m hw_glap.m

XSOURCES.odd	= makefile armexec.doc csliceintr.c csliceintr.s \
		kernel.d porting.tex	# last is tmp


#-----------------------------------------------------------------------------
# Processor specific rules and generic rule overides go here:

# Start and completion messages:
inmess:		; @echo "                                 [[[ Making the Helios ARM Executive images ]]]"
outmess:	; @echo "                                 [[[ Executive images constructed ]]]"


# Overide normal -zl library
%.p : %.c
	$(NC) -zr $(NCFLAGS) -o $*.p -c $<


$(XOBJECTS.a): $(XHEADERS.m) ../gexec.m

$(XOBJECTS.c): $(XHEADERS.c) ../gexec.h

#saboot.i: armboot.a
#	$(AMPP) $(AMPPFLAGS) -dSTANDALONE 1 $(MACRO)/basic.m $< > saboot.s
#	$(ASM) $(ASMFLAGS) -o $@ saboot.s
#	$(RM) saboot.s
#	$(LINK) $< -o $@

$(INCLUDE)/ARM/trapdefs.h: trapdefs.h
ifeq (HELIOS,$(findstring HELIOS,$(HHOST)))
	(cmp $< $@ ; if ( $$status ) $(CP) $< $@ )
else
	@ if cmp $< $@; then echo "$< not changed"; else $(CP) $< $@; fi
endif

$(INCLUDE)/ampp/ARM/trapdefs.m: trapdefs.m
ifeq (HELIOS,$(findstring HELIOS,$(HHOST)))
	(cmp $< $@ ; if ( $$status ) $(CP) $< $@ )
else
	@ if cmp $< $@; then echo "$< not changed"; else $(CP) $< $@; fi
endif


# end of kernel/ARM/makefile
