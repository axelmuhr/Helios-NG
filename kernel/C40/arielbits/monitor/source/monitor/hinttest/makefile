C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -mx -mb -c -g -q -ic:\c40
ASM_OPT=-v40


hinttest.obj:	hinttest.c
	$(C40_C) $(COMPILE_ONLY) hinttest.c

hint.obj:	hint.asm 
	$(C40_ASM) $(ASM_OPT) hint.asm

##########
## Link ##
##########
hinttest.x40: hinttest.obj hint.obj 
	$(C40_LNK) -v40 hinttest.lnk
