static char rcsid[] = "$Header: /giga/HeliosRoot/Helios/filesys/fs/RCS/dev.c,v 1.2 1991/03/21 15:09:40 nick Exp $";

/* $Log: dev.c,v $
 * Revision 1.2  1991/03/21  15:09:40  nick
 * Brought up to date. disables buffer checksumming, several other fixes.
 *
 * Revision 1.3  90/05/30  15:36:05  chris
 * All sorts of things
 * Including passing back of device error code
 * 
 * Revision 1.2  90/02/01  17:36:04  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:02  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                    H E L I O S   F I L E S E R V E R                	**
**                    ---------------------------------                	**
**                                                                      **
**              Copyright (C) 1989, Perihelion Software Ltd.            **
**                        All Rights Reserved.                          **
**                                                                      **
** dev.c								**
**                                                                      **
**	Generic device interface code for fileserver.			**
**                                                                      **
**	Author:   NHG 15/2/89						**
**                                                                      **
*************************************************************************/

#define  DEBUG 	 0

#include "nfs.h"
#include <device.h>

void open_dev  (DiscDevInfo *ddi);
void close_dev (void);
void read_dev (struct packet *);
word read_blk (void *bp, daddr_t bnr);
void write_dev (struct packet *);
word write_blk (void *bp, daddr_t bnr);

DCB *discdcb = NULL;

void open_dev  (DiscDevInfo *ddi)
{
	discdcb = OpenDevice(RTOA(ddi->Name),ddi);
}

void close_dev (void)
{
	CloseDevice(discdcb);
}

void dev_action(DiscReq *req)
{
	Signal(&req->WaitLock);
}

#define do_req(req)  InitSemaphore(&((req)->WaitLock),0);\
			Operate(discdcb,(req)); \
			Wait(&((req)->WaitLock));

word do_dev_seek(word fn, word dev, word timeout, word pos)
{
	DiscReq req;

	req.DevReq.Request = fn;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = dev;
	req.DevReq.Timeout = timeout;
	req.Pos  = pos;
#ifdef IODEBUG
	IOdebug("do_dev_rdwr: function %d,subdevice = %d, no. %d, size %d",
				fn,req.DevReq.SubDevice,pos,size);
#endif
	do_req(&req);
	return req.DevReq.Result;
}

word do_dev_rdwr(word fn, word dev, word timeout, word size, word pos,
				char *buf, word *deverror )
{
	DiscReq req;

	req.DevReq.Request = fn;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = dev;
	req.DevReq.Timeout = timeout;
	req.Size = size;
	req.Pos  = pos;
	req.Buf = buf;
#ifdef IODEBUG
	IOdebug("do_dev_rdwr: function %d,subdevice = %d, no. %d, size %d",
				fn,req.DevReq.SubDevice,pos,size);
#endif
	do_req(&req);
	*deverror = req.DevReq.Result;
	return req.Actual;
}

word checksum(caddr_t b, word size)
{
	int i;
	word *wb = (word *)b;
	word wsize = size/4;
	word r = 0;

	for(i = 0; i < wsize; i++)
		r += wb[i];
	return r;
}

void read_dev (struct packet *tbp)
{
	daddr_t	bnr;
	word    bcnt, done;
	struct buf *lbp, *ibp;
	word err;

	/* first physical block # */
	bnr  = tbp->p_fbp->b_bnr;	
	/* # of blocks to transfer */
	bcnt = tbp->p_blk_cnt;		
	
	if ( (bnr < 0) || (bnr >= maxncg*maxbpg)
	   ||
	     (bcnt <= 0) || (bcnt > phuge) )
	{
IOdebug("read_dev :	INVALID PARAMETERS : Reading %d blocks from %d",bcnt,bnr);
		longjmp(term_jmp, 1);
	}

	done = do_dev_rdwr(FG_Read, tbp->p_fbp->b_dev,
				-1, bcnt*BSIZE, bnr*BSIZE, 
				(char *)tbp->p_fbp->b_un.b_addr, &err);
	done /= BSIZE;

	/* if couldn't succeed */
	if ( done != bcnt ) 
	{
		/* read error */
IOdebug("	read_dev :	ERROR : Read only %d blocks i.o %d at %d!!!",done,bcnt,bnr);
		longjmp(term_jmp, 1);
	} 
	else 
	{
#if DEBUG
printf("read_dev :	Read %d blocks from %d\n",bcnt,bnr);
#endif
		/* update valid blocks counter in the packet header */
		tbp->p_vld_cnt = tbp->p_blk_cnt;
		/* set the valid flags in the buffer headers */
		lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
		for (ibp=tbp->p_fbp; ibp<=lbp; ibp++) {
			if (checksum_allowed) {
				ibp->b_checksum = checksum(ibp->b_un.b_addr,BSIZE);
			}
			ibp->b_vld = TRUE;
		}
	}	 
}


/* 
 * Read block directly, avoiding the use of the buffer-cache
 *
 */
word read_blk (void *bp, daddr_t bnr)
{
	word err;
	word read;
	
#ifdef IODEBUG
	IOdebug("read_info_blk: subdevice = %d",filedrive);
#endif

	read = do_dev_rdwr(FG_Read, filedrive,
				-1, BSIZE, bnr*BSIZE, 
				(char *) bp, &err);
	return (read);
}


void write_dev (struct packet *tbp)
{	
	daddr_t	bnr;
	word    bcnt, done;
	struct buf *lbp, *ibp;
	word err;

	/* first physical block # */
	bnr  = tbp->p_fbp->b_bnr;	
	/* # of blocks to transfer */
	bcnt = tbp->p_blk_cnt;		
	
	if ( (bnr < 0) || (bnr >= maxncg*maxbpg)
	   ||
	     (bcnt <= 0) || (bcnt > phuge) )
	{
IOdebug("write_dev :	INVALID PARAMETERS : Writing %d blocks to %d",bcnt,bnr);
		longjmp(term_jmp, 1);
	}

	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;

	if (checksum_allowed) {
		for (ibp=tbp->p_fbp; ibp<=lbp; ibp++)
		{
			if( ibp->b_checksum != checksum(ibp->b_un.b_addr,BSIZE) )
			{
				IOdebug("	write_dev :	ERROR : Block %d has incorrect checksum !",
							bnr);
				longjmp(term_jmp, 1);
			}
		}
	}

	done = do_dev_rdwr(FG_Write, tbp->p_fbp->b_dev,
				-1, bcnt*BSIZE, bnr*BSIZE, 
				(char *)tbp->p_fbp->b_un.b_addr, &err);
	done /= BSIZE;
	if ( done != bcnt ) 
	{
		/* write error */
IOdebug("	write_dev :	ERROR : Written only %d blocks i.o. %d at %d!!!",done,bcnt,bnr);
		longjmp(term_jmp, 1);
	} 
	else 
	{
#if DEBUG
printf("write_dev :	Wrote %d blocks to %d\n",bcnt,bnr);
#endif
		/* update delayed-write blocks counter in packet header */
		tbp->p_dwt_cnt = 0;
		/* reset delayed-write flags in the buffer headers */
		lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
		for (ibp=tbp->p_fbp; ibp<=lbp; ibp++)
			ibp->b_dwt = FALSE;
	}	 
}

/* 20/03/89 , HJE
 *
 * Write info-block directly to cg 0 by avoiding the use of the buffer-cache
 *
 */
word write_blk (void *bp, daddr_t bnr)
{
	word err;
	word written;
	
#ifdef IODEBUG
	IOdebug("write_blk: subdevice = %d",filedrive);
#endif
	written = do_dev_rdwr(FG_Write, filedrive,
				-1, BSIZE, bnr*BSIZE, 
				(char *) bp, &err);
	return (written);
}


/*
#include <sysdev.c>
*/
