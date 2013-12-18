#include "nfs.h"
#include <sys/time.h>

static struct timeval TIMEOUT = { 25, 0 };

/* #define MAXDATA		2048 */
#define MAXDATA		1024

typedef struct nfsrw
{
	union
	{
		readargs	r;
		writeargs	w;
	} arg;				/* args union	*/
	union
	{
		readres		r;
		attrstat	w;
	} res;				/* result union */
	word	bufsize;		/* qty of data in buffer 	*/
	word	bufpos;			/* current file pos of buffer	*/
	char	buf[MAXDATA];		/* data buffer			*/
} nfsrw;

static void do_read(MCB *mcb, nfshandle *h, nfsrw *ar)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word pos = rw->Pos;
	word size = rw->Size;
	fattr *attr = &h->Attr;
	readargs *args = &ar->arg.r;
	word rdsize;
	word sent = 0;
	word seq = 0;
	Port replyport = mcb->MsgHdr.Reply;
	word e;
				
	if( pos == attr->size )
	{
		InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,ReadRc_EOF);
		PutMsg(mcb);
		return;
	}

	if( pos < 0 || pos > attr->size )
	{
		ErrorMsg(mcb,EC_Error|EG_Parameter|1);
		return;
	}

	if( pos+size > attr->size ) size = attr->size - pos;

	while( sent < size )
	{
		/* first see if we can satisfy the read from the buffer	*/
		if( 	ar->bufsize > 0 && 
			pos >= ar->bufpos && 
			pos < ar->bufpos+ar->bufsize )
		{
			word dsize = size-sent;
			if( pos+dsize > ar->bufpos+ar->bufsize )
				dsize = ar->bufpos+ar->bufsize - pos;

			InitMCB(mcb,0,replyport,NullPort,seq);
			
			if( sent+dsize == size ) mcb->MsgHdr.FnRc |= ReadRc_EOD;
			else mcb->MsgHdr.Flags |= MsgHdr_Flags_preserve;
			
			mcb->MsgHdr.DataSize = dsize;
			mcb->Data = ar->buf+pos-ar->bufpos;
			
			e = PutMsg(mcb);
			
			sent += dsize;
			pos += dsize;
			seq += ReadRc_SeqInc;

			/* if the buffer has been emptied, read some more */
			/* otherwise finish				  */
			if( pos != ar->bufpos+ar->bufsize ) break;
		}
	
		rdsize = MAXDATA;
		if( pos+rdsize > attr->size ) rdsize = attr->size - pos;
		if( rdsize == 0 ) break;
		
		args->file = h->File;
		args->offset = pos;
		args->count = rdsize;
	
		ar->res.r.readres_u.reply.data.data_len = MAXDATA;
		ar->res.r.readres_u.reply.data.data_val = ar->buf;

		Wait(&nfslock);
		while(clnt_call(nfsclnt, NFSPROC_READ, xdr_readargs, args, xdr_readres, &ar->res.r, TIMEOUT ) != RPC_SUCCESS )
		{
			clnt_perror(nfsclnt, "read");
			args->count /= 2;
		}
		Signal(&nfslock);

		if( ar->res.r.status != NFS_OK )
		{
			ErrorMsg(mcb,EC_Error|EG_Errno|nfs_errno(ar->res.r.status));
			return;
		}

		h->Attr = ar->res.r.readres_u.reply.attributes;
	
		ar->bufpos = pos;
		ar->bufsize = ar->res.r.readres_u.reply.data.data_len;
	}
}

static void do_write(MCB *mcb, nfshandle *h, nfsrw *ar)
{
	ReadWrite *rw = (ReadWrite *)mcb->Control;
	word pos = rw->Pos;
	word size = rw->Size;
	fattr *attr = &h->Attr;
	writeargs *args = &ar->arg.w;
	Port dataport = NewPort();
	Port replyport = mcb->MsgHdr.Reply;
	byte *data = mcb->Data;
	word idata = mcb->MsgHdr.DataSize;
	word got;
	word e;
	word seq;

	args->file = h->File;

	/* send back first reply */
	
	InitMCB(mcb,MsgHdr_Flags_preserve,replyport,dataport,WriteRc_Sizes);
	MarshalWord(mcb,MAXDATA);
	MarshalWord(mcb,MAXDATA);
	
	e = PutMsg(mcb);

	InitMCB(mcb,0,dataport,NullPort,0);
	mcb->Data = ar->buf;
	
	e = 0;
	got = 0;	
	seq = 0;
	
	while( got < size )
	{
		word tfr;
		
		e = GetMsg(mcb);

		if( e < Err_Null ) break;
		
		if( (e & ~ReadRc_Mask) != seq )
		{
			e = EC_Warn|SS_HardDisk;
			break;
		}
		
		tfr = mcb->MsgHdr.DataSize;
		
		args->offset = pos;
		args->data.data_len = tfr;
		args->data.data_val = mcb->Data;
		
		Wait(&nfslock);
		while(clnt_call(nfsclnt, NFSPROC_WRITE, xdr_writeargs, args, xdr_attrstat, &ar->res.w, TIMEOUT ) != RPC_SUCCESS )
		{
			clnt_perror(nfsclnt, "write");
		}
		Signal(&nfslock);

		if( ar->res.w.status != NFS_OK )
		{
			if( DEBUG ) IOdebug("NFS: write error %d",ar->res.w.status);
			e = EC_Error|EG_Errno|nfs_errno(ar->res.r.status);
			break;
		}

		h->Attr = ar->res.w.attrstat_u.attributes;
		
		pos += tfr;
		got += tfr;
		seq += ReadRc_SeqInc;
	}

	FreePort(dataport);
	
	InitMCB(mcb,0,replyport,NullPort,e<0?e:WriteRc_Done);
	
	MarshalWord(mcb,got);
	
	e = PutMsg(mcb);

	mcb->Data = data;	
}

static void do_seek(MCB *m, nfshandle *h, nfsrw *ar)
{
	SeekRequest *req = (SeekRequest *)m->Control;
	word curpos = req->CurPos;
	word mode = req->Mode;
	word newpos = req->NewPos;

	switch( mode )
	{
	case S_Beginning:	break;
	case S_Relative:	newpos += curpos; break;
	case S_End:		newpos += h->Attr.size; break;
	}

	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	MarshalWord(m,newpos);

	ar->bufpos = newpos;
	ar->bufsize = 0;
	
	PutMsg(m);
}

static void do_getinfo(MCB *mcb, nfshandle *h)
{
	ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_File);
}

static void do_setinfo(MCB *mcb, nfshandle *h)
{
	ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_File);
}

static void do_getsize(MCB *mcb, nfshandle *h)
{
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	MarshalWord(mcb,h->Attr.size);
	PutMsg(mcb);
}

static void do_setsize(MCB *mcb, nfshandle *h)
{
	ErrorMsg(mcb,EC_Error|EG_WrongFn|EO_File);
}

extern void file_server(MCB *mcb, nfshandle *h)
{
	Port reqport = mcb->MsgHdr.Reply;
	nfsrw *ar = Malloc(sizeof(nfsrw));
	
	if( ar == NULL )
	{
		FreePort(reqport);
		return;
	}

	ar->bufsize = 0;
	ar->bufpos = 0;
	
	for(;;)
	{
		word e;
		InitMCB(mcb,0,reqport,NullPort,0);
		
		e = GetMsg(mcb);
		
		if( e < 0 ) continue;

		mcb->MsgHdr.FnRc = SS_HardDisk;
	
		switch( e & FG_Mask )
		{
		case FG_Read:		do_read(mcb,h,ar);	break;
		case FG_Write:		do_write(mcb,h,ar);	break;
		case FG_Seek:		do_seek(mcb,h,ar);	break;
		case FG_GetInfo:	do_getinfo(mcb,h);	break;
		case FG_SetInfo:	do_setinfo(mcb,h);	break;
		case FG_GetSize:	do_getsize(mcb,h);	break;
		case FG_SetSize:	do_setsize(mcb,h);	break;
		case FG_Select:
			/* a select will always succeed immediately */
			ErrorMsg(mcb,e&Flags_Mode);
			break;
		case FG_Close:		goto done;
		}	
	}
done:
	Free(ar);
	return;
}
