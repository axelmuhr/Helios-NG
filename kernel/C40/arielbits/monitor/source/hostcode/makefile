C40_C=c:\c40\cl30
C40_ASM=c:\c40\asm30
C40_LNK=c:\c40\lnk30
COMPILE_ONLY=-v40 -mx -mb -c -g -q -ic:\c40
ASM_OPT=-v40


main.obj:	main.c hydra.h
	$(C40_C) $(COMPILE_ONLY) main.c

oth_stuf.obj:	oth_stuf.c hydra.h
	$(C40_C) $(COMPILE_ONLY) oth_stuf.c

##########
## Link ##
##########
hostcode.x40: hydra.h boot_oth.obj \
com_port.obj config.obj copy.obj\
eeprom.obj\
halt.obj host.obj\
interupt.obj led.obj main.obj \
oth_stuf.obj runhost.obj \
semaphor.obj\
tcr.obj \
host.lnk
	$(C40_LNK) -v40 host.lnk
