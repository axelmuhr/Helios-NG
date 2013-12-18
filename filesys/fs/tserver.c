static char rcsid[] = "$Header: /usr/perihelion/Helios/filesys/fs/RCS/tserver.c,v 1.1 90/10/05 16:28:11 nick Exp $";

/* $Log:	tserver.c,v $
 * Revision 1.1  90/10/05  16:28:11  nick
 * Initial revision
 * 
 * Revision 1.2  90/02/23  11:24:05  chris
 * When tape open fails -> release sempahore
 * 
 * Revision 1.1  90/02/20  11:25:30  chris
 * Initial revision
 * 
 */
 
/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**             Copyright (C) 1990, Perihelion Software Ltd.             **
**                         All Rights Reserved.                         **
**                                                                      **
** tserver.c								**
**                                                                      **
**	Routines handling Tape Operations.				**
**									**
**************************************************************************
** HISTORY  :								**
** ----------								**
** Author   :  12/01/90  C.Selwyn 					**
*************************************************************************/

#include <root.h>
#include "nfs.h"

static void tdispatch(DispatchInfo *);
static void tworker (MsgBuf *m,DispatchInfo *info);
#define tworkerSS 10000

#define DEBUG 0
#define NAMES 0

NameInfo	TServerInfo =
		{
			NullPort,
			Flags_StripName,
			DefFileMatrix,
			(word *)NULL
		};

static void tdo_open(ServInfo *);
static void tdo_create(ServInfo *);
static void tdo_locate(ServInfo *);
static void tdo_objinfo(ServInfo *);
static void tdo_serverinfo(ServInfo *);
static void tdo_delete(ServInfo *);
static void tdo_rename(ServInfo *);
static void tdo_link(ServInfo *);
static void tdo_protect(ServInfo *);
static void tdo_setdate(ServInfo *);
static void tdo_refine(ServInfo *);
static void tdo_closeobj(ServInfo *);

static int tget_context(ServInfo *);

static void do_tape_read(MCB *m, struct incore_i *tapei);
static void do_tape_write(MCB *m, struct incore_i *tapei);
static void do_tape_seek(MCB *m, struct incore_i *tapei);
static word write_tape(int drive, char *buf, word size);

static DispatchInfo TServerDInfo = 
		{
			NULL,
			NullPort,
			SS_HardDisk,	/* Uh? */
			NULL,
			{
				tdo_open,
				tdo_create,
				tdo_locate,
				tdo_objinfo,
				tdo_serverinfo,
				tdo_delete,
				tdo_rename,
				tdo_link,
				tdo_protect,
				tdo_setdate,
				tdo_refine,
				tdo_closeobj
			}
		};

struct incore_i tapeinode;
char tape_name[32];
word tapedrive;
uword tapeposition = 0;
char *tapebuf = NULL;
word tapebufsize = 0;
word bufdata;
word tape_position;
#define imin(a,b) ((a)<(b)? (a):(b))

void
tserver(void)
{
	Object *tnte;

	memset(&tapeinode,0,sizeof(struct incore_i));
	InitSemaphore(&tapeinode.ii_sem,1);
	tapeinode.ii_i.i_mode = DATA;
	tapeinode.ii_i.i_size = 0;
	tapeinode.ii_i.i_atime = GetDate();
	tapeinode.ii_i.i_mtime = GetDate();
	tapeinode.ii_i.i_ctime = GetDate();
	tapeinode.ii_i.i_cryptkey = NewKey();
	tapeinode.ii_i.i_matrix = DefFileMatrix;
	strcpy(tapeinode.ii_name, tape_name);

	TServerDInfo.reqport = TServerInfo.Port = NewPort();

	{
		Object *o;
		char mcname[100];
		MachineName(mcname);
		o = Locate(NULL,mcname);
		tnte = Create(o,tape_name,Type_Name,sizeof(NameInfo),
				(byte *)&TServerInfo);
		if( tnte == NULL )
		{	IOdebug("Failed to create tape name - %x",Result2(o));
			return;
		}
		Close(o);
	}

	tdispatch(&TServerDInfo);
}

static void 
tdispatch (DispatchInfo *info)

/*
*  Receives the request messages to TapeServer at the Server Port,
*  creating 'fworker' processes to perform each request.
*/

{
	MsgBuf	*m;
	
#if 0
	syncop = FALSE;			/* Initially there are bdwrite- */
					/* operations			*/
#endif

	forever
	{
		/* create a buffer for messages */
		m = Malloc(sizeof(MsgBuf));
		if( m == Null(MsgBuf) ) 
		{ 
			Delay(OneSec*5); 
			continue; 
		}
		/* initialise message header */
		m->mcb.MsgHdr.Dest	= info->reqport;
		m->mcb.Timeout		= OneSec*30;
		m->mcb.Control		= m->control;
		m->mcb.Data		= m->data;
	lab1:
		/* get a request message */
		while( GetMsg(&(m->mcb)) == EK_Timeout );
#if DEBUG
printf("function	>0x%x<\n",m->mcb.MsgHdr.FnRc);
#endif

 		if( MyTask->Flags & Task_Flags_servlib )
 			IOdebug("HFS: tape: %F %s %s",m->mcb.MsgHdr.FnRc,
 				(m->control[0]==-1)?NULL:&m->data[m->control[0]],
 				(m->control[1]==-1)?NULL:&m->data[m->control[1]]);

		/* fork a worker process for each request */
		unless( Fork(tworkerSS, tworker, 8, m, info) )
		{
			/* send an exception if process couldn't be forked */
			ErrorMsg(&m->mcb,EC_Error+EG_NoMemory);
			/* get a new message */
			goto lab1;
		}

	}
}

static void
tworker (MsgBuf *m,DispatchInfo *info)

/*
*  Dynamically created process to deal with FileServer requests.
*/

{
	ServInfo servinfo;
	word fncode = m->mcb.MsgHdr.FnRc;
#if DEBUG
	IOCCommon *req = (IOCCommon *) (m->mcb.Control);
#endif
	
#if DEBUG
printf("	context [%d] >%s<\n",req->Context,
(req->Context>=0)?m->mcb.Data+req->Context:"-1");
printf("	name    [%d] >%s<\n",req->Name,
(req->Name>=0)?m->mcb.Data+req->Name:"-1");
printf("	next    [%d] >%s<\n",req->Next,
(req->Next>=0)?m->mcb.Data+req->Next:"-1");
#endif
	if( setjmp(servinfo.Escape) != 0 )
	{
		Free(m);
		return;
	}

	servinfo.m = &m->mcb;
	MachineName(servinfo.Context);
	pathcat(servinfo.Context,tape_name);
	servinfo.FnCode = fncode;
	m->mcb.MsgHdr.FnRc = info->subsys;	

	if( !tget_context (&servinfo) ) 
		ErrorMsg (&m->mcb,0);
	else	
	{
		word fn = fncode & FG_Mask;
		VoidFnPtr f;

#if NAMES
printf("fwCTXT : >%s<\n",servinfo.Context);
printf("fwPATH : >%s<\n",servinfo.Pathname);
printf("fwTRGT : >%s<\n",servinfo.Target);
#endif

		/* if request message is not valid */
		if( fn < FG_Open || fn > FG_CloseObj )
		{
			WordFnPtr f = info->PrivateProtocol;
			/* send exception message */
			if( (f==NULL) || (!f(&servinfo)) ) 
			{
				m->mcb.MsgHdr.FnRc = Err_Null;
				ErrorMsg(&m->mcb,EC_Error+info->subsys+EG_FnCode );
			}
		}
		/* else jump to the routine which deals this request */
		else 
		{
			f = info->fntab[(fn-FG_Open) >> FG_Shift];
			(*f)(&servinfo);
		}
	}
	Free( m );
}

static int tget_context(servinfo)
ServInfo *servinfo;
{
	MCB *m = servinfo->m;
	IOCCommon *req = (IOCCommon *)(m->Control);
	int context = req->Context;
	int next = req->Next;
	int name = req->Name;
	char *pathname = servinfo->Pathname;
	
	if( context == -1 || (name >0 && next >= name) )
	{
		req->Access.Access = UpdMask(req->Access.Access, 
						tapeinode.ii_i.i_matrix);
		return TRUE;
	}
	else
	{	if( !GetAccess(&req->Access,
					(Key)tapeinode.ii_i.i_cryptkey) )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Invalid+EO_Capability;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		if( req->Access.Access == 0 )
		{
			m->MsgHdr.FnRc |= EC_Error+EG_Protected;
			*(servinfo->Context) = '\0';
			return FALSE;
		}
		strcpy(pathname,servinfo->Context);
		return TRUE;
	}
}

static void tdo_open(servinfo)
ServInfo *servinfo;
{	
	MCB *m  = servinfo->m;
	IOCMsg2 *req = (IOCMsg2 *)m->Control;
	MsgBuf *r;
	Port reqport;
	char *data = m->Data;
	char *pathname = servinfo->Pathname;
	char closing = 0;

	Wait(&tapeinode.ii_sem);

	if( tapeinode.ii_count )
	{
		ErrorMsg(m,EC_Error+EG_InUse+EO_Stream);
		Signal(&tapeinode.ii_sem);
		return;
	}

	tapeinode.ii_count = 1;
	Signal(&tapeinode.ii_sem);

	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}

	IOCRep1(r,m,&tapeinode,Flags_Server+Flags_Closeable,pathname);
	reqport = NewPort();
	r->mcb.MsgHdr.Reply = reqport;
	PutMsg(&r->mcb);
	Free(r);
	
	while(!closing)
	{	word e;
	
		m->MsgHdr.Dest = reqport;
		m->Timeout = StreamTimeout;
		m->Data    = data;
		e = GetMsg(m);
		if( MyTask->Flags & Task_Flags_servlib )
		{
			if( e < Err_Null )
				IOdebug("HFS: tape: stream getmsg error %x",e);
			else
		 		if( MyTask->Flags & Task_Flags_servlib )
					IOdebug("HFS: tape: %F",m->MsgHdr.FnRc);
		}
		if( e == EK_Timeout ) break;
		if( e < Err_Null ) continue;

		switch(m->MsgHdr.FnRc & FG_Mask )
		{
		case FG_Read:
			do_tape_read(m,&tapeinode);
			break;
		case FG_Write:
			do_tape_write(m,&tapeinode);
			break;
		case FG_Close:
		{	word err;

			if( m->MsgHdr.Reply != NullPort ) 
				ErrorMsg(m,Err_Null);
			do_dev_seek(FG_Seek,tapedrive,-1,0);
			tape_position=0;
			if( tapebuf )
			{
				Free(tapebuf);
				tapebuf = NULL;
				tapebufsize = 0;
			}
			closing = 1;
			break;
		}
		case FG_SetSize:
		case FG_GetSize:
			InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
			MarshalWord(m,0);
			PutMsg(m);
			break;
		case FG_Seek:
			do_tape_seek(m,&tapeinode);
			break;
		}
	}
	Free(tapebuf);
	tapebuf = NULL;
	FreePort(reqport);
	tapeinode.ii_count = 0;
}
	
static void tdo_create(servinfo)
ServInfo *servinfo;
{	MsgBuf *r;
	MCB *m = servinfo->m;
	char *pathname = servinfo->Pathname;

	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}
	IOCRep1(r,m,&tapeinode,Flags_Server,pathname);
	PutMsg(&r->mcb);
	Free(r);
}

static void tdo_locate(servinfo)
ServInfo *servinfo;
{	MsgBuf *r;
	MCB *m = servinfo->m;
	char *pathname = servinfo->Pathname;
	
	r = New(MsgBuf);
	if( r == Null(MsgBuf) )
	{
		ErrorMsg(m,EC_Error+EG_NoMemory+EO_Message);
		return;
	}
	strcpy(pathname,servinfo->Context);
	IOCRep1(r,m,&tapeinode,Flags_Server,pathname);
	PutMsg(&r->mcb);
	Free(r);
}

static void tdo_objinfo(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;
	marshal_info(m,&tapeinode);
	PutMsg(m);
}

static void tdo_serverinfo(servinfo)
ServInfo *servinfo;
{	word d = Flags_Server;
	word z = 0;
	MCB *m = servinfo->m;
	
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);
	MarshalData(m,4,(byte *)&d);
	MarshalData(m,4,(byte *)&z);
	MarshalData(m,4,(byte *)&z);
	MarshalData(m,4,(byte *)&z);
	PutMsg(m);
}

static void tdo_delete(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_Delete+EO_File);
}

static void tdo_rename(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
	
}

static void tdo_link(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
}

static void tdo_protect(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
}

static void tdo_setdate(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
}

static void tdo_refine(servinfo)
ServInfo *servinfo;
{	MCB *m = servinfo->m;

	ErrorMsg(m,EC_Error+EG_WrongFn+EO_File);
}

static void tdo_closeobj(servinfo)
ServInfo *servinfo;
{	IOdebug("  do_closeobj: tape: function not implemented");
	NullFn(servinfo);
}

/************************************************
*						*
*	Real Tape access starts here		*
*						*
************************************************/

int ensure_tape_buffer(word reqsize)
{
	if( tapebufsize >= reqsize ) return 1;
	if( tapebuf ) Free(tapebuf);

	tapebuf = Malloc(reqsize);
	if( tapebuf )
	{
		tapebufsize  = reqsize;
		return 1;
	}
	else
		return 0;
}

static word read_tape(int drive, char *buf, word size)
{	DiscReq req;
	word r;
	word err;

	r = do_dev_rdwr(FG_Read,drive,-1,size,tape_position,(char *)buf,&err);
	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: read %d bytes to tape file %d",r,drive);
	tape_position += r;
	return r;
}

static void do_tape_read(MCB *m, struct incore_i *tapei)
{	ReadWrite *req = (ReadWrite *)m->Control;
	uword byte_offset = req->Pos;
	word fn = m->MsgHdr.FnRc;
	Port request = m->MsgHdr.Dest;
	Port reply = m->MsgHdr.Reply;
	word reqsize = req->Size;
	word dataread;
	word sent = 0;
	word firstsend = 1;
	
	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: read %d bytes",reqsize);

	m->MsgHdr.FnRc = 0;
	if( req->Pos != tape_position )
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

/* Ensure that I can fit all of the request into my current buffer */
	if( !ensure_tape_buffer(reqsize) )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Message);
		return;
	}
	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More);
	
	dataread = read_tape(tapedrive, tapebuf, reqsize);
		
	while( sent != dataread || firstsend )
	{	word thissend = imin(dataread-sent,0xffff);
		word seq = 0;
		word e;
		firstsend = 0;
		m->Data = tapebuf+sent;
		m->MsgHdr.DataSize = thissend;

		if( (thissend + sent) == dataread )
		{	m->MsgHdr.Flags = 0;	/* No preserve */
			m->MsgHdr.FnRc  = ReadRc_EOD+seq;
		}
		else
			m->MsgHdr.FnRc  = ReadRc_More+seq;

		if( MyTask->Flags & Task_Flags_servlib )
			IOdebug("TAPE: read returning %d bytes, rc = %x, ",
			           thissend,m->MsgHdr.FnRc);

		e = PutMsg(m);
		seq  += ReadRc_SeqInc;
		sent += thissend;
	}
}

static word write_tape(int drive, char *buf, word size)
{	DiscReq req;
	word r;
	word err;

	r = do_dev_rdwr(FG_Write,drive,-1,size,tape_position,(char *)buf,&err);
	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: written %d bytes to tape file %d",r,drive);
	tape_position += r;
	return r;
}

static void do_tape_write(MCB *m, struct incore_i *tapei)
{	ReadWrite *req = (ReadWrite *)m->Control;
	uword byte_offset = req->Pos;
	word fn = m->MsgHdr.FnRc;
	Port request = m->MsgHdr.Dest;
	Port reply = m->MsgHdr.Reply;
	word reqsize = req->Size;
	word written = 0;
	word msgdata = m->MsgHdr.DataSize;

	m->MsgHdr.FnRc = 0;

	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: req->Pos (= %d), req->Size = %d",req->Pos,req->Size);

	if( req->Pos != tape_position )
	{
		if( MyTask->Flags & Task_Flags_servlib )
			IOdebug("req->Pos != tape_position (= %d)",tape_position);
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		return;
	}

	if( !ensure_tape_buffer(reqsize) )
	{
		ErrorMsg(m,EC_Error|EG_NoMemory|EO_Message);
		return;
	}

	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,WriteRc_Sizes);

	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: requested to write %d bytes",reqsize);

	if( msgdata == 0 )
	{	word e;
		word gotdata = 0;
		MarshalWord(m,imin(0xffff,reqsize));
		MarshalWord(m,0xffff);
		e = PutMsg(m);
		m->MsgHdr.Dest = request;
		m->Timeout = WriteTimeout;
		while( gotdata != reqsize )
		{
			m->Data = tapebuf+gotdata;
			e = GetMsg(m);
			gotdata += m->MsgHdr.DataSize;
		}
		written = write_tape(tapedrive,tapebuf,reqsize);
	}
	else	/* No copies here ! */
		written = write_tape(tapedrive,m->Data,reqsize);

	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: written %d bytes",written);

	InitMCB(m,0,reply,NullPort,WriteRc_Done);
	MarshalWord(m,written);

	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("TAPE: write reply = %M",m);

	PutMsg(m);
}

static void do_tape_seek(MCB *m, struct incore_i *tapei)
{	SeekRequest *req = (SeekRequest *)m->Control;
	word newpos;
	Port reply = m->MsgHdr.Reply;
	word err=Err_Null;

	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("tape_seek: mode %d, newpos = %d",req->Mode,req->NewPos);
	m->MsgHdr.FnRc = 0;
	newpos = do_dev_seek(FG_Seek,tapedrive,-1,req->NewPos);
	if( MyTask->Flags & Task_Flags_servlib )
		IOdebug("tape_seek: newpos = %x",newpos);
	if( newpos < 0 ) err = newpos, newpos = -1;
	
	InitMCB(m,0,reply,NullPort,err);
	MarshalWord(m,newpos);
	PutMsg(m);
}
