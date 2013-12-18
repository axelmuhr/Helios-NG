C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -n -s -g -ic:\c40
COMPILE=-v40 -g -ic:\c40
ASM_OPT=-v40


boot_oth.obj:	boot_oth.c hydra.h
	$(C40_C) $(COMPILE_ONLY) boot_oth.c
	$(C40_ASM) $(ASM_OPT) boot_oth.asm

com_port.obj:	com_port.c hydra.h
	$(C40_C) $(COMPILE_ONLY) com_port.c
	$(C40_ASM) $(ASM_OPT) com_port.asm

getchar.obj:	getchar.c hydra.h
	$(C40_C) $(COMPILE_ONLY) getchar.c
	$(C40_ASM) $(ASM_OPT) getchar.asm

main.obj:	main.c hydra.h
	$(C40_C) $(COMPILE_ONLY) main.c
	$(C40_ASM) $(ASM_OPT) main.asm

led.obj:	led.c hydra.h
	$(C40_C) $(COMPILE_ONLY) led.c
	$(C40_ASM) $(ASM_OPT) led.asm

dmemtest.obj:	dmemtest.c hydra.h
	$(C40_C) $(COMPILE_ONLY) dmemtest.c
	$(C40_ASM) $(ASM_OPT) dmemtest.asm

printf.obj:	printf.c hydra.h
	$(C40_C) $(COMPILE_ONLY) printf.c
	$(C40_ASM) $(ASM_OPT) printf.asm

test.obj:	test.c hydra.h
	$(C40_C) $(COMPILE_ONLY) test.c
	$(C40_ASM) $(ASM_OPT) test.asm

vicvac.obj:	vicvac.c 
	$(C40_C) $(COMPILE_ONLY) vicvac.c
	$(C40_ASM) $(ASM_OPT) vicvac.asm


eeprom.obj:	eeprom.asm 
	$(C40_ASM) $(ASM_OPT) eeprom.asm

interupt.obj:	interupt.asm 
	$(C40_ASM) $(ASM_OPT) interupt.asm

memtest.obj:	memtest.asm 
	$(C40_ASM) $(ASM_OPT) memtest.asm

tcr.obj:	tcr.asm 
	$(C40_ASM) $(ASM_OPT) tcr.asm

##########
## Link ##
##########
hydramon.x40: hydra.h boot_oth.obj  com_port.obj\
dmemtest.obj eeprom.obj getchar.obj \
interupt.obj main.obj memtest.obj printf.obj \
tcr.obj test.obj vicvac.obj
	$(C40_LNK) -v40 monitor.lnk




###########################
# Now make others program #
###########################

others.obj:	others.c 
	$(C40_C) $(COMPILE_ONLY) others.c
	$(C40_ASM) $(ASM_OPT) others.asm

others.x40: others.obj com_port.obj interupt.obj memtest.obj
	$(C40_LNK) -v40 others.lnk
