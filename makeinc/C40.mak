#-----------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC MAKEFILE -
#-----------------------------------------------------------------------------
# 'C40 Processor specific *DEFAULT* make rules.
#
# File: /HSRC/makeinc/C40.mak
#
# This file contains definitions of variables and rules with are
# specific to making the 'C40 processor version of Helios.
#
# SccsId: %W% %G%
# RcsId: $Id: C40.mak,v 1.12 1993/08/04 14:42:44 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

# Default variables:

# Default #defines
# (These should already be produced automatically by your C compiler)
# NCFLAGS := -D__HELIOS -D__HELIOS$(HPROC) -D__$(HPROC)

# to turn off peepholeing use -Zpn1,
# -Zpl1 = few modules ( < 256) used in progs
# -Zpw0 = less than 256K of data used per module
# -Fao  = report used before set variables, and old style K&R fns.
# -Ff   = do not store function names in the code
# -Zps1 = disables stack checking

NCFLAGS := $(NCFLAGS) -D__SMT -Zpl1 -Fao

ifdef SYSDEB
 NCFLAGS := $(NCFLAGS) -DSYSDEB
else
 NCFLAGS := $(NCFLAGS) -Ff
endif

# Default compiler flags:
#
# @@@ assume default SMT
# -wn = don't complain about implicit narrowing casts and reduced precision
# @@@ check what special flags might exist

ifdef RSRC
 NCFLAGS	:= $(NCFLAGS) -I$(RSRC)/$(COMPONENT)/$(HPROC) -I$(RSRC)/$(COMPONENT)
endif

NCFLAGS		:= $(NCFLAGS) -j$(INCLUDE) $(DEFINES)

# Default C Preprocessor flags
CPPFLAGS	:= $(CPPFLAGS)

# Default ampp flags
AMPPFLAGS	:= $(AMPPFLAGS) -dhelios.$(HPROC) 1 -dAUTOLIB 1 -d__SMT 1 -i$(MACRO)/

# if using new (450 series) C40 C compiler then add this to AMPPFLAGS:   -dPCS_3 1

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





