OFILES = vc40load.o vc40cfg.o vc40priv.o vc40top.o vc40bottom.o

LIBDIR = ${VC40DSP}/lib
INCDIR = ${VC40DSP}/include
BINDIR = ${VC40DSP}/bin

all:	vc40.o

%.o: %.c vc40drv.h ${INCDIR}/hyhomon.h
	cc -c -O -o $@ -DKERNEL -D`arch -k` -I${INCDIR} $*.c

vc40.o:	$(OFILES)
	ld -r -o vc40.o $(OFILES)

test: test.c
	cc -I${INCDIR} -L${LIBDIR} -o test test.c -lvc40

load:	vc40dsp.o
	vc40load

clean:
	rm -f *% *.o vc40
