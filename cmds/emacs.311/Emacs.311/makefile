#	Helios makefile for
#		MicroEMACS 3.11
#		(C)Copyright 1990 by Daniel Lawrence
#		all rights reserved

LOG =# >>& logfile

CC = c
# modify the following lines with the proper relative directories

OBJD =./o
SRCD =./src
HDIR =./h

# options and arguments to the C compiler

CFLAGS= -wadfsv -z-ecpz -I$(HDIR) #-g

# list of header files

HFILES = $(HDIR)/estruct.h $(HDIR)/edef.h $(HDIR)/efunc.h $(HDIR)/epath.h $(HDIR)/ebind.h $(HDIR)/eproto.h

#list of object files

F1 = $(OBJD)/basic.o $(OBJD)/bind.o $(OBJD)/buffer.o
F2 = $(OBJD)/char.o $(OBJD)/display.o 
F3 = $(OBJD)/eval.o $(OBJD)/exec.o $(OBJD)/file.o
F4 = $(OBJD)/fileio.o $(OBJD)/heliosio.o $(OBJD)/input.o
F5 = $(OBJD)/isearch.o $(OBJD)/line.o 
F6 = $(OBJD)/main.o $(OBJD)/mouse.o $(OBJD)/screen.o
F7 = $(OBJD)/random.o $(OBJD)/region.o $(OBJD)/search.o
F8 = $(OBJD)/window.o $(OBJD)/word.o 

# product list

all:	emacs done

OFILES =	$(F1) $(F2) $(F3) $(F4) $(F5) $(F6) $(F7) $(F8)

	
emacs:  $(OFILES)
	@echo ================================================== linking emacs
	$(CC) $(CFLAGS) $(OFILES) -ltermcap -lc -lbsd  -s16000 -h4000 -o emacs $(LOG)

$(OBJD)/basic.o: $(SRCD)/basic.c $(HFILES)
	@echo ================================================== compiling basic.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/basic.o $(SRCD)/basic.c $(LOG)
$(OBJD)/bind.o: $(SRCD)/bind.c $(HFILES)
	@echo ================================================== compiling bind.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/bind.o $(SRCD)/bind.c $(LOG)
$(OBJD)/buffer.o: $(SRCD)/buffer.c $(HFILES)
	@echo ================================================== compiling buffer.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/buffer.o $(SRCD)/buffer.c $(LOG)
$(OBJD)/char.o: $(SRCD)/char.c $(HFILES)
	@echo ================================================== compiling char.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/char.o $(SRCD)/char.c $(LOG)
$(OBJD)/display.o: $(SRCD)/display.c $(HFILES)
	@echo ================================================== compiling display.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/display.o $(SRCD)/display.c $(LOG)
$(OBJD)/eval.o: $(SRCD)/eval.c $(HFILES) $(HDIR)/evar.h
	@echo ================================================== compiling eval.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/eval.o $(SRCD)/eval.c $(LOG)
$(OBJD)/exec.o: $(SRCD)/exec.c $(HFILES)
	@echo ================================================== compiling exec.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/exec.o $(SRCD)/exec.c $(LOG)
$(OBJD)/file.o: $(SRCD)/file.c $(HFILES)
	@echo ================================================== compiling file.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/file.o $(SRCD)/file.c $(LOG)
$(OBJD)/fileio.o: $(SRCD)/fileio.c $(HFILES)
	@echo ================================================== compiling fileio.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/fileio.o $(SRCD)/fileio.c $(LOG)
$(OBJD)/input.o: $(SRCD)/input.c $(HFILES)
	@echo ================================================== compiling input.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/input.o $(SRCD)/input.c $(LOG)
$(OBJD)/isearch.o: $(SRCD)/isearch.c $(HFILES)
	@echo ================================================== compiling isearch.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/isearch.o $(SRCD)/isearch.c $(LOG)
$(OBJD)/heliosio.o: $(SRCD)/heliosio.c $(HFILES)
	@echo ================================================== compiling heliosio.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/heliosio.o $(SRCD)/heliosio.c $(LOG)
$(OBJD)/line.o: $(SRCD)/line.c $(HFILES)
	@echo ================================================== compiling line.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/line.o $(SRCD)/line.c $(LOG)
$(OBJD)/main.o: $(SRCD)/main.c $(HFILES)
	@echo ================================================== compiling main.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/main.o $(SRCD)/main.c $(LOG)
$(OBJD)/mouse.o: $(SRCD)/mouse.c $(HFILES)
	@echo ================================================== compiling mouse.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/mouse.o $(SRCD)/mouse.c $(LOG)
$(OBJD)/random.o: $(SRCD)/random.c $(HFILES)
	@echo ================================================== compiling random.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/random.o $(SRCD)/random.c $(LOG)
$(OBJD)/region.o: $(SRCD)/region.c $(HFILES)
	@echo ================================================== compiling region.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/region.o $(SRCD)/region.c $(LOG)
$(OBJD)/screen.o: $(SRCD)/screen.c $(HFILES)
	@echo ================================================== compiling screen.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/screen.o $(SRCD)/screen.c $(LOG)
$(OBJD)/search.o: $(SRCD)/search.c $(HFILES)
	@echo ================================================== compiling search.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/search.o $(SRCD)/search.c $(LOG)
$(OBJD)/window.o: $(SRCD)/window.c $(HFILES)
	@echo ================================================== compiling window.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/window.o $(SRCD)/window.c $(LOG)
$(OBJD)/word.o: $(SRCD)/word.c $(HFILES)
	@echo ================================================== compiling word.o
	$(CC) $(CFLAGS) -c -o $(OBJD)/word.o $(SRCD)/word.c $(LOG)

done:
	@echo done
