#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: General Commands
#
# File: /HSRC/cmds/com/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.40 1994/06/07 12:30:26 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#
#
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
COMPONENT = cmds/com

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HVPATH):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = $(RBINS) $(SBINS)

# default & install are at end of this file due to problem with GNUMake
# See comments there...

#default: $(TARGETS) Xdefault
#
#install	: $(TARGETS) Xinstall
#	$(CP) $(RBINS) $(BIN)
#	$(CP) $(SBINS) $(BIN)
#	$(TOUCH) install
# Stop no args cleaning the cmds!
newdefault: default


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= find.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile

# Bog standard Regular commands
# ps, run, login and logname have been moved to the network directory
RSRCS = ascii.c atob.c basename.c bsplit.c btoa.c cal.c cmp.c cat.c \
	date.c df.c dirname.c dump.c false.c \
	gdi.c mem.c cache.c mkdir.c touch.c clear.c chmod.c refine.c \
	tee.c test.c true.c od.c sleep.c map.c objed.c ln.c \
	waitfor.c which.c sysbuild.c sysbreak.c whichend.c lb.c

# Now commands which need compiling specially in some way
# *Warning* Also update $(SBINS) manually when you update $(SSRCS)
SSRCS := compress.c cp.c du.c find.c ls.c mv.c loadpac.c rm.c rmdir.c c.c

ifeq ($(HPROC),C40)
RSRCS := $(RSRCS) idrom.c
endif

ifeq ($(HPROC),ARM)
RSRCS := $(RSRCS) buildkeymap.c
endif

# toshell is a cutdown login that goes direct to the shell - no messin!
ifneq ($(HPROC),TRAN)
  ifeq ($(HPROC),ARM)
    RSRCS := $(RSRCS) slice.c toshell.c
  else
    RSRCS := $(RSRCS) threadps.c slice.c toshell.c
  endif
endif

# C source files
SOURCES.c	= $(RSRCS) $(SSRCS)

# Transform to lists of object files
ROBJS = $(RSRCS:.c=.o)
SOBJS = $(SSRCS:.c=.o) compress.14.o

default: $(TARGETS) Xdefault

#transform to lists of binaries
RBINS = $(ROBJS:.o=)
SBINS := loadpac ls cp mv find du rm rmdir c compress compress.14 tcp
ifeq ($(HPROC),ARM)
SBINS := $(SBINS) toshell
endif
ifeq ($(HPROC),C40)
SBINS := $(SBINS) toshell
endif

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		   $(HEADERS.c) $(HEADERS.m) \
		   $(SOURCES.odd)

OBJECTS	= $(ROBJS) $(SOBJS)

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS)
else
  NCFLAGS := $(NCFLAGS)
endif

%: %.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $@.o

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

#$(TARGETS): $(CSTART)

# Compile regular and private binaries

$(ROBJS) :

$(RBINS) : $(CSTART)

#------------------------------------------------------------------------
# Special cases

#ls, cp and mv need large stacks as they may use recursion.

ls : $(CSTART) $(LIB)/servlib.def ls.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s20000 -h4000 $^

find : $(CSTART) find.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

cp : $(CSTART) cp.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

tcp : $(CSTART) cp.o	     # yes 'tcp' is the same program as 'cp' but with a different name!
	$(LINK) $(LINKFLAGS) -otcp -n$@ -s8000 -h4000 $^

du : $(CSTART) du.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

mv : $(CSTART) mv.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

rm : $(CSTART) rm.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

rmdir : $(CSTART) rmdir.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

loadpac : $(CSTART) loadpac.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

compress.o : compress.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -DUSERMEM=450000 -o$@ $<
else
	$(CC) -c -DUSERMEM=450000 -o$@ $<
endif

compress.14.o : compress.c
ifeq ($(HPROC),ARM)
	$(NC) -c $(NCFLAGS) -DUSERMEM=128000 -o$@ $<
else
	$(CC) -c -DUSERMEM=128000 -o$@ $<
endif

threadps.o : threadps.c
	$(NC) -c $(NCFLAGS) -I$(HSRC)/kernel -o$@ $<


# cat needs the server library
cat : $(CSTART) cat.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ $^ $(LIB)/servlib.def

# could probably reduce the heap and stack further
toshell:  $(CSTART) toshell.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^

# other processor versions have to setup a special set of defines
ifeq ($(HPROC),TRAN)
c:  $(CSTART) c.o
	$(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h1000 $^
endif

#-----------------------------------------------------------------------------
# Due to the use of ':=', rather than '=', these rules have to appear here.

default: $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(RBINS) $(BIN)
	$(CP) $(SBINS) $(BIN)
ifneq ($(HPROC),TRAN)
	$(CP) c $(BIN)/cc
endif
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall *.ghof

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
