#--------------------------------------------------------------------> makefile
# Helios generic make system
#------------------------------------------------------------------------------
# ARM processor specific make rules for the KERNEL component.
#
# File: /HSRC/kernel/arm/makefile
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
# RcsId: $Id: makefile,v 1.3 1991/04/12 12:40:35 paul Exp $: makefile,v 1.1 90/09/25 19:14:37 paul Exp $
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
# XTARGETS =
Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
#	$(TOUCH) Xinstall

# Remove processor specific files other than targets and objects here:
Xclean:
		-$(RM) imageAB1 imageABFP imageHEVAL \
                       execAB1 execABFP execHEVAL \
		       *.hex items/* *.p mkout session core deleteme* \
		       install Xinstall

#-----------------------------------------------------------------------------
# Extra targets for a ROM based system
# Create complete ROM image including ROM disk
# This is only called after all other Helios targets have been made.

ROMTARGETS = inmess imageAB1 imageABFP imageHEVAL outmess

romsys: $(ROMTARGETS)

	
#-----------------------------------------------------------------------------
# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)
# Or if you are not a library:
#XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	= memory1.c # temporary - should be put back into main sources

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile $(HIEXECSRC) $(EXECSRC) $(LOCALEXECHDRS) $(FPESRC) \
		  hdrAB1.s hdrABFP.s hdrHEVAL.s asm.s bytblt.s kernel.d \
		  fpehdr.s fpeend.s loend.s hdrFPE.s

#-----------------------------------------------------------------------------
# Processor specific rules and generic rule overides go here:

# Start and completion messages:
inmess:		; @echo "                                 [[[ Making the Helios ARM Executive images ]]]"
outmess:	; @echo "                                 [[[ Executive images constructed ]]]"

# -----------------------------------------------------------------------------
# Construct the various Helios ARM Executives.
# The Helios system communicates with these images via SWI instructions.
# ----------------------------------------------------------------------------
# -- extra contruction commands ----------------------------------------------

ARMHAS	  = hobjasm	# ARM Helios assembler
BROM	  = brom	# ROM building utility
SYSSTRIP  = sysstrip	# Helios image header stripping utility

ASINCLUDE = ${INCLUDE}/abcARM/asm
HLIB	  = ${HPROD}/lib
KERNELDEF = ${HLIB}/kernel.def

# -----------------------------------------------------------------------------
# -- Executive Header files ---------------------------------------------------

# The "HDR"s are now to be found in the "{ASINCLUDE}" directory
HOBJHDRS =	${ASINCLUDE}/listopts.s  ${ASINCLUDE}/basic.s \
		${ASINCLUDE}/arm.s       ${ASINCLUDE}/hardAB1.s \
		${ASINCLUDE}/hardABFP.s  ${ASINCLUDE}/hardHFP.s \
		${ASINCLUDE}/exmacros.s  ${ASINCLUDE}/structs.s \
		${ASINCLUDE}/microlink.s ${ASINCLUDE}/exstruct.s \
		${ASINCLUDE}/pdstruct.s  ${ASINCLUDE}/SWI.s \
		${ASINCLUDE}/ROMitems.s  ${ASINCLUDE}/manifest.s \
		${ASINCLUDE}/PCcard.s    ${ASINCLUDE}/module.s \
		${ASINCLUDE}/error.s

LOCALEXECHDRS = fixes.s execwork.s simstate.s

EXECHDRS =	${LOCALEXECHDRS} ${HOBJHDRS}

# -----------------------------------------------------------------------------
# -- Executive source files ---------------------------------------------------

EXECSRC =	loexec.s lomon.s loint.s lomlink.s locard.s loswi1.s loswi2.s \
		loswi3.s loswi4.s loswi5.s loswidbg.s losched.s lodebug.s \
		loend.s

# -----------------------------------------------------------------------------
# -- Hi-Executive source files ------------------------------------------------

HIEXECSRC =	hiexec.s hiproc.s hilinktx.s

# -----------------------------------------------------------------------------
# -- Floating Point Emulator source -------------------------------------------

FPESRC =	fpehtop.s fpemid.s fpeutils.s fpecpdt.s fpecprt.s \
		fpecpst.s fpecpdom.s fpecpdod.s fpelogs.s fpetrig.s

# -----------------------------------------------------------------------------
# -- Executive images ---------------------------------------------------------

execAB1:	hdrAB1.s ${EXECSRC} ${EXECHDRS}
		buildtime
		${ARMHAS} $< deleteme1 -hdr ${ASINCLUDE}
		${LINK} -odeleteme2 deleteme1
		${SYSSTRIP} -r deleteme2 -o$@
		rm deleteme1 deleteme2

execABFP:	hdrABFP.s ${EXECSRC} ${EXECHDRS}
		buildtime
		${ARMHAS} $< deleteme1 -hdr ${ASINCLUDE}
		${LINK} -odeleteme2 deleteme1
		${SYSSTRIP} -r deleteme2 -o$@
		rm deleteme1 deleteme2

execHEVAL:	hdrHEVAL.s ${EXECSRC} ${EXECHDRS}
		buildtime
		${ARMHAS} $< deleteme1 -hdr ${ASINCLUDE}
		${LINK} -odeleteme2 deleteme1
		${SYSSTRIP} -r deleteme2 -o$@
		rm deleteme1 deleteme2

# -----------------------------------------------------------------------------
# -- ROM included binaries ----------------------------------------------------
# These are performed as individual dependencies rather than a concoction of
# VPATH and string manipulations on a large dependency (since we do not want
# to remake all the ROM ITEMs when only a single file has changed).

# VPATH is used to define where we find the relevant source object
VPATH := $(VPATH):${LIB}:${ETC}:${BIN}:${BIN}/private:${HPROD}:${HPROD}/users:\
	${HPROD}/include:${HPROD}/include/sys


# SPECOBJS : must be present in every ROM image
SPECOBJS = items/fpe
# STDOBJECTS : must be present in every ROM image
STDOBJECTS = items/kernel items/syslib items/servlib items/util \
		items/ABClib items/Posix items/Clib items/Fault items/FpLib \
		items/patchlib items/procman items/loader items/window \
		items/rom items/ram items/null items/helios items/keyboard \
		items/config

# INITOBJS
#INITOBJS = items/init items/fifo items/pipe items/serial items/serialdev.dev \
#		items/cshrc items/loginrc
INITOBJS = items/init items/cshrc items/loginrc items/fifo items/pipe \
		items/inkey44.bkm

# ETCOBJS
ETCOBJS = items/emacs.hlp items/faults items/preinitrc items/initrc items/motd

# BINOBJS
BINOBJS = items/toshell items/shell

ROMOBJS = $(SPECOBJS) $(STDOBJECTS) $(INITOBJS) $(ETCOBJS) $(BINOBJS)

# -----------------------------------------------------------------------------
# Configuration information for ROM based systems
items/config:
	buildconfig
	bromitem -a -o $@ -nhelios/etc/config -v0100 ROMconfig
	rm ROMconfig

# Nucleus objects -------------------------------------------------------------

items/kernel:	kernel
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/syslib:	syslib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/servlib:	servlib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/util:	util
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/ABClib:	ABClib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/Posix:	Posix
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/Clib:	Clib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/Fault:	Fault
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/FpLib:	FpLib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/patchlib:	patchlib
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/procman:	procman
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/loader:	loader
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/window:	window
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/keyboard:	keyboard
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/rom:	rom
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/ram:	ram
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/null:	null
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/helios:	helios
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

# initialisation and servers --------------------------------------------------

items/init:	init
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/fifo:	fifo
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/pipe:	pipe
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/serial:	serial
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/serialdev.dev:	serialdev.dev
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/inkey44.bkm:	inkey44.bkm
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/exkey44.bkm:	exkey44.bkm
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/cshrc:	cshrc
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/loginrc:	loginrc
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

# "/etc" files ----------------------------------------------------------------

items/emacs.hlp:	emacs.hlp
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/faults:	faults
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/preinitrc:	preinitrc
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/initrc:	initrc
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/motd:	motd
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

# Default binaries ------------------------------------------------------------

items/shell:	shell
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

items/toshell:	toshell
	bromitem -a -o$@ -nhelios/$(subst $(HPROD)/,,$^) -v0100 $^

# -----------------------------------------------------------------------------
# -- Active Book image --------------------------------------------------------

imageAB1:	execAB1 ${ROMOBJS}
		@echo "					[[[ Making \"imageAB1\" ]]]"
		@${BROM} -o $@ -c80000 -s $<
		@${BROM} -o $@ -a ${ROMOBJS}
		@romsplit -p80000 $@
		@mv $@0 $@0.hex
		@mv $@1 $@1.hex
		@mv $@2 $@2.hex
		@mv $@3 $@3.hex
		@echo "                                 [[[ \"imageAB1\" constructed ]]]"

# -----------------------------------------------------------------------------
# -- Functional Prototype image -----------------------------------------------

imageABFP:	execABFP ${ROMOBJS}
		@echo "					[[[ Making \"imageABFP\" ]]]"
		@${BROM} -o $@ -c80000 -s $<
		@${BROM} -o $@ -a ${ROMOBJS}
		@romsplit -p80000 $@
		@mv $@0 $@0.hex
		@mv $@1 $@1.hex
		@mv $@2 $@2.hex
		@mv $@3 $@3.hex
		@echo "                                 [[[ \"imageABFP\" constructed ]]]"

# -----------------------------------------------------------------------------
# -- HEVAL prototype image ----------------------------------------------------

imageHEVAL:	execHEVAL ${ROMOBJS}
		@echo "					[[[ Making \"imageHEVAL\" ]]]"
		@${BROM} -o $@ -c80000 -s $<
		@${BROM} -o $@ -a ${ROMOBJS}
		@romsplit -p80000 $@
		@mv $@0 $@0.hex
		@mv $@1 $@1.hex
		@mv $@2 $@2.hex
		@mv $@3 $@3.hex
		@echo "                                 [[[ \"imageHEVAL\" constructed ]]]"

# -----------------------------------------------------------------------------
# -- FPE item -----------------------------------------------------------------

items/fpe:	hdrFPE.s ${FPESRC} ${EXECHDRS}
		@echo "					[[[ Making \"FPE\" item ]]]"
		@${ARMHAS} $< deleteme1 -hdr ${ASINCLUDE}
		@${LINK} -odeleteme2 ${KERNELDEF} deleteme1
		@${SYSSTRIP} -r deleteme2 -o$@
		@rm deleteme1 deleteme2

# -----------------------------------------------------------------------------
# -- Hi-Executive -------------------------------------------------------------

hiexec.p:	${HIEXECSRC} ${EXECHDRS}
		@echo "					[[[ Making \"HiExecutive\" ]]]"
		${ARMHAS} hiexec.s $@ -library -hdr ${ASINCLUDE}

# -----------------------------------------------------------------------------
# -- Kernel assembler support -------------------------------------------------

# This "asm.s" dependancy line is a hack to stop the make system converting
# the transputer "asm.a" into an "asm.s" file in this directory (hence
# overwriting our real "asm.s" source file).
asm.s:
		@echo "**** THIS SHOULD NOT BE REMADE (makefile ERROR) ****"

asm.p:		asm.s
		@echo "                                 [[[ Making \"asm.p\" ]]]"
		${ARMHAS} asm.s $@ -library -hdr ${ASINCLUDE}

#------------------------------------------------------------------------------
