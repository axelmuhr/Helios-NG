#
# Makefile for MicroEMACS 3.9 on the Atari ST using Mark Williams C
#
SRC=.
CC=cc
CFLAGS=-I. -A

OBJS=	basic.o bind.o buffer.o crypt.o display.o eval.o exec.o file.o\
	fileio.o input.o isearch.o line.o main.o random.o region.o search.o\
	spawn.o st520.o termio.o word.o window.o

emacs.prg:	$(OBJS)
		$(CC) -o emacs.prg $(OBJS)

basic.o: $(SRC)\basic.c ebind.h epath.h efunc.h edef.h estruct.h
bind.o: $(SRC)\bind.c ebind.h epath.h efunc.h edef.h estruct.h
buffer.o: $(SRC)\buffer.c ebind.h epath.h efunc.h edef.h estruct.h
crypt.o: $(SRC)\crypt.c ebind.h epath.h efunc.h edef.h estruct.h
display.o: $(SRC)\display.c ebind.h epath.h efunc.h edef.h estruct.h
eval.o: $(SRC)\eval.c ebind.h epath.h efunc.h edef.h estruct.h evar.h
exec.o: $(SRC)\exec.c ebind.h epath.h efunc.h edef.h estruct.h
file.o: $(SRC)\file.c ebind.h epath.h efunc.h edef.h estruct.h
fileio.o: $(SRC)\fileio.c ebind.h epath.h efunc.h edef.h estruct.h
input.o: $(SRC)\input.c ebind.h epath.h efunc.h edef.h estruct.h
isearch.o: $(SRC)\isearch.c ebind.h epath.h efunc.h edef.h estruct.h
line.o: $(SRC)\line.c ebind.h epath.h efunc.h edef.h estruct.h
main.o: $(SRC)\main.c ebind.h efunc.h edef.h estruct.h
random.o: $(SRC)\random.c ebind.h epath.h efunc.h edef.h estruct.h
region.o: $(SRC)\region.c ebind.h epath.h efunc.h edef.h estruct.h
search.o: $(SRC)\search.c ebind.h epath.h efunc.h edef.h estruct.h
spawn.o: $(SRC)\spawn.c ebind.h epath.h efunc.h edef.h estruct.h
st520.o: $(SRC)\st520.c ebind.h epath.h efunc.h edef.h estruct.h
termio.o: $(SRC)\termio.c ebind.h epath.h efunc.h edef.h estruct.h
word.o: $(SRC)\word.c ebind.h epath.h efunc.h edef.h estruct.h
window.o: $(SRC)\window.c ebind.h epath.h efunc.h edef.h estruct.h
