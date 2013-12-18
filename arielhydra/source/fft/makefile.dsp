C40_C=cl30
C40_ASM=asm30
C40_LNK=lnk30 -v40
COMPILE_ONLY=-v40 -c -q
ASM_OPT=-v40

all:	fftdsp.x40

interrup.obj:	interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

fftdsp.obj: fftdsp.c
	$(C40_C) $(COMPILE_ONLY) fftdsp.c

fftdsp.x40: fftdsp.obj interrup.obj fftdsp.lnk
	$(C40_LNK) fftdsp.lnk


