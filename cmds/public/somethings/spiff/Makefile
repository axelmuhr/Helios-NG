#
#	START OF LOCAL CONFIGURATION INFORMATION
#	change the following lines to reflect your local environment
#

#
#	name of the directory into which the binary should be installed
#		used only when you use  'make install'
#
INSDIR=../..

#
#	choose one from each of 1) 2) and 3) below
#

#
#	1) SELECTION OF OPERATING SYSTEM VARIETY
#		choose a) b) or c)
#
# a) for BSD derivitives, enable the following line
OSFLAG=

# b) for XENIX systems, enable the following line
#OSFLAG=-DXENIX

# b) for other A.T. and T. derivitives, enable the following line
#OSFLAG=-DATT

#
# 	2) SELECTION OF TERMINAL CONTROL LIBRARY
#		choose either of a) b) or c)
#
# a) if you use termcap, enable the following lines
TFLAG=-DM_TERMCAP
TLIB=termcap

# b) if you are using terminfo on a XENIX machine, enable the following lines
#TFLAG=-DM_TERMINFO
#TLIB=tinfo

# c) if you use terminfo on any other type of machine,
#    enable the following lines
#TFLAG=-DM_TERMINFO
#TLIB=curses

#
#	3) SELECTION OF WINDOW MANAGER AVAILABILITY
#
# if you have the Bellcore's MGR window manager, enable the following lines
#VISFLAG=-DMGR
#VISLIB=/usr/public/pkg/mgr/lib/libmgr.a
#MGRINCDIR=-I/usr/public/pkg/mgr/include
#MGRINCS=$(MGRINC)/dump.h $(MGRINC)/term.h $(MGRINC)/restart.h $(MGRINC)/window.h

#
#	END OF LOCAL CONFIGRATION INFORMATION, the rest of this
#	file may be modified only at great risk
#		-- caveat hackor
#

#                        Copyright (c) 1988 Bellcore
#                            All Rights Reserved
#       Permission is granted to copy or use this program, EXCEPT that it
#       may not be sold for profit, the copyright notice must be reproduced
#       on copies, and credit should be given to Bellcore where it is due.
#       BELLCORE MAKES NO WARRANTY AND ACCEPTS NO LIABILITY FOR THIS PROGRAM.
#

CC=cc
OBJ= spiff.o output.o compare.o float.o strings.o exact.o miller.o parse.o command.o comment.o tol.o line.o token.o floatrep.o misc.o visual.o
CFILES= spiff.c output.c compare.c float.c strings.c exact.c miller.c parse.c command.c comment.c tol.c line.c floatrep.c token.c misc.c visual.c
HFILES=misc.h strings.h line.h float.h floatrep.h tol.h command.h comment.h token.h edit.h parse.h compare.h flagdefs.h exact.h miller.h visual.h output.h
OTHER=README Makefile Sample.1 Sample.2 Sample.3 Sample.4 paper.ms paper.out
MANPAGE=spiff.1

CFLAGS=-O $(OSFLAG) $(TFLAG) $(VISFLAG)

default: spiff

spiff: $(OBJ)
	$(CC) $(CFLAGS) -o spiff $(OBJ) $(VISLIB) -l$(TLIB)

spiff.o: spiff.c misc.h line.h token.h tol.h command.h edit.h parse.h compare.h flagdefs.h exact.h miller.h visual.h

visual.o: visual.c misc.h visual.h $(MGRINCS)
	$(CC) -c $(CFLAGS) $(MGRINCDIR) visual.c

misc.o: misc.c visual.h misc.h

parse.o:  parse.c misc.h line.h command.h float.h tol.h comment.h parse.h token.h flagdefs.h
	@echo compiler may report 4 statement not reached warning messages for parse.c
	$(CC) $(CFLAGS) -c parse.c

command.o: command.c float.h tol.h misc.h

comment.o: comment.c misc.h comment.h

tol.o: tol.c tol.h float.h

output.o: output.c output.h misc.h edit.h flagdefs.h

compare.o: compare.c misc.h strings.h float.h tol.h token.h line.h compare.h flagdefs.h
	@echo compiler may report 1 statement not reached warning message for compare.c
	$(CC) $(CFLAGS) -c compare.c

float.o: float.c misc.h strings.h float.h floatrep.h

floatrep.o: floatrep.c misc.h strings.h floatrep.h

strings.o: strings.c  misc.h strings.h

exact.o: exact.c exact.h misc.h edit.h

miller.o: miller.c miller.h misc.h edit.h token.h

token.o: token.c token.h misc.h

line.o: line.c line.h misc.h

clean:
	rm -f *.o
clobber: clean
	rm -f spiff
ci:
	ci -l -q  '-m $(CIMSG)' $(CFILES) $(HFILES) $(OTHER) $(MANPAGE)
col:
	co -l  $(CFILES) $(HFILES) $(OTHER) $(MANPAGE)
cirev:
	ci -l -r$(REV)  '-m $(CIMSG)' $(CFILES) $(HFILES) $(OTHER) $(MANPAGE)
cirel:
	ci -l -q  -sRel  $(CFILES) $(HFILES) $(OTHER) $(MANPAGE)
lint: 
	lint  $(CFLAGS) $(CFILES)
cpio:
	for i in $(CFILES) $(HFILES) $(OTHER) $(MANPAGE); do echo $$i; done | cpio -ocv  > spiff.cpio

cmd:
	-$(CMD) $(CFILES) $(HFILES) $(OTHER) $(MANPAGE)
install:	spiff $(MANPAGE)
	mv spiff $(INSDIR)/bin
	cp $(MANPAGE) $(INSDIR)/man/man1
	
