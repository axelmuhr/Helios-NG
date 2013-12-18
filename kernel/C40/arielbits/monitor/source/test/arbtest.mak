C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


arbtest.obj:	arbtest.asm
	$(C40_ASM) $(ASM_OPT) arbtest.asm

arb2.obj:	arb2.asm
	$(C40_ASM) $(ASM_OPT) arb2.asm


##########
## Link ##
##########
arbtest.x40: arbtest.obj  
	$(C40_LNK) -v40 arbtest.lnk

arb2.x40: arb2.obj
	$(C40_LNK) -v40 arb2.lnk
