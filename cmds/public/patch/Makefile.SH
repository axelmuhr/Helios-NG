case $CONFIG in
    '') . config.sh ;;
esac
echo "Extracting Makefile (with variable substitutions)"
cat >Makefile <<!GROK!THIS!
# $Header: Makefile.SH,v 1.1 86/08/01 20:18:35 lwall Exp $
#
# $Log:	Makefile.SH,v $
#Revision 1.1  86/08/01  20:18:35  lwall
#Initial revision


CC = $cc
bin = $bin
mansrc = $mansrc
manext = $manext
CFLAGS = $iandd -O
LDFLAGS = $iandd

!GROK!THIS!
cat >>Makefile <<'!NO!SUBS!'

public = patch
private = 
manpages = patch.man
util = Makefile

c = patch.c

obj = patch.o

lintflags = -phbvxac

addedbyconf = Makefile.old bsd config.h config.sh eunice loc pdp11 usg v7 myread

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

clobber:
	rm -f patch *.o core $(addedbyconf)

# The following lint has practically everything turned on.  Unfortunately,
# you have to wade through a lot of mumbo jumbo that can't be suppressed.
# If the source file has a /*NOSTRICT*/ somewhere, ignore the lint message
# for that spot.

lint:
	lint $(lintflags) $(defs) $(c) > patch.fuzz
patch patch.o: config.h
config.h: Configure
	Configure
!NO!SUBS!
$eunicefix Makefile
