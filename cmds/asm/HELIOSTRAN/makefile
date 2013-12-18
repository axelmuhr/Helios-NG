#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Transputer Assembler/Linker
#
# File: /HSRC/cmds/asm/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.6 1991/04/16 16:11:59 martyn Exp $
#		(C) Copyright 1990 Perihelion Software
#
# $Header: /hsrc/cmds/asm/HELIOSTRAN/RCS/makefile,v 1.6 1991/04/16 16:11:59 martyn Exp $
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/asm

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
# "default" - should make this component in its proc subdir
# "install" - copy the new component to $(HPROD) production directory
# "clean"   - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
VPATH := $(HSRC)/$(COMPONENT):../../support

# Note all final generic targets here
TARGETS := asm

default : $(TARGETS)

install: $(TARGETS)
	$(CP) asm $(HHOSTBIN)
	$(TOUCH) install

clean	:
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		keytab.c keyform core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# C source files
SOURCES.c	= asm.c gencode.c genimage.c gencsc.c growcode.c \
                   keyform.c objed.c lex.c mem.c module.c preasm.c \
                   sym.c syn.c encode.c vm.c growdata.c objdump.c 
#  keytab.c is automatically generated

OBJECTS		= asm.o gencode.o genimage.o growdata.o growcode.o keytab.o \
		objed.o lex.o encode.o mem.o module.o preasm.o sym.o syn.o \
		vm.o

# AMPP source files
SOURCES.a	= 

# C header files
HEADERS.c	= asm.h vm.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

NCFLAGS := -t4 -DT800 -DNORCROFT -DVM -D__HELIOS -D__TRAN -D__HELIOSTRAN
#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

%.o: %.c
	cc -wadsv -i,../,../../support/,/helios/include/ -j,../,../../support/,/helios/include/ $(NCFLAGS) $< -s$(TEMP)/$*.s
	$(ASM) -p -o $@ $(TEMP)/$*.s
	-$(RMTEMP) $(TEMP)/$*.s

keytab.c : keyform
	keyform

keyform : keyform.o
	$(ASM) -v -f /helios/lib/cstart.o keyform.o -nkeyform -okeyform

asm:  $(OBJECTS)
	$(ASM) -v -f -s20000 -h4000 -nasm -mvmfile /helios/lib/cstart.o $(OBJECTS) -oasm
