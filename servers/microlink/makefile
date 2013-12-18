# $Header: makefile,v 1.9 91/01/31 13:50:45 charles Locked $
# $Source: /server/usr/users/b/charles/world/microlink/RCS/makefile,v $
# $State: Release_0_0_8_EXPERIMENTAL $

#                                                            'makefile'

# Makefile for microlink server

#----------------------------------------------------------------------
#                                      Requirements for other modules
#----------------------------------------------------------------------

# None

#----------------------------------------------------------------------
#                                                       Default Rules
#----------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .o .s .c .C

.s.o:
	as -o $*.o $*.s

.c.o:
	cc -ansi -c $*.c
	mv $*.o objects

.C.o:
	CC -c $*.C
	mv $*.o objects

#----------------------------------------------------------------------
#                                                 Include directories  
#----------------------------------------------------------------------

INC  = -Ilochdr,../hdr

#----------------------------------------------------------------------
#                                                 Major Dummy Targets
#----------------------------------------------------------------------

# Use 'make secondaries' or just 'make' to make a module's librarys e.t.c.
# Use 'make tertiaries' to make the executables e.t.c.
# Refer to Source Management Documents if confused

# Use 'make xxx_secondaries' (where 'xxx' is 'helios' or 'unix' according)
#     to make the code for only that system. Similarly with 'make xxx_
#     teriaries'.

secondaries: preliminaries
	cd .. && cur
	make real_secondaries

tertiaries: preliminaries externals
	make real_tertiaries

preliminaries:
	@echo "Checking dependencies"
	@chd -j/helios$(hv)/include -Oobjects/helios$(hv) $(INC) source/*.c \
		source/test/*.c -cobjects/helios$(hv)/cache $(tr)

externals:
#	Do nothing

real_secondaries:
#	Nothing to do yet

real_tertiaries: out/helios$(hv)/microlink objects/helios$(hv)/phone.o
real_tertiaries: out/helios$(hv)/digCalibrate
real_tertiaries: out/helios$(hv)/test1
real_tertiaries: out/helios$(hv)/test2
real_tertiaries: out/helios$(hv)/test3
real_tertiaries: objects/helios$(hv)/test4.o
real_tertiaries: out/helios$(hv)/test5
	@echo "Helios Tertiaries made"

#----------------------------------------------------------------------
#                                                  Convenient Aliases
#----------------------------------------------------------------------

sec: real_secondaries

all: tertiaries

quick:	real_tertiaries

#----------------------------------------------------------------------
#                                                   Debugging control
#----------------------------------------------------------------------

DBX = 
#DBX = -g
#DBX = -pg

#----------------------------------------------------------------------
#                                                              Macros
#----------------------------------------------------------------------

# The objects which make up the microlink server ...

objects = \
	objects/helios$(hv)/microlink.o		\
	objects/helios$(hv)/digitiser.o		\
	objects/helios$(hv)/hermv.o		\
	objects/helios$(hv)/divide.o		\
	objects/helios$(hv)/general.o		\
	objects/helios$(hv)/rawdig.o

objectsdg = \
	objects/helios$(hv)/microlink.o		\
	objects/helios$(hv)/digitiserdg.o	\
	objects/helios$(hv)/hermv.o		\
	objects/helios$(hv)/divide.o		\
	objects/helios$(hv)/general.o		\
	objects/helios$(hv)/rawdig.o

opts = 

#----------------------------------------------------------------------
#                                                    Executables
#----------------------------------------------------------------------

out/helios$(hv)/microlink: $(objects)
	@echo "Linking    : out/helios$(hv)/microlink"
	@armlink  -o out/helios$(hv)/microlink \
		/helios$(hv)/lib/cstart.o /helios$(hv)/lib/servlib.def \
		/helios$(hv)/lib/abclib.def $(objects)

out/helios$(hv)/microlinkdg: $(objectsdg)
	@echo "Linking    : out/helios$(hv)/microlinkdg"
	@armlink  -o out/helios$(hv)/microlinkdg			\
		/helios$(hv)/lib/cstart.o /helios$(hv)/lib/servlib.def	\
		/helios$(hv)/lib/abclib.def $(objectsdg)

out/helios$(hv)/digCalibrate: objects/helios$(hv)/digCalibrate.o
	@echo "Linking    : out/helios$(hv)/digCalibrate"
	@armlink -s65536 -o out/helios$(hv)/digCalibrate  /helios$(hv)/lib/cstart.o \
		/helios$(hv)/lib/fault.def ../libs/drawp/helios$(hv)/drawp.o \
		objects/helios$(hv)/digCalibrate.o
		
out/helios$(hv)/test1: objects/helios$(hv)/test1.o
	@echo "Linking    : out/helios$(hv)/test1"
	@armlink -o out/helios$(hv)/test1 /helios$(hv)/lib/cstart.o \
			objects/helios$(hv)/test1.o

out/helios$(hv)/test2: objects/helios$(hv)/test2.o
	@echo "Linking    : out/helios$(hv)/test2"
	@armlink  -o out/helios$(hv)/test2 /helios$(hv)/lib/cstart.o \
		objects/helios$(hv)/test2.o ../libs/drawp/helios$(hv)/drawp.o

out/helios$(hv)/test3: objects/helios$(hv)/test3.o
	@echo "Linking    : out/helios$(hv)/test3"
	@armlink -o out/helios$(hv)/test3  /helios$(hv)/lib/cstart.o \
		objects/helios$(hv)/test3.o /helios$(hv)/lib/fault.def

out/helios$(hv)/test4: objects/helios$(hv)/test4.o
	@echo "Linking    : out/helios$(hv)/test4"
	@armlink -o out/helios$(hv)/test4  /helios$(hv)/lib/cstart.o \
		objects/helios$(hv)/test4.o /helios/lib/fault.def

out/helios$(hv)/test5: objects/helios$(hv)/test5.o
	@echo "Linking    : out/helios$(hv)/test5"
	@armlink -o out/helios$(hv)/test5  /helios$(hv)/lib/cstart.o \
		objects/helios$(hv)/test5.o /helios/lib/fault.def

#----------------------------------------------------------------------
#                                                       Object Files
#----------------------------------------------------------------------

objects/helios$(hv)/microlink.o: source/microlink.c
	@echo "Compiling  : objects/helios$(hv)/microlink.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/microlink.c -o objects/helios$(hv)/microlink.o

objects/helios$(hv)/digitiser.o: source/digitiser.c
	@echo "Compiling  : objects/helios$(hv)/digitiser.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC)	\
		-DDgSwDiags=0 source/digitiser.c 			\
		-o objects/helios$(hv)/digitiser.o

objects/helios$(hv)/digitiserdg.o: source/digitiser.c
	@echo "Compiling  : objects/helios$(hv)/digitiserdg.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC)	\
		-DDgSwDiags=1 source/digitiser.c 			\
		-o objects/helios$(hv)/digitiserdg.o

objects/helios$(hv)/hermv.o: source/hermv.c
	@echo "Compiling  : objects/helios$(hv)/hermv.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/hermv.c -o objects/helios$(hv)/hermv.o

objects/helios$(hv)/general.o: source/general.c
	@echo "Compiling  : objects/helios$(hv)/general.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/general.c -o objects/helios$(hv)/general.o

objects/helios$(hv)/rawdig.o: source/rawdig.c
	@echo "Compiling  : objects/helios$(hv)/rawdig.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/rawdig.c -o objects/helios$(hv)/rawdig.o

objects/helios$(hv)/divide.o: source/divide.s
	@echo "Assembling : objects/helios$(hv)/divide.o"
	@hobjasm -HDR /helios$(hv)/include/abcARM/asm \
		source/divide.s -o objects/helios$(hv)/divide.o

objects/helios$(hv)/phone.o: source/phone.c
	@echo "Compiling  : objects/helios$(hv)/phone.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/phone.c -o objects/helios$(hv)/phone.o

objects/helios$(hv)/digCalibrate.o: source/digCalibrate.c
	@echo "Compiling  : objects/helios$(hv)/digCalibrate.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/digCalibrate.c -o objects/helios$(hv)/digCalibrate.o

objects/helios$(hv)/test1.o: source/test/test1.c
	@echo "Compiling  : objects/helios$(hv)/test1.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/test/test1.c -o objects/helios$(hv)/test1.o

objects/helios$(hv)/test2.o: source/test/test2.c
	@echo "Compiling  : objects/helios$(hv)/test2.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/test/test2.c -o objects/helios$(hv)/test2.o

objects/helios$(hv)/test3.o: source/test/test3.c
	@echo "Compiling  : objects/helios$(hv)/test3.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/test/test3.c -o objects/helios$(hv)/test3.o

objects/helios$(hv)/test4.o: source/test/test4.c
	@echo "Compiling  : objects/helios$(hv)/test4.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/test/test4.c -o objects/helios$(hv)/test4.o

objects/helios$(hv)/test5.o: source/test/test5.c
	@echo "Compiling  : objects/helios$(hv)/test5.o"
	@ncc -j/helios$(hv)/include -Zt $(opts) -ansi -c $(DBX) $(INC) \
		source/test/test5.c -o objects/helios$(hv)/test5.o

#----------------------------------------------------------------------
#                                              Auto-generated section
#----------------------------------------------------------------------

# All lines after the marker '#auto:#' are generated by the output
#   of the 'chd' utility run by the target 'preliminaries' They show
#   the dependencies of the object files that would be generated on
#   the sources.

#auto:#
objects/helios/digCalibrate.o : source/digCalibrate.c
objects/helios/digCalibrate.o : ../hdr/drawp/public.h
objects/helios/digCalibrate.o : ../hdr/drawp/coordinates.h
objects/helios/digCalibrate.o : ../hdr/drawp/pixmaps.h
objects/helios/digCalibrate.o : ../hdr/drawp/clipping.h
objects/helios/digCalibrate.o : /helios/include/stdio.h
objects/helios/digCalibrate.o : ../hdr/drawp/graphicsContexts.h
objects/helios/digCalibrate.o : ../hdr/microlink/digitiser.h
objects/helios/digCalibrate.o : source/drawText.h
objects/helios/digCalibrate.o : /helios/include/gsp.h
objects/helios/digCalibrate.o : /helios/include/syslib.h
objects/helios/digCalibrate.o : /helios/include/task.h
objects/helios/digCalibrate.o : /helios/include/module.h
objects/helios/digCalibrate.o : /helios/include/helios.h
objects/helios/digCalibrate.o : /helios/include/memory.h
objects/helios/digCalibrate.o : /helios/include/queue.h
objects/helios/digCalibrate.o : /helios/include/message.h
objects/helios/digCalibrate.o : /helios/include/protect.h
objects/helios/digCalibrate.o : /helios/include/sem.h
objects/helios/digCalibrate.o : /helios/include/ioevents.h
objects/helios/digCalibrate.o : /helios/include/math.h
objects/helios/digCalibrate.o : /helios/include/attrib.h
objects/helios/digCalibrate.o : /helios/include/nonansi.h
objects/helios/digCalibrate.o : /helios/include/string.h
objects/helios/digCalibrate.o : /helios/include/stdlib.h
objects/helios/digitiser.o : source/digitiser.c
objects/helios/digitiser.o : ../hdr/microlink/private/digitiser.h
objects/helios/digitiser.o : ../hdr/microlink/digitiser.h
objects/helios/digitiser.o : ../hdr/microlink/private/microlink.h
objects/helios/digitiser.o : /helios/include/abcARM/ABClib.h
objects/helios/digitiser.o : /helios/include/abcARM/ROMitems.h
objects/helios/digitiser.o : /helios/include/queue.h
objects/helios/digitiser.o : /helios/include/helios.h
objects/helios/digitiser.o : /helios/include/signal.h
objects/helios/digitiser.o : /helios/include/ioevents.h
objects/helios/digitiser.o : /helios/include/process.h
objects/helios/digitiser.o : /helios/include/stdio.h
objects/helios/digitiser.o : /helios/include/attrib.h
objects/helios/digitiser.o : /helios/include/syslib.h
objects/helios/digitiser.o : /helios/include/task.h
objects/helios/digitiser.o : /helios/include/module.h
objects/helios/digitiser.o : /helios/include/memory.h
objects/helios/digitiser.o : /helios/include/message.h
objects/helios/digitiser.o : /helios/include/protect.h
objects/helios/digitiser.o : /helios/include/sem.h
objects/helios/digitiser.o : /helios/include/servlib.h
objects/helios/digitiser.o : /helios/include/codes.h
objects/helios/digitiser.o : /helios/include/gsp.h
objects/helios/digitiser.o : /helios/include/setjmp.h
objects/helios/digitiser.o : /helios/include/stdlib.h
objects/helios/digitiser.o : /helios/include/stddef.h
objects/helios/digitiser.o : /helios/include/string.h
objects/helios/drawText.o : source/drawText.c
objects/helios/drawText.o : source/drawText.h
objects/helios/drawText.o : ../hdr/drawp/public.h
objects/helios/drawText.o : ../hdr/drawp/coordinates.h
objects/helios/drawText.o : ../hdr/drawp/pixmaps.h
objects/helios/drawText.o : ../hdr/drawp/clipping.h
objects/helios/drawText.o : /helios/include/stdio.h
objects/helios/drawText.o : ../hdr/drawp/graphicsContexts.h
objects/helios/drawText.o : /helios/include/stdlib.h
objects/helios/drawText.o : /helios/include/string.h
objects/helios/general.o : source/general.c
objects/helios/general.o : ../hdr/microlink/private/general.h
objects/helios/general.o : ../hdr/microlink/general.h
objects/helios/general.o : ../hdr/microlink/private/microlink.h
objects/helios/general.o : /helios/include/abcARM/ABClib.h
objects/helios/general.o : /helios/include/abcARM/ROMitems.h
objects/helios/general.o : /helios/include/queue.h
objects/helios/general.o : /helios/include/helios.h
objects/helios/general.o : /helios/include/signal.h
objects/helios/general.o : /helios/include/ioevents.h
objects/helios/general.o : /helios/include/process.h
objects/helios/general.o : /helios/include/stdio.h
objects/helios/general.o : /helios/include/attrib.h
objects/helios/general.o : /helios/include/syslib.h
objects/helios/general.o : /helios/include/task.h
objects/helios/general.o : /helios/include/module.h
objects/helios/general.o : /helios/include/memory.h
objects/helios/general.o : /helios/include/message.h
objects/helios/general.o : /helios/include/protect.h
objects/helios/general.o : /helios/include/sem.h
objects/helios/general.o : /helios/include/servlib.h
objects/helios/general.o : /helios/include/codes.h
objects/helios/general.o : /helios/include/gsp.h
objects/helios/general.o : /helios/include/setjmp.h
objects/helios/general.o : /helios/include/stdlib.h
objects/helios/general.o : /helios/include/stddef.h
objects/helios/general.o : /helios/include/string.h
objects/helios/hermv.o : source/hermv.c
objects/helios/hermv.o : ../hdr/microlink/general.h
objects/helios/hermv.o : ../hdr/microlink/private/hermv.h
objects/helios/hermv.o : ../hdr/microlink/hermv.h
objects/helios/hermv.o : ../hdr/microlink/private/microlink.h
objects/helios/hermv.o : /helios/include/abcARM/ABClib.h
objects/helios/hermv.o : /helios/include/abcARM/ROMitems.h
objects/helios/hermv.o : /helios/include/queue.h
objects/helios/hermv.o : /helios/include/helios.h
objects/helios/hermv.o : /helios/include/signal.h
objects/helios/hermv.o : /helios/include/ioevents.h
objects/helios/hermv.o : /helios/include/process.h
objects/helios/hermv.o : /helios/include/stdio.h
objects/helios/hermv.o : /helios/include/attrib.h
objects/helios/hermv.o : /helios/include/syslib.h
objects/helios/hermv.o : /helios/include/task.h
objects/helios/hermv.o : /helios/include/module.h
objects/helios/hermv.o : /helios/include/memory.h
objects/helios/hermv.o : /helios/include/message.h
objects/helios/hermv.o : /helios/include/protect.h
objects/helios/hermv.o : /helios/include/sem.h
objects/helios/hermv.o : /helios/include/servlib.h
objects/helios/hermv.o : /helios/include/codes.h
objects/helios/hermv.o : /helios/include/gsp.h
objects/helios/hermv.o : /helios/include/setjmp.h
objects/helios/hermv.o : /helios/include/stdlib.h
objects/helios/hermv.o : /helios/include/stddef.h
objects/helios/hermv.o : /helios/include/string.h
objects/helios/microlink.o : source/microlink.c
objects/helios/microlink.o : ../hdr/microlink/general.h
objects/helios/microlink.o : ../hdr/microlink/private/microlink.h
objects/helios/microlink.o : /helios/include/abcARM/ABClib.h
objects/helios/microlink.o : /helios/include/abcARM/ROMitems.h
objects/helios/microlink.o : /helios/include/queue.h
objects/helios/microlink.o : /helios/include/helios.h
objects/helios/microlink.o : /helios/include/signal.h
objects/helios/microlink.o : /helios/include/ioevents.h
objects/helios/microlink.o : /helios/include/process.h
objects/helios/microlink.o : /helios/include/stdio.h
objects/helios/microlink.o : /helios/include/attrib.h
objects/helios/microlink.o : /helios/include/syslib.h
objects/helios/microlink.o : /helios/include/task.h
objects/helios/microlink.o : /helios/include/module.h
objects/helios/microlink.o : /helios/include/memory.h
objects/helios/microlink.o : /helios/include/message.h
objects/helios/microlink.o : /helios/include/protect.h
objects/helios/microlink.o : /helios/include/sem.h
objects/helios/microlink.o : /helios/include/servlib.h
objects/helios/microlink.o : /helios/include/codes.h
objects/helios/microlink.o : /helios/include/gsp.h
objects/helios/microlink.o : /helios/include/setjmp.h
objects/helios/microlink.o : /helios/include/stdlib.h
objects/helios/microlink.o : /helios/include/stddef.h
objects/helios/microlink.o : /helios/include/string.h
objects/helios/pcoord.o : source/pcoord.c
objects/helios/pcoord.o : ../hdr/microlink/digitiser.h
objects/helios/pcoord.o : /helios/include/gsp.h
objects/helios/pcoord.o : /helios/include/syslib.h
objects/helios/pcoord.o : /helios/include/task.h
objects/helios/pcoord.o : /helios/include/module.h
objects/helios/pcoord.o : /helios/include/helios.h
objects/helios/pcoord.o : /helios/include/memory.h
objects/helios/pcoord.o : /helios/include/queue.h
objects/helios/pcoord.o : /helios/include/message.h
objects/helios/pcoord.o : /helios/include/protect.h
objects/helios/pcoord.o : /helios/include/sem.h
objects/helios/pcoord.o : /helios/include/ioevents.h
objects/helios/pcoord.o : /helios/include/math.h
objects/helios/pcoord.o : /helios/include/attrib.h
objects/helios/pcoord.o : /helios/include/nonansi.h
objects/helios/pcoord.o : /helios/include/stdio.h
objects/helios/pcoord.o : /helios/include/string.h
objects/helios/pcoord.o : /helios/include/stdlib.h
objects/helios/phone.o : source/phone.c
objects/helios/phone.o : ../hdr/microlink/general.h
objects/helios/phone.o : ../hdr/microlink/phone.h
objects/helios/phone.o : /helios/include/syslib.h
objects/helios/phone.o : /helios/include/task.h
objects/helios/phone.o : /helios/include/module.h
objects/helios/phone.o : /helios/include/helios.h
objects/helios/phone.o : /helios/include/memory.h
objects/helios/phone.o : /helios/include/queue.h
objects/helios/phone.o : /helios/include/message.h
objects/helios/phone.o : /helios/include/protect.h
objects/helios/phone.o : /helios/include/sem.h
objects/helios/phone.o : /helios/include/stdlib.h
objects/helios/phone.o : /helios/include/stddef.h
objects/helios/phone.o : /helios/include/ctype.h
objects/helios/phone.o : /helios/include/codes.h
objects/helios/phone.o : /helios/include/stdio.h
objects/helios/rawdig.o : source/rawdig.c
objects/helios/rawdig.o : ../hdr/microlink/private/rawdig.h
objects/helios/rawdig.o : ../hdr/microlink/rawdig.h
objects/helios/rawdig.o : ../hdr/microlink/private/microlink.h
objects/helios/rawdig.o : /helios/include/abcARM/ABClib.h
objects/helios/rawdig.o : /helios/include/abcARM/ROMitems.h
objects/helios/rawdig.o : /helios/include/queue.h
objects/helios/rawdig.o : /helios/include/helios.h
objects/helios/rawdig.o : /helios/include/signal.h
objects/helios/rawdig.o : /helios/include/ioevents.h
objects/helios/rawdig.o : /helios/include/process.h
objects/helios/rawdig.o : /helios/include/stdio.h
objects/helios/rawdig.o : /helios/include/attrib.h
objects/helios/rawdig.o : /helios/include/syslib.h
objects/helios/rawdig.o : /helios/include/task.h
objects/helios/rawdig.o : /helios/include/module.h
objects/helios/rawdig.o : /helios/include/memory.h
objects/helios/rawdig.o : /helios/include/message.h
objects/helios/rawdig.o : /helios/include/protect.h
objects/helios/rawdig.o : /helios/include/sem.h
objects/helios/rawdig.o : /helios/include/servlib.h
objects/helios/rawdig.o : /helios/include/codes.h
objects/helios/rawdig.o : /helios/include/gsp.h
objects/helios/rawdig.o : /helios/include/setjmp.h
objects/helios/rawdig.o : /helios/include/stdlib.h
objects/helios/rawdig.o : /helios/include/stddef.h
objects/helios/rawdig.o : /helios/include/string.h
objects/helios/test1.o : source/test/test1.c
objects/helios/test1.o : /helios/include/ctype.h
objects/helios/test1.o : /helios/include/string.h
objects/helios/test1.o : /helios/include/stdio.h
objects/helios/test1.o : /helios/include/stdlib.h
objects/helios/test1.o : /helios/include/stddef.h
objects/helios/test1.o : /helios/include/ioevents.h
objects/helios/test1.o : /helios/include/process.h
objects/helios/test1.o : /helios/include/helios.h
objects/helios/test1.o : /helios/include/attrib.h
objects/helios/test1.o : /helios/include/syslib.h
objects/helios/test1.o : /helios/include/task.h
objects/helios/test1.o : /helios/include/module.h
objects/helios/test1.o : /helios/include/memory.h
objects/helios/test1.o : /helios/include/queue.h
objects/helios/test1.o : /helios/include/message.h
objects/helios/test1.o : /helios/include/protect.h
objects/helios/test1.o : /helios/include/sem.h
objects/helios/test1.o : /helios/include/servlib.h
objects/helios/test1.o : /helios/include/codes.h
objects/helios/test1.o : /helios/include/gsp.h
objects/helios/test1.o : /helios/include/setjmp.h
objects/helios/test2.o : source/test/test2.c
objects/helios/test2.o : ../hdr/drawp/public.h
objects/helios/test2.o : ../hdr/drawp/coordinates.h
objects/helios/test2.o : ../hdr/drawp/pixmaps.h
objects/helios/test2.o : ../hdr/drawp/clipping.h
objects/helios/test2.o : /helios/include/stdio.h
objects/helios/test2.o : ../hdr/drawp/graphicsContexts.h
objects/helios/test2.o : /helios/include/ctype.h
objects/helios/test2.o : /helios/include/string.h
objects/helios/test2.o : /helios/include/stdlib.h
objects/helios/test2.o : /helios/include/stddef.h
objects/helios/test2.o : /helios/include/gsp.h
objects/helios/test2.o : /helios/include/syslib.h
objects/helios/test2.o : /helios/include/task.h
objects/helios/test2.o : /helios/include/module.h
objects/helios/test2.o : /helios/include/helios.h
objects/helios/test2.o : /helios/include/memory.h
objects/helios/test2.o : /helios/include/queue.h
objects/helios/test2.o : /helios/include/message.h
objects/helios/test2.o : /helios/include/protect.h
objects/helios/test2.o : /helios/include/sem.h
objects/helios/test2.o : /helios/include/ioevents.h
objects/helios/test2.o : /helios/include/process.h
objects/helios/test2.o : /helios/include/attrib.h
objects/helios/test2.o : /helios/include/codes.h
objects/helios/test3.o : source/test/test3.c
objects/helios/test3.o : ../hdr/microlink/general.h
objects/helios/test3.o : /helios/include/time.h
objects/helios/test3.o : /helios/include/fault.h
objects/helios/test3.o : /helios/include/syslib.h
objects/helios/test3.o : /helios/include/task.h
objects/helios/test3.o : /helios/include/module.h
objects/helios/test3.o : /helios/include/helios.h
objects/helios/test3.o : /helios/include/memory.h
objects/helios/test3.o : /helios/include/queue.h
objects/helios/test3.o : /helios/include/message.h
objects/helios/test3.o : /helios/include/protect.h
objects/helios/test3.o : /helios/include/sem.h
objects/helios/test3.o : /helios/include/codes.h
objects/helios/test3.o : /helios/include/process.h
objects/helios/test3.o : /helios/include/ctype.h
objects/helios/test3.o : /helios/include/string.h
objects/helios/test3.o : /helios/include/stdio.h
objects/helios/test3.o : /helios/include/stdlib.h
objects/helios/test3.o : /helios/include/stddef.h
objects/helios/test4.o : source/test/test4.c
objects/helios/test4.o : ../hdr/microlink/phone.h
objects/helios/test4.o : /helios/include/syslib.h
objects/helios/test4.o : /helios/include/task.h
objects/helios/test4.o : /helios/include/module.h
objects/helios/test4.o : /helios/include/helios.h
objects/helios/test4.o : /helios/include/memory.h
objects/helios/test4.o : /helios/include/queue.h
objects/helios/test4.o : /helios/include/message.h
objects/helios/test4.o : /helios/include/protect.h
objects/helios/test4.o : /helios/include/sem.h
objects/helios/test4.o : /helios/include/stdlib.h
objects/helios/test4.o : /helios/include/stddef.h
objects/helios/test4.o : /helios/include/time.h
objects/helios/test4.o : /helios/include/fault.h
objects/helios/test4.o : /helios/include/codes.h
objects/helios/test4.o : /helios/include/process.h
objects/helios/test4.o : /helios/include/ctype.h
objects/helios/test4.o : /helios/include/string.h
objects/helios/test4.o : /helios/include/stdio.h
objects/helios/test5.o : source/test/test5.c
objects/helios/test5.o : ../hdr/microlink/digitiser.h
objects/helios/test5.o : /helios/include/time.h
objects/helios/test5.o : /helios/include/fault.h
objects/helios/test5.o : /helios/include/syslib.h
objects/helios/test5.o : /helios/include/task.h
objects/helios/test5.o : /helios/include/module.h
objects/helios/test5.o : /helios/include/helios.h
objects/helios/test5.o : /helios/include/memory.h
objects/helios/test5.o : /helios/include/queue.h
objects/helios/test5.o : /helios/include/message.h
objects/helios/test5.o : /helios/include/protect.h
objects/helios/test5.o : /helios/include/sem.h
objects/helios/test5.o : /helios/include/codes.h
objects/helios/test5.o : /helios/include/process.h
objects/helios/test5.o : /helios/include/ctype.h
objects/helios/test5.o : /helios/include/string.h
objects/helios/test5.o : /helios/include/stdio.h
objects/helios/test5.o : /helios/include/stdlib.h
objects/helios/test5.o : /helios/include/stddef.h
