#------------------------------------------------------------------------------
# Helios generic make system - PROCESSOR SPECIFIC COMPONENT MAKEFILE -
#-----------------------------------------------------------------------------
# ARM processor specific make rules for the standard distibuted text files.
#
# File: /HSRC/text/ARM/makefile
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
# RcsId: $Id: makefile,v 1.36 1993/11/18 16:38:31 nickc Exp $
#		(C) Copyright 1990 Perihelion Software
#
# WARNING: assumes you are using GNUmake.
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
#
# NB/ Make sure that either /hprod/createC40releaseTape.csh (for UNIX systems) or
# f:\apps\helios.c40\install\disk3.bat (for MS-DOS systems) is updated as well.
#

XTARGETS =      tdb416.con dsp1.con hepc2.con tdmb409.con template.con lsiqpc1.con \
		hydra.con hev40.con hesb40.con sangmega.con ttm60.con lsiqpc2.con \
		install.bat install1.bat install2.bat in \
		README flaws hepc spirit40 spirit40.con dsp1.rom

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	$(TOUCH) Xinstall

# Remove any processor specific files other than targets and objects here:
Xclean:
#	-$(RM) $(XTARGETS)


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

XSOURCES.odd	= makefile cshrc loginrc hostcon.dft default.rm default.map \
		  $(XTARGETS)



#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

.PHONY: $(XTARGETS)

tdb416.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

dsp1.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

hepc2.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

tdmb409.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

sangmega.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

ttm60.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

hydra.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

hev40.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

hesb40.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

lsiqpc1.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

lsiqpc2.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

spirit40.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

template.con: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

install.bat: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

install1.bat: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

install2.bat: $(HPROD)/$@
	$(RM) -f $(HPROD)/$@
	$(CP) $@ $(HPROD)
	-$(CHMOD) ug+w $(HPROD)/$@

in: 
	$(RM) -f $(HPROD)/system/in 
	$(CP) $@ $(HPROD)/system
	$(CHMOD) ug+w $(HPROD)/system/in

hepc: $(HPROD)/etc/$@
	$(RM) -fr $(HPROD)/etc/$@
	$(CP) -r $@ $(HPROD)/etc
	$(RM) -fr $(HPROD)/etc/RCS

spirit40: $(HPROD)/etc/$@
	$(RM) -fr $(HPROD)/etc/$@
	$(CP) -r $@ $(HPROD)/etc
	$(RM) -fr $(HPROD)/etc/RCS

README: $(HPROD)/etc/$@
	$(RM) -f $(HPROD)/etc/$@
	$(CP) $@ $(HPROD)/etc
	-$(CHMOD) ug+w $(HPROD)/etc/$@

flaws: $(HPROD)/etc/$@
	$(RM) -f $(HPROD)/etc/$@
	$(CP) $@ $(HPROD)/etc
	-$(CHMOD) ug+w $(HPROD)/etc/$@

dsp1.rom: $(HPROD)/etc/$@
	$(RM) -f $(HPROD)/etc/$@
	$(CP) $@ $(HPROD)/etc
	-$(CHMOD) ug+w $(HPROD)/etc/$@


