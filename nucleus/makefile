#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Nucleus
#
# File: /HSRC/nucleus/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.40 1993/08/06 12:43:12 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then we get local defaults from
# ../makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../makeinc/DFLT.mak
endif

.PHONY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = nucleus

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean syslib.i syslib.def
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "installdef" - If component is a library, make .def file and install in LIB
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.c $(HVPATH)
vpath %.h $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS := servlib.def sstart.o s0.o procman.i loader.i servlib.i \
		SysLib newinit.i ram.i fifo.i null.i pipe.i alias.i

ifeq ($(HPROC),TRAN)
  TARGETS := $(TARGETS) nucleus syslib.sa nucleus.srv #rom.i
  ifdef SYSDEB
    TARGETS := $(TARGETS) nucleus.fs nucleus.ufs 
  endif
endif

ifeq ($(HPROC),ARM)
  TARGETS := $(TARGETS) nucleus
endif

ifeq ($(HPROC),C40)
  TARGETS := $(TARGETS) nucleus nucleus.sml
endif

default : $(TARGETS) Xdefault

install: installdef $(TARGETS) Xinstall
	$(CP) nucleus $(LIB)
	$(CP) ram.i $(LIB)/ram
	$(CP) null.i $(LIB)/null
ifeq ($(HPROC),TRAN)
	$(CP) nucleus.srv $(LIB)
 ifdef SYSDEB
	$(CP) nucleus.fs $(LIB)
	$(CP) nucleus.ufs $(LIB)
 endif
	$(CP) syslib.sa $(LIB)
endif
ifeq ($(HPROC),C40)
	$(CP) nucleus.sml $(LIB)
endif
	$(CP) fifo.i $(LIB)/fifo
	$(CP) pipe.i $(LIB)/pipe
	$(CP) newinit.i $(LIB)/init
	$(CP) alias.i $(LIB)/alias
ifeq ($(HLICENSEE),ABC)		# depreciated functionality - delete me soon
# assumes "lib/kernel is copied by kernel makefile
	sysstrip ../syslib/$(HPROC)/syslib.i -o$(LIB)/syslib
	sysstrip servlib.i -o$(LIB)/servlib
# assumes "lib/util" is copied by relevant "util" makefile
# assumes libraries are copied by relevant makefiles
	sysstrip procman.i -o$(LIB)/procman
	sysstrip loader.i -o$(LIB)/loader
	sysstrip window.i -o$(LIB)/window
	sysstrip rom.i -o$(LIB)/rom
	sysstrip ram.i -o$(LIB)/ram
	sysstrip null.i -o$(LIB)/null
	sysstrip helios.i -o$(LIB)/helios
endif
	$(TOUCH) install

installdef: $(LIB)/syslib.def $(LIB)/servlib.def $(LIB)/s0.o $(LIB)/sstart.o
	$(TOUCH) installdef

$(LIB)/syslib.def: ../syslib/$(HPROC)/syslib.def 
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

$(LIB)/servlib.def: servlib.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

# server startup code
$(LIB)/s0.o: s0.o
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp s0.o $(LIB)/s0.o; if ( $$status ) cp s0.o $(LIB) )
else
	@ if cmp s0.o $(LIB)/s0.o; then echo "s0.o not changed"; else $(CP) s0.o $(LIB) ;fi
endif

$(LIB)/sstart.o: sstart.o
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp sstart.o $(LIB)/sstart.o; if ( $$status ) cp sstart.o $(LIB) )
else
	@ if cmp sstart.o $(LIB)/sstart.o; then echo "sstart.o not changed"; else $(CP) sstart.o $(LIB) ;fi
endif


clean	: Xclean
	-$(RM) $(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall
	$(MAKE) -C ../syslib/$(HPROC) clean

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# Full set of object files - cheat a little - all possibles
OBJECTS		= $(SOURCES.c:.c=.p) $(SOURCES.a:.a=.p) \
		  $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o) \
		  $(SOURCES.c:.c=.si) $(SOURCES.c:.c=.sa) \
		  $(SOURCES.c:.c=.dl)

SOURCES.c 	=	alias.c fifo.c null.c pipe.c ram.c 		\
			newinit.c					\
			loader.c procman.c servlib.c			\
			sasyslib.c

# AMPP source files
SOURCES.a 	= 	modend.a 	\
			servstar.a 	\
			sasysasm.a	\
 			sstart.a

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile oldinit.c


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
# Possibles nucleus flags are: -DPMDEBUG -DPROTECTED -DSTACKCHECK -DSTACKEXTENSION

NCFLAGS := $(NCFLAGS) -DPROTECTED

ifdef SYSDEB
  NCFLAGS := $(NCFLAGS) -DPMDEBUG
endif

ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -DSTACKCHECK
else
  ifeq ($(HPROC),C40)
# -Zpw1 = static data < 64k optimisation
    NCFLAGS := $(NCFLAGS) -Zpw1 -DSTACKEXTENSION -DSTACKCHECK
  else
    ifeq ($(HPROC),TRAN)
      NCFLAGS := $(NCFLAGS) -ps1 -t4 -DSTACKCHECK
      LINKFLAGS := $(LINKFLAGS) -f
    endif
  endif
endif

# Note any library def files to be linked with
# in this case for the sstart.o file
LIBDEFS	:= $(LIB)/kernel.def ../syslib/$(HPROC)/syslib.def servlib.def $(LIB)/util.def

#-----------------------------------------------------------------------------
# Extra server compilation rules:

.SUFFIXES: .si .dl .sa

# Servers incorporated into the system image
ifeq ($(HPROC),TRAN)
%.si:%.c
	$(NC) $(NCFLAGS) -DINSYSTEMIMAGE $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.si $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.si:%.c
	$(NC) -o$*.si -c $(NCFLAGS) -DINSYSTEMIMAGE $<
endif

# Demand loaded servers
ifeq ($(HPROC),TRAN)
%.dl:%.c
	$(NC) $(NCFLAGS) -DDEMANDLOADED $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.dl $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.dl:%.c
	$(NC) -o$*.dl -c $(NCFLAGS) -DDEMANDLOADED $<
endif

# Stand alone servers
ifeq ($(HPROC),TRAN)
%.sa:%.c
	$(NC) $(NCFLAGS) -DSTANDALONE $< -s$(TEMP)/$*.s
	$(ASM) $(ASMFLAGS) -o $*.sa $(TEMP)/$*.s
	$(RMTEMP) $(TEMP)/$*.s
else
%.sa:%.c
	$(NC) -o$*.sa -c $(NCFLAGS) -DSTANDALONE $<
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

SSTART = sstart.o

#-----------------------------------------------------------------------------
# Resident Modules
# These should be included in the system image in SLOT order

SYSLIBS 	:= $(KERNEL)/$(HPROC)/kernel.i ../syslib/$(HPROC)/syslib.i servlib.i \
		$(UTIL)/$(HPROC)/util.i 

SYSLIBS.SML 	:= $(KERNEL)/$(HPROC)/kernel.sml ../syslib/$(HPROC)/syslib.i servlib.i \
		$(UTIL)/$(HPROC)/util.i 


ifeq ($(HPROC),TRAN)
SYSLIBS		:= $(SYSLIBS) $(BOOT)
endif

ifeq ($(HPROC),C40)
SYSLIBS		:= $(SYSLIBS) $(BOOT)
SYSLIBS.SML	:= $(SYSLIBS.SML) $(BOOT)
endif

ifeq ($(HPROC),ARM)
#		no extra servers currently, and no bootstrap either
endif


# system tasks

SYSTASKS := procman.i loader.i

# Build the nucleus
nucleus : $(SYSLIBS) $(SYSTASKS)
	$(SYSBUILD) nucleus $(SYSLIBS) $(SYSTASKS)
ifeq ($(HPROC),TRAN)
  ifdef SYSDEB
	$(CP) nucleus /helios/lib/nucleus
  endif
endif

#-----------------------------------------------------------------------------
# Nucleus incorporating Shared Memory Link kernel

ifeq ($(HPROC),C40)
nucleus.sml : $(SYSLIBS.SML) $(SYSTASKS)
	$(SYSBUILD) nucleus.sml $(SYSLIBS.SML) $(SYSTASKS)
endif

#-----------------------------------------------------------------------------
# Nucleus incorporating HELIOS file system

FSFILES = $(ETC)/devinfo $(LIB)/raw.dev $(LIB)/fs rom.i

nucleus.fs : $(SYSLIBS) $(SYSTASKS) $(FSFILES)
	$(SYSBUILD) nucleus.fs $(SYSLIBS) $(SYSTASKS) $(FSFILES)


#-----------------------------------------------------------------------------
# Nucleus incorporating UNIX file system

UFSFILES = $(LIB)/he1000.d $(LIB)/ufs

nucleus.ufs : $(SYSLIBS) $(SYSTASKS) $(UFSFILES)
	$(SYSBUILD) nucleus.ufs $(SYSLIBS) $(SYSTASKS) $(UFSFILES)


#-----------------------------------------------------------------------------
# Nucleus incorporating the servtask

# Build the nucleus
nucleus.srv : $(SYSLIBS) $(SYSTASKS) $(LIB)/servtask.i
	$(SYSBUILD) nucleus.srv $(SYSLIBS) $(SYSTASKS) $(LIB)/servtask.i


#-----------------------------------------------------------------------------
# Nucleus servers

#loader and procman could have 20k stacks
procman.i : $(SSTART) procman.o
ifeq ($(HPROC),ARM)
	$(LINK) $(LINKFLAGS) -nProcMan -s4000 -h50000 $^ -o$@
else
	$(LINK) $(LINKFLAGS) -nProcMan -s2000 -h40000 $^ -o$@
endif

loader.i : $(SSTART) loader.o
ifeq ($(HPROC),ARM)
	$(LINK) $(LINKFLAGS) -nLoader -s4000 -h8000 $^ -o$@
else
	$(LINK) $(LINKFLAGS) -nLoader -s2000 -h10000 $^ -o$@
endif

newinit.i : $(SSTART) newinit.o
	$(LINK) $(LINKFLAGS) -nInit -s2000 -h6000 $^ -o$@


#-----------------------------------------------------------------------------
# Defunct initialisation code.
# Procman now runs init (newinit.c) that loads initrc
# (Source now called oldinit.c)
#init.i : $(SSTART) init.o
#	$(LINK) $(LINKFLAGS) -nInit -s2000 -h6000 $^ -oinit.i

#initnet.i : init.c $(SSTART)
#	$(NC) $(NCFLAGS) -DNETWORK $< | $(ASM) -p -oinitnet.o
#	$(LINK) $(LINKFLAGS) -nInit -s2000 -h6000 $(SSTART) initnet.o -oinitnet.i


#-----------------------------------------------------------------------------
# General servers
#
# heap size set to < 2*buffer size so buffers are allocated from system &
# returned when not used.
#
# *.si = Server is incorporated into system image 
# *.dl = Demand loaded server - Name table entry already exists
# *.sa = Stand alone servers (run from shell)

# demand loaded or in system image servers

ram.i :	$(SSTART) ram.dl
	$(LINK) $(LINKFLAGS) -nRam -s2000 -h2000 $^ -o$@

null.i : $(SSTART) null.dl
	$(LINK) $(LINKFLAGS) -nNull -s2000 -h2000 $^ -o$@

ifeq ($(HPROC),TRAN)
# Standard rom server
  rom.i :	$(SSTART) rom.si files.si mkrom
	$(LINK) $(LINKFLAGS) -nRom -s2000 -h5000 $(SSTART) rom.si files.si -o$@
	mkrom rom.i -a /helios/etc/devinfo

mkrom : mkrom.c
	$(HOSTCC) mkrom.c -o mkrom
endif


#-----------------------------------------------------------------------------
# demand loaded servers - nametable entry created by procman
# *.dl = Demand loaded server

fifo.i :	$(SSTART) fifo.dl
	$(LINK) $(LINKFLAGS) -nFifo -s2000 -h2000 $^ -o$@

pipe.i :	$(SSTART) pipe.dl
	$(LINK) $(LINKFLAGS) -nPipe -s2000 -h2000 $^ -o$@


#-----------------------------------------------------------------------------
# standalone servers - run from the shell

alias.i :	$(SSTART) alias.sa
	$(LINK) $(LINKFLAGS) -nAlias -s1000 -h2500 $^ -o$@

ptest.i :	$(SSTART) ptest.sa
	$(LINK) $(LINKFLAGS) -nptest -s2000 -h4000 $^ -o$@


#-----------------------------------------------------------------------------
# System Library	

SysLib:
		$(MAKE) -C ../syslib/$(HPROC)

../syslib/$(HPROC)/syslib.def: ../syslib/sysstart.a
		$(MAKE) -C ../syslib/$(HPROC) syslib.def

#-----------------------------------------------------------------------------
# Server Library

servlib.i:	servstar.p servlib.p modend.p \
		$(LIB)/kernel.def ../syslib/$(HPROC)/syslib.def $(LIB)/util.def
	$(LINK.LIBRARY) -nServLib $^ -o$@

servlib.def:	servstar.a
	$(MAKE.DEF)


#-----------------------------------------------------------------------------
# Server startup code for all servers

ifeq ($(HPROC),TRAN)
# s0.o is server startup object code. sstart.o holds s0.o + lib resrefs
sstart.o : sstart.a $(LIBDEFS)
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/x -i$(MACRO)/ $(MACRO)/basic.m $< 
	$(ASM) -p $(TEMP)/x $(LIBDEFS) -o$@

s0.o 	: sstart.a
	$(AMPP) $(AMPPFLAGS) -o$(TEMP)/x -i$(MACRO)/ $(MACRO)/basic.m $< 
	$(ASM) -p $(TEMP)/x -o$@

else

s0.o : sstart.a
	$(AMPPASM)

sstart.o : s0.o $(LIBDEFS)
	cat $^ > sstart.o

endif


#-----------------------------------------------------------------------------
# Stand-Alone SysLib stub
#

syslib.sa: ../syslib/TRAN/sysstart.p sasysasm.p sasyslib.p $(LIB)/kernel.def $(LIB)/util.def modend.p
	$(LINK.LIBRARY) -nSysLib ../syslib/TRAN/sysstart.p sasysasm.p sasyslib.p modend.p \
	$(LIB)/kernel.def $(LIB)/util.def -o$@
	$(CP) $@ $(LIB)

memtst	: $(CSTART) memtst.o sysmem.o
	$(LINK.CPROGRAM)


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
