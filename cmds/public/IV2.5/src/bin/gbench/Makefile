.SILENT:

DIST   = ${DIST1} ${DIST2}
DIST1  = CHANGES COPYRIGHT Makefile-dist README gbench.man
DIST2  = gbench.h cmds.c exec.c main.c ops.c utils.c

all:

distrib:
	echo building distribution
	shar ${DIST} > dist/gbench.shar
	compress -c dist/gbench.shar > dist/gbench.shar.Z
	shar ${DIST1} > dist/gbench-1of2.shar
	shar ${DIST2} > dist/gbench-2of2.shar
	tar cf dist/gbench.tar ${DIST}
	compress -c dist/gbench.tar > dist/gbench.tar.Z

gen:
	echo building makefiles
	genmake SUN4 X11
	genmake VAX X11

SUN4:
	cd SUN4.X11 ; make -k install

VAX:
	cd VAX.X11 ; make -k install

clean:
	cd SUN4.X11 ; make clean
	cd VAX.X11 ; make clean
