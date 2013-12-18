#-----------------------------------------------------------------------------
# TRAN processor specific make rules for the network/packages/farmlib component.
#
# File: /HSRC/network/packages/farmlib/TRAN/makefile
#-----------------------------------------------------------------------------

InProcMake = TRUE

# Get generic component makefile
include ../makefile

#-----------------------------------------------------------------------------
# Add processor specific targets:
XTARGETS = 

Xdefault: $(XTARGETS)


# Install the extra targets:
Xinstall: $(XTARGETS)
	-mkdir $(HPROD)/users
	-mkdir $(HPROD)/users/guest
	-mkdir $(HPROD)/users/guest/examples
	-mkdir $(HPROD)/users/guest/examples/farmlib
	-$(RM) $(HPROD)/users/guest/examples/farmlib/makefile
	$(CP) makefile.hel $(HPROD)/users/guest/examples/farmlib/makefile
	$(CHMOD) ug+w $(HPROD)/users/guest/examples/farmlib/makefile
	$(TOUCH) Xinstall

# Remove any processor specific files other than Xtargets and Xobjects here:
Xclean:
#	-$(RM) xxx


#-----------------------------------------------------------------------------
# Add to lists of files, used for archive, backup and cleaning purposes.

# All extra processor specific sources
XSOURCES	= $(XSOURCES.c) $(XSOURCES.a) \
		  $(XHEADERS.c) $(XHEADERS.m) \
		  $(XSOURCES.odd)


#Full set of library object files
#XOBJECTS	= $(XSOURCES.c:.c=.p) $(XSOURCES.a:.a=.p)
# Or if you are not a library:
XOBJECTS	= $(XSOURCES.c:.c=.o) $(XSOURCES.a:.a=.o)


# Fill in processor specific sources here:
XSOURCES.c	=

XSOURCES.a	=

XHEADERS.c	=

XHEADERS.m	=

XSOURCES.odd	= makefile makefile.hel


#-----------------------------------------------------------------------------
# Any additional processor specific rules and generic rule overides go here:

