#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: C Language Support Library
#
# File: /HSRC/cmds/cc/clib/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.24 1993/12/08 17:32:25 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

  # These variables should already be set by your make script or environment.
  ifndef HPROC
    HPROC = TRAN
  endif

  ifndef HHOST
    HHOST = SUN4
  endif

  ifndef HSRC
    HSRC = /usr/perihelion/Helios
  endif

  ifndef HPROD
    HPROD = $(HSRC)/slashhelios/$(HPROC)
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
COMPONENT = cmds/cc/clib

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONY: default clean Xdefault Xclean
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
HVPATH := $(HSRC)/$(COMPONENT)/$(HPROC):$(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):/$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = clib.def c0.o cstart.o clib.i

default: $(TARGETS) Xdefault

install: installdef $(TARGETS) Xinstall
ifeq ($(HLICENSEE),ABC)
	sysstrip clib.i -o$(LIB)/Clib
else
	$(CP) clib.i $(LIB)/Clib
endif
	$(TOUCH) install


installdef: $(LIB)/clib.def $(LIB)/c0.o $(LIB)/cstart.o
	$(TOUCH) installdef


$(LIB)/clib.def : clib.def
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

$(LIB)/cstart.o : cstart.o
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif

$(LIB)/c0.o : c0.o
ifeq (HELIOS,$(findstring HELIOS,$(HHOST))) #csh default on Helios hosts
	(cmp $^ $@ ; if ( $$status ) cp $^ $@ )
else
	@ if cmp $^ $@; then echo "$^ not changed"; else $(CP) $^ $@ ;fi
endif


clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

# C source files
SOURCES.c	= stdio.c ctype.c string.c printf.c fpprintf.c \
		scanf.c stdlib.c sort.c alloc.c time.c error.c \
		locale.c posixsys.c

# AMPP source files
SOURCES.a	= clibstar.a cstart.a modend.a

# C header files
HEADERS.c	= sysdep.h norcrosys.h fplib.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
#	Possible flags: -DSTACKEXTENSION (requires stackchecking)
ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -ps1 -dT800 -t4 -dPOSIX -DBYTESEX_EVEN
endif
ifeq ($(HPROC),C40)
# -Zpw1 = little data < 64k static data optimisation
# -Zps1 = no stack checks
  NCFLAGS := $(NCFLAGS) -Zpw1 -dPOSIX -DBYTESEX_EVEN -DSTACKEXTENSION
endif
ifeq ($(HPROC),ARM)
# -Zt = tiny memory model
  NCFLAGS := $(NCFLAGS) -dPOSIX -DBYTESEX_EVEN -Zt
endif
ifeq ($(HPROC),ARM6)
  NCFLAGS := $(NCFLAGS) -dPOSIX -DBYTESEX_EVEN -Zt
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/kernel.def $(LIB)/syslib.def $(LIB)/posix.def $(LIB)/util.def

ifeq ($(HPROC),TRAN)
  LIBDEFS := $(LIBDEFS) $(LIB)/fplib.def
endif

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

OBJECTS = clibstar.p stdio.p ctype.p string.p printf.p fpprintf.p \
	scanf.p stdlib.p sort.p alloc.p time.p error.p \
	locale.p posixsys.p $(XOBJECTS)	modend.p

$(OBJECTS): sysdep.h norcrosys.h

# Posix C library
clib.i: $(OBJECTS) $(LIBDEFS)
	$(LINK.LIBRARY) -nClib $^ -oclib.i
#	cp clib.i /lib/clib

clib.def : clibstar.a
	$(MAKE.DEF)


ifeq ($(HPROC),TRAN)
cstart.o: c0.o
	asm -p -o cstart.o		\
		$(LIB)/c0.o		\
		$(LIB)/kernel.def	\
		$(LIB)/syslib.def	\
		$(LIB)/util.def		\
		$(LIB)/clib.def		\
		$(LIB)/fplib.def	\
		$(LIB)/posix.def

c0.o	: cstart.p
	$(CP) cstart.p c0.o

#pctest : pctest.c
#	$(CC) -t4 -j$(INCLUDE)/ pctest.c -s$(FIFO)/x
#	$(ASM) -v -f -npctest $(LIB)/cstart.o $(FIFO)/x -opctest
#	$(CP) pctest $(BIN)

else
# c0.o is actual C startup object code, cstart.o is object + lib resrefs
c0.o : cstart.a
	$(AMPPASM)

cstart.o: c0.o clib.def $(LIBDEFS)
	cat $^ > cstart.o

pctest : pctest.c
	$(NCC) -c pctest.c
	$(LINK) -npctest $(LIB)/cstart.o -opctest pctest.o
	$(CP) pctest $(BIN)
endif


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
