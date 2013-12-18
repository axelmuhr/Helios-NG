#
# Makefile to build the V-C40 Hydra utility libraries
#

LIBOBJS = c40load.o c40util.o
MAPOBJS = vc40map.o vc40maputil.o c40load.o

VC40DSP = /giga/HeliosRoot/Helios/arielhydra/source
INCDIR = ${VC40DSP}/include
LIBDIR = ${VC40DSP}/lib
BINDIR = ${VC40DSP}/bin
LIBS = ${LIBDIR}/libvc40.a
MAPLIB = ${LIBDIR}/libvc40map.a
ARCH = `arch`

c40load.o: c40load.c c40load.h ${INCDIR}/coff.h ${INCDIR}/hyhomon.h ${INCDIR}/vc40map.h
	cc -c -I. -I${INCDIR} -I${VC40DSP}/driver -DSUNOS -D$(ARCH) $*.c

c40util.o: c40util.c c40load.h ${INCDIR}/coff.h ${INCDIR}/hyhomon.h ${INCDIR}/vc40map.h
	cc -c -I. -I${INCDIR} -I${VC40DSP}/driver -DSUNOS -D$(ARCH) $*.c

vc40map.o: vc40map.c c40load.h ${INCDIR}/coff.h ${INCDIR}/hyhomon.h ${INCDIR}/vc40map.h
	cc -c -I. -I${INCDIR} -I${VC40DSP}/driver -DSUNOS -D$(ARCH) $*.c

vc40maputil.o: vc40maputil.c c40load.h ${INCDIR}/coff.h ${INCDIR}/hyhomon.h ${INCDIR}/vc40map.h
	cc -c -I. -I${INCDIR} -I${VC40DSP}/driver -DSUNOS -D$(ARCH) $*.c

all:	${MAPLIB} ${LIBS}

${LIBS}:	${LIBOBJS}
	ar rc $@ $?
	ranlib $@

${MAPLIB}: ${MAPOBJS}
	ar rc $@ $?
	ranlib $@

clean:
	rm -f core *.o *%

new: clean
	rm -f ${LIBS}

install: all clean
