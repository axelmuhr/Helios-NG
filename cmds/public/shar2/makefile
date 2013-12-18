#
#	$Header: makehead,v 1.3 87/02/20 13:37:34 rnovak Exp $
#	$Log:	makehead,v $
# Revision 1.3  87/02/20  13:37:34  rnovak
# *** empty log message ***
# 
# Revision 1.3  87/02/20  13:37:34  rnovak
# extensive changes to handle the 'versions' style of makefile.
# 
#
# Permission to copy this material is granted as long as this notice
# remains with the material and the text is not sold for profit.
# Robert E. Novak {decwrl,ucbvax}!pyramid!rnovak 415/965-7200
#
#
.SUFFIXES: .h
.h.o:
	sed -e "s/extern//" -e "s/\/\*\///g" < $< > $*.c
	$(CC) -c $(CFLAGS) $*.c
	rm -f $*.c
ALL=shar2 unshar2
MANS=shar2.1 unshar2.1
RELFILE=shar2.shar
#
#	$Header: makebody,v 1.4 89/05/21 16:41:47 rnovak Exp $
#	$Log:	makebody,v $
# Revision 1.4  89/05/21  16:41:47  rnovak
# removed -n option on maninstall call, removed *.1 files on clobber
# 
# Revision 1.3  87/02/20  13:37:34  rnovak
# *** empty log message ***
# 
# Revision 1.3  87/02/20  13:37:34  rnovak
# extensive changes to handle the 'versions' style of makefile.
# 
#
# Permission to copy this material is granted as long as this notice
# remains with the material and the text is not sold for profit.
# Robert E. Novak {decwrl,ucbvax}!pyramid!rnovak 415/965-7200
#
#
all:	$(ALL)

BIN = /usr/local/bin
LBIN = $$HOME/bin

#LIB = /usr/local/lib/xxxx
#LLIB = $$HOME/xxxx
#LIBS = LIB1 LIB2

MAN = /usr/man/manl
LMAN = $$HOME/man

RELDIR=/tmp


install:	$(ALL) $(LIBS) $(MANS)
	for i in $(ALL) ; do install $$i $(BIN) ; done
#	for i in $(LIBS) ; do install $$i $(LIB) ; done
	for i in $(MANS) ; do maninstall $$i $(MAN) ; done

linstall:	$(ALL) $(LIBS) $(MANS)
	for i in $(ALL) ; do install $$i $(LBIN) ; done
#	-mkdir $(LLIB)
#	for i in $(LIBS) ; do install $$i $(LLIB) ; done
	for i in $(MANS) ; do maninstall $$i $(LMAN) ; done

release:
	make clobber
	rm -f makefile tags version.h
	rlog `printable *` | awk -f version.awk > version.h
	ci -f -q "-mNew Release" version.h
	co version.h
	shar `printable *` > $(RELDIR)/$(RELFILE)

LOCALDEFS=
local:
	make $(LOCALDEFS)

clean:
	rm -f *.o
clobber:	clean
	rm -f $(ALL) *.1
#
#  makefile for shar2
#

SRC	= shar2.c uushar.c unshar.sh
OBJ	= shar2.o uushar.o
DOC	= shar2.1 unshar.1 shar2.readme

shar2	: ${OBJ}
	$(CC) shar2.o uushar.o -o shar2

unshar2	: unshar2.c
	if test -f /vmunix; \
	then $(CC) $(CFLAGS) unshar2.c -s -o unshar2; \
	else $(CC) $(CFLAGS) -Dindex=strchr unshar2.c -s -o unshar2; \
	fi
shar2.1	:	shar2.tbl
	tbl shar2.tbl > shar2.1
unshar2.1:	unshar2.tbl
	tbl unshar2.tbl > unshar2.1
