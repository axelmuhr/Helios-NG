C40_C=cl30
C40_ASM=asm30
C40_LNK=lnk30
COMPILE_ONLY=-v40 -mx -mb -c -g -q
ASM_OPT=-v40


hinttest.obj:	hinttest.c
	$(C40_C) $(COMPILE_ONLY) hinttest.c

hint.obj:	hint.asm 
	$(C40_ASM) $(ASM_OPT) hint.asm

interrup.obj: interrup.asm
	$(C40_ASM) $(ASM_OPT) interrup.asm

##########
## Link ##
##########
hinttest.x40: hinttest.obj interrup.obj hint.obj 
	$(C40_LNK) -v40 hinttest.lnk
