#
# Makefile for the C Front End to the Norcroft compiler
#
# Copyright (c) 1992 Perihelion Software Ltd
# All Rights Reserved
#
# Author: N Clifton
# RCS Id: $Id: makefile,v 1.6 1993/05/12 09:18:20 nickc Exp $
#

# object files

OBJS = 	$(DEST)/lex.o      \
	$(DEST)/pp.o       \
	$(DEST)/sem.o      \
	$(DEST)/simplify.o \
	$(DEST)/syn.o      \
	$(DEST)/vargen.o

default:
	cd .. ; make all

all: $(OBJS)
	
install:
	cd ..; make install

clean:
	rm -f *.o
	rcsclean
	co -u -q RCS/*

#dependencies

$(DEST)/lex.o: 		 lex.c
	$(CC) $(CFLAGS)  $(DEFINES) lex.c -c
	$(MV) lex.o      $(DEST)/lex.o

$(DEST)/pp.o: 		 pp.c
	$(CC) $(CFLAGS)  $(DEFINES) pp.c -c
	$(MV) pp.o       $(DEST)/pp.o

$(DEST)/sem.o:		 sem.c
	$(CC) $(CFLAGS)  $(DEFINES) sem.c -c
	$(MV) sem.o      $(DEST)/sem.o

$(DEST)/simplify.o:	 simplify.c
	$(CC) $(CFLAGS)  $(DEFINES) simplify.c -c
	$(MV) simplify.o $(DEST)/simplify.o

$(DEST)/syn.o:		 syn.c
	$(CC) $(CFLAGS)  $(DEFINES) syn.c -c
	$(MV) syn.o      $(DEST)/syn.o

$(DEST)/vargen.o:	vargen.c
	$(CC) $(CFLAGS) $(DEFINES) vargen.c -c
	$(MV) vargen.o  $(DEST)/vargen.o
