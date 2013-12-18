# Makefile for uEmacs on a SUN host

COMPONENT = cmds/emacs.311

.PHONY: default clean

vpath %.h $(HSRC)/$(COMPONENT)
vpath %.c $(HSRC)/$(COMPONENT)

HOSTCC = cc
HOSTCFLAGS := -D__SUN4
.c.o: 
	$(HOSTCC) -c $(HOSTCFLAGS) -o $@ $<

TARGETS = me
OBJECTS =  main.o basic.o bind.o eval.o word.o char.o exec.o folding.o \
	file.o fileio.o window.o input.o isearch.o display.o mouse.o \
	screen.o random.o region.o buffer.o search.o line.o unix.o

default: me

clean :
	-$(RM) $(OBJECTS) $(TARGETS) mkout core

me : $(OBJECTS)
	$(HOSTCC) -o $@ $(OBJECTS) -ltermcap


