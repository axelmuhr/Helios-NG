#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# TRANSPUTER Processor specific *DEFAULT* make rules.
#
# File: /HSRC/makeinc/TRAN.mak
#
# This file contains definitions of variables and rules with are
# specific to making the transputer processor version of Helios.
#
# SccsId: %W% %G%
# RcsId: $Id: TRAN.mak,v 1.8 1992/08/20 08:58:25 paul Exp tony $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------
# Default variables:

# Default #defines
# (These should already be produced automatically by your C compiler)
NCFLAGS := $(NCFLAGS) -D__HELIOS -D__HELIOS$(HPROC) -D__$(HPROC)
ifdef SYSDEB
 NCFLAGS := $(NCFLAGS) -DSYSDEB
endif

# Default compiler flags
ifndef RSRC
 NCFLAGS		:= $(NCFLAGS) -i,../,$(INCLUDE)/ -j$(INCLUDE)/ 
else
 NCFLAGS		:= $(NCFLAGS) -i,$(RSRC)/$(COMPONENT)/$(HPROC)/,../,$(RSRC)/$(COMPONENT)/,$(INCLUDE)/ -j$(INCLUDE)/ 
endif

# Default ampp flags
AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.$(HPROC) 1 -i$(MACRO)/

# Default assembler flags
ASMFLAGS	:=  $(ASMFLAGS) -p

# Default linker flags
LINKFLAGS	:= $(LINKFLAGS)

#----------------------------------------------------------------------------- 
# Default rules:

.SUFFIXES:
.SUFFIXES: .c .a .s .p .q .o .def .i

# File extension defaults:
#
# .c	C source file
# .a	AMPP source file
# .s	ASM intermediate text source file
# .o	Conventional object file
# .p	Library object file (no module header or data init code(-l))
# .q	Device Driver object code (no modhdr data/code directives or init (-r))

#----------------------------------------------------------------------------- 
# Rules for compiling C sources

%.o : %.c
	$(NC) $(NCFLAGS) $< -s$(TEMP)/$(notdir $*.s)
	$(ASM) $(ASMFLAGS) -o $*.o $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

# Library modules
%.p : %.c
	$(NC) -r $(NCFLAGS) $< -s$(TEMP)/$(notdir $*.s)
	$(ASM) $(ASMFLAGS) -o $*.p $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

# Device Drivers
%.q : %.c
	$(NC) -r $(NCFLAGS) $< -s$(TEMP)/$(notdir $*.s)
	$(ASM) $(ASMFLAGS) -o $*.q $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

%.s : %.c
	$(NC) $(NCFLAGS) $< -s$*.s


#----------------------------------------------------------------------------- 
# Rules for compiling AMPP sources

%.o : %.a
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/$(notdir $*.s) $(MACRO)/basic.m $<
	$(ASM) $(ASMFLAGS) -o$*.o $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

%.p : %.a
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/$(notdir $*.s) $(MACRO)/basic.m $<
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

%.q : %.a
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/$(notdir $*.s) $(MACRO)/basic.m $<
	$(ASM) $(ASMFLAGS) -o$*.q $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)

%.s : %.a
	$(AMPP) $(AMPPFLAGS) -o$*.s $(MACRO)/basic.m $<


#----------------------------------------------------------------------------- 
# Rules for assembling sources
%.o : %.s
	$(ASM) $(ASMFLAGS) -o$*.o $<


#----------------------------------------------------------------------------- 
# Misc rules:

# Making library definition files
define MAKE.DEF
	$(AMPP) -dmake.def 1 $(AMPPFLAGS) -o$(TEMP)/def.s $(MACRO)/basic.m $^
	$(ASM) $(ASMFLAGS) -o$@ $(TEMP)/def.s
	-$(RMTEMP) $(TEMP)/def.s
endef

# linking libraries
LINK.LIBRARY	=	$(LINK) -v -f

# Linking programs
LINK.CPROGRAM	=	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 \
				$(LIB)/c0.o $^ \
				-l$(LIB)/helios.lib -l$(LIB)/c.lib

# Linking C floating point programs
LINK.FPPROGRAM  =	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s6000 -h4000 \
				$(LIB)/c0.o $^ \
				-l$(LIB)/helios.lib -l$(LIB)/c.lib

# Ammp -> Object
# Convert first dependency source file into an object file
define AMPPASM
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/$(notdir $*.s) $(MACRO)/basic.m $<
	$(ASM) $(ASMFLAGS) -o$*.o $(TEMP)/$(notdir $*.s)
	-$(RMTEMP) $(TEMP)/$(notdir $*.s)
endef

#------------------------------------------------------------------------------
