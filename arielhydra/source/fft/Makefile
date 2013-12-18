VC40DSP = /giga/HeliosRoot/Helios/arielhydra/source
IDIR = ${VC40DSP}/include
LDIR = ${VC40DSP}/lib
DRVDIR = ${VC40DSP}/driver
CFLAGS = -I${IDIR}

all: fftmap fft

fftmap: fftmap.c ${LDIR}/libvc40map.a
	cc -g -o $@ -I${IDIR} -I${DRVDIR} -L${LDIR} fftmap.c -lvc40map -lm

fft: fft.c ${LDIR}/libvc40.a
	cc -O -o fft -I${IDIR} -L${LDIR} fft.c -lvc40 -lm
