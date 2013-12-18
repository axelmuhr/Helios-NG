COMPONENT = cmds/public/yacc-1.4
VPATH := $(HSRC)/$(COMPONENT)
SOURCES = makefile

.c.o:
	c -c -D_BSD -o$@ $<

default: yacc

install: yacc
	cp yacc /helios/local/bin
	touch install

clean:
	rm yacc *.o install

OBJECTS = closure.o error.o lalr.o lr0.o main.o mkpar.o output.o reader.o \
	skeleton.o symtab.o verbose.o warshall.o

yacc: $(OBJECTS)
	c -o $@ $^ -lbsd
