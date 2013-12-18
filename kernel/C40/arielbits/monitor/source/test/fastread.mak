C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


fastread.obj:	fastread.asm
	$(C40_ASM) $(ASM_OPT) fastread.asm



##########
## Link ##
##########
fastread.x40: fastread.obj 
	$(C40_LNK) -v40 fastread.lnk
