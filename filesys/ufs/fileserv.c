/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 7 August 1992                                                */
/* File: fileserv.c                                                   */
/*                                                                    */
/*
 * This file contains the open file server and its service routines.
 * Other service routines for open directories may be found in dirserv.c 
 * with the main service routines in services.c
 */
/*
 * $Id: fileserv.c,v 1.2 1992/11/24 11:23:58 al Exp $ 
 * $Log: fileserv.c,v $
 * Revision 1.2  1992/11/24  11:23:58  al
 * Added support for flock
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#include "param.h"
#include "malloc.h"
#include "buf.h"
#include "filedesc.h"
#include "file.h"
#include "user.h"
#include "stat.h"
#include "proc.h"
#include "mount.h"
#include "vnode.h"
#include "fcntl.h"
#include "ioctl.h"
#include "disklabel.h"

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>
#include <dirent.h>

#include "dispatch.h"
#include "private.h"

/*
 * Prototypes for HELIOS->UNIX system call routines (direct into kernel).
 */
extern sysvnstat();

/*
 * Prototypes for UNIX system call routines.
 */
extern read(), write(), lseek(), ftruncate(), fstat(), close(), ioctl(),
       flock();

/* Reading from a file */
static void do_read(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	byte *oldbuffer = mcb->Data;
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word pos = rw->Pos;
	word size = rw->Size;
	Port replyport = mcb->MsgHdr.Reply;
	struct rdwr_args *args = &ar->rdwr_args;
	word sent = 0;
	word seq = 0;
	word send, e;
	int error, retval;

#ifdef DEBUG
printf("do_read; Request for %d bytes\n",size);
#endif
	/* This seek is absolute.  Subsequent reads (below) need no
	 * seeks as the pointer is advanced automatically anyway.
	 * - WARNING: Pipe check ? 
	 */
	ar->fp->f_offset = pos;

	/* Read up to end of block limit */	
	send = MAXBSIZE - (pos % MAXBSIZE);
	if (send > size) send = size;

	while (send)  {
		/* Prevent OverFlow */
		if (send > MAXBSIZE) send = MAXBSIZE;
	
		/* Setup for call */
		args->fd = ar->handle;
		args->buf = (char *)&ar->buf;
		args->count = send;
		error = syscall(ar->client,read,
				(struct syscall_args *)args,&retval);
		if (error) {
			mcb->Data = oldbuffer;
			ErrorMsg(mcb,EC_Error|EG_Errno|error);
			return;
		}

		/* Add data read */
		sent += retval;
		pos += retval;

		/* Reply */
		InitMCB(mcb,0,replyport,NullPort,seq);
		if (send != retval) {
			/* EOF Reached prematurely */
			mcb->MsgHdr.FnRc |= ReadRc_EOF;
			sent = size;	/* Force exit condition */

#ifdef DEBUG
printf("do_read: EOF; read=%d   wanted=%d  flags=0x%x\n",retval,send,mcb->MsgHdr.Flags);
#endif
		} else if (sent == size) {
			mcb->MsgHdr.FnRc |= ReadRc_EOD;

#ifdef DEBUG
printf("do_read: EOD; read=%d   wanted=%d  flags=0x%x\n",retval,send,mcb->MsgHdr.Flags);
#endif

		} else {	/* More Data Will Follow */
			mcb->MsgHdr.Flags |= MsgHdr_Flags_preserve;

#ifdef DEBUG
printf("do_read: MOR; read=%d   wanted=%d  flags=0x%x\n",retval,send,mcb->MsgHdr.Flags);
#endif

		}
		mcb->MsgHdr.DataSize = retval;
		mcb->Data = (byte *)&ar->buf;

		if ((e = PutMsg(mcb)) < Err_Null) {
			ClientState *cs = ar->client;

			printf("File '%s':%d Read; client %d disappeared.  Fault 0x%x\n",
				cs->ofname, cs->p.p_pid,
				cs->p.p_ucred->cr_uid, e);
#ifdef DEBUG
printf("do_read: PutMsg returned 0x%x  Port=0x%x  Actual=0x%x\n",
	e,replyport,mcb->MsgHdr.Dest);
#endif
		}
		
		/* Keep the ball rolling */
		send = size - sent;
		seq += ReadRc_SeqInc;
	}

	/* Restore the old buffer */
	mcb->Data = oldbuffer;
}

/* Writing to a file */
static void do_write(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	byte *oldbuffer = mcb->Data;
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word pos = rw->Pos;
	word size = rw->Size;
	Port replyport = mcb->MsgHdr.Reply;
	Port dataport = NewPort();
	struct rdwr_args *args = &ar->rdwr_args;
	word e, got, seq;
	word wrt;
	int error, retval;

#ifdef DEBUG
printf("do_write; Request for %d bytes\n",size);
#endif

	/* Send back 1st reply */
	InitMCB(mcb,MsgHdr_Flags_preserve,replyport,dataport,WriteRc_Sizes);
	MarshalWord(mcb,MAXBSIZE);	
	MarshalWord(mcb,MAXBSIZE);	
	e = PutMsg(mcb);
	
	InitMCB(mcb,0,dataport,NullPort,0);
	mcb->Data = (byte *)&ar->buf;
	e = got = seq = 0;
	
	/* This seek is absolute.  Subsequent reads (below) need no
	 * seeks as the pointer is advanced automatically anyway.
	 * - WARNING: Pipe check ? 
	 */
	ar->fp->f_offset = pos;

	while (got < size) {
		e = GetMsg(mcb);
		if (e < Err_Null) break;
		if ((e & ~ReadRc_Mask) != seq) {
			e = EC_Warn|SS_HardDisk;
			break;
		}
		
		wrt = mcb->MsgHdr.DataSize;

		/* Setup for call */
		args->fd = ar->handle;
		args->buf = (char *)&ar->buf;
		args->count = wrt;
		error = syscall(ar->client,write,
				(struct syscall_args *)args,&retval);
		if (error) {
			mcb->Data = oldbuffer;
			mcb->MsgHdr.Reply = replyport;
#ifdef DEBUG
printf("do_write error %d on write\n",error);
#endif
			ErrorMsg(mcb,EC_Error|EG_Errno|error);
			return;
		}

		/* Check (superfluous I hope) */
		if (retval != wrt) 
			printf("do_write failed. Wrote %d when wanted to write %d\n",
				retval,wrt);
			
		/* Update for Next Write */
		got += retval;
		seq += ReadRc_SeqInc;
	}
		
	FreePort(dataport);
	
	InitMCB(mcb,0,replyport,NullPort,e<0?e:WriteRc_Done);
	MarshalWord(mcb,got);
	e = PutMsg(mcb);

	/* Restore Data */
	mcb->Data = oldbuffer;
}

/* Close a file (calls the close routine). */
static void do_close(MyMCB *mymcb, struct ufsrw *ar)
{
	struct fd_args fd_args;
	int error, retval;
	
	fd_args.fd = ar->handle;
	error = syscall(ar->client,close,(struct syscall_args *)&fd_args,
			&retval);
}

/* Wrong Function */
static void do_wrong(MyMCB *mymcb)
{
	MCB *mcb = &mymcb->mcb;
	ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_File);
}

/* Seek to a location */
static void do_seek(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	SeekRequest *req = (SeekRequest *)mcb->Control;
	struct lseek_args lseek_args;
	int error, retval;

	lseek_args.fd = ar->handle;
	lseek_args.off = req->NewPos;
	switch (req->Mode)
	{
	case S_Beginning:	lseek_args.sbase = L_SET;	break;
	case S_Relative:	lseek_args.off += req->CurPos;
				lseek_args.sbase = L_SET; 	break;
	case S_End:		lseek_args.sbase = L_XTND;	break;
	}

	if (error = syscall(ar->client,lseek,(struct syscall_args *)&lseek_args,
			&retval)) {
		ErrorMsg(mcb,EC_Error|EG_Errno|error);
		return;
	}
	
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	MarshalWord(mcb,lseek_args.off);
	PutMsg(mcb);
}

/* Get the file size */
static void do_getsize(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	struct stat stat;
	struct fstat_args fstat_args;
	int retval, error;

	fstat_args.fd = ar->handle;
	fstat_args.buf = &stat;	
	if (error = syscall(ar->client,fstat,
			    (struct syscall_args *)&fstat_args,&retval)) {
		ErrorMsg(mcb,EC_Error|EG_Errno|error);
		return;
	}
	
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	MarshalWord(mcb,stat.st_size);
	PutMsg(mcb);	
}

/* Set a file size (truncate) */
static void do_setsize(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	struct ftrunc_args ftrunc_args;
	word *size = mcb->Control;
	int error, retval;
		
	ftrunc_args.fd = ar->handle;
	ftrunc_args.length = *size;
	if (error = syscall(ar->client,ftruncate,
			(struct syscall_args *)&ftrunc_args,&retval)) {
		ErrorMsg(mcb,EC_Error|EG_Errno|error);
		return;
	}
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	PutMsg(mcb);	
}

/*
 * Perform a UNIX style IOCTL call to read/write data.
 */
static void do_getsetinfo(MyMCB *mymcb, struct ufsrw *ar, word e)
{
	MCB *mcb = &mymcb->mcb;
	int cmd = (int)mcb->Control[0];	/* The IOCTL code */
	int off = (int)mcb->Control[1];	/* Offset to any data */
	struct ioctl_args ioctl_args;
	int retval, error, size;

	if ((e & FF_Mask) == FF_Ioctl) {
		/* Get the size of the data transferral */
		size = IOCPARM_LEN(cmd);

		/* Perform the UNIX style IOCTL */
		ioctl_args.fdes = ar->handle;
		ioctl_args.cmd = cmd;
		if (cmd & IOC_OUT)
			ioctl_args.cmarg = (caddr_t)mcb->Data;
		else	ioctl_args.cmarg = (caddr_t)&(mcb->Data[off]);
#ifdef DEBUG
printf("do_getsetinfo: About to ioctl on file %s  cmd 0x%x\n",
	ar->client->ofname,cmd);
#endif
		if (error = syscall(ar->client,ioctl,
		    (struct syscall_args *)&ioctl_args,&retval)) {
			ErrorMsg(mcb,EC_Error|EG_Errno|error);
			return;
		}
#ifdef DEBUG
printf("do_getsetinfo: File %s error=%d retval=%d\n",
	ar->client->ofname,error,retval);
{
struct disklabel *dl = (struct disklabel *)mcb->Data;
printf("do_getsetioctl: dev_bsize = %d\n",dl->d_secsize);
}
#endif
		/* Send the result and data (if any) */
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
		if ((cmd & IOC_OUT) && size) {
			/* This was a GetInfo ioctl */
			MarshalWord(mcb,cmd);
			MarshalWord(mcb,0);	/* MarshalOffset(mcb); */
			mcb->MsgHdr.DataSize = size;
		}
		PutMsg(mcb);	
	} else {
		do_wrong(mymcb);
	}
}

/*
 * Perform a UNIX flock on the specified file.
 */
static void do_flock(MyMCB *mymcb, struct ufsrw *ar)
{
	MCB *mcb = &mymcb->mcb;
	int cmd = (int)mcb->Control[0];	/* The flock command code */
	struct flock_args flock_args;
	int retval, error;

	/* Perform the UNIX flock on the file */
	flock_args.fdes = ar->handle;
	flock_args.cmd = cmd;

#ifdef DEBUG
printf("do_flock: About to flock on file %s  cmd 0x%x\n",
	ar->client->ofname,cmd);
#endif
	if (error = syscall(ar->client,flock,
	    (struct syscall_args *)&flock_args,&retval)) {
#ifdef DEBUG
printf("do_flock: flock error on file %s  cmd 0x%x  error=%d\n",
	ar->client->ofname,cmd,error);
#endif
		ErrorMsg(mcb,EC_Error|EG_Errno|error);
		return;
	}

	/* Send the result and data (if any) */
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
	PutMsg(mcb);	
}

/*
 * The actual file server server.
 */
void file_server(MyMCB *mymcb, ClientState *client, word handle)
{
	MCB *mcb = &mymcb->mcb;
	Port reqport = mcb->MsgHdr.Reply;
	struct ufsrw *rw = (struct ufsrw *)malloc(sizeof(struct ufsrw),M_TEMP,M_WAITOK);
	word oldtimeout = mcb->Timeout;
	word totaltimeout = 0;
	word e;

	if (rw == NULL) {
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_File);
		printf("file_server panic; No memory left to service requests.\n");
		return;
	}
	
	/* Setup Credentials */
	rw->client = client;
	rw->handle = handle;
	rw->fp = client->p.p_fd->fd_ofiles[handle];	/* Quick Reference */
	
	/* Process Messages */
	while (!client->force_close) {
		InitMCB(mcb,0,reqport,NullPort,0);
		mcb->Timeout = IdleTimeout;	/* Idle, check for force close */
		e = GetMsg(mcb);
		mcb->Timeout = oldtimeout;	/* Restore */
		if (e < 0) {
		    if ((e & EG_Mask) == EG_Timeout) {
			totaltimeout += IdleTimeout;
			if (totaltimeout >= StreamTimeout) {
			  printf("Timeout close of file %s (%d) for user %d\n",
					client->ofname,client->p.p_pid,
					client->p.p_cred->p_ruid);
			  do_close(mymcb,rw);
			  goto done;
			}
		    }
#ifdef DEBUG
printf("file_server: serving open file %s for 0x%x\n",client->ofname,client);
#endif

		    continue;
		}

		mcb->MsgHdr.FnRc = SS_HardDisk;
		totaltimeout = 0;	/* Reset since valid message */
#ifdef DEBUG
printf("file_server 0x%x  file %s\n",e,client->ofname);
#endif
		switch (e & FG_Mask)
		{
		case FG_Read:      do_read(mymcb,rw);		break;
		case FG_Write:     do_write(mymcb,rw);		break;
		case FG_Seek:      do_seek(mymcb,rw);		break;
		case FG_GetInfo:   do_getsetinfo(mymcb,rw,e);	break;
		case FG_SetInfo:   do_getsetinfo(mymcb,rw,e);	break;
		case FG_GetSize:   do_getsize(mymcb,rw);	break;
		case FG_SetSize:   do_setsize(mymcb,rw);	break;
		case FG_Select:
			/* a select will always succeed immediately */
			ErrorMsg(mcb,e&Flags_Mode);
			break;
		case FG_Close:     do_close(mymcb,rw);	goto done;

		/* BSD Specific calls */
		case FG_UfsFlock:  do_flock(mymcb,rw);		break;

		default:		
			ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_File);
			break;
		}
	}	

	/* File was forcabily closed, so actually close it. */
	printf("Forcing close of file %s (%d) for user %d\n",
		client->ofname,client->p.p_pid,	client->p.p_cred->p_ruid);
	do_close(mymcb,rw);

done:
	/* Free Buffers and return */
	free(rw,M_TEMP);
	return;
}



