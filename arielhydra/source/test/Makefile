DRIVER = boot #test copy
MAPPER = bootmap

IDIR = ${VC40DSP}/include
LDIR = ${VC40DSP}/lib
DRVDIR = ${VC40DSP}/driver
CFLAGS = -I${IDIR}

all: ${DRIVER} ${MAPPER}

boot: boot.c ${LDIR}/libvc40.a
	cc -g -o $@ -I${IDIR} -L${LDIR} boot.c -lvc40

bootmap: bootmap.c ${LDIR}/libvc40map.a
	cc -g -o $@ -I${IDIR} -I${DRVDIR} -L${LDIR} bootmap.c -lvc40map

timexfr: timexfr.c
	cc -o $@ -O -I${IDIR} -L${LDIR} timexfr.c -lvc40 -lvc40map

