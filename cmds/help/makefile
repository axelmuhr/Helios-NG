#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: Helios Help
#
# File: /HSRC/cmds/help/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.6 1994/06/10 11:47:57 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(c) Copyright 1994 Perihelion Distributed Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# $(HSRC)/makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
ifndef HPROC
include ../makeinc/DFLT.mak
endif

.PHONEY: notarget
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = cmds/help

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables
include $(HSRC)/makeinc/$(HPROC).mak	# Default processor rules and vars.

# Manditory phoney targets
.PHONEY: default clean Xdefault Xclean
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
# Phoney targets prefixed by X are for adding processor specific targets.
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.
HVPATH := $(HSRC)/$(COMPONENT)

ifdef RSRC
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = help dbbuild truncate dbdump dbprint

default: $(TARGETS) Xdefault

install: $(TARGETS) Xinstall
	$(CP) $(TARGETS) $(BIN)
	-$(CP) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.txt $(LIB)
	-$(CP) $(HSRC)/$(COMPONENT)/$(HPROC)/help0.dct $(LIB)
	$(TOUCH) install

clean: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		dbprint.o dbbuild.o dbdump.o help.o truncate.o\
		core ../mkout install Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS		= big.o lrutils.o stem.o utils.o storage.o \
		  insert.o search.o btree.o lrucache.o \
		  seq.o delete.o lruhash.o split.o updutils.o

# C source files
SOURCES.c	= big.c help.c lrutils.c stem.c utils.c btree.c \
		  insert.c search.c storage.c dbbuild.c lrucache.c \
		  seq.c delete.c lruhash.c split.c updutils.c \
		  truncate.c dbdump.c dbprint.c

# C header files
HEADERS.c  	= btree.h db.h endian.h lrucache.h

# AMPP source files
SOURCES.a	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile generic.txt c40.txt arm.txt transputer.txt \
		  vectlib.txt winman.txt stopword.lst not_tran.txt


#-----------------------------------------------------------------------------
# Other generic component variables:

# Set any flags required

NCFLAGS := $(NCFLAGS) -D_BSD

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

$(OBJECTS): $(HEADERS.c)

truncate: $(CSTART) truncate.o stem.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ $^ -l$(LIB)/bsd.lib

dbdump: $(CSTART) dbdump.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ $^ -l$(LIB)/bsd.lib

dbprint: $(CSTART) dbprint.o
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ $^ -l$(LIB)/bsd.lib

help: $(CSTART) help.o $(OBJECTS)
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s20000 -h20000 $^ -l$(LIB)/bsd.lib

dbbuild: $(CSTART) dbbuild.o $(OBJECTS)
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o$@ -n$@ -s20000 -h20000 $^ -l$(LIB)/bsd.lib

#-----------------------------------------------------------------------------
endif #InProcMake

# This line must be at EOF
