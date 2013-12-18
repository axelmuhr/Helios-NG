C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40



TestMem.obj:	TestMem.c
	$(C40_C) $(COMPILE_ONLY) TestMem.c
	$(C40_ASM) $(ASM_OPT) TestMem.asm

DMemTest.obj:	DMemTest.c
	$(C40_C) $(COMPILE_ONLY) DMemTest.c
	$(C40_ASM) $(ASM_OPT) DMemTest.asm

MemTest.obj:	MemTest.asm
	$(C40_ASM) $(ASM_OPT) MemTest.asm


##########
## Link ##
##########
TestMem.x40: TestMem.obj DMemTest.obj MemTest.obj
	$(C40_LNK) -v40 TestMem.lnk
