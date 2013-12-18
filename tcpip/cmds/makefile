#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: TCP/IP (ARPANET) cmds package
#
# File: /HSRC/tcpip/cmds/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.5 1992/12/02 15:38:54 nickc Exp $
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
include ../../makeinc/DFLT.mak
endif

notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = tcpip/cmds

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
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
TARGETS = ftp ftpd inetd ping route rexecd rlogin rlogind rsh rshd telnet \
	telnetd rcp binmail sendmail lpr

.PHONY: $(TARGETS) install

default : Xdefault
	$(foreach TARGET, $(TARGETS), $(MAKE) -C ../$(TARGET)/$(HPROC);)


install	: Xinstall
	$(foreach TARGET, $(TARGETS), $(MAKE) -C ../$(TARGET)/$(HPROC) install;)
	$(TOUCH) install

clean	: Xclean
	-$(RM) mkout core install ../mkout
	$(foreach TARGET, $(TARGETS), $(MAKE) -C ../$(TARGET)/$(HPROC) clean;)

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files that are not in individual server
# directories.

# Note all sources
SOURCES		= $(SOURCES.c) $(SOURCES.a) \
		  $(HEADERS.c) $(HEADERS.m) \
		  $(SOURCES.odd)

OBJECTS	= $(SOURCES.c:.c=.o) $(SOURCES.a:.a=.o)

# C source files
SOURCES.c	=

# AMPP source files
SOURCES.a	=

# C header files
HEADERS.c	=

# AMPP macro files
HEADERS.m	=

# Any other source files
SOURCES.odd	= makefile


#-----------------------------------------------------------------------------
# Other generic component variables:
# Start and completion messages:
inmessage:	; @echo "                                 [[[ Making the Helios/$(HPROC) tcpip commands ]]]"
outmessage:	; @echo "                                 [[[ Helios/$(HPROC) tcpip commands constructed ]]]"

#-----------------------------------------------------------------------------
# Individual servers in their own directories go here:

ftp:
	$(MAKE) -C ../ftp/$(HPROC)

ftpd :
	$(MAKE) -C ../ftpd/$(HPROC)

inetd:
	$(MAKE) -C ../inetd/$(HPROC)

ping :
	$(MAKE) -C ../ping/$(HPROC)

route :
	$(MAKE) -C ../route/$(HPROC)

rexecd:
	$(MAKE) -C ../rexecd/$(HPROC)

rlogin:
	$(MAKE) -C ../rlogin/$(HPROC)

rsh:
	$(MAKE) -C ../rsh/$(HPROC)

rshd:
	$(MAKE) -C ../rshd/$(HPROC)

telnet:
	$(MAKE) -C ../telnet/$(HPROC)

telnetd:
	$(MAKE) -C ../telnetd/$(HPROC)

binmail:
	$(MAKE) -C ../binmail/$(HPROC)

sendmail:
	$(MAKE) -C ../sendmail/$(HPROC)

lpr:
	$(MAKE) -C ../lpr/$(HPROC)

#-----------------------------------------------------------------------------

endif #InProcMake	# This line must be at EOF
