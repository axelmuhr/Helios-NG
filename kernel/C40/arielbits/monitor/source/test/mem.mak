C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


mem1.obj:	mem1.asm
	$(C40_ASM) $(ASM_OPT) mem1.asm

mem2.obj:	mem2.asm
	$(C40_ASM) $(ASM_OPT) mem2.asm


##########
## Link ##
##########
mem1.x40: mem1.obj  
	$(C40_LNK) -v40 mem1.lnk

mem2.x40: mem2.obj
	$(C40_LNK) -v40 mem2.lnk
