C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


test.obj:	test.asm
	$(C40_ASM) $(ASM_OPT) test.asm


##########
## Link ##
##########
test.x40: test.obj 
	$(C40_LNK) -v40 test.lnk
