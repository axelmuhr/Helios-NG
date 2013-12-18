# Makefile for peepgen and genhdrs and testing thereof.
#
# Copyright (C) Advanced RISC Machines 1991.
#
# This makefile is intended to be used as follows:
#
# 1/  Check the definitions of places containing source files and tools.
#     Modify them as required (no modification should be necessary if you
#     are using the supplied release structure).
#
# 2/  Do 'make' or 'make utils'. The warning outputs from topcc can be
#     ignored.
#
# 3/  Do 'make tests' to check that the utilities have built correctly.
#
# 4/  Do 'make tidy' to clean up the results of testing, leaving only the
#     generated binaries
#
# 5/  Do 'make clean' if you need to start again (removing all generated
#     files).

.SILENT:

# Headers included by peepgen
MCDPRIV=../arm/h/mcdpriv
JOPCODE=../mip/h/jopcode

# Tools and their command-line options
TOPCC=../../topcc/topcc
CC=cc
CFLAGS=-O

utils: genhdrs peepgen
        echo ""
        echo "Don't forget to test the tools using 'make tests'"
        echo "Tidy up using 'make tidy'"
        echo "Start again using 'make clean'"
        echo ""

sources: genhdrs.c peepgen.c mcdpriv.h jopcode.h

genhdrs: genhdrs.c
        echo "Building genhdrs..."
        $(CC) -o genhdrs $(CFLAGS) genhdrs.c && echo "genhdrs built OK"

genhdrs.c: c/genhdrs
        $(TOPCC) c/genhdrs genhdrs.c

peepgen: peepgen.c mcdpriv.h jopcode.h
        echo "Building peepgen..."
        $(CC) -o peepgen $(CFLAGS) peepgen.c && echo "peepgen built OK"

peepgen.c: c/peepgen
        $(TOPCC) c/peepgen peepgen.c

mcdpriv.h: $(MCDPRIV)
        $(TOPCC) $(MCDPRIV) mcdpriv.h

jopcode.h: $(JOPCODE)
        $(TOPCC) $(JOPCODE) jopcode.h

#
# Testing the tools...
#

# List and location of the standard ANSI headers
HDRFILES = \
assert.h ctype.h  errno.h  float.h limits.h locale.h math.h setjmp.h \
signal.h stdarg.h stddef.h stdio.h stdlib.h string.h time.h

AH=../cl/stdh/
ANSIHDRS = \
$(AH)assert.h $(AH)ctype.h  $(AH)errno.h  $(AH)float.h  $(AH)limits.h \
$(AH)locale.h $(AH)math.h   $(AH)setjmp.h $(AH)signal.h $(AH)stdarg.h \
$(AH)stddef.h $(AH)stdio.h  $(AH)stdlib.h $(AH)string.h $(AH)time.h

# ... and locations of the compiler error message definitions...
LH=../cfe/h/
PH=../mip/h/
MH=../arm/h/

# ... and the location of the peephole patterns.
PEEP=../arm

# Test data...
PEEPPAT=../ccarm/c/peeppat
HEADERS=../ccarm/c/headers
ERRORS=../ccarm/h/errors

tests test: errors.h headers.c peeppat.c
        echo "Comparing genhdrs output with expected values:"
        - diff $(ERRORS) errors.h && echo " - errors.h generated OK by genhdrs"
        tail +3 $(HEADERS) > h0
        tail +3 headers.c  > h1
        - diff h0 h1 && echo " - headers.c generated OK by genhdrs"
        rm h0 h1
        echo "Comparing peepgen output with expected value:"
        - diff $(PEEPPAT) peeppat.c && echo " - peeppat.c generated OK by peepgen"
        echo ""

errors.h headers.c: $(ANSIHDRS) genhdrs ${PH}miperrs ${LH}feerrs ${MH}mcerrs
        echo "Generating test outputs from genhdrs..."
        genhdrs -oheaders.c -eerrors.h -q${PH}miperrs -q${LH}feerrs \
                -q${MH}mcerrs -s -I$(AH) $(HDRFILES)

peeppat.c: $(PEEP)/peeppat
        echo "Generating test output from peepgen..."
        peepgen $(PEEP)/peeppat peeppat.c

tidy:
        rm -f peepgen.c peepgen.o jopcode.h mcdpriv.h genhdrs.c genhdrs.o \
        peeppat.c headers.c errors.h

clean:
        rm -f peepgen.c peepgen.o jopcode.h mcdpriv.h genhdrs.c genhdrs.o \
        peeppat.c headers.c errors.h peepgen genhdrs
