
#
#   ISERVER makefile
#


#   uncomment the lines appropriate to your host


#
#   sun3, sun386 and sun4 / sunos 4.0 / sun c (BSD Universe)
#
#   use these 3 lines for a B011
#HW=B011
#LINKMODULE=b011link
#CFLAGS= -pipe -g -D`arch` -DSUN -DBOARD_ID=$(HW)
#   or these 2 for a B014 with S502 device driver
#HW=B014
#LINKMODULE=b014link
#CFLAGS= -pipe -g -D`arch` -DSUN -DBOARD_ID=$(HW)
#   or these two for a B008 in a Sun 386i
#HW=B008
#LINKMODULE=s386link
#   or these 3 for udplink
#HW=UDP
#LINKMODULE=udplink
#CFLAGS= -pipe -g -D`arch` -DSUN -DFASTPEEK -DBOARD_ID=$(HW)
 
#CC=cc -c
#E=
#O=.o
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) $(LINKMODULE)$(O)
#LINK=cc -g -o iserver$(E) $(OBJECTS)



#
#   pc / msdos 3+ / microsoft c 5.1 [and masm 5.1]
#
#   uncomment the next 3 lines, then a link driver, then a further 2 lines at the end
O=.obj
CC=cl
E=.exe
#   use the next 2 lines for b004 or b008  with assembler support
#HW=B004
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) msdosc$(O) b004fast$(O) b004asm$(O)
#   use the next 2 lines for b010  with assembler support
#HW=B004
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) msdosc$(O) b004fast$(O) b010asm$(O)
#   use the next 3 lines for b004/8/10 using c only
#HW=B004
#LINKMODULE=b004link
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) msdosc$(O) $(LINKMODULE)$(O)
#   use the next 3 lines for b008 if you have S708 device driver
HW=B008
LINKMODULE=b008link
OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) msdosc$(O) $(LINKMODULE)$(O)
#   use the next line for NEC PC
#CFLAGS=-c -Ox -DMSC -DNEC -DBOARD_ID=B010 # -DBOARD_ID=B015
#   or this for regular PC
CFLAGS=-c -Ox -DMSC -DBOARD_ID=$(HW)
#   and uncomment this for all versions
LINK=cl -Ox -o iserver$(E) /F 4000 $(OBJECTS)



#
#   b004 type subsystem, helios 1.0
#
#HW=B008
#LINKMODULE=helios
#CC=cc
#CFLAGS=-s $*.s -dHELIOS -dBOARD_ID=$(HW) -t4 -wd -ec -j,/helios/include/
#E=
#O=.s
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) $(LINKMODULE)$(O)
#LINK=asm -v /helios/lib/cstart.o $(OBJECTS) -o iserver$(E) -s 10000



#
#   vax / vms / vax vms c v2.3
#
#  uncomment these 3 lines if using qt0link
#LINKMODULE=qt0link
#HW=QT0
#CFLAGS=/DEFINE=(BOARD_ID=$(HW),VMS)
#  or these 2 if using udp
#LINKMODULE=UDPLINK
#HW=UDP
# and this is using udp & fast file handling
#CFLAGS=/DEFINE=(BOARD_ID=$(HW),WIN_TCP,VMS,FASTPEEK,FASTFILEVMS)
# and this if using udp & ordinary file handling
#CFLAGS=/DEFINE=(BOARD_ID=$(HW),WIN_TCP,FASTPEEK,VMS)

#CC=cc
#E=.exe
#O=.obj
#OBJECTS=iserver$(O) filec$(O) hostc$(O) serverc$(O) $(LINKMODULE)$(O) vmserr$(O)
# uncomment if using qt0link instead of udp
#LINK=link iserver,filec,hostc,serverc,qt0link,vmserr
# uncomment if using udp
#LINK=link iserver,filec,hostc,serverc,udplink,vmserr


#
#  the targets
#

iserver$(E):		$(OBJECTS)
			$(LINK)

iserver$(O):		iserver.c iserver.h inmos.h
			$(CC) $(CFLAGS) iserver.c

filec$(O):		filec.c iserver.h pack.h inmos.h
			$(CC) $(CFLAGS) filec.c

# uncomment these 2 lines for udp versions
#hostc$(O):		hostc.c udplink.c iserver.h pack.h inmos.h
#			$(CC) $(CFLAGS) hostc.c
# uncomment these 2 lines for non-udp versions
hostc$(O):              hostc.c iserver.h pack.h inmos.h
	$(CC) $(CFLAGS) hostc.c

serverc$(O):		serverc.c iserver.h pack.h inmos.h
			$(CC) $(CFLAGS) serverc.c

msdosc$(O):		msdosc.c iserver.h inmos.h
			$(CC) $(CFLAGS) msdosc.c

$(LINKMODULE)$(O):	$(LINKMODULE).c inmos.h
			$(CC) $(CFLAGS) $(LINKMODULE).c

#
#  these two targets are just for b00[4,8,10] with asm support
#
b004fast.obj:		b004link.c inmos.h
			$(CC) $(CFLAGS) -DB4ASM -Fob004fast.obj b004link.c

b004asm.obj:		b004asm.asm
			masm /ML /MX b004asm;

b010asm.obj:  b010asm.asm
			masm /ML /MX b010asm;

#
# this target is for VMS only
#
vmserr$(O):    vmserr.msg
	message/nolist vmserr


#
#  eof
#
