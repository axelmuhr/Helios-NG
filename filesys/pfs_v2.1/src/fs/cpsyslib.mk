CP	=	cp

HELIOS	=	/helios
TMP	=	/ram
SYSLIB	=	$(HELIOS)/lib
TLIB	=	$(TMP)/lib

TLIBS	=	$(TLIB)/kernel.def \
		$(TLIB)/syslib.def \
		$(TLIB)/util.def \
		$(TLIB)/s0.o

$(TLIB)/%.def	:	$(SYSLIB)/%.def
	$(CP) $< $@

$(TLIB)/%.o	:	$(SYSLIB)/%.o
	$(CP) $< $@

all	: $(TLIBS)
