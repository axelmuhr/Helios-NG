#	
# 
#
.SUFFIXES: .h
.h.o:
	sed -e "s/extern//" -e "s/\/\*\///g" < $< > $*.c
	$(CC) -c $(CFLAGS) $*.c
	rm -f $*.c
SHELL=/bin/sh
#	
# 
#
AR=ar
SHAR=shar2
RANLIB=ranlib
ALL = tools.a
all:	$(ALL)

BIN = ../../bin
LBIN = ../../bin

LIB = ../../lib
LLIB = ../../lib
LIBS = tools.a
INSTALL = mv
MANINSTALL = cp

MAN = ../../man/man3
LMAN = ../../man/man3
MANS = max.3 min.3 strcar.3 strcdr.3 mstrcpy.3 efopen.3s fgetsmod.3s \
	getopt.3 traverse.3

CFLAGS = -I../../binsrc/include

RELDIR=/tmp
RELFILE=toolslib.shar

TOOLS = max.o mstrcpy.o strcar.o strcdr.o min.o \
	efopen.o fgetsmod.o getopt.o traverse.o version.o
tools.a:	$(TOOLS)
	$(AR) cr tools.a $(TOOLS)
	$(RANLIB) tools.a

min.3:
	ln max.3 min.3

strcdr.3:
	ln strcar.3 strcdr.3

install:	$(ALL) $(LIBS) $(MANS)
#	for i in $(ALL) ; do $(INSTALL) $$i $(BIN) ; done
	for i in $(LIBS) ; do $(INSTALL) $$i $(LIB) ; done
	for i in $(LIBS) ; do $(RANLIB) $(LIB)/$$i ; done
	for i in $(MANS) ; do $(MANINSTALL) $$i $(MAN) ; done

linstall:	$(ALL) $(LIBS) $(MANS)
#	for i in $(ALL) ; do $(INSTALL) $$i $(LBIN) ; done
	-mkdir $(LLIB)
	for i in $(LIBS) ; do $(INSTALL) $$i $(LLIB) ; done
	for i in $(LIBS) ; do $(RANLIB) $(LLIB)/$$i ; done
	for i in $(MANS) ; do maninstall $$i $(LMAN) ; done

release:
	make clobber
	rm -f makefile tags version.h
	rlog `printable *` | awk -f version.awk > version.h
	ci -f -q "-mNew Release" version.h
	co version.h
	$(SHAR) `printable *` > $(RELDIR)/$(RELFILE)

local:
	make $(LOCALDEFS)

clean:
	rm -f *.o
clobber:	clean
	rm -f $(ALL)
