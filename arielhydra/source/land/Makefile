#/****************************************************************/
#/*                          Ariel Corp.                         */
#/*                        433 River Road                        */
#/*                Highland Park, NJ 08904, U.S.A.               */
#/*                     Tel:  (908) 249-2900                     */
#/*                     Fax:  (908) 249-2123                     */
#/*                     BBS:  (908) 249-2124                     */
#/*                  E-Mail:  ariel@ariel.com                    */
#/*                                                              */
#/*                 Copyright (C) 1991 Ariel Corp.               */
#/****************************************************************/


OWHOME:sh= if [ a${OPENWINHOME} = a ] ; then echo /usr/openwin ; else echo ${OPENWINHOME} ; fi

BINDIR=${VC40DSP}/bin
INCDIR=${VC40DSP}/include
LIBDIR=${VC40DSP}/lib
C40LIB=${LIBDIR}/libvc40.a

SPARC=landc40
C40=manddsp.x40

INCDIRS = -I${OWHOME}/include -I${INCDIR}
LIBDIRS = -L${OWHOME}/lib -L${LIBDIR}
LIBS = -lxview -lX11 -lolgx -lvc40 -lm

CSW = ${INCDIRS} -O

all:	${SPARC}

landc40:	land.o land_vc40.o ${C40LIB}
	cc -o $@ ${LIBDIRS} land.o land_vc40.o ${LIBS}

land.o:	land.c land_dsp.h
	cc -c ${CSW} land.c

land_vc40.o:	land_vc40.c land_dsp.h ${INCDIR}/hyhomon.h
	cc -c ${CSW} land_vc40.c

clean:
	rm -f *% *.o *.map core ifile *.lst *.obj *.map

new:	clean
	rm -f ${SPARC} # *.x40

install: all clean
	mv ${SPARC} ${BINDIR}/.
	cp ${C40} colormap ${BINDIR}/.
