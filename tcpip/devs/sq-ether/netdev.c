/*************************************************************************
**									**
**		S E E Q   E T H E R N E T   D R I V E R			**
**		---------------------------------------			**
**									**
**		    Copyright (C) 1990, Parsytec GmbH			**
**			  All Rights Reserved.				**
**									**
**									**
** sqdev.c								**
**									**
**	- Helios driver routines for TPM - ETN				**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	24.08.90   G. Jodlauk					**
*************************************************************************/

#define __in_sqdev	1		/* flag that we are in this module */

/*---  TOGGLES  -----------------------------------------------------------*/

#define DBG_STAT	0
#define DBG_PKTS	0
#define NO_WRITE	0
#define NO_READ		0

/*-------------------------------------------------------------------------*/

#include <helios.h>
#include <stdio.h>
#include <queue.h>
typedef word Code;
#include <event.h>
#include <device.h>
#include <syslib.h>
#include <nonansi.h>
#include <sem.h>
#include <codes.h>
#include <task.h>
#include <memory.h>
#include <string.h>
#include <trace.h>

#include "if_sq.h"

/* --------  SEEQ 8005 Ethernet Data Link Controller Interface  --------- */
 
#define DEC3	3	/*    value	Rev.    comment
			 *
			 *	1	1.1	20V8
			 *	2	1.2	22V10
			 *	3	1.2++	22V10	A10 verlegt
			 */
			 	

#define ADDRPROM 1	/* FALSE fuer SEEQ-Controller bis 1.2
			 *
			 * TRUE fuer SEEQ-Controller, die Adressprom
			 * lesen, sonst wird die Hardwareadresse aus
			 * dem Source eingetragen. 
			 */

/*-------------------------------------------------------------------------*/

typedef struct
{
	DCB 		dcb;
	Semaphore	lock;
	Semaphore	seeq;
	/* Event handling */
	Event		event;
	Semaphore	nintr;
	/* read requests */
	List		readq;
	Semaphore	nread;
	/* write requests */
	List		startq;
	int 		stqlen;
	int 		quelimit;
	Semaphore	nstart;
	Semaphore	txdone;
	/* Termination locks */
	bool		terminate;
	Semaphore	termintr;
	Semaphore	termread;
	Semaphore	termstart;
#if DBG_STAT
	Semaphore	termdbg;
#endif
	/* Ethernet hardware address */
	byte		etaddr[6];
	/* wordaligned buffer for dma */
	byte		*Rbuf;
	byte		*Wbuf;
	/* generic interface statistics */
	struct {
		int	ipackets;	/* packets received on interface */
		int	ierrors;	/* input errors on interface */
		int	opackets;	/* packets sent on interface */
		int	oerrors;	/* output errors on interface */
		int	collisions;	/* collisions on interface */
		int	ioverflows;	/* buffer overflows on interface */
		int	ooverflows;	/* actually drops on startqueue */
		int	maxquelen;	/* limit for startqueue */
		unsigned glavg;		/* gliding average of startqueue */
#define 	AVG_SHIFT	8	/* value stored shifted to the left */
		unsigned eventsall;	/* count of events at all */
		unsigned eventsrx;	/* count of receive events */
		unsigned eventstx;	/* count of transmit events */
		unsigned eventsnull;	/* count of events not for us */
	} sq_stat;
	struct {
		int gl_tea;		/* This 8 bit transmit end area
					   register value mainly defines
					   the sizes of the ring buffers */
		int gl_rec_area_l;
		int gl_xmit_area_l;
		int gl_tea_addr;
		int gl_com_reg;
		int gl_xmit_start;
		int gl_rec_start;
		word gl_rcv_was_on;
	} sq_gl;
	struct {
		/* areas */
		int *reg_etn_ffs;
		int *reg_dma_win_base;
		int *reg_tct_write_addr;
		int *reg_tct_read_addr;
		/*registers */
		int *reg_command;
		int *reg_status;
		int *reg_conf_1;
		int *reg_conf_2;
		int *reg_rea;
		int *reg_buf_win;
		int *reg_rp;
		int *reg_tp;
		int *reg_dma_ad;
	} sq_reg;
} ETN_DCB;

/*----------------  prototypes for SEEQ driver functions  -----------------*/

word	Sq_GetInfo	(ETN_DCB *dcb, NetInfo *info, int size);
word	Sq_SetInfo	(ETN_DCB *dcb, NetInfo *info, int size);
void	Reader		(ETN_DCB *dcb);
void	Starter		(ETN_DCB *dcb);
void	Sq_Recv		(ETN_DCB *dcb);
void	sqintr		(ETN_DCB *dcb);		/* signals sqintr_process */
void	sqintr_process	(ETN_DCB *dcb);		/* event handling process */
void	sq_start	(ETN_DCB *dcb);
void 	sq_getaddr	(ETN_DCB *dcb, byte *);
int	sq_reset	(ETN_DCB *dcb);
int	sq_config	(ETN_DCB *dcb); 
void	sq_dadma	(ETN_DCB *dcb);
void	sq_darcv	(ETN_DCB *dcb);
void	sq_daxmit	(ETN_DCB *dcb);
void	sq_dmadone	(ETN_DCB *dcb);
void	sq_earcv	(ETN_DCB *dcb);
void	sq_eaxmit	(ETN_DCB *dcb);
int	sq_upload	(ETN_DCB *dcb, byte *buf, int size);
int	sq_recvdone	(ETN_DCB *dcb, byte *buf, int *size);
void	sq_xmitdone	(ETN_DCB *dcb);
int	sq_xmitstat	(ETN_DCB *dcb);
int	xmit_enabled	(ETN_DCB *dcb);
int	sq_recvstat	(byte *);

/*-----------------------------------------------------------------------*/
#if DBG_PKTS

void DbgPaket(char *txt, byte *buf, int len)
{
	byte *ed = buf;
	byte *es = buf+6;
	byte *et = buf+12;
	byte *ih = buf+14;
	byte *is = buf+26;
	byte *id = buf+30;

	byte *hwf = buf+14;	
	byte *prf = buf+14+2;	
	byte *hwl = buf+14+4;	
	byte *prl = buf+14+5;	
	byte *opt = buf+14+6;	
	byte *sha = buf+14+8;	
	byte *spa = buf+14+14;	
	byte *tha = buf+14+18;	
	byte *tpa = buf+14+24;

	IOdebug("");
	
	if (et[0] != 8) 
		goto end;

	if (et[1] == 0) {
		IOdebug("%s --> IP Packet len %d\n\
ET_DST: %x.%x.%x.%x.%x.%x  ET_SRC %x.%x.%x.%x.%x.%x\n\
IP_HDR  %x %x %x\n\
IP_SRC  %d.%d.%d.%d        IP_DST   %d,%d,%d,%d",
		txt, len,
		ed[0],ed[1],ed[2],ed[3],ed[4],ed[5],
		es[0],es[1],es[2],es[3],es[4],es[5],
		*(word *)ih,*(word *)(ih+4),*(word *)(ih+8),
		is[0],is[1],is[2],is[3],
		id[0],id[1],id[2],id[3] );
		return;
	}
	if (et[1] == 6) {
		IOdebug("%s --> ARP Packet len %d\n\
header: hwf %x prf %x hwl %d prl %d opt = %d -> %s\n\
sender hardware address  %x.%x.%x.%x.%x.%x\n\
sender protocol address  %d.%d.%d.%d\n\
target hardware address  %x.%x.%x.%x.%x.%x\n\
target protocol address  %d.%d.%d.%d",
		txt, len,
		*(short*)hwf, *(short*)prf, hwl[0], prl[0], (*(short*)opt)>>8,
		(*(short*)opt)>>8 == 1 ? "REQUEST":"REPLY",
		sha[0],sha[1],sha[2],sha[3],sha[4],sha[5],
		spa[0],spa[1],spa[2],spa[3],
		tha[0],tha[1],tha[2],tha[3],tha[4],tha[5],
		tpa[0],tpa[1],tpa[2],tpa[3] );
		return;
	}

end:	IOdebug("unknown packet type %d.%d", et[0], et[1]);	

}
#else

#define DbgPaket(a,b,c) 	;

#endif
/*-----------------------------------------------------------------------*/
#if DBG_STAT

void DbgStatistics(ETN_DCB *dcb)
{  
    int cnt = 0, minutes = 0;
   
    for(;;Delay(OneSec))
    {
    	if (dcb->terminate) 
    	{
    		Signal(&dcb->termdbg);
    		return;
    	}
    	if (cnt == 0) 
    	{
	    IOdebug("");
	    IOdebug("SEEQ ethernet interface statistics");
	    IOdebug("----------------------------------");
	    IOdebug("time running     : %d min", minutes);
	    IOdebug("");
	    IOdebug("packets received : %d", dcb->sq_stat.ipackets);
	    IOdebug("packets sent     : %d", dcb->sq_stat.opackets);
	    IOdebug("input errors     : %d", dcb->sq_stat.ierrors);
	    IOdebug("input overflows  : %d", dcb->sq_stat.ioverflows);
	    IOdebug("output errors    : %d", dcb->sq_stat.oerrors);
   	    IOdebug("output overflows : %d", dcb->sq_stat.ooverflows);
	    IOdebug("output collisions: %d", dcb->sq_stat.collisions);
   	    IOdebug("outputqueue limit: %d", dcb->quelimit);
   	    IOdebug("outputqueue max  : %d", dcb->sq_stat.maxquelen);
   	    IOdebug("outputqueue act  : %d", dcb->sq_stat.glavg >> AVG_SHIFT);
	    IOdebug("events over all  : %d", dcb->sq_stat.eventsall);
	    IOdebug("events output    : %d", dcb->sq_stat.eventstx);
	    IOdebug("events input     : %d", dcb->sq_stat.eventsrx);
	    IOdebug("events not for us: %d", dcb->sq_stat.eventsnull);
	    IOdebug("----------------------------------");
	    IOdebug("");
	    minutes++;
	}
	if (++cnt > 60)
		cnt = 0;
    }
}
#endif
/*-------------------------------------------------------------------------*/
#define average(a) \
	dcb->sq_stat.glavg = (dcb->sq_stat.glavg << 8) - dcb->sq_stat.glavg; \
	dcb->sq_stat.glavg += (dcb->stqlen << AVG_SHIFT); \
	dcb->sq_stat.glavg >>= AVG_SHIFT;

void DevOperate(ETN_DCB *dcb, NetDevReq *req)
{
	NetInfoReq *ireq = (NetInfoReq *)req;
	NetInfo *info = &ireq->NetInfo;
	
	Wait(&dcb->lock);

	switch(req->DevReq.Request)
	{
	case FG_Read:

/*		IOdebug("Net Read %x %d",req->Buf,req->Size); */

#if NO_READ == 0
		AddTail(&dcb->readq,&req->DevReq.Node);
		Signal(&dcb->nread);
#endif
		Signal(&dcb->lock);
		return;
		
	case FG_Write:

/*		DbgPaket("FG_Write", req->Buf, req->Size); */

#if NO_WRITE == 0
		if (dcb->stqlen < dcb->quelimit)
		{
			if (dcb->sq_stat.maxquelen < ++dcb->stqlen)
				dcb->sq_stat.maxquelen = dcb->stqlen; 
			average(0);
			AddTail(&dcb->startq, &req->DevReq.Node);
			Signal(&dcb->nstart);
			Signal(&dcb->lock);
			return;	
		}
		average(0);
		dcb->sq_stat.ooverflows++;
#endif
#if 0
		req->Actual = 0;
		req->DevReq.Result = -1;
#else
		req->Actual = req->Size;
		req->DevReq.Result = 0;
#endif
		break;
		
	case FG_SetInfo:
/*
	IOdebug("Net SetInfo %x %x %x [%",info->Mask,info->Mode,info->State);
		{
			int i;
			for(i = 0; i < 6; i++ ) IOdebug("%d %",info->Addr[i]);
			IOdebug("]");
		}
*/
		req->DevReq.Result = Sq_SetInfo(dcb, info, sizeof(NetInfo));
		break;

	case FG_GetInfo:

		info->Mask = 7;
		req->DevReq.Result = Sq_GetInfo(dcb, info,sizeof(NetInfo));

/*
	IOdebug("Net GetInfo %x %x %x [%",info->Mask,info->Mode,info->State);
		{
			int i;
			for(i = 0; i < 6; i++ ) IOdebug("%d %",info->Addr[i]);
			IOdebug("]");
		}
*/

		break;
	}
	
	Signal(&dcb->lock);
	
	(*req->DevReq.Action)(dcb,req);
}

/*-------------------------------------------------------------------------*/

void sq_set_com_bits( ETN_DCB *, int);

void Sq_Recv(ETN_DCB *dcb)
{
	int got, result = 0;
	NetDevReq *req;

	Wait(&dcb->lock);
	req = (NetDevReq *)RemHead(&dcb->readq);
	Signal(&dcb->lock);
	if( req == NULL ) return;

more:	
	got = (int)req->Size;
	Wait(&dcb->seeq);
	result = sq_recvdone(dcb, req->Buf, &got);
	Signal(&dcb->seeq);

	if (got > 0) {
		/* Success, pass packet up */
/*		IOdebug("FG_Read size %d got %d",req->Size,got); */
		req->Actual = got;
		req->DevReq.Result = 0;

/*		DbgPaket("FG_Read", req->Buf, got); */
		(*req->DevReq.Action)(dcb,req);	

	} else {
		/* No packet, queue request */
		Wait(&dcb->lock);
		AddTail(&dcb->readq,&req->DevReq.Node);
		Signal(&dcb->lock);
	}

	/* Get all received packets */	
	if (result) 
	{
		Wait(&dcb->lock);
		req = (NetDevReq *)RemHead(&dcb->readq);
		Signal(&dcb->lock);
		if( req != NULL )
			goto more;
	}

 	/* If Receiver is off and was on before, 
 	   then there was an overflow. */

	Wait(&dcb->seeq);
	if ( ! ( *sq_status & SQ_RX_ON ) & sq_rcv_was_on ) 
	{
		/* Re-enable receive after overflow */

		/*  Define Receive  End Area */
		*sq_rea	= SQ_REA;
		sq_rec_start = sq_tea_addr;

		/*  Define Receive Pointer */
		*sq_rp = sq_rec_start;

		/* Re-enable receive interrupt */
      		sq_set_com_bits (dcb, SQ_EN_RX_INT);
  		sq_earcv(dcb);

		dcb->sq_stat.ioverflows++;
    	} 
	Signal(&dcb->seeq);
}

/*-------------------------------------------------------------------------*/

void Reader(ETN_DCB *dcb)
{
	forever 
	{		
		Wait(&dcb->nread);
		if (dcb->terminate) {
			Signal(&dcb->termread);
			return;
		}

		Sq_Recv(dcb);
	}
}

/*-------------------------------------------------------------------------*/

void Starter(ETN_DCB *dcb)
{
	forever 
	{		
		Wait(&dcb->nstart);
		if (dcb->terminate) {
			Signal(&dcb->termstart);
			return;
		}

		sq_start(dcb);

	}
}

/*-------------------------------------------------------------------------*/

word DevClose(ETN_DCB *dcb)
{
	Wait(&dcb->lock);

       	/* Disable receiver */	
	sq_darcv(dcb);
	
       	/* Disable transmitter */	
	sq_daxmit(dcb);
	
	if ( RemEvent( &dcb->event ) != 0 )
		IOdebug("SERIOUS - SEEQ ethernet interface: RemEvent failed");

	/* Wait for termination of all forked processes */
	dcb->terminate = TRUE;
	Signal(&dcb->lock);
	Signal(&dcb->nintr);
	Signal(&dcb->nread);
	Signal(&dcb->nstart);
	Signal(&dcb->txdone);
	Wait(&dcb->termintr);
	Wait(&dcb->termread);
	Wait(&dcb->termstart);
#if DBG_STAT
	Wait(&dcb->termdbg);
#endif

	Free(dcb->Wbuf);
	Free(dcb->Rbuf);
	Free(dcb);

	return(0);
}

/*-----------------------------------------------------------------------*/

word SetEvent(Event *event);
word RemEvent(Event *event);
word PriFork(word stsize, VoidFnPtr fn, word argsize, ... );

/*-------------------------------------------------------------------------*/

ETN_DCB *DevOpen(Device *dev, void *info)
{
	ETN_DCB *dcb = NULL;
	word err = 0;
	
	info = info;
	
	dcb = Malloc(sizeof(ETN_DCB));
	if (dcb == NULL)
		/* no memory at all available */
		return NULL;
	memset(dcb, 0x0, sizeof(ETN_DCB));

	dcb->Rbuf = Malloc(ETHERPKTSIZE);
	if (dcb->Rbuf == NULL) {
		Free(dcb);
		return NULL;
	}
	memset(dcb->Rbuf, 0xff, ETHERPKTSIZE);

	dcb->Wbuf = Malloc(ETHERPKTSIZE);
	if (dcb->Wbuf == NULL) {
		Free(dcb->Rbuf);
		Free(dcb);
		return NULL;
	}
	memset(dcb->Wbuf, 0xff, ETHERPKTSIZE);

	dcb->dcb.Device = dev;
	dcb->dcb.Operate = DevOperate;
	dcb->dcb.Close = DevClose;
	
	InitSemaphore(&dcb->lock, 1);
	InitSemaphore(&dcb->seeq, 1);

	/* Event handling */
#ifdef __TRAN
	dcb->event.Code = (VoidFnPtr) sqintr;
#else
	dcb->event.Code = (WordFnPtr) sqintr;
#endif
	dcb->event.Data = (void *) dcb;
	InitSemaphore(&dcb->nintr, 0);		

	/* read requests */
	InitList(&dcb->readq);	
	InitSemaphore(&dcb->nread, 0);		

	/* write requests */
	InitList(&dcb->startq);	
	dcb->stqlen = 0;
	dcb->quelimit = 25;
	InitSemaphore(&dcb->nstart, 0);		
	InitSemaphore(&dcb->txdone, 0);		

	/* Termination locks */
	dcb->terminate = FALSE;
	InitSemaphore(&dcb->termintr, 0);
	InitSemaphore(&dcb->termread, 0);
	InitSemaphore(&dcb->termstart, 0);
#if DBG_STAT
	InitSemaphore(&dcb->termdbg, 0);
#endif

	/* Ethernet hardware address */
	dcb->etaddr[0] = 0x00;
	dcb->etaddr[1] = 0x80;
	dcb->etaddr[2] = 0x75;
#if ADDRPROM
	dcb->etaddr[3] = 0xff;
	dcb->etaddr[4] = 0xff;
	dcb->etaddr[5] = 0xff;
#else
	dcb->etaddr[3] = 0x23;
	dcb->etaddr[4] = 0x01;
	dcb->etaddr[5] = 0x08;
#endif

	/*   Special registers / areas for 8005   */
	dcb->sq_reg.reg_etn_ffs		=  (int *) pery_base  +  0x0400;
	dcb->sq_reg.reg_dma_win_base	=  (int *) pery_base  +  0x4000;
	dcb->sq_reg.reg_tct_write_addr	=  (int *) pery_base  +  0x7FFF;
	dcb->sq_reg.reg_tct_read_addr	=  (int *) pery_base  +  0x7FFE;

	/*   Register set of SEEQ 8005   */
	dcb->sq_reg.reg_command	= (int *) sq_base;
	dcb->sq_reg.reg_status	= (int *) sq_base;
	dcb->sq_reg.reg_conf_1	= (int *) sq_base + 0x2;
	dcb->sq_reg.reg_conf_2	= (int *) sq_base + 0x4;
	dcb->sq_reg.reg_rea	= (int *) sq_base + 0x6;
	dcb->sq_reg.reg_buf_win	= (int *) sq_base + 0x8;
	dcb->sq_reg.reg_rp	= (int *) sq_base + 0xA;
	dcb->sq_reg.reg_tp	= (int *) sq_base + 0xC;
	dcb->sq_reg.reg_dma_ad	= (int *) sq_base + 0xE;

	sq_reset(dcb);
	sq_getaddr(dcb, dcb->etaddr);

	if ( SetEvent( &dcb->event ) != 0 ) 
	{
		IOdebug("SERIOUS - SEEQ ethernet interface: SetEvent failed");
		Free(dcb);
		return NULL;
	};

	err = PriFork(2000, Reader, 4, dcb);
	if (err == 0 )
		goto bad;
	
	err = PriFork(2000, Starter, 4, dcb);
	if (err == 0 )
		goto bad;

	err = PriFork(2000, sqintr_process, 4, dcb);
	if (err == 0 )
		goto bad;
		
	
#if DBG_STAT
	err = Fork(1000, DbgStatistics, 4, dcb);
	if (err == 0)
		goto bad;
#endif
		
	sq_config(dcb);
	sq_earcv(dcb);	
	
	return dcb;

bad:	IOdebug("SEEQ ethernet interface: Fork failed");
	Free(dcb);
	return NULL;
}

/*-----------------------------------------------------------------------*/

void init_lpb_par (ETN_DCB *dcb)
{
	sq_tea_addr = (sq_tea << 8) + 0x0100;
  	sq_xmit_area_l = sq_tea_addr - SQ_LPB_START;
  	sq_rec_area_l = SQ_LPB_L - sq_xmit_area_l;
}

/* ------------------------------------------------------ */

void sq_set_buf_code (ETN_DCB *dcb, int buf_code)
{
  	*sq_conf_1 = (*sq_conf_1 & 0xFFF0) |  buf_code;
}

/* ------------------------------------------------------ */

void sq_write_buf_win (ETN_DCB *dcb, int buf_code, int value)  
{
  	sq_set_buf_code( dcb, buf_code );
  	*sq_buf_win = value;
}

/* ------------------------------------------------------ */

int sq_read_buf_win (ETN_DCB *dcb, int buf_code)
{
  	sq_set_buf_code( dcb, buf_code );
  	return( *sq_buf_win & LOW_HALF );
}

/* ------------------------------------------------------ */

void sq_set_com_bits( ETN_DCB *dcb, int mask )
{
  	sq_com_reg |= mask;
  	*sq_command = sq_com_reg;
  	sq_com_reg &= 0xF;
}

/* ------------------------------------------------------ */

void sq_clr_com_bits( ETN_DCB *dcb, int mask )
{
  	sq_com_reg &= ~mask;
  	*sq_command = sq_com_reg;
  	sq_com_reg &= 0xF;
}

/* ------------------------------------------------------ */

void sq_if_write_wait_fifo ( ETN_DCB *dcb )
{
  	if ( ( *sq_status & SQ_FIFO_DIR ) == 0 )
    	{
    		int loop = 100 ;	
    		while  ( (*sq_status & SQ_FIFO_EMPTY) == 0  &&  --loop >= 0 ) 
       			;		
	    	if (loop < 0)
    			_Trace((int)dcb, 0, 0);
    	}
}

/* ------------------------------------------------------ */

void sq_set_dma_addr( ETN_DCB *dcb, int addr )
{
  	sq_if_write_wait_fifo (dcb);
  	sq_set_buf_code ( dcb, BC_LPB );
  	*sq_dma_ad = addr;
}

/* ------------------------------------------------------ */

int read_tr_area( ETN_DCB *dcb, int addr )
{
  	if ( (sq_tea_addr <= addr) && (addr < sq_tea_addr + 16) )     
              addr  -=  sq_tea_addr;

  	if  ( (0 <= addr)  && (addr < sq_tea_addr - 1) )
    	{
    		sq_set_dma_addr ( dcb, addr );
    		sq_set_com_bits ( dcb, SQ_FIFO_READ );
    		return ( *sq_buf_win & LOW_HALF );
    	}
  	elif   ( addr == sq_tea_addr - 1 )
    	{
    		int  l_byte , h_byte ;
    
    		sq_set_dma_addr ( dcb, addr );
    		sq_set_com_bits ( dcb, SQ_FIFO_READ );
    		l_byte  =  *sq_buf_win & LOW_BYTE;

    		sq_set_dma_addr ( dcb, 0 );
    		sq_set_com_bits ( dcb, SQ_FIFO_READ );
    		h_byte  =  *sq_buf_win & LOW_BYTE;

    		return ( ( h_byte << 8 ) + l_byte );
    	}
    	else _Trace((int)dcb, addr, 0);

	return 0;
}

/* ------------------------------------------------------ */

void write_tr_area ( ETN_DCB *dcb, int addr, int value )
{
  	if ( (sq_tea_addr <= addr)  &&  (addr < sq_tea_addr + 16) )
    		addr  -=  sq_tea_addr ;
	
  	if  ( (0 <= addr)  &&  (addr < sq_tea_addr) )
    	{
    		sq_if_write_wait_fifo (dcb);
    		sq_set_com_bits( dcb, SQ_FIFO_WRITE );
    		sq_set_dma_addr( dcb, addr );
    		*sq_buf_win = value;
    	}
    	else _Trace((int)dcb, addr, value);
}

/* ------------------------------------------------------ */

int read_rc_area( ETN_DCB *dcb, int addr )
{  
  	if ( (SQ_LPB_L <= addr)  &&  ( addr < (SQ_LPB_L + 0x10) ) ) 
    		addr  -=  sq_rec_area_l ;
    
  	if  ( (sq_tea_addr <= addr)  &&  (addr < SQ_LPB_L) )
    	{
    		sq_set_dma_addr ( dcb, addr );
    		sq_set_com_bits ( dcb, SQ_FIFO_READ );
    		return ( *sq_buf_win & LOW_HALF );
    	}
    	else _Trace((int)dcb, addr, 0);

	return 0;
}

/* ------------------------------------------------------ */

void write_rc_area( ETN_DCB *dcb, int addr, int value )
{
  	if ( (SQ_LPB_L <= addr)  &&  ( addr < (SQ_LPB_L + 0x10) ) ) 
    		addr  -=  sq_rec_area_l ;
    
  	if ( (sq_tea_addr <= addr)  &&  (addr < 0x10000) )
    	{
    		if   ( addr != 0xFFFF )
      		{
      			sq_if_write_wait_fifo (dcb);
      			sq_set_com_bits ( dcb, SQ_FIFO_WRITE );
      			sq_set_dma_addr ( dcb, addr );
      			*sq_buf_win = value;
      		}
    		else
        		_Trace((int)dcb, addr, value);
    	}
  	else
        		_Trace((int)dcb, addr, value);
}

/* ------------------------------------------------------ */

int read_tr_area_p ( ETN_DCB *dcb, int addr )
{
  	int	val ;
	
	*sq_conf_2 |= SQ_BYTE_SWAP;
  	val = read_tr_area ( dcb, addr );
  	*sq_conf_2 &= ~SQ_BYTE_SWAP;
  	return (val);
}

/* ------------------------------------------------------ */

void write_tr_area_p ( ETN_DCB *dcb, int addr , int pointer )
{
  	*sq_conf_2 |= SQ_BYTE_SWAP;
  	write_tr_area( dcb, addr , pointer );
  	*sq_conf_2 &= ~SQ_BYTE_SWAP ;
}

/* ------------------------------------------------------ */

int read_rc_area_p ( ETN_DCB *dcb, int addr )
{
  	int	val ;

  	*sq_conf_2 |= SQ_BYTE_SWAP;
  	val = read_rc_area  ( dcb, addr );
  	*sq_conf_2 &= ~SQ_BYTE_SWAP;
  	return (val);
}

/* ------------------------------------------------------ */

void write_rc_area_p ( ETN_DCB *dcb, int addr , int pointer )
{
  	*sq_conf_2 |= SQ_BYTE_SWAP;
  	write_rc_area ( dcb, addr , pointer );
  	*sq_conf_2 &= ~SQ_BYTE_SWAP;
}

/* ------------------------------------------------------ */

void sq_en_read_dma (ETN_DCB *dcb, int dma_addr)
{
	sq_set_dma_addr ( dcb, dma_addr );
	
	sq_set_com_bits ( dcb, SQ_FIFO_READ );
	sq_set_com_bits ( dcb, SQ_SET_DMA_ON );
	
	*etn_ffs = SQ_FF_IDAP;

#if DEC3 < 3
	*etn_ffs = SQ_FF_IDAP | SQ_FF_DMA;
#else
	*etn_ffs = 0;
	/* SQ_FF_DWR not set means read mode */
#endif
	*etn_ffs  =  SQ_FF_DMA	;
}

/* ------------------------------------------------------ */

void sq_en_write_dma ( ETN_DCB *dcb, int dma_addr )
{
  	sq_if_write_wait_fifo (dcb);

  	sq_set_com_bits ( dcb, SQ_FIFO_WRITE );
  	sq_set_com_bits ( dcb, SQ_SET_DMA_ON );

  	sq_set_dma_addr ( dcb, dma_addr );
	
  	*etn_ffs = SQ_FF_IDAP;
#if DEC3 < 3
  	*etn_ffs = SQ_FF_IDAP | SQ_FF_DWR | SQ_FF_DMA;
#else
  	*etn_ffs = 0;
  	*etn_ffs = SQ_FF_DWR;
  	/*  DWR * /DMA sets FF TG in PAL DAP  */
#endif
  	*etn_ffs = SQ_FF_DWR | SQ_FF_DMA ;
}

/* ------------------------------------------------------ */

void sq_disable_dma (ETN_DCB *dcb)
{  
	int  loop = 100;
  	while ( ( *etn_ffs & SQ_DREQ_BIT )  &&  ( --loop >= 0 ) )  
  		;

  	if (loop < 0)
      		_Trace((int)dcb, 0, 0);
    
  	*etn_ffs = SQ_FF_IDAP;
  	*etn_ffs = 0x0;

  	sq_if_write_wait_fifo (dcb);
  	sq_set_com_bits ( dcb, SQ_SET_DMA_OFF );
}

/* ------------------------------------------------------ */

word Sq_GetInfo(ETN_DCB *dcb, NetInfo *info, int size)
{
	if ( (info == 0) || (size < sizeof(NetInfo) ) )
		return(-1);
	info->Mask &= NetInfo_Mask_Addr;
	if (info->Mask & NetInfo_Mask_Addr) 
		memcpy(info->Addr, dcb->etaddr, 6);
	return(0);
}

/* ------------------------------------------------------ */

word Sq_SetInfo(ETN_DCB *dcb, NetInfo *info, int size)
{
	if ( (info == 0) || (size > sizeof(NetInfo) ) )
		return(-1);
	return(0);

	dcb = dcb;
}

/* ------------------------------------------------------ */

/*
 * SEEQ Interrupt Service Routines
 */
 
void sqintr(ETN_DCB *dcb)
{
	Signal(&dcb->nintr);
}

void sqintr_process (ETN_DCB *dcb)
{
  	int status;
	int events;
	
	forever
	{
		Wait(&dcb->nintr);
		if (dcb->terminate)
		{
			Signal(&dcb->termintr);
			return;
		}

		Wait(&dcb->seeq);
		dcb->sq_stat.eventsall++;
		events = 0;
		
		while( 
			(
			   ((status = *sq_status) & SQ_RX_INT )  
			   &&
			   ( status & SQ_RX_INT_EN )  
			)  ||  (
	 		   ( status & SQ_TX_INT )  
	 		   &&
	 		   ( status & SQ_TX_INT_EN ) 
	 		)
		     ) 
		{
		    	/* Check if receive interrupt occurred */
		    	if  ( 
		    		( status & SQ_RX_INT )  
		    		&&
		    		( status & SQ_RX_INT_EN )
		    	    ) 
		      	{   
				dcb->sq_stat.eventsrx++;
				events++;
				
		        	/* Disable receive interrupt */	
		        	sq_clr_com_bits ( dcb, SQ_EN_RX_INT  ) ;
	
		        	/* Ack' receive interrupt */	
		        	sq_set_com_bits ( dcb, SQ_RX_INT_ACK ) ;
		
				Signal(&dcb->seeq);
				Sq_Recv(dcb);
				Wait(&dcb->seeq);
	
				/* Re-enable receive interrupt */
		      		sq_set_com_bits ( dcb, SQ_EN_RX_INT ) ;
			}
		    	/* Check if transmit done interrupt */
		    	if   ( 
		    		( status & SQ_TX_INT )  
		    		&&  
		    		( status & SQ_TX_INT_EN )
		    	     ) 
		      	{
				dcb->sq_stat.eventstx++;
				events++;
				
		        	/* Disable transmit interrupt */	
		        	sq_clr_com_bits ( dcb, SQ_EN_TX_INT  ) ;
	
				/* Ack' transmit interrupt */
		      		sq_set_com_bits ( dcb, SQ_TX_INT_ACK ) ;
	
				sq_xmitdone(dcb);
	
				/* Re-enable transmit interrupt */
		      		sq_set_com_bits ( dcb, SQ_EN_TX_INT ) ;
		  	}
		  	
			unless (events) dcb->sq_stat.eventsnull++;
			
		}
		Signal(&dcb->seeq);
	}
}  

/* ------------------------------------------------------ */

/* Use carefully, hence all SEEQ activities will be disabled */
void sq_getaddr	( ETN_DCB *dcb, byte *physaddr ) 
{
	int i;
	
	sq_dadma  ( dcb );
	sq_darcv  ( dcb );
	sq_daxmit ( dcb );

	sq_set_dma_addr ( dcb, STA_0_ADDR );
	sq_set_buf_code ( dcb, BC_PROM );

#if ADDRPROM
	for  ( i=0 ; i < 6 ; i++ )
  		*(physaddr++) = dcb->etaddr[i] = (byte)(*sq_buf_win & LOW_BYTE);
#else
	for  ( i=0 ; i < 6 ; i++ )
  		*(physaddr++) = dcb->etaddr[i];
#endif
	IOdebug("TPM-ETN hardware address: %x %x %x %x %x %x",
	dcb->etaddr[0], dcb->etaddr[1], dcb->etaddr[2],
	dcb->etaddr[3], dcb->etaddr[4], dcb->etaddr[5]);
}

/* ------------------------------------------------------ */

int sq_config ( ETN_DCB *dcb ) 
{
	int	value, i;
	
	sq_tea = 0x10;
	init_lpb_par( dcb );
		
	sq_com_reg = 0;
	sq_rcv_was_on = FALSE; 
	
	/*  Init.  Configuration Register 2   */
	value  =  0x0000;
	if  ( conf_watch_dis ) 		value  |=  SQ_WATCH_T_DIS	;
	if  ( conf_loop_back )		value  |=  SQ_LOOP_BACK 	;
	if  ( 0 )			value  |=  SQ_XMIT_NO_CRC	;
	if  ( 0 )			value  |=  SQ_RECV_CRC	 	;
	if  ( 0 )			value  |=  SQ_ADDR_LENG 	;
	if  ( 0 )			value  |=  SQ_XMIT_NO_PRE 	;
	if  ( 0 )			value  |=  SQ_SLOT_TIME 	;
	if  ( conf_acc_short_fr )	value  |=  SQ_SHORT_FR_EN 	;
	if  ( conf_acc_drib_err )	value  |=  SQ_DRIB_ERR_EN 	;
	if  ( conf_acc_crc_err  )	value  |=  SQ_CRC_ERR_EN 	;
	if  ( conf_auto_rea     )	value  |=  SQ_AUTO_U_REA 	;
	if  ( conf_byte_swap  )		value  |=  SQ_BYTE_SWAP 	;
	*sq_conf_2 = value  ;
	
	*sq_rea	= SQ_REA;		/*  Define Receive  End Area */
		
	sq_rec_start = sq_tea_addr;
	*sq_rp = sq_rec_start;		/*  Define Receive Pointer */
	
	sq_xmit_start = 0;
	*sq_tp = sq_xmit_start;		/*  Define Transmit Pointer */
	
	/*  Define Address Match Mode */
	value = ( conf_addr_mm << 14 ) & 0xC000  ;
	
	/*  Init.  Configuration Register 1   */
	
	value |= SQ_STA_0_EN;		/* subunit 0 in use */
	*sq_conf_1 = value;
		
	/*   Define Station Addresses   */
	sq_darcv ( dcb );
	sq_set_buf_code  ( dcb, 0 );
	for  ( i = 0 ; i < 6 ; i++ )
		*sq_buf_win = dcb->etaddr[i];

	/*  Enable transmit / receive interrupt   */
	sq_write_buf_win (dcb, BC_TEA, sq_tea);  /*  Define Transmit End Area */
	sq_set_com_bits (dcb, SQ_EN_RX_INT | SQ_EN_TX_INT );

	return (0) ;	
}

/*-----------------------------------------------------------------*/

void sq_start ( ETN_DCB *dcb ) 
{
  /*	Note:
  	DMA accesses to the 8005 by the T8 are wordwise ( 32 bits).
  	The last transferred word of a DMA block is sometimes
  	corrupted, although the interface of the 8005 is correct.
  	Solution here:  The small loop, which transfers the DMA data,
  	transfers at loop end an additional dummy word, 
  	which, when corrupted, will do no harm. 
  	( See constant  ONE_FOR_BAD_LAST_INT.)
  */

	int 		block_l, size, loc_dma_ad;
	byte		low_pp, high_pp, com_byte, status_byte, *hp;
	short		header[2];
	NetDevReq 	*req;

	Wait(&dcb->lock);
	req = (NetDevReq *)RemHead(&dcb->startq);
	if (req) 
		dcb->stqlen--;
	Signal(&dcb->lock);
	if(req == NULL)
		return;
	
	size = (int)req->Size;
	if ( req->Size > ETHERMTU + ETHERHDR ) {
		size = (int)(req->Size > 2000 ? 2000 : req->Size);
		IOdebug("SEEQ ethernet interface: packet to long, %d bytes", req->Size);
	}

/*	DbgPaket("sq_start req", req->Buf, req->Size); */

	Wait(&dcb->seeq);

	/* build SEEQ transmit header */
	low_pp      =  ( (size + SQ_XMITHDR) & LOW_BYTE );
	high_pp     =  ( (size + SQ_XMITHDR) & HIGH_BYTE ) >> 8;
	
	com_byte    =  0xAC; 
	status_byte =  0x00;
	
	hp     =  (byte *) header; 
	*hp++  =  high_pp;
	*hp++  =  low_pp;
	*hp++  =  com_byte;
	*hp++  =  status_byte;

	/* set dma address */
	loc_dma_ad = sq_xmit_start;

	/* transfer SEEQ transmit header */
	sq_if_write_wait_fifo (dcb);
	sq_set_com_bits  (dcb, SQ_FIFO_WRITE);
	sq_set_dma_addr  (dcb, loc_dma_ad);
	*sq_buf_win = header[0];
	*sq_buf_win = header[1];

	/* adjust dma address */
	loc_dma_ad += 4;
	
	/* calculate dma length */
	block_l = ( 
		size / BYTES_PER_INT 
		+  ONE_FOR_ROUND_UP
		+  ONE_FOR_BAD_LAST_INT
	     )
		*  BYTES_PER_INT;
		
	/* copy the packet into the wordaligned buffer for dma */	

	memcpy( dcb->Wbuf , req->Buf, size );

/*	DbgPaket("sq_start dma", dcb->Wbuf, size ); */

	/* transfer the packet */	
	sq_en_write_dma (dcb, loc_dma_ad);

	memcpy( (byte *)dma_win_base , dcb->Wbuf, block_l);

	*tct_write_addr = ONE_FOR_BAD_LAST_INT;
	sq_disable_dma (dcb);

	/* Enable transmit */
	*sq_tp = sq_xmit_start;
	sq_eaxmit( dcb );

	Signal(&dcb->seeq);

	req->Actual = req->Size;
	req->DevReq.Result = 0;
	(*req->DevReq.Action)(dcb,req);	

	Wait(&dcb->txdone);
}

/* ------------------------------------------------------ */

void sq_dadma (ETN_DCB *dcb) 
{
	sq_disable_dma (dcb);
}

/* ------------------------------------------------------ */

void sq_darcv (ETN_DCB *dcb) 
{
	sq_rcv_was_on = FALSE;
	sq_set_com_bits (dcb, SQ_SET_RX_OFF);
}

/* ------------------------------------------------------ */

void sq_daxmit (ETN_DCB *dcb) 
{
	sq_set_com_bits (dcb, SQ_SET_TX_OFF);
}

/* ------------------------------------------------------ */

void sq_earcv(ETN_DCB *dcb) 
{
	sq_rcv_was_on = TRUE;
	sq_clr_com_bits  (dcb, SQ_SET_RX_OFF); 
	sq_set_com_bits  (dcb, SQ_SET_RX_ON); 
}


/* ------------------------------------------------------ */

void sq_eaxmit (ETN_DCB *dcb) 
{
       	sq_clr_com_bits (dcb, SQ_SET_TX_OFF);
	sq_set_com_bits (dcb, SQ_SET_TX_ON);
}

/* ------------------------------------------------------ */

int sq_recvdone	(ETN_DCB *dcb, byte *buf, int *size)
{
	int	next, len;	
	
  	/* test if there is a packet in the recievequeue */
	unless ( read_rc_area ( dcb, sq_rec_start ) & LOW_BYTE ) {
		*size = 0;
		return(0); 
	}

	dcb->sq_stat.ipackets++;

	len = sq_upload(dcb, dcb->Rbuf, 2000);

 	if ( 
 		(len <= 0)
 		||
 		(sq_recvstat( 0 ) != 0) 
 	   )
 	{
		dcb->sq_stat.ierrors++;	
		goto testnext ;
	}

	/* remember there is a seeq header in front of the packet */
/*	DbgPaket("sq_recvdone dma", dcb->Rbuf + SQ_RCVHDR, len); */

	len = len > *size ? *size : len;
	memcpy(buf, dcb->Rbuf + SQ_RCVHDR, len);

/*	DbgPaket("sq_recvdone req", buf, len); */

testnext:
	*size = len;
	
  	/* if there is another paket in the recievequeue next is != 0 */
	next = read_rc_area ( dcb, sq_rec_start ) & LOW_BYTE; 
		
	return (next);
}

/* ------------------------------------------------------ */

int sq_recvstat (byte *buffer) 
{
	return ( *(buffer + BO_FR_ST) & SQ_ERRMSK );
}

/* ------------------------------------------------------ */

int sq_reset (ETN_DCB *dcb) 
{
	*etn_ffs = SQ_FF_IDAP;
	*etn_ffs = 0x0;
	
	*sq_conf_2 = SQ_RESET;
	Delay ( 200 );
	*sq_conf_2 = 0;
	Delay ( 200 );
	return( 0 );
}

/* ------------------------------------------------------ */

int sq_upload(ETN_DCB *dcb, byte *buffer, int size) 
{
  /*	Note:
  	DMA accesses to the 8005 by the T8 are wordwise ( 32 bits).
  	The last transferred word of a DMA block is sometimes
  	corrupted, although the interface of ther 8005 is correct.
  	Solution here:  The small loop, which transfers the DMA data,
  	transfers at loop end an additional dummy word, 
  	which, when corrupted, will do no harm. 
  	( See constant  ONE_FOR_BAD_LAST_INT.)
  	Receiver buffers must be supplied in size accordingly.
  */

  	int  n;				/* number of uploaded bytes */
  	int  next_header ;
  	int  block_l , *ad ;

  	/* If next header address is empty, then the packet ,
     		which triggered the event, is already uploaded. */
  	next_header = read_rc_area_p (dcb, sq_rec_start);
  	if ( 
  		( ( next_header & HIGH_BYTE ) == 0 ) 
  		||
		( ( read_rc_area(dcb, sq_rec_start + 2) & SQ_RCVDONE ) == 0 )
	   )
    	{
		return(-1);
    	}

  	/*  Compute number of bytes to upload */
  	n = ( next_header + sq_rec_area_l - sq_rec_start ) % sq_rec_area_l;

  	/* Compute number of byte-accesses */
  	block_l = ( n / BYTES_PER_INT 
	 	   + ONE_FOR_ROUND_UP 
	  	   + ONE_FOR_BAD_LAST_INT
		  ) * BYTES_PER_INT;

	if ( block_l > (size - 8) )
		block_l = size - 8;

  	/* Get Received Packet from buffer */
  	sq_en_read_dma(dcb, sq_rec_start);
	memcpy( buffer, dma_win_base, block_l);
	ad = (int *)(buffer + block_l);
  	*ad++  =  *tct_read_addr;	/*  First read activates DMA TCT    */
  	*ad++  =  *dma_win_base;	/*  Second read is normal DMA read  */

  	sq_disable_dma (dcb);

  	/* Update sq_rec_start for next call of sq_upload */
  	sq_rec_start = next_header ;

  	/*  Free associated memory in SEEQ receive area = Update REA  */
  	*sq_rea = ( ( next_header >> 8 ) & LOW_BYTE );
  
  	if  ( ! ( (sq_tea_addr <= next_header )&&( next_header < SQ_LPB_L ) ) )
	{
		sq_darcv(dcb);
		*sq_rea	= SQ_REA;		/*  Define Receive  End Area */
		sq_rec_start = sq_tea_addr;
		*sq_rp = sq_rec_start;		/*  Define Receive Pointer */
		/* Re-enable receive interrupt */
      		sq_set_com_bits (dcb, SQ_EN_RX_INT);
		sq_earcv(dcb);
	}

  	return (n - SQ_RCVHDR) ;
}

/* ------------------------------------------------------ */

int sq_xmitstat	(ETN_DCB *dcb)
{
	return( read_tr_area( dcb, sq_xmit_start + 3) & SQ_ERRMSK );    
}

/* ------------------------------------------------------ */

int xmit_enabled (ETN_DCB *dcb)
{
	return( (*sq_status & SQ_TX_ON) != 0 );      
}

/* ------------------------------------------------------ */

void sq_xmitdone (ETN_DCB *dcb) 
{
	int stat;

	sq_daxmit(dcb);
	dcb->sq_stat.opackets++;
	
	/*  Check transmit status  */
	stat =  sq_xmitstat (dcb);
	
	if ( (stat & SQ_XMIT16COL) == SQ_XMIT16COL)  
  	{
  		dcb->sq_stat.collisions += 16 ;
  		dcb->sq_stat.oerrors++ ;
  	} 
	if ( (stat & SQ_XMIT1COL) == SQ_XMIT1COL)  
  		dcb->sq_stat.collisions++;

	if ( (stat & SQ_XMITBBL) == SQ_XMITBBL) 
	  	dcb->sq_stat.oerrors++;

	Signal(&dcb->txdone);
}

/* ------------------------------------------------------ */

word 
PriFork ( word stsize, VoidFnPtr fn, word argsize, ... )
{
    byte	*args	= ( ( byte * ) &argsize ) + sizeof ( argsize );
    word	*proc	= ( word * ) NewProcess ( stsize, fn, argsize );

    if( proc == NULL ) 
    	return FALSE;

    memcpy ( (byte *)proc, args, (int)argsize );
	
    StartProcess ( proc, 0 );

    return TRUE;
}


/* end of sqdev.c */
