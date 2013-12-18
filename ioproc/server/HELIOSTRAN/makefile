#-----------------------------------------------------------------------------
# Helios generic make system - GENERIC (NATIVE) COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# Helios generic component make rules for: server
#
# File: /HSRC/ioproc/server/makefile
#
# These are the host/processor independent rules and variables for this
# component of Helios. It is included by the processor specific makefile
# That may add to some of its functionality. You should only alter this
# makefile by using "ifneq ($(HPROC),YYY)" clauses for a specific processor.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.3 1993/05/21 10:33:51 bart Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

# Manditory variables for a component:
# name of this dir relative to $(HSRC)

COMPONENT = ioproc/server#
SRCHOME   = $(HSRC)/$(COMPONENT)#

# Manditory includes
include $(HSRC)/makeinc/$(HHOST).mak	# Default host system variables

# Manditory phoney targets
.PHONY: default clean install 
# "default"    - should make this component in its proc subdir
# "install"    - copy the new component to $(HPROD) production directory
# "clean"      - remove all intermediate and executable files from proc subdir
#
# Processor specific sources are held in the processor specific subdir.,
# generic ones in the directory above it.

VPATH := $(HSRC)/$(COMPONENT)

#-----------------------------------------------------------------------------
# Generic component make rules and variables added below here

# Note all final generic targets here
 TARGETS = #server hydra hydramon serverwindow

default : $(TARGETS) 

install	: $(TARGETS) 
	$(TOUCH) install

clean	: 
	-$(RM)	$(TARGETS) mkout \
		core ../mkout install 

#-----------------------------------------------------------------------------
# for archive, backup and cleaning purposes,
# note all generic SOURCE and OBJECT files:

# Note all sources (used by main makefile srctar)
SOURCES		= $(SOURCES.c) $(HEADERS.c) $(SOURCES.odd)

# C source files:
SOURCES.c =	server.c terminal.c tload.c cofuns.c debug.c gem.c \
		linkio.c devices.c files.c smlgen.c tpc.c

# C header files:
HEADERS.c =	barthdr debugopt.h defines.h fundefs.h protocol.h \
		sccs.h server.h structs.h sml.h

GENHEADERS.c =	../barthdr ../debugopt.h ../defines.h ../fundefs.h \
		../protocol.h ../sccs.h ../server.h ../structs.h \
		../sml.h

# Any other source files:
SOURCES.odd	= makefile


# The generic server bits

server.o : sccs.h

SERVOBJ	= server.o files.o devices.o tload.o cofuns.o \
		  debug.o terminal.o linkio.o
