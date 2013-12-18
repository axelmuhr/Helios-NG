C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -c -g -q -ic:\c40
ASM_OPT=-v40


blink.obj:	blink.c
	$(C40_C) $(COMPILE_ONLY) blink.c

led.obj:	led.c
	$(C40_C) $(COMPILE_ONLY) led.c

interrup.obj:	interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

##########
## Link ##
##########
blink.x40: blink.obj led.obj interrup.asm
	$(C40_LNK) -v40 blink.lnk
