#-----------------------------------------------------------------------------
# Helios generic make system
#-----------------------------------------------------------------------------
# Helios generic component make rules for: KERNEL
#
# File: /HSRC/tcpip/cmds/sendmail/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1993/02/26 14:55:28 paul Exp $
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
COMPONENT = tcpip/cmds/sendmail

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


TARGETS	=  common sendmail smtp

default : $(TARGETS) Xdefault

install	: installdef $(TARGETS) Xinstall
	$(CP) sendmail $(LIB)
	$(CP) smtp $(LIB)
	$(RM) $(ETC)/sendmail.hf
	$(CP) ../sendmail.hf $(ETC)
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

COMMON_OBJS = 	$(COMMON_SRCS.c:.c=.o)
SENDMAIL_OBJS = 	$(SENDMAIL_SRCS.c:.c=.o)
SMTP_OBJS = 	$(SMTP_SRCS.c:.c=.o)
OBJECTS=	${COMMON_OBJS} ${SENDMAIL_OBJS} ${SMTP_OBJS}

# C source files
COMMON_SRCS.c=	arpadate.c collect.c daemon.c deliver.c err.c extras.c \
		queue.c sysexits.c usersmtp.c util.c version.c 
SMTP_SRCS.c=	smtp.c
SENDMAIL_SRCS.c=	main.c 

SOURCES.c=	${COMMON_SRCS.c} ${SENDMAIL_SRCS.c} ${SMTP_SRCS.c}

SOURCES.a =

# C header files
HEADERS.c =	conf.h def.h sendmail.h useful.h

# AMPP macro files
HEADERS.m =

SOURCES.odd = makefile sendmail.hf

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
LIBDEFS	:= $(LIB)/bsd.lib common

LINKFLAGS := $(LINKFLAGS)

#-----------------------------------------------------------------------------
# Generic targets and their dependencies:

# uncomment if you have compiler or makedepend generated dependencies
#include depends

$(OBJECTS): $(HEADERS.c)

#common: ${COMMON_OBJS} ${CLIB}
#	asm -v -f -p -l$(CLIB) -l$(DLIB) $(COMMON_OBJS) -o $@
#smtp: ${SMTP_OBJS}
#	asm -v -f -s20000 $(CSTART) $(SMTP_OBJS) $(LIBS) -o $@
#sendmail: ${SENDMAIL_OBJS}
#	asm -v -f -s10000 $(CSTART) $(SENDMAIL_OBJS) $(LIBS) -o $@

sendmail: $(CSTART) $(LIBDEFS) $(SENDMAIL_OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s10000 $^
smtp: $(CSTART) $(LIBDEFS) $(SMTP_OBJS)
	$(LINK) $(LINKFLAGS) -o$@ -s20000 $^
common: $(COMMON_OBJS)
ifeq ($(HPROC),TRAN)
	$(LINK) -v -f -p -o$@ $^
else
	cat $^ > $@
endif

#-----------------------------------------------------------------------------
endif #InProcMake	# This line must be at EOF
