/* Turbo C Driver for FTP Software's packet driver interface.
 * Graciously donated to the public domain by Phil Karn.
 */
#include <stdio.h>
#include <dos.h>
#include "pktdrvr.h"

static int access_type __ARGS((int intno,int if_class,int if_type,int if_number,
	char *type,unsigned typelen,INTERRUPT (*receiver) __ARGS((void)) ));
static int driver_info __ARGS((int intno,int handle,int *version,
	int *class,int *type,int *number,int *basic));
static int release_type __ARGS((int intno,int handle));
static int get_address __ARGS((int intno,int handle,char *buf,int len));
static int get_parameters __ARGS((int intno));
static int send_pkt __ARGS((int intno,char *buffer,unsigned length));

static INTERRUPT (*Pkvec[])() = { pkvec0,pkvec1,pkvec2 };
static struct pktdrvr Pktdrvr[PK_MAX];
static int Npk;
static int Derr;
static char Pkt_sig[] = "PKT DRVR";	/* Packet driver signature */

char buffer[1514];

/* Packet driver receive routine. Called from an assembler hook that pushes
 * the caller's registers on the stack so we can access and modify them.
 * This is a rare example of call-by-location in C.
 */
void
pkint(dev,di,si,bp,dx,cx,bx,ax,ds,es)
int dev;
unsigned short di,si,bp,dx,cx,bx,ax,ds,es;
{
	register struct pktdrvr *pp;
	struct phdr *phdr;

	if(dev >= Npk)
		return;	/* Unknown packet */
	pp = &Pktdrvr[dev];

	switch(ax){
	case 0:	/* Space allocate call */
		if((pp->buffer = alloc_mbuf(cx+sizeof(struct phdr))) != NULLBUF){
			es = FP_SEG(buffer);
			di = FP_OFF(buffer);
		} else {
			es = di = 0;
		}
		break;
	case 1:	/* Packet complete call */
		enqueue(&Hopper,pp->buffer);
		pp->buffer = NULLBUF;
		break;
	default:
		break;
	}
}

/* Test for the presence of a packet driver at an interrupt number.
 * Return 0 if no packet driver.
 */
int
test_for_pd(intno)
unsigned int intno;
{
	long drvvec;
	char sig[8];	/* Copy of driver signature "PKT DRVR" */

	/* Verify that there's really a packet driver there, so we don't
	 * go off into the ozone (if there's any left)
	 */
	drvvec = (long)getvect(intno);
	movblock(FP_OFF(drvvec)+3, FP_SEG(drvvec),
		FP_OFF(sig),FP_SEG(sig),strlen(Pkt_sig));
	return !strncmp(sig,Pkt_sig,strlen(Pkt_sig));
}

static int
access_type(intno,if_class,if_type,if_number,type,typelen,receiver)
int intno;
int if_class;
int if_type;
int if_number;
char *type;
unsigned typelen;
INTERRUPT (*receiver)();
{
	union REGS regs;
	struct SREGS sregs;

	segread(&sregs);
	regs.h.dl = if_number;		/* Number */
	sregs.ds = FP_SEG(type);	/* Packet type template */
	regs.x.si = FP_OFF(type);
	regs.x.cx = typelen;		/* Length of type */
	sregs.es = FP_SEG(receiver);	/* Address of receive handler */
	regs.x.di = FP_OFF(receiver);
	regs.x.bx = if_type;		/* Type */
	regs.h.ah = ACCESS_TYPE;	/* Access_type() function */
	regs.h.al = if_class;		/* Class */
	int86x(intno,&regs,&regs,&sregs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	} else
		return regs.x.ax;
}
static int
release_type(intno,handle)
int intno;
int handle;
{
	union REGS regs;

	regs.x.bx = handle;
	regs.h.ah = RELEASE_TYPE;
	int86(intno,&regs,&regs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	} else
		return 0;
}
static int
send_pkt(intno,buffer,length)
int intno;
char *buffer;
unsigned length;
{
	union REGS regs;
	struct SREGS sregs;

	segread(&sregs);
	sregs.ds = FP_SEG(buffer);
	sregs.es = FP_SEG(buffer); /* for buggy univation pkt driver - CDY */
	regs.x.si = FP_OFF(buffer);
	regs.x.cx = length;
	regs.h.ah = SEND_PKT;
	int86x(intno,&regs,&regs,&sregs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	} else
		return 0;
}
static int
driver_info(intno,handle,version,class,type,number,basic)
int intno;
int handle;
int *version,*class,*type,*number,*basic;
{
	union REGS regs;

	regs.x.bx = handle;
	regs.h.ah = DRIVER_INFO;
	regs.h.al = 0xff;
	int86(intno,&regs,&regs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	}
	if(version != NULL)
		*version = regs.x.bx;
	if(class != NULL)
		*class = regs.h.ch;
	if(type != NULL)
		*type = regs.x.dx;
	if(number != NULL)
		*number = regs.h.cl;
	if(basic != NULL)
		*basic = regs.h.al;
	return 0;
}
static int
get_parameters(intno)
int intno;
{
	union REGS regs;
	struct SREGS sregs;
	char far *param;

	regs.h.ah = GET_PARAMETERS;
	int86x(intno,&regs,&regs,&sregs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	}
	param = MK_FP(sregs.es, regs.x.di);
	return uchar(param[4]) + 256 * uchar(param[5]);
}
static int
get_address(intno,handle,buf,len)
int intno;
int handle;
char *buf;
int len;
{
	union REGS regs;
	struct SREGS sregs;

	segread(&sregs);
	sregs.es = FP_SEG(buf);
	regs.x.di = FP_OFF(buf);
	regs.x.cx = len;
	regs.x.bx = handle;
	regs.h.ah = GET_ADDRESS;
	int86x(intno,&regs,&regs,&sregs);
	if(regs.x.cflag){
		Derr = regs.h.dh;
		return -1;
	}
	return 0;
}
