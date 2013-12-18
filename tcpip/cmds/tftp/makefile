# $Header: /hsrc/tcpip/cmds/tftp/RCS/makefile,v 1.2 1991/10/04 10:06:07 nickc Exp $
#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.2 (Berkeley) 2/6/86
#
SRC=	main.c tftp.c tftpsubs.c
COBJ=	main.o tftp.o tftpsubs.o
DOBJ=	tftpd.o tftpsubs.o
OBJ=	$(COBJ) $(DOBJ)
#INC=   -I../include -I/usr/local/include
DEF=	-DSYSLOG -D_BSD -DCHROOT
# CFLAGS= -g $(INC) $(DEF)
CFLAGS= $(INC) $(DEF) -wA -FA
LLIB=   # /usr/local/lib/libsyslog.a -lresolv
#LIB=    -lBSD # $(LLIB)
LIB=	-lbsd
DESTDIR=/usr/local/bin
ALL=	tftp tftpd
CC=	c

all:		$(ALL)

tftp:		$(COBJ)
		$(CC) -V -O $(COBJ) -o tftp $(LIB)

tftpd:		$(DOBJ)
		$(CC) -V -O $(DOBJ) -o tftpd $(LIB)

clean:
		-rm -f $(OBJ) core tftp.shar

install:
		cpset tftp $(DESTDIR) 711 bin bin
		cpset tftpd $(DESTDIR) 6711 tftpd tftpd
