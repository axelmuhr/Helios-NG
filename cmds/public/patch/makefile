#
#	$Header: savesrc.sh,v 1.3 87/02/20 13:37:34 rnovak Exp $
#	$Log:	savesrc.sh,v $
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
ALL=patch
MANS=patch.1
RELFILE=patch.shar
# : Makefile.SH,v 1.1 86/08/01 20:18:35 lwall Exp $
#
# :	Makefile.SH,v $


CC = cc
bin = /u3/syseng/rnovak/bin
mansrc = /u3/syseng/rnovak/man
manext = n
CFLAGS =  -O
LDFLAGS = 


public = patch
private = 
manpages = patch.man
util = Makefile

c = patch.c

obj = patch.o

lintflags = -phbvxac

addedbyconf = Makefile.old bsd config.h config.sh eunice loc pdp11 usg v7

# grrr
SHELL = /bin/sh

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: $(public) $(private) $(util)
	touch all

patch: $(obj)
	$(CC) $(LDFLAGS) $(obj) $(libs) -o patch

# won't work with csh
install: patch
	export PATH || exit 1
	- mv $(bin)/patch $(bin)/patch.old
	- if test `pwd` != $(bin); then cp $(public) $(bin); fi
	cd $(bin); chmod 755 $(public)
	- if test `pwd` != $(mansrc); then \
for page in $(manpages); do \
cp $$page $(mansrc)/`basename $$page .man`.$(manext); \
done; \
fi

clean:
	rm -f *.o

realclean:
	rm -f patch *.o core $(addedbyconf)

# The following lint has practically everything turned on.  Unfortunately,
# you have to wade through a lot of mumbo jumbo that can't be suppressed.
# If the source file has a /*NOSTRICT*/ somewhere, ignore the lint message
# for that spot.

lint:
	lint $(lintflags) $(defs) $(c) > patch.fuzz

#
#	$Header: savesrc.sh,v 1.3 87/02/20 13:37:34 rnovak Exp $
#	$Log:	savesrc.sh,v $
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
	rm -f $(ALL)
