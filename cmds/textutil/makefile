#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: General Commands
#
# File: /HSRC/cmds/textutil/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.10 1994/06/10 11:34:56 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(c) Copyright 1994 Perihelion Distributed Software
#
# WARNING: assumes you are using GNUmake.
#
#
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

ifndef HPROC
include /hsrc/makeinc/DFLT.mak
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
COMPONENT = cmds/textutil

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
TARGETS = comm join uniq fold colrm rev yes leave sort diff egrep grep \
	  fgrep head tail more pr split strings xlatecr wc

default: $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

OBJECTS		= $(SOURCES.c:.c=.o)

# C header files
HEADERS.c	= linebuffer.h system.h regexp.h

# Any other source files
SOURCES.odd	= makefile

# Bog standard Regular commands
SOURCES.c	 = colrm.c fold.c leave.c rev.c sort.c yes.c comm.c join.c \
		   uniq.c error.c linebuffer.c xmalloc.c getopt.c getopt1.c \
		   regexp.c regsub.c fgrep.c head.c diff.c egrep.c grep.c \
	  	   tail.c more.c pr.c split.c strings.c \
		   xlatecr.c wc.c

#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required
ifeq ($(HPROC),TRAN)
  NCFLAGS := $(NCFLAGS) -wadfsv -ecpz -D_BSD
else
  NCFLAGS := $(NCFLAGS) -D_BSD
endif

%: %.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $(CSTART) $@.o

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

#------------------------------------------------------------------------

comm : $(CSTART) comm.o linebuffer.o error.o xmalloc.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 -l$(LIB)/bsd.lib $^

join : $(CSTART) join.o linebuffer.o error.o xmalloc.o getopt.o getopt1.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

uniq : $(CSTART) uniq.o linebuffer.o error.o xmalloc.o getopt.o getopt1.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

fold : $(CSTART) fold.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

colrm:  $(CSTART) colrm.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

yes:  $(CSTART) yes.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

rev:  $(CSTART) rev.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

tail:  $(CSTART) tail.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

more:  $(CSTART) more.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

leave:  $(CSTART) leave.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

head:  $(CSTART) head.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

diff:  $(CSTART) diff.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

sort:  $(CSTART) sort.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

pr:  $(CSTART) pr.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

split:  $(CSTART) split.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

strings:  $(CSTART) strings.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

xlatecr:  $(CSTART) xlatecr.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^ -l$(LIB)/bsd.lib

wc:  $(CSTART) wc.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^

regsub.o regexp.o grep.o: regexp.h

grep:  $(CSTART) grep.o regexp.o regsub.o 
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

egrep:  $(CSTART) egrep.o regexp.o regsub.o 
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s8000 -h4000 $^

fgrep:  $(CSTART) fgrep.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s2000 -h4000 $^


comm.o : comm.c linebuffer.h system.h
join.o : join.c linebuffer.h system.h
uniq.o : uniq.c linebuffer.h system.h

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
