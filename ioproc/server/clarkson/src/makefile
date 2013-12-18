#
# makefile for Packet Drivers for Turbo Make.  MS-Make loses.
#

# if you use Borland products
ASM	=	tasm
LINK	=	tlink

# If you use Microsoft products (some lossage will ensue).  Great
# lossage will ensue if you use masm 4.0, but most things will
# assemble.  For those that do not, simply create the .obj file by
# hand, then create the .com file.  That will enable make to skip
# over them.  Please send the editor email
# <nelson@clutx.clarkson.edu>, or a FAX <315-268-7600> if Microsoft
# support is useful to you.
#ASM	=	masm
#LINK	=	link

# These are split into ASM and C files because you may not have Turbo C.
# If you don't, then remove the "c" dependency from the following:
all:	drivers utilities c

drivers: \
	ni5010.com ni5210.com ni9210.com ni6510.com \
	3c501.com 3c503.com 3c505.com 3c507.com 3c523.com \
	generic.com wd8003e.com slip8250.com isolan.com isolink.com \
	ibmtoken.com ne1000.com ne2000.com ne2.com arcnet.com \
	at&t.com localtlk.com hppclan.com de600.com \
	tiara.com ubnicpc.com ubnicps2.com nti16.com ncret105.com \
	nb.com ipxpkt.com depca.com express.com ar450.com

utilities: \
	pktchk.com trace.com termin.com pktmode.com pktaddr.com pktall.com \
	pktmulti.com pktsend.com pktstat.com pktring.com

c:	dump.exe ipxstat.exe

.asm.obj:
	$(ASM) $*;

dump.obj: dump.c
	tcc -c dump

dump.exe: dump.obj
	tcc -edump dump.obj

ipxstat.obj: ipxstat.c
	tcc -c ipxstat

ipxstat.exe: ipxstat.obj
	tcc -eipxstat ipxstat.obj

termin.com: termin.obj
	$(LINK) termin;
	exe2com termin
	del termin.exe

trace.com: trace.obj
	$(LINK) trace;
	exe2com trace
	del trace.exe

pktmode.com: pktmode.obj defs.asm
	$(LINK) pktmode;
	exe2com pktmode
	del pktmode.exe

pktaddr.com: pktaddr.obj defs.asm
	$(LINK) pktaddr;
	exe2com pktaddr
	del pktaddr.exe

pktall.com: pktall.obj defs.asm
	$(LINK) pktall;
	exe2com pktall
	del pktall.exe

pktmulti.com: pktmulti.obj defs.asm
	$(LINK) pktmulti;
	exe2com pktmulti
	del pktmulti.exe

pktchk.com: pktchk.obj defs.asm
	$(LINK) pktchk;
	exe2com pktchk
	del pktchk.exe

pktsend.com: pktsend.obj defs.asm
	$(LINK) pktsend;
	exe2com pktsend
	del pktsend.exe

pktstat.com: pktstat.obj defs.asm
	$(LINK) pktstat;
	exe2com pktstat
	del pktstat.exe

pktring.com: pktring.obj defs.asm
	$(LINK) pktring;
	exe2com pktring
	del pktring.exe

head.obj: head.asm defs.asm

3c501.com: head.obj 3c501.obj tail.obj
	$(LINK) head 3c501 tail,3c501/m;
	exe2com 3c501
	del 3c501.exe

3c503.obj: 3c503.asm 8390.asm

3c503.com: head.obj 3c503.obj tail.obj
	$(LINK) head 3c503 tail,3c503/m;
	exe2com 3c503
	del 3c503.exe

3c505.com: head.obj 3c505.obj tail.obj
	$(LINK) head 3c505 tail,3c505/m;
	exe2com 3c505
	del 3c505.exe

3c507.obj: 3c507.asm 82586.asm

3c507.com: head.obj 3c507.obj tail.obj
	$(LINK) head 3c507 tail,3c507/m;
	exe2com 3c507
	del 3c507.exe

3c523.obj: 3c523.asm 82586.asm

3c523.com: head.obj 3c523.obj tail.obj
	$(LINK) head 3c523 tail,3c523/m;
	exe2com 3c523
	del 3c523.exe

at&t.obj: at&t.asm 82586.asm

at&t.com: head.obj at&t.obj tail.obj
	$(LINK) head at&t tail,at&t/m;
	exe2com at&t
	del at&t.exe

localtlk.com: head.obj localtlk.obj tail.obj
	$(LINK) head localtlk tail,localtlk/m;
	exe2com localtlk
	del localtlk.exe

ubnicpc.com: head.obj ubnicpc.obj tail.obj
	$(LINK) head ubnicpc tail,ubnicpc/m;
	exe2com ubnicpc
	del ubnicpc.exe

ubnicps2.obj: ubnicps2.asm 82586.asm

ubnicps2.com: head.obj ubnicps2.obj tail.obj
	$(LINK) head ubnicps2 tail,ubnicps2/m;
	exe2com ubnicps2
	del ubnicps2.exe

ncret105.obj: ncret105.asm 82586.asm

ncret105.com: head.obj ncret105.obj tail.obj
	$(LINK) head ncret105 tail,ncret105/m;
	exe2com ncret105
	del ncret105.exe

nti16.com: head.obj nti16.obj tail.obj
	$(LINK) head nti16 tail,nti16/m;
	exe2com nti16
	del nti16.exe

hppclan.obj: hppclan.asm 8390.asm

hppclan.com: head.obj hppclan.obj tail.obj
	$(LINK) head hppclan tail,hppclan/m;
	exe2com hppclan
	del hppclan.exe

ni5010.com: head.obj ni5010.obj tail.obj
	$(LINK) head ni5010 tail,ni5010/m;
	exe2com ni5010
	del ni5010.exe

ni5210.obj: ni5210.asm 82586.asm

ni5210.com: head.obj ni5210.obj tail.obj
	$(LINK) head ni5210 tail,ni5210/m;
	exe2com ni5210
	del ni5210.exe

ni9210.obj: ni9210.asm 82586.asm

ni9210.com: head.obj ni9210.obj tail.obj
	$(LINK) head ni9210 tail,ni9210/m;
	exe2com ni9210
	del ni9210.exe

ni6510.com: head.obj ni6510.obj tail.obj
	$(LINK) head ni6510 tail,ni6510/m;
	exe2com ni6510
	del ni6510.exe

depca.com: head.obj depca.obj tail.obj
	$(LINK) head depca tail,depca/m;
	exe2com depca
	del depca.exe

express.com: head.obj express.obj tail.obj
	$(LINK) head express tail,express/m;
	exe2com express
	del express.exe

ar450.com: head.obj ar450.obj tail.obj
	$(LINK) head ar450 tail,ar450/m;
	exe2com ar450
	del ar450.exe

isolan.com: head.obj isolan.obj tail.obj
	$(LINK) head isolan tail,isolan/m;
	exe2com isolan
	del isolan.exe

isolink.com: head.obj isolink.obj tail.obj
	$(LINK) head isolink tail,isolink/m;
	exe2com isolink
	del isolink.exe

slip8250.com: head.obj slip8250.obj tail.obj
	$(LINK) head slip8250 tail,slip8250/m;
	exe2com slip8250
	del slip8250.exe

wd8003e.obj: wd8003e.asm 8390.asm

wd8003e.com: head.obj wd8003e.obj tail.obj
	$(LINK) head wd8003e tail,wd8003e/m;
	exe2com wd8003e
	del wd8003e.exe

ibmtoken.com: head.obj ibmtoken.obj tail.obj
	$(LINK) head ibmtoken tail,ibmtoken/m;
	exe2com ibmtoken
	del ibmtoken.exe

ne1000.obj: ne1000.asm 8390.asm

ne1000.com: head.obj ne1000.obj tail.obj
	$(LINK) head ne1000 tail,ne1000/m;
	exe2com ne1000
	del ne1000.exe

ne2000.obj: ne2000.asm 8390.asm

ne2000.com: head.obj ne2000.obj tail.obj
	$(LINK) head ne2000 tail,ne2000/m;
	exe2com ne2000
	del ne2000.exe

ne2.com: head.obj ne2.obj tail.obj
	$(LINK) head ne2 tail,ne2/m;
	exe2com ne2
	del ne2.exe

arcnet.com: head.obj arcnet.obj tail.obj
	$(LINK) head arcnet tail,arcnet/m;
	exe2com arcnet
	del arcnet.exe

generic.com: head.obj generic.obj tail.obj
	$(LINK) head generic tail,generic/m;
	exe2com generic
	del generic.exe

nb.com: head.obj nb.obj tail.obj
	$(LINK) head nb tail,nb/m;
	exe2com nb
	del nb.exe

ipxpkt.com: head.obj ipxpkt.obj tail.obj
	$(LINK) head ipxpkt tail,ipxpkt/m;
	exe2com ipxpkt
	del ipxpkt.exe

tiara.com: head.obj tiara.obj tail.obj
	$(LINK) head tiara tail,tiara/m;
	exe2com tiara
	del tiara.exe

de600.com: head.obj tail.obj de600.obj
        $(LINK) head de600 tail,de600/m;
	exe2com de600
	del de600.exe

versions: nul
	grep -o "version.equ" *.asm

clean: nul
	del *.obj
	del *.com
	del *.exe
	del *.map
