# Makefile for Mg 2a

SYS	= helios
LIBS	= -ltermcap
# CDEFS gets defines, and gets passed to lint. CFLAGS gets flags, and doesn't
# get passed to lint.
#
# (Common) compile-time options:
#
#	DO_METAKEY	-- if bit 7 is set for a key, treat like a META key
#	STARTUP		-- look for and handle initialization file
#	SYMBLINK	-- follow symbolic links the same as the BSD kernel
#	XKEYS		-- use termcap function key definitions. Warning -
#				XKEYS and bsmap mode do _not_ get along.
#	BACKUP		-- enable "make-backup-files"
#	PREFIXREGION	-- enable function "prefix-region"
#	REGEX		-- create regular expression functions
#
#CDEFS	=  -dDO_METAKEY
CDEFS	=  -dDO_METAKEY -dPREFIXREGION -dANSI -dNO_DIRED -dNO_DPROMPT -dFKEYS \
           -dHELIOS
CFLAGS	= -g $(CDEFS)

# Objects which only depend on the "standard" includes
OBJS	= basic.p dir.p dired.p file.p line.p match.p paragraph.p \
	  random.p region.p search.p version.p window.p word.p

# Those with unique requirements
IND	= buffer.p display.p echo.p extend.p help.p kbd.p keymap.p \
	  macro.p main.p modes.p regex.p re_search.p

# System dependent objects
OOBJS = cinfo.p spawn.p ttyio.p ttykbd.p

OBJ = $(OBJS) $(IND) $(OOBJS) fileio.p

OSRCS	= cinfo.c fileio.c spawn.c ttyio.c tty.c ttykbd.c
SRCS	= basic.c dir.c dired.c file.c line.c match.c paragraph.c \
	  random.c region.c search.c version.c window.c word.c \
	  buffer.c display.c echo.c extend.c help.c kbd.c keymap.c \
	  macro.c main.c modes.c regex.c re_search.c

OINCS =	ttydef.h sysdef.h chrdef.h
INCS =	def.h

#
.SUFFIXES:
.SUFFIXES: .z .p .c

INCDIR   = /usr/perihelion/Helios/include
LIBDIR   = /usr/perihelion/Helios/lib
BINDIR   = /usr/perihelion/Helios/bin
CC       = nc

CFLAGS  = -t4 -j$(INCDIR)/ $(CDEFS)

.c.p :  
	$(CC) $(CFLAGS) $*.c | asm -p -otmp
	mv tmp $*.p

mg.z:	$(OBJ)
	asm -v $(LIBDIR)/cstart.o $(OBJ) -o$*.z
	objed -i -n$* -s20000 -h10000 $*.z

# strip mg once you're satisfied it'll run -- makes it much smaller
strip:
	strip mg

lint: $(SRCS) $(OSRCS) $(INCS) $(OINCS)
	lint -ahbz $(CDEFS) $(SRCS) $(OSRCS)

$(OBJ):		$(INCS) $(OINCS)


dir.r search.p:	$(INCS) $(OINCS)

regex.p re_search.p:	$(INCS) $(OINCS) regex.h

kbd.p:	$(INCS) $(OINCS) macro.h kbd.h key.h

macro.p main.p:	$(INCS) $(OINCS) macro.h

buffer.p display.p keymap.p help.p modes.p dired.p fileio.p: \
	$(INCS) $(OINCS) kbd.h

extend.p:	$(INCS) $(OINCS) kbd.h macro.h key.h

help.p:	$(INCS) $(OINCS) kbd.h key.h macro.h

echo.p:	$(INCS) $(OINCS) key.h macro.h

$(OOBJS):	$(INCS) $(OINCS)

sysdef.h:	sys/$(SYS)/sysdef.h	# Update links, if needed.
	rm -f sysdef.h
	ln sys/$(SYS)/sysdef.h .

ttydef.h:	sys/$(SYS)/ttydef.h
	rm -f ttydef.h
	ln sys/$(SYS)/ttydef.h .

chrdef.h:	sys/default/chrdef.h
	rm -f chrdef.h
	ln sys/default/chrdef.h .

fileio.c:	sys/$(SYS)/fileio.c
	rm -f fileio.c
	ln sys/$(SYS)/fileio.c .

spawn.c:	sys/$(SYS)/spawn.c
	rm -f spawn.c
	ln sys/$(SYS)/spawn.c .

tty.c:		sys/default/tty.c
	rm -f tty.c
	ln sys/default/tty.c .

ttyio.c:	sys/$(SYS)/ttyio.c
	rm -f ttyio.c
	ln sys/$(SYS)/ttyio.c .

ttykbd.c:	sys/default/ttykbd.c
	rm -f ttykbd.c
	ln sys/default/ttykbd.c .

cinfo.c:	sys/default/cinfo.c
	rm -f cinfo.c
	ln sys/default/cinfo.c .

helios_maps.c:	sys/$(SYS)/helios_maps.c
	rm -f helios_maps.c
	ln sys/$(SYS)/helios_maps.c .

port: $(SRCS) $(INCS)
	rm -f port
	tar cfb port 1 $?

clean:;	rm -f $(OBJ) $(OSRCS) $(OINCS)

