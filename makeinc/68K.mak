#-----------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# 68K Processor specific *DEFAULT* make rules.
#
# File: /HSRC/makeinc/68K.mak
#
# This file contains definitions of variables and rules with are
# specific to making the 68K processor version of Helios.
#
# SccsId: %W% %G%
# RcsId: $Id: 68K.mak,v 1.1 1993/06/21 08:40:09 nickc Exp $
#		(C) Copyright 1993 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

# Default variables:

# Default #defines
# (These should already be produced automatically by your C compiler)
# NCFLAGS := -D__HELIOS -D__HELIOS$(HPROC) -D__$(HPROC)

#NCFLAGS := $(NCFLAGS) -D__SMT 
NCFLAGS := $(NCFLAGS)

ifdef SYSDEB
 NCFLAGS := $(NCFLAGS) -DSYSDEB
else
 NCFLAGS := $(NCFLAGS) -Ff
endif

# Default compiler flags:
#

ifdef RSRC
 NCFLAGS	:= $(NCFLAGS) -I$(RSRC)/$(COMPONENT)/$(HPROC) -I$(RSRC)/$(COMPONENT)
endif

NCFLAGS		:= $(NCFLAGS) -j$(INCLUDE) $(DEFINES)

# Default C Preprocessor flags
CPPFLAGS	:= $(CPPFLAGS)

# Default ampp flags
#AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.$(HPROC) 1 -dAUTOLIB 1 -d__SMT 1 -i$(MACRO)/
AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.$(HPROC) 1 -dAUTOLIB 1  -i$(MACRO)/

ifdef SYSDEB
 AMPPFLAGS	:= $(AMPPFLAGS) -dSYSDEB 1
endif

# Default assembler flags (-d = disable auto generation of start/end module)
ASMFLAGS	:= $(ASMFLAGS) -d

# Default linker flags
LINKFLAGS	:= $(LINKFLAGS)

#----------------------------------------------------------------------------- 
# Default rules:

.SUFFIXES:
.SUFFIXES: .c .a .s .p .q .o .def .i .t

# File extension defaults:
#
# .c	C source file
# .a	AMPP source file
# .s	ASM intermediate text source file
# .o	Conventional object file
# .p	Library object file (no module header or data init code (-Zl))
# .q	Device Driver object code (no modhdr data/code directives or init (-r))

#----------------------------------------------------------------------------- 
# Rules for compiling C sources

%.o : %.c
	$(NC) $(NCFLAGS) -c $<

# Library modules
%.p : %.c
	$(NC) -zl $(NCFLAGS) -o $*.p -c $<

# Device drivers
%.q : %.c
	$(NC) -zr $(NCFLAGS) -o $*.q -c $<

%.s : %.c
	$(NC) $(NCFLAGS) -S $<


#----------------------------------------------------------------------------- 
# Rules for compiling AMPP sources

%.o : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s
	$(ASM) $(ASMFLAGS) -o $*.o $*.s

%.p : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s
	$(ASM) $(ASMFLAGS) -o $*.p $*.s

%.q : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s
	$(ASM) $(ASMFLAGS) -o $*.q $*.s

%.s : %.a
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s


#----------------------------------------------------------------------------- 
# Rules for assembling sources

%.o : %.s
	$(ASM) $(ASMFLAGS) -o $*.o $<


#----------------------------------------------------------------------------- 
# Misc rules:

# Making library definition files
define MAKE.DEF
	$(AMPP) -dmake.def 1 $(AMPPFLAGS) $(MACRO)/basic.m $^ > $*.s
	$(ASM) $(ASMFLAGS) -o $@ $*.s
	$(RMTEMP) $*.s
endef
# 	$(AMPP) -dmake.def 1 $(AMPPFLAGS) $(MACRO)/basic.m $^ \
	$(ASM) $(ASMFLAGS) -o $@

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
	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< > $*.s
	$(ASM) $(ASMFLAGS) -o $*.o $*.s
	$(RMTEMP) $*.s
endef
#	$(AMPP) $(AMPPFLAGS) $(MACRO)/basic.m $< | $(ASM) $(ASMFLAGS) -o $*.o

#------------------------------------------------------------------------------





