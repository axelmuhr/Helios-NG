
#.SUFFIXES :
#.SUFFIXES : .o .s .c

#.c.o : 
#	c -v -wacdfpsvz -DT800 -I. -c $*.c

#.c.s : 
#	c -v -wacdfpsvz -DT800 -I. -S $*.c

CFLAGS := -DT800 -wA

OBJS = debug.o dbdecode.o xplib.o encode.o

debug : $(OBJS)
	c $(OBJS) -o debug
#	cp debug $(BIN)

disasm : disasm.o decode.o loadfile.o
	link -with disasm -nolist -debug

tload : tload.o xplib.o
	cc tload.o xplib.o -o tload.prg
