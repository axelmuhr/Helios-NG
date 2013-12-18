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

SPARC=mandc40
C40=manddsp.x40

INCDIRS = -I${OWHOME}/include -I${INCDIR}
LIBDIRS = -L${OWHOME}/lib -L${LIBDIR}
LIBS = -lxview -lX11 -lolgx -lvc40 -lm

CSW = ${INCDIRS} -O

all:	${SPARC}

mandc40:	mand.o mand_vc40.o ${C40LIB}
	cc -g -o $@ ${LIBDIRS} mand.o mand_vc40.o ${LIBS}

mand.o:	mand.c mand_dsp.h
	cc -g -c ${CSW} mand.c

mand_vc40.o:	mand_vc40.c mand_dsp.h ${INCDIR}/hyhomon.h
	cc -g -c ${CSW} mand_vc40.c

clean:
	rm -f *% *.o *.map core ifile *.lst *.obj *.map

new:	clean
	rm -f ${SPARC} # *.x40

install: all clean
	mv ${SPARC} ${BINDIR}/.
	cp ${C40} colormap ${BINDIR}/.
