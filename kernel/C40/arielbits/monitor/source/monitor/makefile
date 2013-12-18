C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -mx -mb -c -g -q -ic:\c40
ASM_OPT=-v40


boot_oth.obj:	boot_oth.c hydra.h
	$(C40_C) $(COMPILE_ONLY) boot_oth.c

com_port.obj:	com_port.c hydra.h
	$(C40_C) $(COMPILE_ONLY) com_port.c

config.obj:	config.c hydra.h
	$(C40_C) $(COMPILE_ONLY) config.c

crc.obj:	crc.c hydra.h
	$(C40_C) $(COMPILE_ONLY) crc.c

dmemtest.obj:	dmemtest.c hydra.h
	$(C40_C) $(COMPILE_ONLY) dmemtest.c

getchar.obj:	getchar.c hydra.h
	$(C40_C) $(COMPILE_ONLY) getchar.c

go.obj:	go.c hydra.h
	$(C40_C) $(COMPILE_ONLY) go.c

host.obj:	host.c hydra.h host.h
	$(C40_C) $(COMPILE_ONLY) host.c

led.obj:	led.c hydra.h
	$(C40_C) $(COMPILE_ONLY) led.c

main.obj:	main.c hydra.h
	$(C40_C) $(COMPILE_ONLY) main.c

monitor.obj:	monitor.c hydra.h
	$(C40_C) $(COMPILE_ONLY) monitor.c

printf.obj:	printf.c hydra.h
	$(C40_C) $(COMPILE_ONLY) printf.c

reg_dump.obj:	reg_dump.c hydra.h
	$(C40_C) $(COMPILE_ONLY) reg_dump.c

restore.obj:	restore.c hydra.h
	$(C40_C) $(COMPILE_ONLY) restore.c

services.obj:	services.c hydra.h
	$(C40_C) $(COMPILE_ONLY) services.c

set_brk.obj:	set_brk.c hydra.h
	$(C40_C) $(COMPILE_ONLY) set_brk.c

step.obj:	step.c hydra.h
	$(C40_C) $(COMPILE_ONLY) step.c

test.obj:	test.c hydra.h
	$(C40_C) $(COMPILE_ONLY) test.c

vicvac.obj:	vicvac.c hydra.h
	$(C40_C) $(COMPILE_ONLY) vicvac.c


####################
## Assembly files ##
####################
boot.obj:	boot.asm 
	$(C40_ASM) $(ASM_OPT) boot.asm

break_pt.obj:	break_pt.asm 
	$(C40_ASM) $(ASM_OPT) break_pt.asm

copy.obj:	copy.asm 
	$(C40_ASM) $(ASM_OPT) copy.asm

eeprom.obj:	eeprom.asm 
	$(C40_ASM) $(ASM_OPT) eeprom.asm

halt.obj:	halt.asm 
	$(C40_ASM) $(ASM_OPT) halt.asm

init.obj:	init.asm 
	$(C40_ASM) $(ASM_OPT) init.asm

interupt.obj:	interupt.asm 
	$(C40_ASM) $(ASM_OPT) interupt.asm

memtest.obj:	memtest.asm 
	$(C40_ASM) $(ASM_OPT) memtest.asm

run.obj:	run.asm 
	$(C40_ASM) $(ASM_OPT) run.asm

runhost.obj:	runhost.asm 
	$(C40_ASM) $(ASM_OPT) runhost.asm

semaphor.obj:	semaphor.asm 
	$(C40_ASM) $(ASM_OPT) semaphor.asm

tcr.obj:	tcr.asm 
	$(C40_ASM) $(ASM_OPT) tcr.asm

##########
## Link ##
##########
hydramon.x40: hydra.h boot.obj boot_oth.obj \
break_pt.obj com_port.obj config.obj copy.obj\
crc.obj eeprom.obj getchar.obj \
go.obj halt.obj host.obj init.obj \
interupt.obj led.obj main.obj \
memtest.obj monitor.obj printf.obj \
reg_dump.obj restore.obj run.obj runhost.obj \
semaphor.obj services.obj set_brk.obj step.obj \
tcr.obj test.obj vicvac.obj \
monitor.lnk
	$(C40_LNK) -v40 monitor.lnk




###########################
# Now make others program #
###########################

others.obj:	others.c 
	$(C40_C) $(COMPILE_ONLY) others.c

others.x40: boot.obj copy.obj others.obj com_port.obj \
halt.obj interupt.obj memtest.obj runhost.obj semaphor.obj \
others.lnk
	$(C40_LNK) -v40 others.lnk
