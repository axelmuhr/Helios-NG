#
# @(#)Makefile	1.6 87/11/30 3.9 RPCSRC
#
#   Build and install everything.
#
# These directories are presumed to exist in DESTDIR:
#    /usr/lib /usr/bin /usr/include
#
DESTDIR=
CFLAGS=-O
MAKE=make

SUBDIR= rpcgen etc rpcsvc

all install:	rpclib ${SUBDIR}

rpclib:	FRC
	@echo "Building and installing RPC library"
	cd rpc; $(MAKE) ${MFLAGS} all DESTDIR=${DESTDIR}; \
        $(MAKE) ${MFLAGS} install DESTDIR=${DESTDIR}

${SUBDIR}:	FRC
	@echo "Building and installing files from: $@"
	cd $@; $(MAKE) ${MFLAGS} DESTDIR=${DESTDIR} CFLAGS=${CFLAGS};\
        $(MAKE) ${MFLAGS} install DESTDIR=${DESTDIR}

FRC:

