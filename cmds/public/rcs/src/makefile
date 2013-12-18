#$Header: /usr/src/local/bin/rcs/src/RCS/Makefile,v 1.12 89/04/10 10:41:04 rlw Exp Locker: narten $
# Copyright (C) 1982, 1988, 1989 Walter Tichy
#   Distributed under license by the Free Software Foundation, Inc.
#
# This file is part of RCS.
#
# RCS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 1, or (at your option)
# any later version.
#
# RCS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RCS; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Report problems and direct all questions to:
#
#    rcs-bugs@cs.purdue.edu
#

.SUFFIXES: .c .o

.c.o:
	cc $(CFLAGS) -s $*.s $*.c
	asm -p -o $*.o $*.s
	rm $*.s

LIB = /helios/lib
LIBFILES = $(LIB)/cstart.o

#               INSTRUCTIONS
#               ============
# 0.  If you are porting to different hardware,
#     check the instructions in rcsbase.h
#
# Note: instructions 1-5 are normally taken care of by the calling make,
# which passes in DIFF, DIFF3, RCSDIR, OLDDIR, OS, and LOCKING.
#
# 1. Early version of diff did not support options needed by RCS.
#    Newer versions of diff, support them and diff is no longer distributed with
#    RCS. The following definitions apply only if you have both old versions
#    and new versions of diff, and you want RCS to use the new versions.
#    The default should be :
#

DIFF      = /helios/local/bin/gdiff
DIFF3     = /usr/lib/diff3

# 2.  Figure out where to put the RCS commands; define RCSDIR accordingly.

RCSDIR  = /helios/local/bin


# 3.  Figure out where to put your old RCS commands; define OLDDIR
#     accordingly. This is not needed if you don't have an old release of RCS.
OLDDIR    = /helios/old/bin

# 4.  Set the operating system variable OS. If you run on 4.1 BSD, do not
#     define OS:
#     OS  =
#     If you run on USG Unix or UN*X, use:
#     OS  = -DUSG
#     If you run on Version 6 Unix, use:
#     OS  = -DV6
#     If you run on 4.2 or 4.3 BSD, use:
#     OS = -DV4_2BSD
OS        = -DUSG -DHELIOS -DPOSIX

#     Also, set SIGNAL_TYPE to be what your signal() handlers yield.
#     Old-fashioned signal handlers yield 'int'; the newer ones 'void'.

SIGNAL_TYPE = void

# 5.  Decide whether you want strict locking as the default or not.
#     If you want strict locking, use:
#     LOCKING = 1
#     otherwise:
#     LOCKING = 0
LOCKING   = 0

# 6.  Decide what loader flags you need, and define LDFLAGS accordingly.
#     On the Vax you need none, on PDP-11s with split i/d you need -i.
#LDFLAGS  = -i
LDFLAGS   =

# 7.  RCS can maintain a log of the use of the co and ci commands. If you
#     want the logging, you must
#     a) define SNOOP (that's where the process that writes the log goes),
#     b) define SNOOPFILE (that's the file where SNOOP writes the log;
#        it must be owned by the same user that owns SNOOP),
#     c) include SNOOP and SNOOPFILE in the CFLAGS macro.
#     If you don't want the logging of command usage, omit the SNOOP and
#     SNOOPFILE definitions from CFLAGS.
#     The two variants of the CFLAG macros are given below.
#SNOOPFILE = /usr/log/rcslog/log
#SNOOP	= /usr/local/bin/snoop

DEFINES= $(OS) -DSIGNAL_TYPE=$(SIGNAL_TYPE) -DSTRICT_LOCKING=$(LOCKING) -DDIFF='"$(DIFF)"' -DCO='"${RCSDIR}/co"' -DMERGE='"${RCSDIR}/merge"' -Dxputer
CFLAGS	= -wfd $(DEFINES)
LINTFLAGS = -c -u
LINT = lint $(LINTFLAGS) -Dlint $(DEFINES)

# 8.  If you don't have release 2 RCS files around (e.g., if you don't
#     have release 2 of RCS), remove the word -DCOMPAT2 from whatever
#     variant of CFLAGS you selected. -DCOMPAT2 assures that release 2
#     RCS files can still be read. (After all release 2 RCS files have
#     been updated with release 3 or 4 ci or rcs, you can remake RCS without
#     -DCOMPAT2.) Also, make sure that the co commands at the end of this
#     file are commented out; otherwise, Make will complain.
#
# 9.  Now you are ready. Try to make 'depend' and 'all'. If all went well, make
#     'install' if you want no logging of command usage, 'installog' otherwise.
#
# 10. IMPORTANT! When you have RCS installed, rename old
#     release 2 RCS files as follows (if you have any):
#     If the working file was "f.c" and the RCS file "f.c.v", rename the
#     RCS file to "f.c,v". If the working file was "f.c" and the RCS file
#     "f.v", rename the RCS file "f.c,v". Thus, suffixes are no longer
#     dropped and RCS files end in ",v" rather than ".v".
#
# 11. If you want to maintain RCS with itself, be sure you preserve the
#     original revision numbers, dates, etc. This is done by checking the
#     files in with the -k option. When done, remove the comments from the
#     co-rules at the end of this file.
#
BCOMMANDS   =   ci ident rcs rcsdiff rcsmerge rlog co
# binary comands

RCSCOMMANDS = merge $(BCOMMANDS)
# all commands

BOBJECTS = ci.o co.o ident.o rcs.o rcsdiff.o rcsedit.o\
	rcsfcmp.o rcsfnms.o rcsgen.o rcskeep.o rcskeys.o rcslex.o rcsmerge.o\
	rcsrev.o rcssyn.o rcsutil.o rlog.o snoop.o\
	rcssynTST.o rcsrevTST.o rcsfnmsTST.o rcsfcmpTST.o rcskeepTST.o

all:	$(BCOMMANDS)

#INSTALL = install
#INSTALLFLAGS = -m 775 -o root -s

INSTALL = cp
INSTALLFLAGS = 

$(BOBJECTS):	rcsbase.h

partime.o maketime.o co.o ci.o rcs.o rlog.o rcsutil.o:        time.h

install:	all 
#	Save the old commands if you have any.
#	-cd ${RCSDIR}; mv $(RCSCOMMANDS) ${RCSDIR}$(OLDDIR)
	${INSTALL} $(INSTALLFLAGS) ci ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) ident ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) rcs ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) rcsdiff ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) rcsmerge ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) rlog ${RCSDIR}
	${INSTALL} $(INSTALLFLAGS) co ${RCSDIR}
#	${INSTALL} $(INSTALLFLAGS) merge ${RCSDIR}
#	${INSTALL} $(INSTALLFLAGS) rcsclean ${RCSDIR}
#	${INSTALL} $(INSTALLFLAGS) rcsfreeze.sh ${RCSDIR}/rcsfreeze

installog:	installsnoop install

installsnoop:	snoop
	install -s snoop ${RCSDIR}$(SNOOP)
	chmod u+s ${RCSDIR}$(SNOOP)
	touch ${RCSDIR}$(SNOOPFILE)
	chmod 644 ${RCSDIR}$(SNOOPFILE)
# CAUTION: You may have to do a chown of SNOOP and SNOOPFILE (if not owned by root).
lint: ci.lint co.lint ident.lint rlog.lint rcs.lint rcsdiff.lint rcsmerge.lint snoop.lint

clean:
	-rm -f *.o; rm -f $(RCSCOMMANDS) snoop

CIFILES = ci.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcskeys.o rcsrev.o rcsutil.o rcsfnms.o partime.o maketime.o rcskeep.o rcsfcmp.o

CISRC = ci.c rcslex.c rcssyn.c rcsgen.c rcsedit.c rcskeys.c rcsrev.c rcsutil.c rcsfnms.c partime.c maketime.c rcskeep.c rcsfcmp.c

#ci.lint:
#	$(LINT) $(CISRC)

ci:	$(CIFILES)
	asm -o ci $(LIBFILES) $(CIFILES)

COFILES = co.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcskeys.o rcsrev.o rcsutil.o rcsfnms.o partime.o maketime.o

COSRC = co.c rcslex.c rcssyn.c rcsgen.c rcsedit.c rcskeys.c rcsrev.c rcsutil.c rcsfnms.c partime.c maketime.c
#co.lint:
#	$(LINT) $(COSRC)
co:	$(COFILES)
	asm -o co $(LIBFILES) $(COFILES)

ident:	ident.o rcskeys.o
	asm -o ident $(LIBFILES) ident.o rcskeys.o

#ident.lint:
#	$(LINT) ident.c rcskeys.c

merge:	merge.sh
	sed -e '/^#/d' \
		-e 's:DIFF=.*$$:DIFF=$(DIFF):' \
		-e 's:DIFF3=.*$$:DIFF3=$(DIFF3):' \
		merge.sh > merge
#	This takes out the comment lines and substitutes in DIFF and DIFF3.
#	(Comments are not permitted in some older shells.)
	chmod 755 merge

RLOG = rlog.o rcslex.o rcssyn.o rcsrev.o rcsutil.o partime.o maketime.o rcsfnms.o
RLOGSRC = rlog.c rcslex.c rcssyn.c rcsrev.c rcsutil.c partime.c maketime.c rcsfnms.c
rlog:	$(RLOG)
	asm -o rlog $(LIBFILES) $(RLOG)

#rlog.lint:
#	$(LINT) $(RLOGSRC)

RCS = rcs.o rcslex.o rcssyn.o rcsrev.o rcsutil.o rcsgen.o rcsedit.o rcskeys.o rcsfnms.o

RCSSRC = rcs.c rcslex.c rcssyn.c rcsrev.c rcsutil.c rcsgen.c rcsedit.c rcskeys.c rcsfnms.c
rcs:	$(RCS)
	asm -o rcs $(LIBFILES) $(RCS)

#rcs.lint:
#	$(LINT) $(RCSSRC)

rcsclean:	rcsclean.sh
	sed -e '/^#/d' rcsclean.sh > rcsclean
#	This takes out the comments (not permitted in older shells).
	chmod 755 rcsclean

RCSDIFF = rcsdiff.o rcsutil.o rcsfnms.o rcsrev.o rcssyn.o rcslex.o
RCSDIFFSRC = rcsdiff.c rcsutil.c rcsfnms.c rcsrev.c rcssyn.c rcslex.c

rcsdiff:	$(RCSDIFF)
	asm -o rcsdiff ${LIBFILES} $(RCSDIFF)

#rcsdiff.lint:
#	$(LINT) $(RCSDIFFSRC)
RCSMERGE = rcsmerge.o rcsutil.o rcsfnms.o rcsrev.o rcssyn.o rcslex.o
RCSMERGESRC = rcsmerge.c rcsutil.c rcsfnms.c rcsrev.c rcssyn.c rcslex.c
rcsmerge:	$(RCSMERGE)
	asm -o rcsmerge ${LIBFILES} $(RCSMERGE)
#rcsmerge.lint:
#	$(LINT) $(RCSMERGESRC)
	
snoop:	snoop.o
	asm -o snoop ${LIBFILES} snoop.o
#snoop.lint:
#	$(LINT) snoop.c

SOURCE=	ci.c co.c curdir.c ident.c maketime.c partime.c rcs.c \
	rcsclean.c rcsdiff.c rcsedit.c rcsfcmp.c rcsfnms.c rcsgen.c \
	rcskeep.c rcskeys.c rcslex.c rcsmerge.c rcsrev.c rcssyn.c rcsutil.c \
	rlog.c snoop.c

HFILES=	rcsbase.h time.h

depend:	${SOURCE} ${HFILES}
	(sed '/^# DO NOT DELETE THIS LINE/q' Makefile && \
	 cc -M ${CFLAGS} ${SOURCE} | sed 's/\.\///; /\//d' \
	) >Makefile.new
	cp Makefile Makefile.bak
	cp Makefile.new Makefile
	rm -f Makefile.new


# DO NOT DELETE THIS LINE - 
ci.o: ci.c
ci.o: rcsbase.h
ci.o: time.h
co.o: co.c
co.o: rcsbase.h
co.o: time.h
curdir.o: curdir.c
curdir.o: rcsbase.h
ident.o: ident.c
ident.o: rcsbase.h
maketime.o: maketime.c
maketime.o: rcsbase.h
maketime.o: time.h
partime.o: partime.c
partime.o: time.h
rcs.o: rcs.c
rcs.o: rcsbase.h
rcsclean.o: rcsclean.c
rcsclean.o: rcsbase.h
rcsdiff.o: rcsdiff.c
rcsdiff.o: rcsbase.h
rcsedit.o: rcsedit.c
rcsedit.o: rcsbase.h
rcsfcmp.o: rcsfcmp.c
rcsfcmp.o: rcsbase.h
rcsfnms.o: rcsfnms.c
rcsfnms.o: rcsbase.h
rcsgen.o: rcsgen.c
rcsgen.o: rcsbase.h
rcskeep.o: rcskeep.c
rcskeep.o: rcsbase.h
rcskeys.o: rcskeys.c
rcskeys.o: rcsbase.h
rcslex.o: rcslex.c
rcslex.o: rcsbase.h
rcsmerge.o: rcsmerge.c
rcsmerge.o: rcsbase.h
rcsrev.o: rcsrev.c
rcsrev.o: rcsbase.h
rcssyn.o: rcssyn.c
rcssyn.o: rcsbase.h
rcsutil.o: rcsutil.c
rcsutil.o: rcsbase.h
rlog.o: rlog.c
rlog.o: time.h
rlog.o: rcsbase.h
snoop.o: snoop.c
snoop.o: rcsbase.h
