#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# TRAN processor specific make rules for the text files.
#
# File: /HSRC/XXX/YYY/makefile
#
# This file contains the extra definitions of variables and rules that are
# required to make a processor specific version of this component.
#
# If you wish to overide a targets rule that has been defined in the generic
# part of the makefile, you should bracket it with "ifneq ($(HPROC),YYY)" and
# supply the new rule in this makefile.
#
# This makefile is always executed within this processor specific subdir.
# All processor specific source files should be kept here.
# All intermediate, object and binary files should be made in here.
#
# SccsId: %W% %G%
# RcsId: $Id: makefile,v 1.7 1993/11/17 16:06:54 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = host.con2 install.bat install1.bat install2.bat in2

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(TOUCH) Xinstall

# Remove any processor specific files other than targets and objects here:
Xclean:
#	-$(RM) xxx


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		   $(XHEADERS.c) $(XHEADERS.m) \
		   $(XSOURCES.odd)


#Full set of library object files
XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)
# Or if you are not a library:
#XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	=

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile hostcon.dft hostcon.pc hydracon.dft install.bat install1.bat install2.bat in2 in


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

host.con2: hostcon.pc
	$(RM) -f $(HPROD)/host.con
	$(CP) hostcon.dft $(HPROD)/unixhost.con
	$(CP) hostcon.pc $(HPROD)/host.con
	$(RM) -f $(HPROD)/hydra.con
	$(CP) hydracon.dft $(HPROD)/hydra.con
	$(CHMOD) ug+w $(HPROD)/host.con* $(HPROD)/hydra.con
	$(TOUCH) host.con2

install.bat:
	$(CP) install.bat $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/install.bat

install1.bat:
	$(CP) install1.bat $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/install1.bat

install2.bat:
	$(CP) install2.bat $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/install2.bat

in2: in
	$(RM) -f $(HPROD)/system/in 
	$(CP) $< $(HPROD)/system
	$(CHMOD) ug+w $(HPROD)/system/in
	$(TOUCH) in2

