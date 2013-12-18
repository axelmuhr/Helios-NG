C40_C=cl30
C40_ASM=asm30
C40_LNK=lnk30 -v40
COMPILE_ONLY=-v40 -c -q
ASM_OPT=-v40

all:	landdsp.x40

interrup.obj:	interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

landdsp.obj: landdsp.c
	$(C40_C) $(COMPILE_ONLY) landdsp.c

landdsp.x40: landdsp.obj interrup.obj landdsp.lnk
	$(C40_LNK) landdsp.lnk


