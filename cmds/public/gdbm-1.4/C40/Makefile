# Makefile for gdbm.
#
# for System V, add "-DSYSV" to CFLAGS.  Also, if you need alloca(),
# add "-lPW" to LIBS.
#
# for BSD, nothing is required
#
# for gcc, uncomment the CC = gcc line.

CC      = ccc40
CFLAGS  = -D_BSD -wadfpv
LDFLAGS =
LIBS    = -lbsd
DBM     = #-ldbm
NDBM    = #-lc
LIBC    = #-lc

LINTFLAGS =

DBM_CF = dbminit.c delete.c fetch.c store.c seq.c

NDBM_CF = dbmopen.c dbmdelete.c dbmfetch.c dbmstore.c dbmseq.c \
	dbmclose.c dbmdirfno.c dbmpagfno.c

GDBM_CF = gdbmopen.c gdbmdelete.c gdbmfetch.c  gdbmstore.c gdbmclose.c \
	gdbmreorg.c gdbmseq.c \
	bucket.c falloc.c findkey.c global.c hash.c update.c version.c

HFILES = gdbmdefs.h extern.h gdbmerrno.h systems.h dbm.h ndbm.h


MSCFILES = COPYING ChangeLog Makefile README gdbm.proto conv2gdbm.c

TESTFILES = testdbm.c testndbm.c testgdbm.c


DBM_OF = dbminit.o delete.o fetch.o store.o seq.o

NDBM_OF = dbmopen.o dbmdelete.o dbmfetch.o dbmstore.o dbmseq.o \
	dbmclose.o dbmdirfno.o dbmpagfno.o

GDBM_OF = gdbmopen.o gdbmdelete.o gdbmfetch.o  gdbmstore.o gdbmclose.o \
	gdbmreorg.o gdbmseq.o \
	bucket.o falloc.o findkey.o global.o hash.o update.o version.o


LIBDIR  = /hprod/C40/lib
INCLUDE = /hprod/C40/include

default:	
	@echo "\"make libgdbm.a\" to make the library"
	@echo "\"make testgdbm\" to make the gdbm test program"
	@echo "\"make testdbm\" to make the dbm compatability test program"
	@echo "\"make testndbm\" to make the ndbm compatability test program"
	@echo "\"make allgdbm\" to make all of the above"
	@echo "\"make tdbm\" to make the dbm test program (uses dbm, not gdbm)"
	@echo "\"make tndbm\" to make the ndbm test program (uses ndbm)"
	@echo "\"make alldbm\" to make the two previous programs"
	@echo "\"make conv2gdbm\" to make the conversion program"
	@echo "\"make all\" to make all of the above"
	@echo "\"make install\" to install the library"
	@echo "\"make dist\" to make gdbm.tar.Z distribution file"

all:	allgdbm conv2gdbm alldbm

alldbm: tdbm #tndbm

allgdbm: libgdbm.a testgdbm testdbm testndbm

libgdbm.a:	$(DBM_OF) $(NDBM_OF) $(GDBM_OF)
	rm -f libgdbm.a
	cat $(DBM_OF) $(NDBM_OF) $(GDBM_OF) > libgdbm.a

gdbm.h:	gdbm.proto gdbmerrno.h
	cp gdbm.proto gdbm.h
	chmod +w gdbm.h
	grep _ gdbmerrno.h >> gdbm.h

install: gdbm.a gdbm.h
	cp libgdbm.a /hprod/C40/lib
	cp gdbm.h /hsrc/include

testgdbm: testgdbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o testgdbm testgdbm.o -lgdbm $(LIBS)

testdbm: testdbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o testdbm testdbm.o -lgdbm $(LIBS)

tdbm: testdbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o tdbm testdbm.o $(DBM) $(LIBS)

testndbm.o: testndbm.c
	$(CC) -c $(CFLAGS) -DGNU testndbm.c

testndbm: testndbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o testndbm testndbm.o -lgdbm $(LIBS)

tndbm.o: testndbm.c
	rm -f tndbm.c
	ln testndbm.c tndbm.c
	$(CC) $(CFLAGS) -DGNU -c tndbm.c
	rm -f tndbm.c

tndbm: tndbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o tndbm tndbm.o $(LIBS) $(NDBM)

conv2gdbm: conv2gdbm.o gdbm.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o conv2gdbm conv2gdbm.o $(DBM) $(LIBC) \
	-lgdbm $(LIBS)

lintgdbm: 
	lint $(LFLAGS) $(DBM_CF) $(NDBM_CF) $(GDBM_CF) testgdbm.c

clean:
	rm -f $(DBM_OF) $(NDBM_OF) $(GDBM_OF) gdbm.h -lgdbm \
	testdbm testndbm testgdbm testdbm.o testndbm.o testgdbm.o \
	tdbm tndbm tndbm.o tndbm.c conv2gdbm conv2gdbm.o

dist:
	OF=`sed -n 's/.*\([0-9][0-9]*\.[0-9][0-9]*\).*/gdbm-\1/p' version.c` \
	;mkdir $$OF   		  \
	;ln $(MSCFILES) $(DBM_CF) $(NDBM_CF) $(GDBM_CF) $(HFILES) \
	    $(TESTFILES)  $$OF	  \
	;tar cvohf $$OF.tar $$OF  \
	;compress $$OF.tar        \
	;rm -rf $$OF


# dbm files
dbminit.o:	gdbmdefs.h extern.h gdbmerrno.h systems.h
delete.o:	gdbmdefs.h extern.h
fetch.o:	gdbmdefs.h extern.h
store.o:	gdbmdefs.h extern.h
seq.o:		gdbmdefs.h extern.h

# ndbm files
dbmopen.o:	gdbmdefs.h extern.h gdbmerrno.h systems.h
dbmdelete.o:	gdbmdefs.h extern.h
dbmfetch.o:	gdbmdefs.h extern.h
dbmstore.o:	gdbmdefs.h extern.h
dbmseq.o:	gdbmdefs.h extern.h
dbmclose.o:	gdbmdefs.h systems.h
dbmpagfno.o:	gdbmdefs.h extern.h
dbmdirfno.o:	gdbmdefs.h extern.h


# gdbm files
gdbmclose.o:	gdbmdefs.h systems.h
gdbmdelete.o:	gdbmdefs.h gdbmerrno.h systems.h
gdbmfetch.o:	gdbmdefs.h gdbmerrno.h systems.h
gdbmopen.o:	gdbmdefs.h gdbmerrno.h systems.h 
gdbmreorg.o:	gdbmdefs.h gdbmerrno.h systems.h extern.h
gdbmseq.o:	gdbmdefs.h systems.h
gdbmstore.o:	gdbmdefs.h gdbmerrno.h systems.h

# gdbm support files
bucket.o:	gdbmdefs.h systems.h
falloc.o:	gdbmdefs.h systems.h
findkey.o:	gdbmdefs.h systems.h
global.o:	gdbmdefs.h gdbmerrno.h 
hash.o:		gdbmdefs.h
update.o:	gdbmdefs.h systems.h
version.o:
extern.h:	

# other programs
testgdbm.o:	gdbmdefs.h extern.h gdbmerrno.h systems.h
testdbm.o:
testndbm.o:	ndbm.h
tdbm.o:
conv2gdbm.o:	gdbm.h
