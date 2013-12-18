C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -mx -mb -c -g -q -ic:\c40
ASM_OPT=-v40

makeeepr.obj:	makeeepr.c
	$(C40_C) $(COMPILE_ONLY) makeeepr.c

vicvac.obj:	vicvac.c
	$(C40_C) $(COMPILE_ONLY) vicvac.c

eeprom.obj:	eeprom.asm
	$(C40_ASM) $(ASM_OPT) eeprom.asm


##########
## Link ##
##########
makeeepr.x40: makeeepr.obj vicvac.obj eeprom.obj
	$(C40_LNK) -v40 makeeepr.lnk
