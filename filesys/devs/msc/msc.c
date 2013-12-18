/*************************************************************************
**                                                                      **
**                     H E L I O S   F I L E S E R V E R                **
**                     ---------------------------------                **
**                                                                      **
**             Copyright (C) 1988, Parsytec GmbH                        **
**                        All Rights Reserved.                          **
**                                                                      **
**  msc.c								**
**                                                                      **
**	Routines of block I/O to the MSC driver.			**
**                                                                      **
**	Author:   M.Clauss 1/9/88					**
**                                                                      **
**      Modified: A.Ishan  9/1/89                                       **
**                                                                      **
**      Modified: A.Ishan  9/6/89                                       **
**                                                                      **
*************************************************************************/

#define DEBUG 0

#include <helios.h>
#include <sem.h>
#include <stdio.h>

#include <stdlib.h>

#include <syslib.h>
#include <device.h>
#include <codes.h>
#include <root.h>
#include <gsp.h>

typedef word	daddr_t;

#include "o_inter.h"

#include "chan_io.h"

#define	CURRENT_DIR	"/helios/lib"
#define	MSC_DRIVER	"msc02.gen"

#define	BLK_SIZE	512	/* size of transfer unit to MSC */
#define	DUMMY_SIZE	32	/* size of dummy read buffer */
#define	COMMAND_SIZE	32	/* size of common command buffer */

typedef struct DiscDCB {
	DCB		DCB;
	Channel 	cmd_out, data_in, data_out, cmd_in, dum_1, dum_2;
	word 		dummy_buffer[DUMMY_SIZE];
 	word 		command[COMMAND_SIZE];
	bool 		ok;
	Semaphore	Lock;
} DiscDCB;

/* ==================================================================== */

#define	MSC_SCSI_ADDR	7	/* SCSI address of MSC board */
#define	WINNIES		1	/* number of winchester devices */
#define	FLOPPIES	0	/* number of floppy devices */
#define	STREAMER	0	/* number of streamer tapes */

#define	CLASS		1	/* Device class in init () */
#define	DEVICE		0	/* Device no in init () */
#define	DEV_TYPE	0	/* Device_type in init () */
#define	ADR		0	/* SCSI address for winchester in init () */
#define PID		0	/* default pid number */
#define PRI		0	/* default priority */
#define LUN		0	/* default lun in init() */

/* ==================================================================== */

/* General procedures */

DiscDCB *DevOpen(Device *, char *);
word DevClose(DiscDCB *);
void DevOperate(DiscDCB *,DiscReq *);

int load_code (string,string, O_HEADER *, byte **, byte **, byte **, word);

/* ==================================================================== */

/* Sub procedures 1 */

void msc_clear (DiscDCB *);
void msc_init (DiscDCB *);
void msc_load (DiscDCB *);
	static void reserve (DiscDCB *,word, word);
	static void release (DiscDCB *,word, word);
word msc_read (DiscDCB *,word, word, byte *);	
word msc_write (DiscDCB *,word, word, byte *);	
void msc_finish (DiscDCB *);

/* ==================================================================== */

/* Sub procedures 2 */

void dummy_c_read (DiscDCB *);
void dummy_c_write (DiscDCB *);
void wait_for_result (DiscDCB *);

/* ==================================================================== */

/* command tags for MSC protocol */

#define	CO_OK		0

#define	CO_SPECIAL	0
#define	CO_WINCH	1
#define	CO_FLOPPY	2

#define	CO_CLEAR	0
#define	CO_INIT		1
#define	CO_LOAD		2
#define	CO_READ		3
#define	CO_WRITE	4
#define	CO_FINISH	11
#define	CO_RESERVE	21
#define	CO_RELEASE	22

/* ==================================================================== */
/* ==================================================================== */

DiscDCB *
DevOpen(Device *dev, char *discserver)

/*
*  After loading the MSC-board with MSC-driver routines,
*  initialises MSC-driver protocol with communiction routines.
*/

{
	string fname,cdname; 
	O_HEADER ohdr; 
	byte *code, *wspace, *vspace;
	word psize, load;
	byte *proc;
	
	DiscDCB *dcb = Malloc(sizeof(DiscDCB));
	
	if( dcb == NULL )
		return NULL;

	dcb->DCB.Device = dev;
	dcb->DCB.Operate = DevOperate;
	dcb->DCB.Close = DevClose;
/*	InitList(&dcb->DCB.Requests);		*/
/*	InitList(&dcb->DCB.Replies);		*/
	
	InitSemaphore(&dcb->Lock,1);
	
	dcb->cmd_out = MinInt;
	dcb->cmd_in = MinInt;
	dcb->data_out = MinInt;
	dcb->data_in = MinInt;
	
	cdname = CURRENT_DIR;
	fname = MSC_DRIVER;
	psize = sizeof(Channel *)*6;
	load = load_code (cdname,fname,&ohdr,&code,&wspace,&vspace,psize);
	if (load) {
IOdebug("	open_dev :	Failed to load MSC_DRIVER !");
		return NULL;
	} 
	
	proc = code + ohdr.entry;
	O_Run (proc, wspace, ohdr.wspace, vspace, psize, 
	        &dcb->cmd_out, &dcb->cmd_in, &dcb->data_in, &dcb->data_out,
	        &dcb->dum_1, &dcb->dum_2);

	/* initialize MSC driver protocol */
	msc_clear (dcb);
	msc_init (dcb);
	msc_load (dcb);

	{
		RootStruct *root;
		root = GetRoot();
		root->MaxBuffers = 400;
	}
	return dcb;
}

/* ==================================================================== */

word 
DevClose(DiscDCB *dcb)

/*
*  Terminates communication with the MSC-board.
*/

{
	Wait(&dcb->Lock);
	
	msc_finish (dcb);

	return Err_Null;
}

/* ==================================================================== */

void 
DevOperate(DiscDCB *dcb,DiscReq *req)

{
	daddr_t bnr = req->Pos / BLK_SIZE;
	word bcnt = req->Size / BLK_SIZE;
	byte *buf = req->Buf;
	
	Wait(&dcb->Lock);

	switch( req->DevReq.Request & FG_Mask )
	{

	case FG_Read:
		req->Actual = 0;
		/* read requested size of blocks into pointed packet */
		req->Actual = msc_read (dcb,bnr, bcnt, buf) * BLK_SIZE;
		if (req->Actual == req->Size) req->DevReq.Result = 0;
		else req->DevReq.Result = 1;
		break;	

	case FG_Write:
		req->Actual = 0;
		/* write requested size of blocks from the pointed packet */
		req->Actual = msc_write (dcb,bnr, bcnt, buf) * BLK_SIZE;
		if (req->Actual == req->Size) req->DevReq.Result = 0;
		else req->DevReq.Result = 1;
		break;	

	}
	Signal(&dcb->Lock);

	(*req->DevReq.Action)(req);
	
	return;	
}

/* ==================================================================== */

/* load_code:								     */
/* =========								     */	
/*	Load the given object file (occam generated) into a buffer , and     */
/* 	return the information necessary to run this code.	             */

int load_code(	string cdname,
		string fname, 
		O_HEADER *ohdr, 
		byte **code, byte **wspace, byte **vspace,
		word  psize )
	
{
	Stream	*sfile;
	Object  *current;
	int	nwords;

	current = Locate(NULL,cdname);
	if ((sfile = Open(current,fname,O_ReadOnly)) == Null(Stream))
		{
IOdebug("Object file '%s'does not exist\n",fname);
		return(-1);}
		
	if (Read(sfile, (BYTE *)ohdr, sizeof(O_HEADER), IOCTimeout) 
		!= sizeof(O_HEADER))
		{
IOdebug("Error reading %s\n",fname);
		return(-1);}

	*code  = Malloc(ohdr->csize);

	if (Read(sfile, *code, ohdr->csize, IOCTimeout) != ohdr->csize)
		{
IOdebug("Error reading %s\n",fname);
		return(-1);}

	/* determine how many words of parameters 		*/
	/* N.B all byte parameters are still passed as words 	*/
	
	nwords = (psize / bytesperword);
	nwords += psize%bytesperword ? 1:0;
	
	/* allocate wspace */
	
	ohdr->wspace = (ohdr->wspace+nwords+2);
	*wspace = Malloc(ohdr->wspace * sizeof(word));
	if (*wspace == Null(byte))
		{return(-1);}
#if DEBUG
IOdebug("allocating %d words work space",ohdr->wspace);	
#endif
	if (ohdr->vspace > 0)
		{
#if DEBUG			
IOdebug("allocating %d words vector space",ohdr->vspace);
#endif
		*vspace = Malloc(ohdr->vspace * sizeof(word));
		if (*vspace == Null(byte))
			{Free(*wspace); return(-1);}
		}
	else
		{*vspace = Null(byte);}
		
	
	return(0);

}
	
/* ==================================================================== */
/* ==================================================================== */

void 
msc_clear (DiscDCB *dcb)

{	
	word *pw, len;
						    
	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = CO_SPECIAL;
	*pw++ = CO_CLEAR;
	*pw++ = MSC_SCSI_ADDR;
	*pw++ = WINNIES;
	*pw++ = FLOPPIES;
	*pw++ = STREAMER;
	len = 8;
#if DEBUG
printf ("clear: wslice->"); fflush (stdout);
#endif
	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
#if DEBUG
printf (":"); fflush (stdout);
#endif
	dummy_c_read (dcb);
	dummy_c_write (dcb);

	wait_for_result (dcb);
}

/* ==================================================================== */

void 
msc_init (DiscDCB *dcb)

{	
	word *pw, len;
	bool first_try = TRUE;
	
retry:		
	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = CLASS;
	*pw++ = DEVICE;
	*pw++ = CO_INIT;
	*pw++ = BLK_SIZE;
	*pw++ = DEV_TYPE;
	*pw++ = ADR;
	*pw++ = LUN;
	len = 9;
	
	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
	dummy_c_read (dcb);
	dummy_c_write (dcb);

	wait_for_result (dcb);
	if (!dcb->ok && first_try)
	{
		first_try = FALSE;
		goto retry;
	}
}

/* ==================================================================== */

static void 
reserve (DiscDCB *dcb,word class, word device)

{	
	word *pw, len;
						    
	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = class;
	*pw++ = device;
	*pw++ = CO_RESERVE;
	len = 5;

	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
	dummy_c_read (dcb);
	dummy_c_write (dcb);

	wait_for_result (dcb);
}

/* ==================================================================== */

static void 
release (DiscDCB *dcb,word class, word device)

{	
	word *pw, len;
 
	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = class;
	*pw++ = device;
	*pw++ = CO_RELEASE;
	len = 5;

	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
	dummy_c_read (dcb);
	dummy_c_write (dcb);

	wait_for_result (dcb);
}

/* ==================================================================== */

void 
msc_load (DiscDCB *dcb)

{	
	word *pw, len;
						    
	pw = dcb->command;

	if (CLASS != CO_FLOPPY)
		reserve (dcb,CLASS, DEVICE);

	if (dcb->ok) 
	{
		*pw++ = PID;
		*pw++ = PRI;
		*pw++ = CLASS;
		*pw++ = DEVICE;
		*pw++ = CO_LOAD;
		len = 5;

		ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
		ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
		dummy_c_read (dcb);
		dummy_c_write (dcb);

		wait_for_result (dcb);
	}
	
	if (CLASS != CO_FLOPPY)
		release (dcb,CLASS, DEVICE);
}
	
/* ==================================================================== */

word 
msc_read (DiscDCB *dcb,word blknr, word blk_cnt, byte *buffer)

/* Returns the number of blocks read successfully */

{	
	word *pw, len;
						      
#if DEBUG
printf ("READ  %d blocks from %d\n", blk_cnt, blknr);
#endif

	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = CLASS;
	*pw++ = DEVICE;
	*pw++ = CO_READ;
	*pw++ = blknr;
	*pw++ = blk_cnt;
	len = 7;

	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);

	ChanRead (&dcb->data_in, (byte *)&len, 4);
	if (len > 0)	
		ChanRead (&dcb->data_in, (byte *)buffer, len*4);
	/* len is in units of words */
	dummy_c_write (dcb);

	wait_for_result (dcb);
	if (!dcb->ok) 
	{
		len = 0;
	} 
	else 
	{
		len = len*4;		/* in bytes */
		len = len/BLK_SIZE;	/* in blocks */
	}
	return (len);
}

/* ==================================================================== */

word 
msc_write (DiscDCB *dcb,word blknr, word blk_cnt, byte *buffer)

/* Returns the number of blocks written successfully */

{	
	word *pw, len;
						    
#if DEBUG
printf ("WRITE %d blocks from %d\n", blk_cnt, blknr);
#endif

	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = CLASS;
	*pw++ = DEVICE;
	*pw++ = CO_WRITE;
	*pw++ = blknr;
	*pw++ = blk_cnt;
	len = 7;

	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
	dummy_c_read (dcb);
	
	len = BLK_SIZE*blk_cnt;	/* len in bytes */
	len = len/4;		/* len in words */
	ChanWrite (&dcb->data_out, (byte *)&len, 4);
	ChanWrite (&dcb->data_out, (byte *)buffer, len*4);

	wait_for_result (dcb);

	return ( dcb->ok ? blk_cnt : 0 );
}

/* ==================================================================== */

void 
msc_finish (DiscDCB *dcb)

{	
	word *pw, len;
						    
	pw = dcb->command;
	*pw++ = PID;
	*pw++ = PRI;
	*pw++ = CO_SPECIAL;
	*pw++ = CO_FINISH;
	len = 4;

	ChanWrite (&dcb->cmd_out, (byte *)&len, 4);
	ChanWrite (&dcb->cmd_out, (byte *)dcb->command, len*4);
	dummy_c_read (dcb);
	dummy_c_write (dcb);

	wait_for_result (dcb);
}

/* ==================================================================== */
/* ==================================================================== */

void 
dummy_c_read (DiscDCB *dcb)

/* Reads unexpected, senseless data from data channel */

{
	word len;
						      
#if DEBUG
printf ("d.rd"); fflush (stdout);
#endif
	ChanRead (&dcb->data_in, (byte *)&len, 4);
	if (len > 0)
		ChanRead (&dcb->data_in, (byte *)dcb->dummy_buffer, len*4);
#if DEBUG
printf (": "); fflush (stdout);
#endif
}

/* ==================================================================== */

void 
dummy_c_write (DiscDCB *dcb)				

/* Writes a Null-slice to the data channel */

{
	word len = 0;
#if DEBUG
printf ("d.wt"); fflush (stdout);
#endif
	ChanWrite (&dcb->data_out, (byte *)&len, 4);
#if DEBUG
printf (": "); fflush (stdout);
#endif
}

/* ==================================================================== */

void
wait_for_result (DiscDCB *dcb)

/* Expects an answer on command channel and set global 'ok' accordingly */

{	
	word len, i;

	ChanRead (&dcb->cmd_in, (byte *)&len, 4);
	if (len > 0)
		ChanRead (&dcb->cmd_in, (byte *)dcb->dummy_buffer, len*4);
#if DEBUG
printf ("result (%d): ", len);
for (i=0;i<len;i++)
    printf ("%x ", dcb->dummy_buffer[i]);
printf ("\n"); 
#endif
	dcb->ok = (dcb->dummy_buffer[1] == CO_OK);
	if (!dcb->ok)
	{
		IOdebug ("result (%d): ", (len-1));
		for (i=1;i<len;i+=4)
		    IOdebug ("%x %x %x %x", dcb->dummy_buffer[i],dcb->dummy_buffer[i+1],dcb->dummy_buffer[i+2],dcb->dummy_buffer[i+3]);
		IOdebug ("\n"); 
	}
}

/* ==================================================================== */

/* end of msc.c */

