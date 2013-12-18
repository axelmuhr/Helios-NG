C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


eeprom.obj:	eeprom.asm
	$(C40_ASM) $(ASM_OPT) eeprom.asm

eetest.obj:	eetest.c
	$(C40_C) $(COMPILE_ONLY) eetest.c
	$(C40_ASM) $(ASM_OPT) eetest.asm


##########
## Link ##
##########
eetest.x40: eetest.obj eeprom.obj 
	$(C40_LNK) -v40 eetest.lnk
