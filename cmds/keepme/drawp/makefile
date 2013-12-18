# $Header: makefile,v 1.35 90/11/23 15:45:53 charles Locked $
# $Source: /server/usr/users/b/charles/world/drawp/RCS/makefile,v $
# $State: Release_1_0_34_GLOBAL $

#                                                            'makefile'

# Blitter makefile
# Makefile for blitter ported onto RISCiX from e2s RISCOS version
# About to be converted into a mush better drawing package

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

INC = -Ilochdr,../hdr

#----------------------------------------------------------------------
#                                                 Major Dummy Targets
#----------------------------------------------------------------------

# Use 'make secondaries' or just 'make' to make a module's librarys e.t.c.
# Use 'make tertiaries' to make the executables e.t.c.
# Refer to Source Management Documents if confused

secondaries: preliminaries
	make real_secondaries

heliosOnly: helios_preliminaries
	make helios_secondaries

unixOnly:     unix_preliminaries
	make unix_secondaries

tertiaries: preliminaries externals
	make real_tertiaries

preliminaries:
	chd -ansi   -cobjects/unix/cache   -Oobjects/unix   $(INC) source/*.c source/*/*.c
	chd -helios -cobjects/helios/cache -Oobjects/helios $(INC) source/*.c source/*/*.c -t

unix_preliminaries:
	chd -ansi   -cobjects/unix/cache   -Oobjects/unix   \
		$(INC) source/*.c source/*/*.c

helios_preliminaries:
	chd -helios -cobjects/helios/cache -Oobjects/helios \
		$(INC) source/*.c source/*/*.c

externals:
# No other module's secondaries required.

real_secondaries: unix_secondaries helios_secondaries

unix_secondaries: out/unix/drawp.a
	@echo "The Unix draw package library has been made"

helios_secondaries: out/helios/drawp.o
	@echo "The Helios draw package library has been made"

real_tertiaries: unix_tertiaries helios_tertiaries

unix_tertiaries: unix_secondaries
unix_tertiaries: out/unix/test1  out/unix/test2  out/unix/test3
unix_tertiaries: out/unix/test4  out/unix/test5  out/unix/test6
unix_tertiaries: out/unix/test7  out/unix/test1L out/unix/test1C 
unix_tertiaries: out/unix/test1P out/unix/test1M out/unix/test2M

helios_tertiaries: helios_secondaries
helios_tertiaries: out/helios/test1  out/helios/test2  out/helios/test3
helios_tertiaries: out/helios/test4  out/helios/test5  out/helios/test6
helios_tertiaries: out/helios/test7  out/helios/test1L out/helios/test1P

#----------------------------------------------------------------------
#                                                  Convenient Aliases
#----------------------------------------------------------------------

# 'make all' is easier to type that 'make tertiaries'

all: tertiaries

quick:   real_tertiaries
quickh:  helios_tertiaries
quicku:  unix_tertiaries

rect.l: listings/rectangles
lines.l: listings/lines

#----------------------------------------------------------------------
#                                                         Unix Macros 
#----------------------------------------------------------------------

unix_objs = 	\
	objects/unix/rectangles.o	objects/unix/gc_decoder.o	\
	objects/unix/construct.o	objects/unix/debugger.o		\
	objects/unix/lines.o		objects/unix/thinLines.o	\
	objects/unix/anyLines.o		objects/unix/clipping.o		\
	objects/unix/blockOps.o		objects/unix/polygons.o		\
	objects/unix/tables.o		objects/unix/circles.o		\
	objects/unix/arithmetic.o	objects/unix/rectRequests.o	\
	objects/unix/capJoinStyles.o

# The eight quadrant object files compiled from 'source/lineQuadrant.c':

unix_quads =	\
	objects/unix/quadXnYnMx.o	\
	objects/unix/quadXnYnMy.o	\
	objects/unix/quadXnYpMx.o	\
	objects/unix/quadXnYpMy.o	\
	objects/unix/quadXpYnMx.o	\
	objects/unix/quadXpYnMy.o	\
	objects/unix/quadXpYpMx.o	\
	objects/unix/quadXpYpMy.o

unix_libraries = out/unix/drawp.a -lm 

#----------------------------------------------------------------------
#                                                   Unix Dependencies
#----------------------------------------------------------------------

# The executables:

out/unix/test1: objects/unix/test1.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test1"
	@cc -ansi -o out/unix/test1  objects/unix/maskCtl.o \
		objects/unix/test1.o $(unix_libraries)

out/unix/test2: objects/unix/test2.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test2"
	@cc -ansi -o out/unix/test2  objects/unix/maskCtl.o \
		objects/unix/test2.o $(unix_libraries)

out/unix/test3: objects/unix/test3.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test3"
	@cc -ansi -o out/unix/test3  objects/unix/maskCtl.o \
		objects/unix/test3.o $(unix_libraries)

out/unix/test4: objects/unix/test4.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test4"
	@cc -ansi -o out/unix/test4  objects/unix/maskCtl.o \
		objects/unix/test4.o $(unix_libraries)

out/unix/test5: objects/unix/test5.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test5"
	@cc -ansi -o out/unix/test5  objects/unix/maskCtl.o \
		objects/unix/test5.o $(unix_libraries)

out/unix/test6: objects/unix/test6.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test6"
	@cc -ansi -o out/unix/test6  objects/unix/maskCtl.o \
		objects/unix/test6.o $(unix_libraries)

out/unix/test7: objects/unix/test7.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test7"
	@cc -ansi -o out/unix/test7  objects/unix/maskCtl.o \
		objects/unix/test7.o $(unix_libraries)

out/unix/test1L: objects/unix/test1L.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test1L"
	@cc -ansi -o out/unix/test1L  objects/unix/maskCtl.o \
		objects/unix/test1L.o $(unix_libraries)

out/unix/test1C: objects/unix/test1C.o out/unix/drawp.a
	@echo "Linking : out/unix/test1C"
	@cc -ansi -o out/unix/test1C  objects/unix/test1C.o $(unix_libraries)

out/unix/test1P: objects/unix/test1P.o objects/unix/maskCtl.o out/unix/drawp.a
	@echo "Linking : out/unix/test1P"
	@cc -ansi -o out/unix/test1P  objects/unix/maskCtl.o \
		objects/unix/test1P.o $(unix_libraries)

out/unix/test1M: objects/unix/test1M.o out/unix/drawp.a
	@echo "Linking : out/unix/test1M"
	@cc -ansi -o out/unix/test1P  objects/unix/maskCtl.o \
		objects/unix/test1P.o $(unix_libraries)

out/unix/test2M: objects/unix/test2M.o out/unix/drawp.a
	@echo "Linking : out/unix/test2M"
	@cc -ansi -o out/unix/test2M  objects/unix/test2M.o $(unix_libraries)

# The library:

out/unix/drawp.a: $(unix_objs) objects/unix/quadsMade
	@echo "Archiving all the unix object files"
	@-rm out/unix/drawp.a
	@ar r out/unix/drawp.a $(unix_objs) $(unix_quads)
	@ranlib out/unix/drawp.a

# The objects:

objects/unix/rectangles.o: source/rectangles.s  objects/unix/macasm 
objects/unix/rectangles.o: scripts/assemble
objects/unix/rectangles.o: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/unixMode.s objects/giveMode.ss
	scripts/assemble "objasm -Q" source/rectangles.s \
		objects/unix/rectangles.o

objects/unix/lines.o: source/lines.s objects/unix/macasm scripts/assemble
objects/unix/lines.o: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/unixMode.s objects/giveMode.ss
	scripts/assemble "objasm -Q" source/lines.s objects/unix/lines.o

objects/unix/arithmetic.o: source/arithmetic.s
objects/unix/arithmetic.o: objects/unix/macasm
objects/unix/arithmetic.o: lochdr/utility.ss
objects/unix/arithmetic.o: objects/unix/constants.ss
objects/unix/arithmetic.o: scripts/assemble
	@-rm -f objects/giveMode.ss
	cp lochdr/unixMode.s objects/giveMode.ss
	scripts/assemble "objasm -Q" source/arithmetic.s \
		objects/unix/arithmetic.o

objects/unix/quadsMade: source/lineClipping.c ext/clipping.h
objects/unix/quadsMade: lochdr/code_interface.h
objects/unix/quadsMade: lochdr/private.h
objects/unix/quadsMade: ext/clipping.h
objects/unix/quadsMade: scripts/quadrants scripts/doquad
	scripts/quadrants unix cc -ansi $(INC)
	touch objects/unix/quadsMade

objects/unix/constants.ss: objects/unix/constantXfer
	objects/unix/constantXfer >objects/unix/constants.ss

objects/unix/constantXfer: objects/unix/constantXfer.o
	cc -ansi -o objects/unix/constantXfer objects/unix/constantXfer.o

objects/unix/constantXfer.o:	source/constantXfer.c
	@echo "Compiling for Unix : constantXfer.c"
	@cc -ansi $(INC) -c source/constantXfer.c    \
			-o objects/unix/constantXfer.o

objects/unix/macros.o:	source/tools/macros.c
	@echo "Compiling for Unix : tools/macros.c"
	@cc -ansi -c source/tools/macros.c           \
			-o objects/unix/macros.o

objects/unix/anyLines.o:	source/anyLines.c
	@echo "Compiling for Unix : anyLines.c"
	@cc -ansi $(INC) -c source/anyLines.c        \
			-o objects/unix/anyLines.o

objects/unix/thinLines.o:	source/thinLines.c
	@echo "Compiling for Unix : thinLines.c"
	@cc -ansi $(INC) -c source/thinLines.c       \
			-o objects/unix/thinLines.o

objects/unix/gc_decoder.o:	source/gc_decoder.c
	@echo "Compiling for Unix : gc_decoder.c"
	@cc -ansi $(INC) -c source/gc_decoder.c      \
			-o objects/unix/gc_decoder.o

objects/unix/construct.o:	source/construct.c
	@echo "Compiling for Unix : construct.c"
	@cc -ansi $(INC) -c source/construct.c       \
			-o objects/unix/construct.o

objects/unix/clipping.o:	source/clipping.c
	@echo "Compiling for Unix : clipping.c"
	@cc -ansi $(INC) -c source/clipping.c        \
			-o objects/unix/clipping.o

objects/unix/blockOps.o:	source/blockOps.c
	@echo "Compiling for Unix : blockOps.c"
	@cc -ansi $(INC) -c source/blockOps.c        \
			-o objects/unix/blockOps.o

objects/unix/polygons.o:	source/polygons.c
	@echo "Compiling for Unix : polygons.c"
	@cc -ansi $(INC) -c source/polygons.c        \
			-o objects/unix/polygons.o

objects/unix/capJoinStyles.o:	source/capJoinStyles.c
	@echo "Compiling for Unix : capJoinStyles.c"
	@cc -ansi $(INC) -c source/capJoinStyles.c   \
			-o objects/unix/capJoinStyles.o

objects/unix/tables.o:	objects/unix/tables.c
	@echo "Compiling for Unix : objects/unix/tables.c"
	@cc -ansi $(INC) -c objects/unix/tables.c    \
			-o objects/unix/tables.o

objects/unix/tables.c:	objects/unix/tableGenerator
	objects/unix/tableGenerator >objects/unix/tables.c

objects/unix/tableGenerator: objects/unix/tableGenerator.o
	cc -ansi -o objects/unix/tableGenerator objects/unix/tableGenerator.o -lm

objects/unix/tableGenerator.o:	source/tableGenerator.c
	@echo "Compiling for Unix : tableGenerator.c"
	@cc -ansi $(INC) -c source/tableGenerator.c  \
			-o objects/unix/tableGenerator.o

objects/unix/circles.o:	source/circles.c
	@echo "Compiling for Unix : circles.c"
	@cc -ansi $(INC) -c source/circles.c         \
			-o objects/unix/circles.o

objects/unix/rectRequests.o:	source/rectRequests.c
	@echo "Compiling for Unix : rectRequests.c"
	@cc -ansi $(INC) -c source/rectRequests.c    \
			-o objects/unix/rectRequests.o

objects/unix/debugger.o:	source/debugger.c
	@echo "Compiling for Unix : debugger.c"
	@cc -ansi $(INC) -c source/debugger.c        \
			-o objects/unix/debugger.o

objects/unix/maskCtl.o:	source/test/maskCtl.c
	@echo "Compiling for Unix : test/maskCtl.c"
	@cc -ansi $(INC) -c source/test/maskCtl.c  -o objects/unix/maskCtl.o

objects/unix/test1.o:	source/test/test1.c
	@echo "Compiling for Unix : test/test1.c"
	@cc -ansi $(INC) -c source/test/test1.c    -o objects/unix/test1.o

objects/unix/test2.o:	source/test/test2.c
	@echo "Compiling for Unix : test/test2.c"
	@cc -ansi $(INC) -c source/test/test2.c    -o objects/unix/test2.o

objects/unix/test3.o:	source/test/test3.c
	@echo "Compiling for Unix : test/test3.c"
	@cc -ansi $(INC) -c source/test/test3.c    -o objects/unix/test3.o

objects/unix/test4.o:	source/test/test4.c
	@echo "Compiling for Unix : test/test4.c"
	@cc -ansi $(INC) -c source/test/test4.c    -o objects/unix/test4.o

objects/unix/test5.o:	source/test/test5.c
	@echo "Compiling for Unix : test/test5.c"
	@cc -ansi $(INC) -c source/test/test5.c    -o objects/unix/test5.o

objects/unix/test6.o:	source/test/test6.c
	@echo "Compiling for Unix : test/test6.c"
	@cc -ansi $(INC) -c source/test/test6.c    -o objects/unix/test6.o

objects/unix/test7.o:	source/test/test7.c
	@echo "Compiling for Unix : test/test7.c"
	@cc -ansi $(INC) -c source/test/test7.c    -o objects/unix/test7.o

objects/unix/test1L.o:	source/test/test1L.c
	@echo "Compiling for Unix : test/test1L.c"
	@cc -ansi $(INC) -c source/test/test1L.c   -o objects/unix/test1L.o

objects/unix/test1C.o:	source/test/test1C.c
	@echo "Compiling for Unix : test/test1C.c"
	@cc -ansi $(INC) -c source/test/test1C.c   -o objects/unix/test1C.o

objects/unix/test1P.o:	source/test/test1P.c
	@echo "Compiling for Unix : test/test1P.c"
	@cc -ansi $(INC) -c source/test/test1P.c   -o objects/unix/test1P.o

objects/unix/test1M.o:	source/test/test1M.c
	@echo "Compiling for Unix : test/test1M.c"
	@cc -ansi $(INC) -c source/test/test1M.c   -o objects/unix/test1M.o

objects/unix/test2M.o:	source/test/test2M.c
	@echo "Compiling for Unix : test/test2M.c"
	@cc -ansi $(INC) -c source/test/test2M.c   -o objects/unix/test2M.o

# Other intermediates:

objects/unix/macasm: objects/unix/macros.o
	cc -ansi -o objects/unix/macasm objects/unix/macros.o

#----------------------------------------------------------------------
#                                                       Unix Listings
#----------------------------------------------------------------------

listings/unix/rectangles: objects/unix/rectangles-s
listings/unix/rectangles: lochdr/structures.ss lochdr/utility.ss
	objasm objects/unix/rectangles-s objects/unix/rectangles.o -Q -P -WI 159 \
	> listings/unix/rectangles

objects/unix/rectangles-s: source/rectangles.s objects/unix/macasm
	objects/unix/macasm source/rectangles.s objects/unix/rectangles-s

objects/unix/lines-s: source/lines.s objects/unix/macasm
	objects/unix/macasm source/lines.s objects/unix/lines-s

# Data:

data: objects/data
	objects/data >data

objects/data: source/data.c
	cc -ansi source/data.c -o objects/data -lm
	@rm *.o

#----------------------------------------------------------------------
#                                                       Helios Macros 
#----------------------------------------------------------------------

helios_objs = 	\
	objects/helios/rectangles.o	objects/helios/gc_decoder.o	\
	objects/helios/construct.o	objects/helios/debugger.o	\
	objects/helios/lines.o		objects/helios/thinLines.o	\
	objects/helios/anyLines.o	objects/helios/clipping.o	\
	objects/helios/blockOps.o	objects/helios/polygons.o	\
	objects/helios/tables.o		objects/helios/circles.o	\
	objects/helios/arithmetic.o	objects/helios/rectRequests.o	\
	objects/helios/capJoinStyles.o

# The eight quadrant object files compiled from 'source/lineQuadrant.c':

helios_quads =	\
	objects/helios/quadXnYnMx.o	\
	objects/helios/quadXnYnMy.o	\
	objects/helios/quadXnYpMx.o	\
	objects/helios/quadXnYpMy.o	\
	objects/helios/quadXpYnMx.o	\
	objects/helios/quadXpYnMy.o	\
	objects/helios/quadXpYpMx.o	\
	objects/helios/quadXpYpMy.o

helios_libraries = out/helios/drawp.o links/fplib.o

#----------------------------------------------------------------------
#                                                 Helios Dependencies
#----------------------------------------------------------------------

# The executables:

out/helios/test1: objects/helios/test1.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test1"
	@armlink -s65536 -o out/helios/test1  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test1.o  $(helios_libraries)

out/helios/test2: objects/helios/test2.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test2"
	@armlink -s65536 -o out/helios/test2  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test2.o $(helios_libraries)

out/helios/test3: objects/helios/test3.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test3"
	@armlink -s65536 -o out/helios/test3  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test3.o $(helios_libraries)

out/helios/test4: objects/helios/test4.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test4"
	@armlink -s65536 -o out/helios/test4  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test4.o $(helios_libraries)

out/helios/test5: objects/helios/test5.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test5"
	@armlink -s65536 -o out/helios/test5  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test5.o $(helios_libraries)

out/helios/test6: objects/helios/test6.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test6"
	@armlink -s65536 -o out/helios/test6  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test6.o $(helios_libraries)

out/helios/test7: objects/helios/test7.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test7"
	@armlink -s65536 -o out/helios/test7  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test7.o $(helios_libraries)

out/helios/test1L: objects/helios/test1L.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test1L"
	@armlink -s65536 -o out/helios/test1L  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test1L.o $(helios_libraries)

out/helios/test1P: objects/helios/test1P.o objects/helios/maskCtl.o out/helios/drawp.o
	@echo "Linking : out/helios/test1P"
	@armlink -s65536 -o out/helios/test1P  /helios/lib/cstart.o \
		objects/helios/maskCtl.o	\
		objects/helios/test1P.o $(helios_libraries)

# The library:

out/helios/drawp.o: $(helios_objs) objects/helios/quadsMade
	@echo "Archiving all the helios object files"
	@cat $(helios_objs) $(helios_quads) > out/helios/drawp.o

# The objects:

objects/helios/rectangles.o: source/rectangles.s  objects/unix/macasm
objects/helios/rectangles.o: scripts/assemble
objects/helios/rectangles.o: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/heliosMode.s objects/giveMode.ss
	scripts/assemble hobjasm source/rectangles.s objects/helios/rectangles.o

objects/helios/lines.o: source/lines.s objects/unix/macasm scripts/assemble
objects/helios/lines.o: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/heliosMode.s objects/giveMode.ss
	scripts/assemble hobjasm source/lines.s objects/helios/lines.o

objects/helios/arithmetic.o: source/arithmetic.s
objects/helios/arithmetic.o: objects/unix/macasm
objects/helios/arithmetic.o: lochdr/utility.ss
objects/helios/arithmetic.o: objects/unix/constants.ss
objects/helios/arithmetic.o: scripts/assemble
	@-rm -f objects/giveMode.ss
	cp lochdr/heliosMode.s objects/giveMode.ss
	scripts/assemble hobjasm source/arithmetic.s objects/helios/arithmetic.o

objects/helios/quadsMade: source/lineClipping.c ext/clipping.h
objects/helios/quadsMade: lochdr/code_interface.h
objects/helios/quadsMade: lochdr/private.h
objects/helios/quadsMade: ext/clipping.h
objects/helios/quadsMade: scripts/quadrants scripts/doquad
	scripts/quadrants helios ncc -Zt -j/helios/include -DHeliosMode $(INC)
	touch objects/helios/quadsMade

objects/helios/anyLines.o:	source/anyLines.c
	@echo "Compiling for Helios : anyLines.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/anyLines.c        \
			-o objects/helios/anyLines.o

objects/helios/thinLines.o:	source/thinLines.c
	@echo "Compiling for Helios : thinLines.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/thinLines.c       \
			-o objects/helios/thinLines.o

objects/helios/gc_decoder.o:	source/gc_decoder.c
	@echo "Compiling for Helios : gc_decoder.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/gc_decoder.c      \
			-o objects/helios/gc_decoder.o

objects/helios/construct.o:	source/construct.c
	@echo "Compiling for Helios : construct.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/construct.c       \
			-o objects/helios/construct.o

objects/helios/clipping.o:	source/clipping.c
	@echo "Compiling for Helios : clipping.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/clipping.c        \
			-o objects/helios/clipping.o

objects/helios/blockOps.o:	source/blockOps.c
	@echo "Compiling for Helios : blockOps.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/blockOps.c        \
			-o objects/helios/blockOps.o

objects/helios/polygons.o:	source/polygons.c
	@echo "Compiling for Helios : polygons.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/polygons.c        \
			-o objects/helios/polygons.o

objects/helios/capJoinStyles.o:	source/capJoinStyles.c
	@echo "Compiling for Helios : capJoinStyles.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/capJoinStyles.c   \
			-o objects/helios/capJoinStyles.o

objects/helios/tables.o:	objects/unix/tables.c
	@echo "Compiling for Helios : objects/unix/tables.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c objects/unix/tables.c    \
			-o objects/helios/tables.o

objects/helios/circles.o:	source/circles.c
	@echo "Compiling for Helios : circles.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/circles.c         \
			-o objects/helios/circles.o

objects/helios/rectRequests.o:	source/rectRequests.c
	@echo "Compiling for Helios : rectRequests.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/rectRequests.c    \
			-o objects/helios/rectRequests.o

objects/helios/debugger.o:	source/debugger.c
	@echo "Compiling for Helios : debugger.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/debugger.c        \
			-o objects/helios/debugger.o

objects/helios/maskCtl.o: source/test/maskCtl.c
	@echo "Compiling for Helios : test/maskCtl.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/maskCtl.c    \
			-o objects/helios/maskCtl.o

objects/helios/test1.o:	source/test/test1.c
	@echo "Compiling for Helios : test/test1.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test1.c      \
			-o objects/helios/test1.o

objects/helios/test2.o:	source/test/test2.c
	@echo "Compiling for Helios : test/test2.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test2.c      \
			-o objects/helios/test2.o

objects/helios/test3.o:	source/test/test3.c
	@echo "Compiling for Helios : test/test3.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test3.c      \
			-o objects/helios/test3.o

objects/helios/test4.o:	source/test/test4.c
	@echo "Compiling for Helios : test/test4.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test4.c      \
			-o objects/helios/test4.o

objects/helios/test5.o:	source/test/test5.c
	@echo "Compiling for Helios : test/test5.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test5.c      \
			-o objects/helios/test5.o

objects/helios/test6.o:	source/test/test6.c
	@echo "Compiling for Helios : test/test6.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test6.c      \
			-o objects/helios/test6.o

objects/helios/test7.o:	source/test/test7.c
	@echo "Compiling for Helios : test/test7.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test7.c      \
			-o objects/helios/test7.o

objects/helios/test1L.o:	source/test/test1L.c
	@echo "Compiling for Helios : test/test1L.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test1L.c     \
			-o objects/helios/test1L.o

objects/helios/test1C.o:	source/test/test1C.c
	@echo "Compiling for Helios : test/test1C.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test1C.c     \
			-o objects/helios/test1C.o

objects/helios/test1P.o:	source/test/test1P.c
	@echo "Compiling for Helios : test/test1P.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test1P.c     \
			-o objects/helios/test1P.o

objects/helios/test1M.o:	source/test/test1M.c
	@echo "Compiling for Helios : test/test1M.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test1M.c     \
			-o objects/helios/test1M.o

objects/helios/test2M.o:	source/test/test2M.c
	@echo "Compiling for Helios : test/test2M.c"
	@ncc -Zt -j/helios/include -DHeliosMode $(INC) -c source/test/test2M.c     \
			-o objects/helios/test2M.o

#----------------------------------------------------------------------
#                                                     Helios Listings
#----------------------------------------------------------------------

listings/helios/rectangles: objects/unix/rectangles-s
listings/helios/rectangles: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/heliosMode.s objects/giveMode.ss
	hobjasm objects/unix/rectangles-s objects/helios/rectangles.o -Q -P -WI 159 \
	> listings/helios/rectangles

listings/helios/lines: objects/unix/lines-s
listings/helios/lines: lochdr/structures.ss lochdr/utility.ss
	@-rm -f objects/giveMode.ss
	cp lochdr/heliosMode.s objects/giveMode.ss
	hobjasm objects/unix/lines-s objects/helios/lines.o -Q -P -WI 159 \
	> listings/helios/lines

#----------------------------------------------------------------------
#                                              Auto-generated section
#----------------------------------------------------------------------

# All lines after the marker '#auto:#' are generated by the output
#   of the 'chd' utility run by the target 'preliminaries' They show
#   the dependencies of the object files that would be generated on
#   the sources.

#auto:#
objects/unix/anyLines.o : source/anyLines.c
objects/unix/anyLines.o : lochdr/private.h
objects/unix/anyLines.o : ../hdr/drawp/debug.h
objects/unix/anyLines.o : /usr/include/ansi/stdio.h
objects/unix/anyLines.o : ../hdr/drawp/public.h
objects/unix/anyLines.o : ../hdr/drawp/coordinates.h
objects/unix/anyLines.o : ../hdr/drawp/pixmaps.h
objects/unix/anyLines.o : ../hdr/drawp/clipping.h
objects/unix/anyLines.o : ../hdr/drawp/graphicsContexts.h
objects/unix/anyLines.o : lochdr/flattening.h
objects/unix/anyLines.o : lochdr/code_interface.h
objects/unix/anyLines.o : /usr/include/ansi/stdlib.h
objects/unix/blockOps.o : source/blockOps.c
objects/unix/blockOps.o : lochdr/private.h
objects/unix/blockOps.o : ../hdr/drawp/debug.h
objects/unix/blockOps.o : /usr/include/ansi/stdio.h
objects/unix/blockOps.o : ../hdr/drawp/public.h
objects/unix/blockOps.o : ../hdr/drawp/coordinates.h
objects/unix/blockOps.o : ../hdr/drawp/pixmaps.h
objects/unix/blockOps.o : ../hdr/drawp/clipping.h
objects/unix/blockOps.o : ../hdr/drawp/graphicsContexts.h
objects/unix/blockOps.o : lochdr/flattening.h
objects/unix/blockOps.o : lochdr/code_interface.h
objects/unix/blockOps.o : /usr/include/ansi/stdlib.h
objects/unix/capJoinStyles.o : source/capJoinStyles.c
objects/unix/capJoinStyles.o : lochdr/private.h
objects/unix/capJoinStyles.o : ../hdr/drawp/debug.h
objects/unix/capJoinStyles.o : /usr/include/ansi/stdio.h
objects/unix/capJoinStyles.o : ../hdr/drawp/public.h
objects/unix/capJoinStyles.o : ../hdr/drawp/coordinates.h
objects/unix/capJoinStyles.o : ../hdr/drawp/pixmaps.h
objects/unix/capJoinStyles.o : ../hdr/drawp/clipping.h
objects/unix/capJoinStyles.o : ../hdr/drawp/graphicsContexts.h
objects/unix/capJoinStyles.o : lochdr/flattening.h
objects/unix/capJoinStyles.o : lochdr/code_interface.h
objects/unix/capJoinStyles.o : /usr/include/ansi/stdlib.h
objects/unix/capJoinStyles.o : /usr/include/ansi/stddef.h
objects/unix/circles.o : source/circles.c
objects/unix/circles.o : lochdr/private.h
objects/unix/circles.o : ../hdr/drawp/debug.h
objects/unix/circles.o : /usr/include/ansi/stdio.h
objects/unix/circles.o : ../hdr/drawp/public.h
objects/unix/circles.o : ../hdr/drawp/coordinates.h
objects/unix/circles.o : ../hdr/drawp/pixmaps.h
objects/unix/circles.o : ../hdr/drawp/clipping.h
objects/unix/circles.o : ../hdr/drawp/graphicsContexts.h
objects/unix/circles.o : lochdr/flattening.h
objects/unix/circles.o : lochdr/code_interface.h
objects/unix/circles.o : /usr/include/ansi/stdlib.h
objects/unix/circles.o : /usr/include/ansi/stddef.h
objects/unix/clipping.o : source/clipping.c
objects/unix/clipping.o : lochdr/private.h
objects/unix/clipping.o : ../hdr/drawp/debug.h
objects/unix/clipping.o : /usr/include/ansi/stdio.h
objects/unix/clipping.o : ../hdr/drawp/public.h
objects/unix/clipping.o : ../hdr/drawp/coordinates.h
objects/unix/clipping.o : ../hdr/drawp/pixmaps.h
objects/unix/clipping.o : ../hdr/drawp/clipping.h
objects/unix/clipping.o : ../hdr/drawp/graphicsContexts.h
objects/unix/clipping.o : lochdr/flattening.h
objects/unix/clipping.o : lochdr/code_interface.h
objects/unix/clipping.o : /usr/include/ansi/stdlib.h
objects/unix/clipping.o : /usr/include/ansi/limits.h
objects/unix/clipping.o : /usr/include/ansi/stddef.h
objects/unix/constantXfer.o : source/constantXfer.c
objects/unix/constantXfer.o : lochdr/private.h
objects/unix/constantXfer.o : ../hdr/drawp/debug.h
objects/unix/constantXfer.o : /usr/include/ansi/stdio.h
objects/unix/constantXfer.o : ../hdr/drawp/public.h
objects/unix/constantXfer.o : ../hdr/drawp/coordinates.h
objects/unix/constantXfer.o : ../hdr/drawp/pixmaps.h
objects/unix/constantXfer.o : ../hdr/drawp/clipping.h
objects/unix/constantXfer.o : ../hdr/drawp/graphicsContexts.h
objects/unix/constantXfer.o : lochdr/flattening.h
objects/unix/constantXfer.o : lochdr/code_interface.h
objects/unix/constantXfer.o : /usr/include/ansi/stdlib.h
objects/unix/constantXfer.o : /usr/include/ansi/stddef.h
objects/unix/construct.o : source/construct.c
objects/unix/construct.o : lochdr/sys/photron.h
objects/unix/construct.o : lochdr/sys/ioctl.h
objects/unix/construct.o : lochdr/sys/ttydev.h
objects/unix/construct.o : lochdr/sys/ttychars.h
objects/unix/construct.o : lochdr/sys/vcioctl.h
objects/unix/construct.o : lochdr/sys/fbioctl.h
objects/unix/construct.o : lochdr/sys/types.h
objects/unix/construct.o : lochdr/private.h
objects/unix/construct.o : ../hdr/drawp/debug.h
objects/unix/construct.o : /usr/include/ansi/stdio.h
objects/unix/construct.o : ../hdr/drawp/public.h
objects/unix/construct.o : ../hdr/drawp/coordinates.h
objects/unix/construct.o : ../hdr/drawp/pixmaps.h
objects/unix/construct.o : ../hdr/drawp/clipping.h
objects/unix/construct.o : ../hdr/drawp/graphicsContexts.h
objects/unix/construct.o : lochdr/flattening.h
objects/unix/construct.o : lochdr/code_interface.h
objects/unix/construct.o : /usr/include/ansi/stdlib.h
objects/unix/construct.o : /usr/include/ansi/stddef.h
objects/unix/data.o : source/data.c
objects/unix/data.o : /usr/include/ansi/math.h
objects/unix/data.o : /usr/include/ansi/stdio.h
objects/unix/data.o : /usr/include/ansi/stdlib.h
objects/unix/data.o : /usr/include/ansi/stddef.h
objects/unix/debugger.o : source/debugger.c
objects/unix/debugger.o : ../hdr/drawp/debug.h
objects/unix/debugger.o : /usr/include/ansi/stdio.h
objects/unix/debugger.o : lochdr/private.h
objects/unix/debugger.o : ../hdr/drawp/public.h
objects/unix/debugger.o : ../hdr/drawp/coordinates.h
objects/unix/debugger.o : ../hdr/drawp/pixmaps.h
objects/unix/debugger.o : ../hdr/drawp/clipping.h
objects/unix/debugger.o : ../hdr/drawp/graphicsContexts.h
objects/unix/debugger.o : lochdr/flattening.h
objects/unix/debugger.o : lochdr/code_interface.h
objects/unix/debugger.o : /usr/include/ansi/stdlib.h
objects/unix/debugger.o : /usr/include/ansi/ctype.h
objects/unix/debugger.o : /usr/include/ansi/stddef.h
objects/unix/gc_decoder.o : source/gc_decoder.c
objects/unix/gc_decoder.o : lochdr/private.h
objects/unix/gc_decoder.o : ../hdr/drawp/debug.h
objects/unix/gc_decoder.o : /usr/include/ansi/stdio.h
objects/unix/gc_decoder.o : ../hdr/drawp/public.h
objects/unix/gc_decoder.o : ../hdr/drawp/coordinates.h
objects/unix/gc_decoder.o : ../hdr/drawp/pixmaps.h
objects/unix/gc_decoder.o : ../hdr/drawp/clipping.h
objects/unix/gc_decoder.o : ../hdr/drawp/graphicsContexts.h
objects/unix/gc_decoder.o : lochdr/flattening.h
objects/unix/gc_decoder.o : lochdr/code_interface.h
objects/unix/gc_decoder.o : /usr/include/ansi/stdlib.h
objects/unix/lineClipping.o : source/lineClipping.c
objects/unix/lineClipping.o : lochdr/code_interface.h
objects/unix/lineClipping.o : /usr/include/ansi/stdlib.h
objects/unix/lineClipping.o : /usr/include/ansi/stdio.h
objects/unix/lineClipping.o : ../hdr/drawp/pixmaps.h
objects/unix/lineClipping.o : ../hdr/drawp/clipping.h
objects/unix/lineClipping.o : lochdr/private.h
objects/unix/lineClipping.o : ../hdr/drawp/debug.h
objects/unix/lineClipping.o : ../hdr/drawp/public.h
objects/unix/lineClipping.o : ../hdr/drawp/coordinates.h
objects/unix/lineClipping.o : ../hdr/drawp/graphicsContexts.h
objects/unix/lineClipping.o : lochdr/flattening.h
objects/unix/polygons.o : source/polygons.c
objects/unix/polygons.o : lochdr/private.h
objects/unix/polygons.o : ../hdr/drawp/debug.h
objects/unix/polygons.o : /usr/include/ansi/stdio.h
objects/unix/polygons.o : ../hdr/drawp/public.h
objects/unix/polygons.o : ../hdr/drawp/coordinates.h
objects/unix/polygons.o : ../hdr/drawp/pixmaps.h
objects/unix/polygons.o : ../hdr/drawp/clipping.h
objects/unix/polygons.o : ../hdr/drawp/graphicsContexts.h
objects/unix/polygons.o : lochdr/flattening.h
objects/unix/polygons.o : lochdr/code_interface.h
objects/unix/polygons.o : /usr/include/ansi/stdlib.h
objects/unix/polygons.o : /usr/include/ansi/limits.h
objects/unix/polygons.o : /usr/include/ansi/stddef.h
objects/unix/rectRequests.o : source/rectRequests.c
objects/unix/rectRequests.o : lochdr/private.h
objects/unix/rectRequests.o : ../hdr/drawp/debug.h
objects/unix/rectRequests.o : /usr/include/ansi/stdio.h
objects/unix/rectRequests.o : ../hdr/drawp/public.h
objects/unix/rectRequests.o : ../hdr/drawp/coordinates.h
objects/unix/rectRequests.o : ../hdr/drawp/pixmaps.h
objects/unix/rectRequests.o : ../hdr/drawp/clipping.h
objects/unix/rectRequests.o : ../hdr/drawp/graphicsContexts.h
objects/unix/rectRequests.o : lochdr/flattening.h
objects/unix/rectRequests.o : lochdr/code_interface.h
objects/unix/rectRequests.o : /usr/include/ansi/stdlib.h
objects/unix/tableGenerator.o : source/tableGenerator.c
objects/unix/tableGenerator.o : lochdr/private.h
objects/unix/tableGenerator.o : ../hdr/drawp/debug.h
objects/unix/tableGenerator.o : /usr/include/ansi/stdio.h
objects/unix/tableGenerator.o : ../hdr/drawp/public.h
objects/unix/tableGenerator.o : ../hdr/drawp/coordinates.h
objects/unix/tableGenerator.o : ../hdr/drawp/pixmaps.h
objects/unix/tableGenerator.o : ../hdr/drawp/clipping.h
objects/unix/tableGenerator.o : ../hdr/drawp/graphicsContexts.h
objects/unix/tableGenerator.o : lochdr/flattening.h
objects/unix/tableGenerator.o : lochdr/code_interface.h
objects/unix/tableGenerator.o : /usr/include/ansi/stdlib.h
objects/unix/tableGenerator.o : /usr/include/ansi/math.h
objects/unix/tableGenerator.o : /usr/include/ansi/stddef.h
objects/unix/test2.o : source/test2.c
objects/unix/test2.o : /usr/include/ansi/stdio.h
objects/unix/thinLines.o : source/thinLines.c
objects/unix/thinLines.o : lochdr/code_interface.h
objects/unix/thinLines.o : /usr/include/ansi/stdlib.h
objects/unix/thinLines.o : /usr/include/ansi/stdio.h
objects/unix/thinLines.o : ../hdr/drawp/pixmaps.h
objects/unix/thinLines.o : ../hdr/drawp/clipping.h
objects/unix/thinLines.o : lochdr/private.h
objects/unix/thinLines.o : ../hdr/drawp/debug.h
objects/unix/thinLines.o : ../hdr/drawp/public.h
objects/unix/thinLines.o : ../hdr/drawp/coordinates.h
objects/unix/thinLines.o : ../hdr/drawp/graphicsContexts.h
objects/unix/thinLines.o : lochdr/flattening.h
objects/unix/maskCtl.o : source/test/maskCtl.c
objects/unix/maskCtl.o : source/test/sys/time.h
objects/unix/maskCtl.o : /usr/include/ansi/time.h
objects/unix/maskCtl.o : source/test/masks.h
objects/unix/maskCtl.o : ../hdr/drawp/debug.h
objects/unix/maskCtl.o : /usr/include/ansi/stdio.h
objects/unix/maskCtl.o : ../hdr/drawp/public.h
objects/unix/maskCtl.o : ../hdr/drawp/coordinates.h
objects/unix/maskCtl.o : ../hdr/drawp/pixmaps.h
objects/unix/maskCtl.o : ../hdr/drawp/clipping.h
objects/unix/maskCtl.o : ../hdr/drawp/graphicsContexts.h
objects/unix/maskCtl.o : /usr/include/ansi/string.h
objects/unix/maskCtl.o : /usr/include/ansi/stddef.h
objects/unix/maskCtl.o : /usr/include/ansi/stdlib.h
objects/unix/test1.o : source/test/test1.c
objects/unix/test1.o : ../hdr/drawp/debug.h
objects/unix/test1.o : /usr/include/ansi/stdio.h
objects/unix/test1.o : source/test/masks.h
objects/unix/test1.o : ../hdr/drawp/public.h
objects/unix/test1.o : ../hdr/drawp/coordinates.h
objects/unix/test1.o : ../hdr/drawp/pixmaps.h
objects/unix/test1.o : ../hdr/drawp/clipping.h
objects/unix/test1.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test1.o : /usr/include/ansi/stddef.h
objects/unix/test1.o : /usr/include/ansi/stdlib.h
objects/unix/test1C.o : source/test/test1C.c
objects/unix/test1C.o : ../hdr/drawp/debug.h
objects/unix/test1C.o : /usr/include/ansi/stdio.h
objects/unix/test1C.o : ../hdr/drawp/public.h
objects/unix/test1C.o : ../hdr/drawp/coordinates.h
objects/unix/test1C.o : ../hdr/drawp/pixmaps.h
objects/unix/test1C.o : ../hdr/drawp/clipping.h
objects/unix/test1C.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test1C.o : /usr/include/ansi/math.h
objects/unix/test1C.o : /usr/include/ansi/stddef.h
objects/unix/test1C.o : /usr/include/ansi/stdlib.h
objects/unix/test1L.o : source/test/test1L.c
objects/unix/test1L.o : source/test/sys/time.h
objects/unix/test1L.o : /usr/include/ansi/time.h
objects/unix/test1L.o : ../hdr/drawp/debug.h
objects/unix/test1L.o : /usr/include/ansi/stdio.h
objects/unix/test1L.o : source/test/masks.h
objects/unix/test1L.o : ../hdr/drawp/public.h
objects/unix/test1L.o : ../hdr/drawp/coordinates.h
objects/unix/test1L.o : ../hdr/drawp/pixmaps.h
objects/unix/test1L.o : ../hdr/drawp/clipping.h
objects/unix/test1L.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test1L.o : /usr/include/ansi/math.h
objects/unix/test1L.o : /usr/include/ansi/stddef.h
objects/unix/test1L.o : /usr/include/ansi/stdlib.h
objects/unix/test1M.o : source/test/test1M.c
objects/unix/test1M.o : ../hdr/drawp/debug.h
objects/unix/test1M.o : /usr/include/ansi/stdio.h
objects/unix/test1M.o : ../hdr/drawp/public.h
objects/unix/test1M.o : ../hdr/drawp/coordinates.h
objects/unix/test1M.o : ../hdr/drawp/pixmaps.h
objects/unix/test1M.o : ../hdr/drawp/clipping.h
objects/unix/test1M.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test1M.o : /usr/include/ansi/math.h
objects/unix/test1M.o : /usr/include/ansi/stddef.h
objects/unix/test1M.o : /usr/include/ansi/stdlib.h
objects/unix/test1P.o : source/test/test1P.c
objects/unix/test1P.o : source/test/sys/time.h
objects/unix/test1P.o : /usr/include/ansi/time.h
objects/unix/test1P.o : ../hdr/drawp/debug.h
objects/unix/test1P.o : /usr/include/ansi/stdio.h
objects/unix/test1P.o : source/test/masks.h
objects/unix/test1P.o : ../hdr/drawp/public.h
objects/unix/test1P.o : ../hdr/drawp/coordinates.h
objects/unix/test1P.o : ../hdr/drawp/pixmaps.h
objects/unix/test1P.o : ../hdr/drawp/clipping.h
objects/unix/test1P.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test1P.o : /usr/include/ansi/math.h
objects/unix/test1P.o : /usr/include/ansi/stddef.h
objects/unix/test1P.o : /usr/include/ansi/stdlib.h
objects/unix/test2.o : source/test/test2.c
objects/unix/test2.o : source/test/sys/time.h
objects/unix/test2.o : /usr/include/ansi/time.h
objects/unix/test2.o : ../hdr/drawp/debug.h
objects/unix/test2.o : /usr/include/ansi/stdio.h
objects/unix/test2.o : source/test/masks.h
objects/unix/test2.o : ../hdr/drawp/public.h
objects/unix/test2.o : ../hdr/drawp/coordinates.h
objects/unix/test2.o : ../hdr/drawp/pixmaps.h
objects/unix/test2.o : ../hdr/drawp/clipping.h
objects/unix/test2.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test2.o : /usr/include/ansi/stddef.h
objects/unix/test2.o : /usr/include/ansi/stdlib.h
objects/unix/test2M.o : source/test/test2M.c
objects/unix/test2M.o : ../hdr/drawp/debug.h
objects/unix/test2M.o : /usr/include/ansi/stdio.h
objects/unix/test2M.o : ../hdr/drawp/public.h
objects/unix/test2M.o : ../hdr/drawp/coordinates.h
objects/unix/test2M.o : ../hdr/drawp/pixmaps.h
objects/unix/test2M.o : ../hdr/drawp/clipping.h
objects/unix/test2M.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test2M.o : /usr/include/ansi/math.h
objects/unix/test2M.o : /usr/include/ansi/stddef.h
objects/unix/test2M.o : /usr/include/ansi/stdlib.h
objects/unix/test3.o : source/test/test3.c
objects/unix/test3.o : ../hdr/drawp/debug.h
objects/unix/test3.o : /usr/include/ansi/stdio.h
objects/unix/test3.o : source/test/masks.h
objects/unix/test3.o : ../hdr/drawp/public.h
objects/unix/test3.o : ../hdr/drawp/coordinates.h
objects/unix/test3.o : ../hdr/drawp/pixmaps.h
objects/unix/test3.o : ../hdr/drawp/clipping.h
objects/unix/test3.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test3.o : /usr/include/ansi/math.h
objects/unix/test3.o : /usr/include/ansi/stddef.h
objects/unix/test3.o : /usr/include/ansi/stdlib.h
objects/unix/test4.o : source/test/test4.c
objects/unix/test4.o : source/test/sys/time.h
objects/unix/test4.o : /usr/include/ansi/time.h
objects/unix/test4.o : ../hdr/drawp/debug.h
objects/unix/test4.o : /usr/include/ansi/stdio.h
objects/unix/test4.o : source/test/masks.h
objects/unix/test4.o : ../hdr/drawp/public.h
objects/unix/test4.o : ../hdr/drawp/coordinates.h
objects/unix/test4.o : ../hdr/drawp/pixmaps.h
objects/unix/test4.o : ../hdr/drawp/clipping.h
objects/unix/test4.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test4.o : /usr/include/ansi/stddef.h
objects/unix/test4.o : /usr/include/ansi/stdlib.h
objects/unix/test5.o : source/test/test5.c
objects/unix/test5.o : source/test/sys/time.h
objects/unix/test5.o : /usr/include/ansi/time.h
objects/unix/test5.o : ../hdr/drawp/debug.h
objects/unix/test5.o : /usr/include/ansi/stdio.h
objects/unix/test5.o : source/test/masks.h
objects/unix/test5.o : ../hdr/drawp/public.h
objects/unix/test5.o : ../hdr/drawp/coordinates.h
objects/unix/test5.o : ../hdr/drawp/pixmaps.h
objects/unix/test5.o : ../hdr/drawp/clipping.h
objects/unix/test5.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test5.o : /usr/include/ansi/math.h
objects/unix/test5.o : /usr/include/ansi/stddef.h
objects/unix/test5.o : /usr/include/ansi/stdlib.h
objects/unix/test6.o : source/test/test6.c
objects/unix/test6.o : source/test/sys/time.h
objects/unix/test6.o : /usr/include/ansi/time.h
objects/unix/test6.o : ../hdr/drawp/debug.h
objects/unix/test6.o : /usr/include/ansi/stdio.h
objects/unix/test6.o : source/test/masks.h
objects/unix/test6.o : ../hdr/drawp/public.h
objects/unix/test6.o : ../hdr/drawp/coordinates.h
objects/unix/test6.o : ../hdr/drawp/pixmaps.h
objects/unix/test6.o : ../hdr/drawp/clipping.h
objects/unix/test6.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test6.o : /usr/include/ansi/math.h
objects/unix/test6.o : /usr/include/ansi/stddef.h
objects/unix/test6.o : /usr/include/ansi/stdlib.h
objects/unix/test7.o : source/test/test7.c
objects/unix/test7.o : source/test/sys/time.h
objects/unix/test7.o : /usr/include/ansi/time.h
objects/unix/test7.o : ../hdr/drawp/debug.h
objects/unix/test7.o : /usr/include/ansi/stdio.h
objects/unix/test7.o : source/test/masks.h
objects/unix/test7.o : ../hdr/drawp/public.h
objects/unix/test7.o : ../hdr/drawp/coordinates.h
objects/unix/test7.o : ../hdr/drawp/pixmaps.h
objects/unix/test7.o : ../hdr/drawp/clipping.h
objects/unix/test7.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test7.o : /usr/include/ansi/math.h
objects/unix/test7.o : /usr/include/ansi/stddef.h
objects/unix/test7.o : /usr/include/ansi/stdlib.h
objects/unix/test8.o : source/test/test8.c
objects/unix/test8.o : source/test/sys/time.h
objects/unix/test8.o : /usr/include/ansi/time.h
objects/unix/test8.o : source/test/masks.h
objects/unix/test8.o : ../hdr/drawp/public.h
objects/unix/test8.o : ../hdr/drawp/coordinates.h
objects/unix/test8.o : ../hdr/drawp/pixmaps.h
objects/unix/test8.o : ../hdr/drawp/clipping.h
objects/unix/test8.o : /usr/include/ansi/stdio.h
objects/unix/test8.o : ../hdr/drawp/graphicsContexts.h
objects/unix/test8.o : /usr/include/ansi/math.h
objects/unix/test8.o : /usr/include/ansi/stddef.h
objects/unix/test8.o : /usr/include/ansi/stdlib.h
objects/unix/macros.o : source/tools/macros.c
objects/unix/macros.o : /usr/include/ansi/errno.h
objects/unix/macros.o : /usr/include/ansi/ctype.h
objects/unix/macros.o : /usr/include/ansi/string.h
objects/unix/macros.o : /usr/include/ansi/stdio.h
objects/unix/macros.o : /usr/include/ansi/stddef.h
objects/unix/macros.o : /usr/include/ansi/stdlib.h
