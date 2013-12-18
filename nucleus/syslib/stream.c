/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/stream.c							--
--                                                                      --
--	Stream operation.						--
--									--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: stream.c,v 1.25 1992/09/04 10:31:46 nickc Exp $ */

#define _in_stream

#include "sys.h"

static word _Seek(Stream *stream, WORD mode, WORD pos);

/*--------------------------------------------------------
-- ReOpen						--
--							--
-- Reopen a stream which has been timed out by the	--
-- the server.						--
--							--
--------------------------------------------------------*/

extern word ReOpen(Stream *stream)
{
	word rc = Err_Null;
	MCB *mcb;
	IOCReply1 *rep;
	Port reply;
	word mode = stream->Flags & Flags_SaveMode;

#ifdef SYSDEB
	SysDebug(stream)("ReOpen(%S)",stream);
#endif
	if( (rc=CheckStream(stream,0)) != Err_Null ) return rc;
	
	/* Re-allocate reply port. This avoids confusion and allows for */
	/* very long intervals during which the reply port may have been*/
	/* garbage collected.						*/
	
	rc = FreePort(stream->Reply);
	reply = stream->Reply = NewPort();

#if 0	
	if( (stream->Type != Type_Pseudo) && (stream->Flags & Flags_NoReOpen) )
	{
		/* On a no-reopen stream, try and re-allocate the reply	*/
		/* port before failing totally				*/
		if( rc >= Err_Null )
		{
			rc = EC_Error|SS_SysLib|EG_Broken|EO_Stream;
			stream->Result2 = rc;
			return rc;
		}
		else return Err_Null;
	}
#endif
	
	mcb = NewMsgBuf(0);
	rep = (IOCReply1 *)mcb->Control;
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Open|stream->FnMod);

	/* Flag that this is a ReOpen except when its a Pseudo Stream */
	if( stream->Type != Type_Pseudo ) mcb->MsgHdr.FnRc |= FF_ReOpen;
	
	MarshalString(mcb,stream->Name);
	MarshalWord(mcb,-1);
	MarshalWord(mcb,1);
	MarshalCap(mcb,&stream->Access);
	MarshalWord(mcb,stream->Flags & Flags_SaveMode);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null )
	  {
	    goto Done;
	  }
	

	if( SetupStream( stream, mcb ) ) stream->Flags |= mode;
			
	rc = Err_Null;

	if( mode & Flags_Append ) _Seek(stream, S_End, 0);
    Done:

#ifdef SYSDEB
	SysDebug(stream)("ReOpen: %S %E",stream,rc);
#endif

	FreeMsgBuf(mcb);

	stream->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Read							--
--							--
-- Read a buffer full of data from the stream.		--
-- 							--
--------------------------------------------------------*/

PUBLIC word Read(Stream *s, byte *buf, word size, word timeout)
{
	typedef struct { MCB mcb; ReadWrite rw; } Req;
	Req r;
	Port server;
	Port reply;
	word e = 0;
	word res = 0;
	word seq;
	word reqtime;
	bool extended = false;
	
	extern word PipeReOpen(Stream *s);
	WordFnPtr reopen = ReOpen;
	
#ifdef SYSDEB
	SysDebug(stream)("Read(%S,%P,%d,%d) from %s",s,buf,size,timeout,procname(returnlink_(s)));
#endif

	if ( (e = CheckStream(s,C_ReOpen)) != Err_Null ) return e;

	if( s->Type == Type_Pipe ) reopen = PipeReOpen;
			
	r.mcb.Control = (word *)&r.rw;
	
	Wait( &s->Mutex );
	
retry:
	if( (e = FindPorts(s,FG_Read,&server,&reply)) != Err_Null) goto done;

	if( server == NullPort ) { e = ReadRc_EOF; goto done; }
		
	extended = s->Flags & Flags_Extended;
	
	res = 0;
	
	InitMCB(&r.mcb,MsgHdr_Flags_preserve,server,reply,FC_GSP|FG_Read|s->FnMod);
	
	reqtime = (timeout != -1) && (timeout < IdleTimeout) ? timeout : IdleTimeout;
	
	MarshalWord(&r.mcb,s->Pos);
	MarshalWord(&r.mcb,size);
	MarshalWord(&r.mcb,reqtime);

	if( (e = PutMsg( &r.mcb ) ) != Err_Null ) 
	{
		if( (e & EC_Mask) >= EC_Error ) goto done;
		if( ((e & EC_Mask) == EC_Warn) && (e = reopen(s)) != Err_Null ) goto done;
		goto retry;
	}

	r.mcb.MsgHdr.Dest = reply;
	r.mcb.MsgHdr.Reply = NullPort;
	r.mcb.Data	= buf;
	r.mcb.Timeout	= s->Timeout+reqtime;

	seq = 0;

	do
	{
		word dsize;
		word csize;

		e = GetMsg(&r.mcb);

		if( r.mcb.MsgHdr.Reply != NullPort )
		{
#ifdef SYSDEB
			_SysDebug("Unexpected reply port in read %M",&r.mcb);
#endif
			FreePort(r.mcb.MsgHdr.Reply);
			r.mcb.MsgHdr.Reply = NullPort;
		}

		if ( e < Err_Null )
		{
			if( ((e&EG_Mask)==EG_Timeout) && (timeout != -1) )
			{
				timeout -= reqtime;
				if( timeout <= 0 ) goto done;
			}
#ifdef SYSDEB
			SysDebug(error)("Read Data Error %F on %S",e,s);
#endif
			if( (e & EC_Mask) >= EC_Error ) goto done1;
			if( ((e & EC_Mask) == EC_Warn) && (e = reopen(s)) != Err_Null ) goto done;
			goto retry;
		}

		if( (e & ~ReadRc_Mask) != seq )
		{ 
#ifdef SYSDEB
			SysDebug(error)("Read sequence error e %d seq %d",e,seq);
#endif
			FreePort(reply);
			s->Reply = reply = NewPort();
			e = EC_Warn;
			goto done;
		}

		/* we have a message */
		dsize = r.mcb.MsgHdr.DataSize;
		csize = r.mcb.MsgHdr.ContSize;

#ifdef SYSDEB
		if( (dsize+4*csize) > (size-res) ) IOdebug("Read: data > buffer dsize %d, csize %d, size %d, res %d (now %d > toget %d)",dsize,csize,size,res, dsize+4*csize, size-res);
#endif
		/* we also allow some data to be sent in the control vector */
		/* this usually only happens when we are being sent a full  */
		/* 64k.							    */
		if ( csize > 0 )
		{
			memcpy(	&buf[res+dsize],
				(byte *)(r.mcb.Control),
				(int)csize*sizeof(word) );
			dsize += csize*sizeof(word);
		}

		res       += dsize;

		r.mcb.Data = &buf[res];

		seq       += ReadRc_SeqInc;

	} while( (e & ReadRc_Mask) == ReadRc_More );

done:
	if( extended )
	{
		word err = e;

		
		InitMCB(&r.mcb,MsgHdr_Flags_preserve,server,reply,FC_GSP|FG_ReadAck|s->FnMod);
		
		r.mcb.Timeout         = s->Timeout * 2;
		r.mcb.Control[0]      = err < 0 ? err : res;
		r.mcb.MsgHdr.ContSize = 1;
		
		e = XchMsg(&r.mcb,0);
		
		if( ((word)(err & EC_Mask) <= (word)EC_Warn) && (timeout > 0) )
			goto retry;
		
		e = err;
	}
done1:
	s->Pos += res;
	if( e < Err_Null )
	{
		s->Result2 = e;
	}
	else
	{
		s->Result2 = e & ReadRc_Mask;
		if( res == 0 && e == ReadRc_EOF ) res = -1;
	}
	Signal( &s->Mutex );
	
#ifdef SYSDEB
	SysDebug(stream)("Read: %d %x",res,s->Result2);
#endif

	return res;
}

/*--------------------------------------------------------
-- Write						--
--							--
-- Write a buffer full of data to the stream.		--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD Write(Stream *s, byte *buf, word size, word timeout)
{
	typedef struct { MCB mcb; ReadWrite rw; } Req;
	struct { word send; word sent; word pos; } tfr,tot;
	Port server;
	Port reply;
	Port dataport;
	word e = 0;
	word seq;
	word dsize;
	word reqtime;
	WriteReply *rep;
	Req r;
	
	extern word PipeReOpen(Stream *s);
	WordFnPtr reopen = ReOpen;
	
#ifdef SYSDEB
	SysDebug(stream)("Write(%S,%P,%d,%d) from %s",s,buf,size,timeout,procname(returnlink_(s)));
#endif
	
	if( (e = CheckStream(s,C_ReOpen)) != Err_Null ) return e;

	if( s->Type == Type_Pipe ) reopen = PipeReOpen;
	
	r.mcb.Control  = (word *)&r.rw;
	rep = (WriteReply *)r.mcb.Control;

	Wait(&s->Mutex);
	
	tot.send = size;
	tot.sent = 0;
	tot.pos = s->Pos;
	
retry:
	tfr.send = tot.send-tot.sent;
	tfr.sent = 0;
	tfr.pos = tot.pos;
#ifdef SYSDEB
	SysDebug(stream)("Write: tfr send %d sent %d pos %d",tfr);
#endif
	
	if( (e = FindPorts(s,FG_Write,&server,&reply)) != Err_Null) {
		Signal( &s->Mutex );
		return e;
	}

	/* if no server, just pretend to send data */
	if( server == NullPort ) {tot.sent = tot.send; goto done; }

	dataport = server;
		
	seq = 0;
	
	InitMCB(&r.mcb,MsgHdr_Flags_preserve,
		server,reply,FC_GSP+FG_Write|s->FnMod);

	reqtime = (timeout!=-1)&&(timeout<IdleTimeout)?timeout:IdleTimeout;
	
	MarshalWord(&r.mcb,tfr.pos);
	MarshalWord(&r.mcb,tfr.send);
	MarshalWord(&r.mcb,reqtime);

	/* if this is a small write, send the data with the request	*/
	/* this trades two extra messages for a copy in the server	*/
	if( (s->Flags&Flags_NoIData) == 0 && tfr.send <= IOCDataMax )
	{
#ifdef SYSDEB
		SysDebug(stream)("Write: Immediate Data %d",tfr.send);
#endif
		r.mcb.Data = buf;
		r.mcb.MsgHdr.DataSize = (unsigned short)tfr.send;
		tfr.sent += tfr.send;
		seq += ReadRc_SeqInc;
	}

	/* send off request */
	while( (e = PutMsg( &r.mcb ) ) != Err_Null ) 
	{
#ifdef SYSDEB
		SysDebug(error)("Write: Request Error %x",e);
#endif
		if( (e & EC_Mask) >= EC_Error ) goto done;
		if( (e & EC_Mask) == EC_Warn )
		{
			if( (e = reopen(s)) != Err_Null ) goto done;
			goto retry;
		}
	}

	/* now wait for first reply... */

	r.mcb.MsgHdr.Dest = reply;
	r.mcb.Timeout = s->Timeout+reqtime;

	if ( (e = GetMsg(&r.mcb)) < Err_Null )
	{
		if( ((e&EG_Mask)==EG_Timeout) && (timeout != -1) )
		{
			timeout -= reqtime;
			if( timeout <= 0 ) goto done;
		}
#ifdef SYSDEB
		SysDebug(error)("Write: Reply1 Error %x",e);
#endif
		if( (e & EC_Mask) >= EC_Error ) goto done;
		if( ((e & EC_Mask) == EC_Warn) && (e = reopen(s)) != Err_Null ) goto done;
		goto retry;
	}

	if( e == WriteRc_Done ) 
	{
#ifdef SYSDEB
		SysDebug(stream)("Write: Immediate write done");
#endif		
		/* In the case of immediate data, the server has no	*/
		/* opportunity to redefine the tfr size as it does with	*/
		/* the Sizes reply. Therefore it is allowed to ACK a	*/
		/* different value to indicate that not all data has	*/
		/* been written.					*/
		if( r.mcb.MsgHdr.ContSize > 0 ) tfr.sent = rep->first;
		goto writedone;
	}
	elif( e == WriteRc_Already )
	{
#ifdef SYSDEB
		SysDebug(stream)("Write: Data already at server");
#endif		
		tfr.sent = rep->first;
		goto writedone;
	}
	
	/* Here the server has replied with the first and subsequent	*/
	/* data sizes.							*/

	dsize = rep->first;

	/* if rest is zero, then only send a block of size of first	*/

	if( rep->rest == 0 ) tfr.send = dsize;

	/* if present Control[2] is a max size for the tfr		*/
	/* this and the rep->rest feature should not be used together	*/

	if( r.mcb.MsgHdr.ContSize >= 3 ) tfr.send = rep->max;

	if( r.mcb.MsgHdr.Reply != NullPort ) dataport = r.mcb.MsgHdr.Reply;
	InitMCB(&r.mcb,MsgHdr_Flags_preserve,dataport,NullPort,0);
	r.mcb.Data = buf;
	r.mcb.Timeout = s->Timeout;
	
#ifdef SYSDEB
	SysDebug(stream)("Write: Reply1 first %d rest %d max %d dataport %x",
					dsize,rep->rest,rep->max,dataport);
	{
		PortInfo info;
		GetPortInfo(dataport,&info);
		SysDebug(stream)("Write: dataport %x : %x %x %x",dataport,info);
	}
#endif

	while( tfr.sent < tfr.send )
	{
		if ( dsize > tfr.send-tfr.sent ) dsize = tfr.send-tfr.sent;

		if( dsize == (64*1024) )
		{	/* fix to get 64k message */
			r.mcb.MsgHdr.DataSize = (unsigned short)dsize - 4;
			r.mcb.Control = (word *)(r.mcb.Data + dsize - 4);
			r.mcb.MsgHdr.ContSize = 1;
		}
		else r.mcb.MsgHdr.DataSize = (unsigned short)dsize;

		if( tfr.sent+dsize == tfr.send )
		{
			r.mcb.MsgHdr.FnRc = seq|ReadRc_EOD;
			if( dataport != server  ) r.mcb.MsgHdr.Flags = 0;
		}
		else r.mcb.MsgHdr.FnRc = seq;

#ifdef SYSDEB
		SysDebug(stream)("Write: Sending seq %x size %d",r.mcb.MsgHdr.FnRc,dsize);
#endif
		if( (e = PutMsg( &r.mcb ) ) < Err_Null )
		{
#ifdef SYSDEB
			SysDebug(error)("Write: Data PutMsg error %E",e);
#endif
			if( (e & EC_Mask) >= EC_Error ) goto done;
			if( ((e & EC_Mask) == EC_Warn) && (e = reopen(s)) != Err_Null ) goto done;
			goto retry;	
		}
#ifdef SYSDEB
		SysDebug(stream)("Write: Send completed, return val = %x",e);
#endif

		tfr.sent += dsize;
		dsize = rep->rest;	
		r.mcb.Data = &buf[tfr.sent];
		seq += ReadRc_SeqInc;
	}

	/* now get confirmation message */

	r.mcb.MsgHdr.Dest = reply;
	r.mcb.Timeout = s->Timeout+reqtime;

	if ( (e = GetMsg(&r.mcb)) < Err_Null )
	{
#ifdef SYSDEB
		SysDebug(error)("Write: Confirmation error %E",e);
#endif
		if( (e & EC_Mask) >= EC_Error ) goto done;
		if( ((e & EC_Mask) == EC_Warn) && (e = reopen(s)) != Err_Null ) goto done;
		goto retry;
	}

writedone:
#ifdef SYSDEB
	SysDebug(stream)("Write: Confirm got %d wrote %d",rep->first,r.mcb.MsgHdr.ContSize==2?rep->rest:rep->first);
#endif
	/* The first word of the final reply indicates how much data the   */
	/* server received, If this disagrees with what we sent, try again */
	
	if( tfr.sent != rep->first ) goto retry;

	/* If the second word of the reply has been sent, this indicates */
	/* how much of this data the server actually wrote. If this does */
	/* not agree with the first word, terminate the tfr here.	 */
	
	if( r.mcb.MsgHdr.ContSize == 2 && rep->rest != rep->first )
	{
		tot.sent += rep->rest;
		goto done;
	}
	
	tot.sent += tfr.sent;
	
	if( tot.sent < tot.send )
	{
		buf += tfr.sent;
		tot.pos += tfr.sent;
		goto retry;
	}

done:
#ifdef SYSDEB
	SysDebug(stream)("Write: %d %x",tot.sent,e);
#endif
	s->Result2 = e;
	s->Pos += tot.sent;
	Signal( &s->Mutex );
	if( tot.sent > 0 ) return tot.sent;
	else return e<0?e:0;
}

/*--------------------------------------------------------
-- Seek							--
--							--
-- Seek the current file position in the file. Since	--
-- all servers are stateless and do not maintain a file --
-- position, this is purely local to syslib.		--
-- 							--
--------------------------------------------------------*/

static WORD _Seek(Stream *stream, WORD mode, word pos)
{ 	MCB		mcb;
  	SeekRequest 	seekreq;
	word		e;

	mcb.Control = (word *)&seekreq;

	InitMCB(&mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_Seek|stream->FnMod);

	MarshalWord(&mcb,stream->Pos);
	MarshalWord(&mcb,mode);
	MarshalWord(&mcb,pos);

	if( (e = StreamMsg(&mcb, stream )) >= Err_Null )
		stream->Pos = seekreq.CurPos;

	if( mcb.MsgHdr.Reply != NullPort ) FreePort(mcb.MsgHdr.Reply);
	stream->Result2 = e;
	return(e);
}

PUBLIC WORD
Seek(
     Stream *	stream,
     WORD	mode,
     WORD	pos )
{
	word e = Err_Null;
	word res = -1;

#ifdef SYSDEB
	SysDebug(stream)("Seek(%S,SEEK_%s,%d)",stream,
		mode==0?"SET":mode==1?"CUR":mode==2?"END":"???",pos);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;
	
	/* if no server, no seek possible */
	if( stream->Server == NullPort ) return 0;

	Wait(&stream->Mutex);

	if ((e = _Seek(stream, mode, pos)) >= Err_Null)
 	 res = stream->Pos;

#ifdef SYSDEB
	SysDebug(stream)("Seek: %d error %E",res,e);
#endif
	Signal(&stream->Mutex);
	return res;
}

/*--------------------------------------------------------
-- GetFileSize						--
--							--
-- Get the size of the file. 				--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD
GetFileSize( Stream * stream )
{
	word e = Err_Null;
	word res = 0;
	MCB mcb;
	Port server,reply;
	
#ifdef SYSDEB
	SysDebug(stream)("GetFileSize(%S)",stream);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	mcb.Control = &res;

	Wait(&stream->Mutex);

	if( (e=FindPorts(stream,FG_GetSize,&server,&reply)) != Err_Null ) goto done;

	if( server == NullPort ) goto done;

	InitMCB(&mcb,MsgHdr_Flags_preserve,
		server,reply,FC_GSP+FG_GetSize|stream->FnMod);

	if( (e = StreamMsg( &mcb, stream) ) != Err_Null ) goto done;
	
done:
#ifdef SYSDEB
	SysDebug(stream)("GetFileSize: %d error %E",res,e);
	if( mcb.MsgHdr.Reply != NullPort ) SysDebug(error)("GetFileSize: Non-Null Reply port %x",mcb.MsgHdr.Reply);
#endif
	if( mcb.MsgHdr.Reply != NullPort ) FreePort(mcb.MsgHdr.Reply);
	stream->Result2 = e;
	Signal(&stream->Mutex);
	return res;

}

/*--------------------------------------------------------
-- SetFileSize						--
--							--
-- Set the size of the file. This may truncate a file.	--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD
SetFileSize(
	    Stream *	stream,
	    WORD	size )
{
	word e = Err_Null;
	word res = size;
	MCB mcb;
	Port server,reply;

#ifdef SYSDEB
	SysDebug(stream)("SetFileSize(%S,%d)",stream,size);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	mcb.Control = &res;

	Wait(&stream->Mutex);

	if( (e=FindPorts(stream,FG_GetSize,&server,&reply)) != Err_Null ) goto done;

	if( server == NullPort ) goto done;
	
	InitMCB(&mcb,MsgHdr_Flags_preserve,
		server,reply,FC_GSP+FG_SetSize|stream->FnMod);

	mcb.MsgHdr.ContSize = 1;
	
	if ( (e = StreamMsg(&mcb, stream)) < Err_Null ) goto done;

	if( res < stream->Pos ) stream->Pos = res;
	
done:
#ifdef SYSDEB
	SysDebug(stream)("SetFileSize: %d error %E",res,e);
	if( mcb.MsgHdr.Reply != NullPort ) SysDebug(error)("SetFileSize: Non-Null Reply port %x",mcb.MsgHdr.Reply);
#endif
	if( mcb.MsgHdr.Reply != NullPort ) FreePort(mcb.MsgHdr.Reply);
	stream->Result2 = e;
	Signal(&stream->Mutex);
	return res;
}


/*--------------------------------------------------------
-- EnableEvents						--
--							--
-- Enable the reporting of events on the given stream	--
-- this is used to get mouse movements, keypresses and	--
-- console signals from the device.			--
-- 							--
--------------------------------------------------------*/

PUBLIC Port EnableEvents(Stream *stream, word mask)
{
	word e = Err_Null;
	MCB m;
	Port eventport;
	
#ifdef SYSDEB
	SysDebug(stream)("EnableEvents(%S,%x)",stream,mask);
#endif
	eventport = NullPort;
	
	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) goto done;

	eventport = NewPort();

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,eventport,FC_GSP+FG_EnableEvents|stream->FnMod);

	m.Control = &mask;
	m.MsgHdr.ContSize = 1;

	e = StreamMsg( &m, stream );
	
done:
	if( e < Err_Null || mask == 0 )
	{
		FreePort(eventport);
		eventport = NullPort;
		stream->Result2 = e;
	}
	else {
		stream->Result2 = mask;
	}
#ifdef SYSDEB
	SysDebug(stream)("EnableEvents: %x %E",eventport,stream->Result2);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("EnableEvents: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	if( m.MsgHdr.Reply != NullPort ) FreePort(m.MsgHdr.Reply);
	Signal(&stream->Mutex);
	return eventport;
}


/*--------------------------------------------------------
-- NegAcknowledge					--
-- Acknowledge						--
--							--
-- Acknowledge routines for Event protocol.		--
--							--
--------------------------------------------------------*/

void NegAcknowledge(Stream *stream, word counter)
{
	MCB m;
#ifdef SYSDEB
	SysDebug(stream)("NegAcknowledge(%S,%d)",stream,counter);
#endif

	if( CheckStream(stream,C_ReOpen) != Err_Null ) return;

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,NullPort,FC_GSP+FG_NegAcknowledge | stream->FnMod);

	m.Control = &counter;
	m.MsgHdr.ContSize = 1;
	
	PutMsg( &m );
	
	Signal(&stream->Mutex);
}

void Acknowledge(Stream *stream, WORD counter)
{
	MCB m;
#ifdef SYSDEB
	SysDebug(stream)("Acknowledge(%S,%d)",stream,counter);
#endif

	if( CheckStream(stream,C_ReOpen) != Err_Null ) return;

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,NullPort,FC_GSP+FG_Acknowledge | stream->FnMod);

	m.Control = &counter;
	m.MsgHdr.ContSize = 1;
	
	PutMsg( &m );
	
	Signal(&stream->Mutex);
}

/* end of stream.c */
