#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: GNU Make
#
# File: /HSRC/cmds/gnu/gmake/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# RcsId: $Id: makefile,v 1.15 1994/05/17 08:38:50 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#	(C) Copyright 1994 Perihelion Distributed Software
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
COMPONENT = cmds/gnu/gmake

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
HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
endif

vpath %.h $(HVPATH)
vpath %.a $(HVPATH)
vpath %.c $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
 TARGETS = make

default : $(TARGETS) Xdefault

install	: $(TARGETS) Xinstall
	$(CP) make $(HPROD)/local/bin/gmake
	$(TOUCH) install

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) mkout \
		core ../mkout install installdef Xinstall *.ghof


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)
#ifeq ($(HPROC),C40)
  ALLOCA=alloca.o
#endif

OBJECTS	=	glob.o commands.o job.o dir.o file.o load.o misc.o main.o \
		read.o remake.o remote.o rule.o implicit.o default.o \
		variable.o expand.o function.o vpath.o version.o \
		$(ARCHIVES) $(ALLOCA) $(extras)

# C source files
SOURCES.c	= glob.c commands.c job.c dir.c file.c load.c misc.c main.c \
		read.c remake.c remote.c rule.c implicit.c default.c \
		variable.c expand.c function.c vpath.c version.c $(ALLOCASRC) \
		$(ARCHIVES_SRC) remote-stub.c

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	= commands.h dep.h file.h job.h make.h rule.h variable.h

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile README CHANGES TAGS makefile.hel makefile.sun \
		make-3.57-3.58 makefile.bsd


#-----------------------------------------------------------------------------
# Other generic component variables:

defines = -DDIRENT -DHELIOS -DNO_ARCHIVES -DHAVE_VFORK \
 -DHAVE_DUP2 -DPOSIX -DSTACK_DIRECTION=-1 -D_BSD -DHAVE_SYS_WAIT

# Set any flags required
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) $(defines) -w
else
  NCFLAGS := $(NCFLAGS) $(defines) -wvdf
endif

LINKFLAGS := $(LINKFLAGS) -s20000

#-----------------------------------------------------------------------------

make: $(OBJECTS)
	@ echo Linking $@ ...
	@ $(LINK) $(LINKFLAGS) -o make $(LIB)/cstart.o $(OBJECTS) -l $(LIB)/bsd.lib

ifeq ($(HPROC),TRAN)
load.o: load.c
	$(NC) $(NCFLAGS) $(LOAD_AVG) $< | asm -p -o load.o

remote.o: remote.c
	$(NC) $(NCFLAGS) $(REMOTE) $< | asm -p -o remote.o
else
load.o: load.c
	@ echo Compiling $(<F) ...
	@ $(NC) $(NCFLAGS) $(LOAD_AVG) -c ../load.c

remote.o: remote.c
	@ echo Compiling $(<F) ...
	@ $(NC) $(NCFLAGS) $(REMOTE) -c ../remote.c
endif

#-----------------------------------------------------------------------------
# Original GNU makefile (with a few hacks of course):
#-----------------------------------------------------------------------------

# Copyright (C) 1988, 1989 Free Software Foundation, Inc.
# This file is part of GNU Make.
# 
# GNU Make is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# GNU Make is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GNU Make; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

#
#	Makefile for GNU Make
#

CFLAGS = $(defines) -wvdf
LDFLAGS = -s20000

# Define nothing for BSD, USG for System V, and USGr3 (as well as USG) for
# SVR3, HPUX for HP-UX (as well as USG or USGr3 as appropriate).  If you have a
# USG hybrid with <sys/wait.h> and wait3, define HAVE_SYS_WAIT.  If you are USG
# but have sys_siglist, define HAVE_SIGLIST.  If you are USG, and not USGr3,
# but have dup2, define HAVE_DUP2.  If your `cc' command doesn't grok -o
# options with -c (true for many 4.2 BSD derivatives), define
# NO_MINUS_C_MINUS_O.  If you don't want archive support, define NO_ARCHIVES.
# If you want to avoid use of floating-point numbers, define NO_FLOAT.  If your
# compiler can handle `enum' bitfields (and it's not GCC), define
# ENUM_BITFIELDS.
defines = -DDIRENT -DUSG -DHELIOS -DNO_ARCHIVES -DHAVE_VFORK\
 -DHAVE_DUP2 -DPOSIX -DSTACK_DIRECTION=-1

# Define UMAX here to use Encore's inq_stats call.  If the load average is in a
# symbol in /dev/kmem, define KERNEL_FILE_NAME if not "/vmunix", LDAV_SYMBOL if
# not "_avenrun", LDAV_TYPE if not `long int', and LDAV_CVT to convert the
# LDAV_TYPE value from LDAV_SYMBOL (in `load') to a double if this is not
# "(double) load".  If a `struct nlist' (as defined in <nlist.h>) has a `n_un'
# union, rather than a simple `n_name' member, define NLIST_NAME_UNION.  If the
# `n_name' member of a `struct nlist' is an array that must be copied into,
# define NLIST_NAME_ARRAY.  Otherwise, define NO_LDAV.
LOAD_AVG = -DNO_LDAV

# If you don't want archive support, comment these out.
#ARCHIVES = arscan.o ar.o
#ARCHIVES_SRC = arscan.c ar.c

# If your system needs extra libraries loaded in, define them here.
# System V probably need -lPW for alloca.
FPFILES =
LOADLIBES = $(LIB)/c0.o -lhelios

# If your system doesn't have alloca, or the one provided is bad,
# get it from the Emacs distribution and define these.
ALLOCA = alloca.o
ALLOCASRC = alloca.c

# If there are remote execution facilities defined,
# enable them with switches here (see remote-*.c).
REMOTE =

# Any extra object files your system needs.
extras =

# Directory to install `make' in.
bindir = $(prefix)/usr/perihelion/bin
# Directory to install the man page in.
mandir = $(prefix)/usr/local/man/man1
# Number to put on the man page filename.
manext = 1

objs = glob.o commands.o job.o dir.o file.o load.o misc.o main.o read.o \
       remake.o remote.o rule.o implicit.o default.o variable.o expand.o \
       function.o vpath.o version.o $(ARCHIVES) $(ALLOCA) $(extras)
srcs = glob.c commands.c job.c dir.c file.c load.c misc.c main.c read.c \
       remake.c remote.c rule.c implicit.c default.c variable.c expand.c \
       function.c vpath.c version.c $(ALLOCASRC) $(ARCHIVES_SRC) \
       commands.h dep.h file.h job.h make.h rule.h variable.h


.SUFFIXES:
.SUFFIXES: .o .c .h .ps .dvi .texinfo


ifeq ($(HPROC),TRAN)
LINK = asm
endif

.PHONY: all doc
all: make
doc: make-info make.dvi


# Take your pick.
#makeinfo = emacs -batch make.texinfo -f texinfo-format-buffer -f save-buffer
makeinfo = makeinfo make.texinfo

make-info: make.texinfo
	$(makeinfo)


make.dvi: make.texinfo
	-tex make.texinfo
	texindex make.cp make.fn make.ky make.pg make.tp make.vr
	-tex make.texinfo

make.ps: make.dvi
	dvi2ps make.dvi > make.ps

#make: $(objs)
#	$(LINK) $(LOADLIBES) $(LDFLAGS) $(objs) -o make.new

#load.o: load.c
#	$(CC) $(CFLAGS) $(LOAD_AVG) -c load.c
#remote.o: remote.c
#	$(CC) $(CFLAGS) $(REMOTE) -c remote.c

TAGS: $(srcs)
	etags -tw $(srcs)
tags: $(srcs)
	ctags -tw $(srcs)

#.PHONY: install
#install: $(bindir)/make $(mandir)/make.$(manext)

$(bindir)/make: make
	cp make $@.new
	mv $@.new $@

$(mandir)/make.$(manext): make.man
	cp make.man $@

.PHONY: clean realclean
#clean:
#	-rm -f make *.o core
realclean: clean
	-rm -f TAGS tags make.info* make-* make.dvi
	-rm -f make.?? make.??s make.log make.toc make.*aux

# Automatically generated dependencies.
glob.o : glob.c 
commands.o : commands.c make.h dep.h commands.h file.h variable.h job.h 
job.o : job.c make.h commands.h job.h file.h variable.h 
dir.o : dir.c make.h 
file.o : file.c make.h commands.h dep.h file.h variable.h 
load.o : load.c make.h commands.h job.h 
misc.o : misc.c make.h dep.h 
main.o : main.c make.h commands.h dep.h file.h variable.h job.h 
read.o : read.c make.h commands.h dep.h file.h variable.h 
remake.o : remake.c make.h commands.h job.h dep.h file.h 
remote.o : remote.c remote-stub.c make.h commands.h 
rule.o : rule.c make.h commands.h dep.h file.h variable.h rule.h 
implicit.o : implicit.c make.h rule.h dep.h file.h 
default.o : default.c make.h rule.h dep.h file.h commands.h variable.h 
variable.o : variable.c make.h commands.h variable.h dep.h file.h 
expand.o : expand.c make.h commands.h file.h variable.h 
function.o : function.c make.h variable.h dep.h commands.h job.h 
vpath.o : vpath.c make.h file.h variable.h 
version.o : version.c 
arscan.o : arscan.c 
ar.o : ar.c make.h file.h 


#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF

