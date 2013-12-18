C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


blink.obj:	blink.c
	$(C40_C) $(COMPILE_ONLY) blink.c
	$(C40_ASM) $(ASM_OPT) blink.asm

led.obj:	led.c
	$(C40_C) $(COMPILE_ONLY) led.c
	$(C40_ASM) $(ASM_OPT) led.asm

ledcont.obj:	ledcont.asm
	$(C40_ASM) $(ASM_OPT) ledcont.asm

vicvac.obj:	vicvac.c
	$(C40_C) $(COMPILE_ONLY) vicvac.c
	$(C40_ASM) $(ASM_OPT) vicvac.asm


##########
## Link ##
##########
blink.x40: blink.obj led.obj
	$(C40_LNK) -v40 blink.lnk
