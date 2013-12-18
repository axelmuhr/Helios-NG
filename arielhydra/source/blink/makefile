C40_C=cl30
C40_ASM=asm30
C40_LNK=lnk30 -v40
COMPILE_ONLY=-v40 -c -g -q
ASM_OPT=-v40

all:	blink.x40 int.x40

blink.obj:	blink.c
	$(C40_C) $(COMPILE_ONLY) blink.c

led.obj:	led.c
	$(C40_C) $(COMPILE_ONLY) led.c

interrup.obj:	interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

blink.x40: blink.obj led.obj interrup.asm blink.lnk
	$(C40_LNK) blink.lnk


int.x40: int.obj int.lnk
	$(C40_LNK) int.lnk

int.obj: int.c
	$(C40_C) $(COMPILE_ONLY) int.c

