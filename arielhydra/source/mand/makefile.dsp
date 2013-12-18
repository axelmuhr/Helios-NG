C40_C=cl30
C40_ASM=asm30
C40_LNK=lnk30 -v40
COMPILE_ONLY=-v40 -c -k -al -q
ASM_OPT=-v40

all:	manddsp.x40

interrup.obj:	interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

manddsp.obj: manddsp.c
	$(C40_C) $(COMPILE_ONLY) manddsp.c

manddsp.x40: manddsp.obj interrup.obj manddsp.lnk
	$(C40_LNK) manddsp.lnk


