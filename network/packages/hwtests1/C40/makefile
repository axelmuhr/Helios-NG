#-----------------------------------------------------------------------------
# TRAN processor specific make rules for the network/packages/hwtests1 component.
#
# File: /HSRC/network/packages/hwtests1/TRAN/makefile
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
	test -d  $(HPROD)/local/tests || mkdir -p $(HPROD)/local/tests
	test -d  $(HPROD)/local/tests/hwtests1 || mkdir $(HPROD)/local/tests/hwtests1
	cat $(HSRC)/$(COMPONENT)/$(HPROC)/makefile.hel > $(HPROD)/local/tests/hwtests1/makefile
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

