C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


mem_tst.obj:	mem_tst.c
	$(C40_C) $(COMPILE_ONLY) mem_tst.c
	$(C40_ASM) $(ASM_OPT) mem_tst.asm

memtest.obj:	memtest.asm
	$(C40_ASM) $(ASM_OPT) memtest.asm


##########
## Link ##
##########
mem_tst.x40: mem_tst.obj memtest.obj
	$(C40_LNK) -v40 mem_tst.lnk
