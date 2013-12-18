#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: TCP/IP (ARPANET) comms package
#
# File: /HSRC/tcpip/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.25 1994/01/05 14:14:50 paul Exp $
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

#.PHONY: notarget cmds
notarget:
	@$(MAKE) -C $(HPROC)

.DEFAULT:
	@$(MAKE) -C $(HPROC) $@

else
#-----------------------------------------------------------------------------
# This part of the makefile is always run in a processor specific subdirectory.

# Manditory variables for a component:
# name of this dir relative to $(HSRC)
COMPONENT = tcpip

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
VPATH := $(HSRC)/$(COMPONENT):$(HSRC)/$(COMPONENT)/etc

ifdef RSRC
 HVPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT)
 VPATH := $(RSRC)/$(COMPONENT)/$(HPROC):$(HSRC)/$(COMPONENT):$(RSRC)/$(COMPONENT):$(HSRC)/$(COMPONENT)/etc
endif

vpath %.h $(HVPATH)
vpath %.c $(HVPATH)
vpath %.a $(HVPATH)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here

.PHONY: notarget

DEMOS = example laserp tinybiff

ifeq ($(HPROC),TRAN)
	REMOTETARGETS = cmds Proto Sockets Net Helios pc-ether sq-ether in-ether ims-b431 nfs
else
REMOTETARGETS = cmds Proto Sockets Net Helios pc-ether nfs
ifeq ($(HPLATFORM),VY86PID)
	REMOTETARGETS := $(REMOTETARGETS) pidether
endif
endif

LOCALTARGETS = tcpip dbfiles demos
TARGETS = inmessage $(REMOTETARGETS) $(LOCALTARGETS) outmessage 

default: $(REMOTETARGETS) Xdefault


install: Xinstall $(REMOTETARGETS) localinstall 
	$(MAKE) -C ../netinet/$(HPROC) install
	$(MAKE) -C ../net/$(HPROC) install
	$(MAKE) -C ../sys/$(HPROC) install
	$(MAKE) -C ../helios/$(HPROC) install
	$(MAKE) -C ../cmds/$(HPROC) install
	$(MAKE) -C ../devs/pc-ether/$(HPROC) install

ifeq ($(HPROC),TRAN)
	$(MAKE) -C ../devs/sq-ether/$(HPROC) install
	$(MAKE) -C ../devs/in-ether/$(HPROC) install
	$(MAKE) -C ../devs/ims-b431/$(HPROC) install
endif

ifeq ($(HPLATFORM),VY86PID)
	$(MAKE) -C ../devs/pidether/$(HPROC) install
endif
	$(MAKE) -C ../rpc/rpc/$(HPROC) install
	$(MAKE) -C ../nfs/$(HPROC) install
	$(foreach DEMO, $(DEMOS), \
		$(MAKE) -C ../$(DEMO)/$(HPROC) install ;)
	$(TOUCH) install

localinstall:	$(LOCALTARGETS)
	$(CP) tcpip $(LIB)/tcpip
	$(TOUCH) localinstall

clean: Xclean
	-$(RM) mkout core install tcpip proto sockets net helios localinstall \
	       dbfiles demos localdefault
	$(MAKE) -C ../netinet/$(HPROC) clean
	$(MAKE) -C ../net/$(HPROC) clean
	$(MAKE) -C ../sys/$(HPROC) clean
	$(MAKE) -C ../helios/$(HPROC) clean
	$(MAKE) -C ../cmds/$(HPROC) clean
	$(MAKE) -C ../devs/pc-ether/$(HPROC) clean
ifeq ($(HPROC),TRAN)
	$(MAKE) -C ../devs/sq-ether/$(HPROC) clean
	$(MAKE) -C ../devs/in-ether/$(HPROC) clean
	$(MAKE) -C ../devs/ims-b431/$(HPROC) clean
endif
ifeq ($(HPLATFORM),VY86PID)
	$(MAKE) -C ../devs/pidether/$(HPROC) clean
endif
	$(MAKE) -C ../rpc/rpc/$(HPROC) clean
	$(MAKE) -C ../nfs/$(HPROC) clean

	$(foreach DEMO, $(DEMOS), \
		$(MAKE) -C ../$(DEMO)/$(HPROC) clean ;)

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
inmessage:	; @echo "          [[[ Making the Helios/$(HPROC) tcpip servers and device drivers ]]]"
outmessage:	; @echo "          [[[ Helios/$(HPROC) tcpip servers and device drivers constructed ]]]"

#-----------------------------------------------------------------------------
# Individual servers in their own directories go here:

# shorthand to just make the tcpip server
# AxM: Corrected spacing/tabs to make gmake happy
tcpip: Proto Sockets Net Helios
ifeq ($(HPROC),TRAN)
	asm $(TRACE) -v -f -o tcpip -ntcpip $(LIB)/s0.o \
		proto net sockets helios \
		-l$(LIB)/helios.lib -l$(LIB)/bsd.lib
else
	$(LINK) -o tcpip -ntcpip $(LIB)/s0.o \
	proto net sockets helios \
	-l$(LIB)/helios.lib -l$(LIB)/bsd.lib
endif

Proto:
	$(MAKE) -C ../netinet/$(HPROC) install

Net:
	$(MAKE) -C ../net/$(HPROC) install

Sockets:
	$(MAKE) -C ../sys/$(HPROC) install

Helios  :
	$(MAKE) -C ../helios/$(HPROC) install

cmds :
	$(MAKE) -C ../cmds/$(HPROC) 

pc-ether :
	$(MAKE) -C ../devs/pc-ether/$(HPROC) 

sq-ether :
	$(MAKE) -C ../devs/sq-ether/$(HPROC) 

in-ether :
	$(MAKE) -C ../devs/in-ether/$(HPROC) 

pidether :
	$(MAKE) -C ../devs/pidether/$(HPROC) 

ims-b431 :
	$(MAKE) -C ../devs/ims-b431/$(HPROC) 

nfs :
	$(MAKE) -C ../nfs/$(HPROC) 

# vpath has been set to $(TCPIP)/etc to pick these up.
DBFILES = hosts services networks protocols inetd.conf socket.conf \
	  termcap devinfo.net hosts.equiv

dbfiles: $(DBFILES)
	$(foreach DBF, $(DBFILES), $(RM) $(ETC)/$(DBF);)
	$(TCP) $^ $(ETC)
	$(foreach DBF, $(DBFILES), $(CHMOD) ug+w $(ETC)/$(DBF);)
	$(TOUCH) dbfiles

demos:
	$(foreach DEMO, $(DEMOS), \
		$(MAKE) -C ../$(DEMO)/$(HPROC);)
	$(TOUCH) demos

#-----------------------------------------------------------------------------

endif #InProcMake	# This line must be at EOF
