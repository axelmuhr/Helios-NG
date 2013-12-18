#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: KERNEL
#
# File: /HSRC/tcpip/cmds/lpr/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.9 1993/02/26 14:57:59 paul Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

ifndef InProcMake
# This part of the makefile is run in the components generic source directory

# The variables HPROC, HHOST, HSRC, and HPROD should already be set by 
# your make script or environment, if not then get local defaults from
# makeinc/DFLT.mak. Note that the following pathname cannot use $(HSRC).
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
COMPONENT = tcpip/cmds/lpr

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak
include $(HSRC)/makeinc/$(HPROC).mak

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)


#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
LPD=	lpd lpd_aux
LPC=	lpc
OTHERS=	lpr lpq lprm lptest

TARGETS	= utils $(LPD) $(LPC) $(OTHERS) # FILTERS	# VFILTERS

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	$(CP) lpd $(LIB)
	$(CP) lpd_aux $(LIB)
	$(CP) lpr $(BIN)
	$(CP) lpq $(BIN)
	$(CP) lprm $(BIN)
	$(CP) lpc $(BIN)
	$(CP) lptest $(BIN)
	$(RM) $(ETC)/printcap
	$(CP) ../printcap $(ETC)
	$(TOUCH) install

installdef:
	$(TOUCH) installdef

clean	: Xclean
	-$(RM)	$(OBJECTS) $(TARGETS) $(XOBJECTS) $(XTARGETS) \
		mkout core ../mkout install installdef Xinstall


#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

SOURCES	=	$(SOURCES.c) $(SOURCES.a) $(HEADERS.c) $(HEADERS.m) \
		$(SOURCES.odd)

L1OBJS = 	$(L1SRCS.c:.c=.o)
L2OBJS = 	$(L2SRCS.c:.c=.o)
L3OBJS = 	$(L3SRCS.c:.c=.o)
L4OBJS = 	$(L4SRCS.c:.c=.o)
L5OBJS = 	$(L5SRCS.c:.c=.o)
L6OBJS = 	$(L6SRCS.c:.c=.o)
L7OBJS = 	$(L7SRCS.c:.c=.o)
L8OBJS = 	$(L8SRCS.c:.c=.o)
OBJECTS=	${L1OBJS} ${L2OBJS} ${L3OBJS} ${L4OBJS} ${L5OBJS} ${L6OBJS} \
		${L7OBJS} ${L8OBJS}

# C source files
L1SRCS.c =	lpd.c printcap.c version.c
L2SRCS.c =	lpr.c startdaemon.c printcap.c
L3SRCS.c =	lpq.c displayq.c common.c printcap.c
L4SRCS.c =	lprm.c rmjob.c startdaemon.c common.c printcap.c
L5SRCS.c =	lpc.c cmds.c cmdtab.c startdaemon.c common.c printcap.c
L6SRCS.c =	lptest.c
L7SRCS.c =	lpd_aux.c printjob.c startdaemon.c lpdchar.c common.c printcap.c
L8SRCS.c =	utils.c f_lock.c

SOURCES.c=	lpd.c printcap.c version.c lpr.c startdaemon.c lpq.c \
		displayq.c common.c lprm.c rmjob.c lpc.c cmds.c cmdtab.c \
		lptest.c lpd_aux.c printjob.c lpdchar.c utils.c f_lock.c

SOURCES.a =

# C header files
HEADERS.c =	bsd.h common.h lp.h lp_local.h lpc.h lpd.h printcap.h utils.h \
		f_lock.h

# AMPP macro files
HEADERS.m =

SOURCES.odd = makefile printcap

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

# Turn off stack check and function header generation.
ifeq ($(HPROC),ARM)
  NCFLAGS := $(NCFLAGS) -Zps1 -D_BSD
else
#  NCFLAGS := $(NCFLAGS) -wd -D_BSD -DMEM_CHECK -DDEBUG
  ifeq ($(HPROC),C40)
    NCFLAGS := $(NCFLAGS) -Zps1 -D_BSD
  else
    ifdef SYSDEB
      NCFLAGS := $(NCFLAGS) -ps1
    endif
    NCFLAGS := $(NCFLAGS) -wd -D_BSD
  endif
endif

# Note any library def files to be linked with
LIBDEFS	:= $(LIB)/bsd.lib utils

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

$(OBJECTS): $(HEADERS.c)

#lpd: ${L1OBJS} ${CSTART}
#	asm -v -f -s10000 $(CSTART) $(L1OBJS) $(LIBS) -o $@
#lpr: ${L2OBJS} ${CSTART}
#	asm -v -f -s10000 $(CSTART) $(L2OBJS) $(LIBS) -o $@
#lpq: ${L3OBJS} ${CSTART}
#	asm -v -f -s10000 $(CSTART) $(L3OBJS) $(LIBS) -o $@
#lprm: ${L4OBJS} ${CSTART}
#	asm -v -f -s10000 $(CSTART) $(L4OBJS) $(LIBS) -o $@
#lpc: ${L5OBJS} ${CSTART} 
#	asm -v -f -s10000 $(CSTART) $(L5OBJS) $(LIBS) -o $@
#lptest: ${L6OBJS} ${CSTART}
#	asm -v -f $(CSTART) $(L6OBJS) $(LIBS) -o $@
#lpd_aux: ${L7OBJS} ${CSTART
#	asm -v -f -s10000 $(CSTART) $(L7OBJS) $(LIBS) -o $@
#utils: ${L8OBJS} ${CLIB}
#	asm -v -f -p -l$(CLIB) -l$(DLIB) utils.o -o $@

lpd: $(CSTART) $(LIBDEFS) $(L1OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
lpr: $(CSTART) $(LIBDEFS) $(L2OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
lpq: $(CSTART) $(LIBDEFS) $(L3OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
lprm: $(CSTART) $(LIBDEFS) $(L4OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
lpc: $(CSTART) $(LIBDEFS) $(L5OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
lptest: $(CSTART) $(L6OBJS)
	$(LINK) $(LINKFLAGS) -o$@ $^
lpd_aux: $(CSTART) $(LIBDEFS) $(L7OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
utils: $(L8OBJS)
ifeq ($(HPROC),TRAN)
	$(LINK) -v -f -p -o$@ $^
else
	cat $^ > $@
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
