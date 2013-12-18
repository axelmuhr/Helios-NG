#-----------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# ARM Processor specific *DEFAULT* make rules.
#
# File: /HSRC/makeinc/ARM.mak
#
# This file contains definitions of variables and rules with are
# specific to making the ARM processor version of Helios.
#
# SccsId: %W% %G%
# RcsId: $Id: ARM.mak,v 1.16 1993/08/12 08:16:44 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifeq ($(HLICENCEE), ABC)
 # Define SERIALLINK to build system with serial line as link 1
 SERIALLINK = 1
endif

# Default variables:

# Default #defines
# (These should already be produced automatically by your C compiler)
# NCFLAGS		:= $(NCFLAGS) -D__HELIOS -D__HELIOS$(HPROC) -D__$(HPROC)

# Compile time options for Helios:
#
# __RRD		Robust ram disk (reset proof).
# __MI		Memory indirection functions for relocatable memory support.
# __CARD	Support for insertable memory cards.
# __SMT		Split module table version.

ifeq ($(HPLATFORM), ABC)
 NCFLAGS := $(NCFLAGS) -D__RRD -D__MI -D__CARD -D__SMT -D__ABC
else
 ifeq ($(HPLATFORM), ARCHIMEDES)
  # defined for initial archimedes port:
  NCFLAGS := $(NCFLAGS) -D__SMT -D__ARCHIMEDES -D__IOC
 else
  ifeq ($(HPLATFORM), VY86PID)
   # defined for initial archimedes port:
   NCFLAGS := $(NCFLAGS) -D__SMT -D__VY86PID
  endif
 endif
endif

# -Ff  - do not insert function names into generated code

ifdef SYSDEB
 NCFLAGS := $(NCFLAGS) -DSYSDEB
else
 NCFLAGS := -Ff
endif

# Default compiler flags:
#
# -Zt = tiny address model
# -Zs = *NOT* split module table
# -wn = don't complain about implicit narrowing casts and reduced precision

ifdef RSRC
 NCFLAGS	:= $(NCFLAGS) -I$(RSRC)/$(COMPONENT)/$(HPROC) -I$(RSRC)/$(COMPONENT)
endif

NCFLAGS		:= $(NCFLAGS) -j$(INCLUDE) #-Zt

# Add compiler define for second link using serial line if required
ifdef SERIALLINK
  NCFLAGS	:= $(NCFLAGS) -D__SERIALLINK
endif

# Default ampp flags
#AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.$(HPROC) 1 -i$(MACRO)/
# for hysterical (historical) reasons helios.arm is in lowercase

# SMT.arm should be depreciated to __SMT

ifeq ($(HPLATFORM), ABC)
  AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.arm 1 -dAUTOLIB 1 -dSMT.arm 1 -i$(MACRO)/ -d__MI 1 -d__RRD 1 -d__ABC 1
else
 ifeq ($(HPLATFORM), ARCHIMEDES)
    AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.arm 1 -dAUTOLIB 1 -d__SMT 1 -d__$(HPLATFORM) 1 -d__IOC 1 -dSMT.arm 1 -i$(MACRO)/
 else
    AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.arm 1 -dAUTOLIB 1 -d__$(HPLATFORM) 1 -d__SMT 1 -dSMT.arm 1 -i$(MACRO)/
 endif
endif

# Default assembler flags
ASMFLAGS	:=  $(ASMFLAGS) -d

# Default linker flags
LINKFLAGS	:= $(LINKFLAGS)

#----------------------------------------------------------------------------- 
# Default rules:

.SUFFIXES:
.SUFFIXES: .c .a .s .p .o .q .def .i .t

# File extension defaults:
#
# .c	C source file
# .a	AMPP source file
# .s	ASM intermediate text source file
# .o	Conventional object file
# .p	Library object file (no module header or data init code)
# .q	Device Driver object code (no modhdr data/code directives or init (-r))

#----------------------------------------------------------------------------- 
# Rules for compiling C sources

#% : %.c
#	$(CC) $(NCFLAGS) -o $* $<
#	$(NC) $(NCFLAGS) -o $* $<

%.o : %.c
	$(NC) $(NCFLAGS) -c $<

# Library modules
%.p : %.c
	$(NC) -zl $(NCFLAGS) -o $*.p -c $<

# Device Drivers
%.q : %.c
	$(NC) -zr $(NCFLAGS) -o $*.q -c $<

%.s : %.c
	$(NC) $(NCFLAGS) -S $<


#----------------------------------------------------------------------------- 
# Rules for compiling AMPP sources

%.o : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.o $(TEMP)/$*.s

%.p : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.p $(TEMP)/$*.s

%.q : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.q $(TEMP)/$*.s

%.s : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s


#----------------------------------------------------------------------------- 
# Rules for assembling sources
%.o : %.s
	$(ASM) $(ASMFLAGS) -o$*.o $<


#----------------------------------------------------------------------------- 
# Misc rules:

# Making library definition files
define MAKE.DEF
	$(AMPP) -dmake.def 1 $(AMPPFLAGS) $(MACRO)/basic.m $^ > $(TEMP)/def.s
	$(ASM) $(ASMFLAGS) -o$@ $(TEMP)/def.s
endef

# linking libraries
LINK.LIBRARY	=	$(LINK) $(LINKFLAGS) -v

# Linking programs
LINK.CPROGRAM	=	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s4000 -h4000 \
				$(CSTART) $^

# Linking C floating point programs
LINK.FPPROGRAM  =	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s6000 -h4000 \
				$(CSTART) $(LIB)/fplib.def $^

# Ammp -> Object
# Convert first dependency source file into an object file
define AMPPASM
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o$*.o $(TEMP)/$*.s
endef

#------------------------------------------------------------------------------
