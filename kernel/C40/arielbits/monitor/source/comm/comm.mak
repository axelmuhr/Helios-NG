C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -c -ic:\c40
ASM_OPT=-v40

test_com.obj:	test_com.c
	$(C40_C) $(COMPILE_ONLY) test_com.c
	$(C40_ASM) $(ASM_OPT) test_com.asm

com_port.obj:	com_port.c
	$(C40_C) $(COMPILE_ONLY) com_port.c
	$(C40_ASM) $(ASM_OPT) com_port.asm

en_port.obj:	en_port.asm
	$(C40_ASM) $(ASM_OPT) en_port.asm

dis_port.obj:	dis_port.asm
	$(C40_ASM) $(ASM_OPT) dis_port.asm

o_crdy.obj:	o_crdy.asm
	$(C40_ASM) $(ASM_OPT) o_crdy.asm

i_crdy.obj:	i_crdy.asm
	$(C40_ASM) $(ASM_OPT) i_crdy.asm

stcomisr.obj:	stcomisr.asm
	$(C40_ASM) $(ASM_OPT) stcomisr.asm

##########
## Link ##
##########
comm.x40: test_com.obj com_port.obj en_port.obj dis_port.obj o_crdy.obj \
i_crdy.obj stcomisr.obj
	$(C40_LNK) -v40 comm.lnk
