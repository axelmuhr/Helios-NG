C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


eeprom.obj:	eeprom.asm
	$(C40_ASM) $(ASM_OPT) eeprom.asm

setuptes.obj:	setuptes.c
	$(C40_C) $(COMPILE_ONLY) setuptes.c
	$(C40_ASM) $(ASM_OPT) setuptes.asm

vicvac.obj:	vicvac.c
	$(C40_C) $(COMPILE_ONLY) vicvac.c
	$(C40_ASM) $(ASM_OPT) vicvac.asm

##########
## Link ##
##########
setuptes.x40: setuptes.obj eeprom.obj vicvac.obj
	$(C40_LNK) -v40 setuptes.lnk
