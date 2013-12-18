/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/socket.c							--
--                                                                      --
--	Interface to socket mechanism. This supports both external	--
--	networks such as TCP/IP and an internal AF_HELIOS network.	--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: socket.c,v 1.14 1993/07/09 13:36:16 nickc Exp $ */

#include "sys.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/hel.h>

#define MAX_HPATH 108

extern Capability DefaultCap;

static void MarshalStruct(MCB *mcb, void *data, word size)
{
	if( data == NULL || size == 0 ) MarshalWord(mcb,-1); 
	else {
		MarshalOffset(mcb);
		MarshalData(mcb,sizeof(word),(byte *)&size);
		MarshalData(mcb,size,(byte *)data);
	}
}

static word UnMarshalStruct(byte *data, void *str)
{
	word size = *(word *)data;

	if( str == NULL ) return 0;
	
	data += 4;
	
	memcpy(str,data,(int)size);
	
	return size;
}

/* In AF_UNIX/HELIOS it is acceptable to omit the terminating 0 from the*/
/* socket pathname. This routine adds it if required.			*/
static void MarshalAddr(MCB *mcb, void *data, word size)
{
	struct sockaddr_hel *sh = (struct sockaddr_hel *)data;
	if( sh != NULL && sh->sh_family == AF_HELIOS)
	{
		/* if we have been give sizeof(sockaddr_hel) then just	*/
		/* ensure that string is terminated.			*/
		if( size == sizeof(struct sockaddr_hel) ) sh->sh_path[108] = 0;
		elif( sh->sh_path[size-sizeof(short)] != 0 )
			size++,sh->sh_path[size-sizeof(short)] = 0;
	}
	MarshalStruct(mcb,data,size);
}

extern Stream *Socket(char *domain, word type, word protocol)
{
	Stream *s;
	char name[108];
	Capability cap;
	int i;	

	cap = DefaultCap;

	/* ensure there is enough space in the stream for a Helios path */
	for( i = 0 ; i < MAX_HPATH; i++ ) name[i] = 'x';
	name[MAX_HPATH-1] = 0;

	s = NewStream(name,&cap,0);
	
	if( s != NULL )
	{
		strcpy(s->Name,domain);
	
		s->Type = Type_Socket;
		s->Pos = (protocol<<8)|type;
	}
	
	return s;
}

extern word Bind(Stream *stream, byte *addr, word len)
{
	word e = Err_Null;
	MCB *mcb;
	Port reply = NullPort;
		
#ifdef SYSDEB
	SysDebug(stream)("Bind(%S,%P,%d)",stream,addr,len);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( stream->Type != Type_Socket ) 
		return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;

	Wait(&stream->Mutex);

	mcb = NewMsgBuf(0);
	
	reply = NewPort();
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Bind|stream->FnMod);

	if( stream->Server == NullPort )
	{
		MarshalWord(mcb,-1);
		MarshalString(mcb,stream->Name);
	}
	else
	{
		MarshalString(mcb,stream->Name);
		MarshalWord(mcb,-1);
	}

	MarshalWord(mcb,1);
	MarshalCap(mcb,&stream->Access);
	MarshalWord(mcb,stream->Pos);		/* protocol & type actually */
	MarshalAddr(mcb,addr,len);

	e = IOCMsg( mcb, NULL );

	/* we do a SetupStream even if the bind failed because we need to */
	/* be able to re-contact this new socket.			  */
	
	if( SetupStream( stream, mcb ) ) stream->Pos = 0;

#ifdef SYSDEB
	SysDebug(stream)("Bind: %E",e);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Bind: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif

	stream->Result2 = e;
	Signal(&stream->Mutex);
	if( e < 0 ) FreePort(reply);
	FreeMsgBuf(mcb);
	return e;
}

extern word Listen(Stream *stream, word len)
{
	word e = Err_Null;
	MCB m;
	
#ifdef SYSDEB
	SysDebug(stream)("Listen(%S,%d)",stream,len);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( stream->Type != Type_Socket || stream->Server == NullPort )
		return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;
		
	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_Listen|stream->FnMod);

	m.Control = &len;
	m.MsgHdr.ContSize = 1;

	e = StreamMsg( &m, stream );

#ifdef SYSDEB
	SysDebug(stream)("Listen: %E",e);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("Listen: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	if( m.MsgHdr.Reply != NullPort ) FreePort(m.MsgHdr.Reply);
	stream->Result2 = e;
	Signal(&stream->Mutex);
	return e;
}

extern Stream *Accept(Stream *stream, byte *addr, word *len)
{
	word rc = Err_Null;
	Stream *stream1 = NULL;
	MCB *mcb;
	IOCReply1 *rep;
	word stlen;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Accept(%S,%x,%d)",stream,addr,len);
#endif

	if( CheckStream(stream,C_ReOpen) != Err_Null ) return NULL;

	if( stream->Type != Type_Socket || stream->Server == NullPort )
	{
		stream->Result2 = EC_Error|SS_SysLib|EG_Invalid|EO_Stream;
		return NULL;
	}
		
	reply = NewPort();

	Wait ( &stream->Mutex );

	mcb = NewMsgBuf(0);
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,reply,FC_GSP|FG_Accept|stream->FnMod);

	if( (rc = StreamMsg(mcb,stream)) < Err_Null ) goto Done;

	rep = (IOCReply1 *)mcb->Control;
	
	stlen = sizeof(Stream) + (word)strlen(mcb->Data+rep->Pathname) + 1;

	stream1 = (Stream *)Malloc(stlen);
	
	if( stream1 == NULL ) 
	{
		rc = EC_Error|SS_SysLib|EG_NoMemory|EO_Stream;
		goto Done;
	}
	else memclr( (void *)stream1, (int)stlen );

	stream1->Type = Type_Pseudo;
	AddStream( stream1 );	
	InitSemaphore(&stream1->Mutex,1);

	if( SetupStream( stream1, mcb ) ) stream1->Pos = 0;
	
	if( addr != NULL )
		*len = UnMarshalStruct(mcb->Data+rep->Object,addr);
	
	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Accept: %E stream: %S",rc,stream1);
#endif
	Signal( &stream->Mutex );

	if( rc < Err_Null ) FreePort(reply);

	FreeMsgBuf(mcb);
	
	stream->Result2 = rc;
	return stream1;
}

extern word Connect(Stream *stream, byte *addr, word len)
{
	word e = Err_Null;
	MCB *mcb;

#ifdef SYSDEB
	SysDebug(stream)("Connect(%S,%P,%d)",stream,addr,len);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( stream->Type != Type_Socket ) return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;
	
	if( stream->Server == NullPort && (e=Bind(stream,NULL,0)) < 0 ) return e;
	
	Wait(&stream->Mutex);

	mcb = NewMsgBuf(0);
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_Connect|stream->FnMod);

	MarshalAddr(mcb, addr, len);
	
	e = StreamMsg( mcb, stream );

	/* if that succeeded && there is some data, use it to	*/
	/* re-initialize the Stream structure.			*/
	if( (e >= 0) && (mcb->MsgHdr.ContSize > 0) ) 
	{
		if( SetupStream( stream, mcb ) ) stream->Pos = 0;
	}
	
#ifdef SYSDEB
	SysDebug(stream)("Connect: %E",e);
#endif
	stream->Result2 = e;
	FreeMsgBuf(mcb);
	Signal(&stream->Mutex);
	return e;
}

extern word SendMessage(Stream *stream, word flags, ...)
{
	struct msghdr *msg = *(struct msghdr **)(&flags+1);
	struct iovec *iov = msg->msg_iov;
	int msgmax;
	int datamax = 0;
	int i;
	word	e;
	MCB *mcb = NULL;
	DataGram *dg;
	bool isstream = FALSE;
		
	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( (stream->Type & ~Type_Flags) != Type_Socket ) return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;
	
	if( stream->Server == NullPort && (e=Bind(stream,NULL,0)) < 0 ) return e;
	
	isstream = (stream->Type & Type_Stream) != 0;
	
	Wait(&stream->Mutex);

	for( i = 0 ; i < msg->msg_iovlen; i++ ) datamax+=iov[i].iov_len;

	msgmax = datamax + 128*3;

	if( msgmax > 65535 )
	{ e = EC_Error|SS_SysLib|EG_WrongSize|EO_Message; goto done; }
	
	mcb = NewMsgBuf(msgmax);

again:
	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FG_SendMessage|stream->FnMod);
	MarshalWord(mcb,flags);
	MarshalWord(mcb,datamax);
	MarshalWord(mcb,stream->Timeout);
	MarshalStruct(mcb,msg->msg_accrights,msg->msg_accrightslen);
	MarshalAddr(mcb,msg->msg_name,msg->msg_namelen);

	if( (e = StreamMsg( mcb, stream )) < 0 ) goto done;

	dg = (DataGram *)mcb->Control;

	mcb->MsgHdr.Flags = 0;
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;
	mcb->MsgHdr.Reply = isstream?stream->Reply:NullPort;
	mcb->MsgHdr.FnRc = Err_Null;

	if( dg->DataSize < datamax ) datamax = (int)dg->DataSize;
	else dg->DataSize = datamax;
	
	MarshalOffset(mcb);
	
	for( i = 0 ; datamax > 0 && i < msg->msg_iovlen; i++ )
	{
		int dsize = iov[i].iov_len;
		if( dsize > datamax ) dsize = datamax;
		MarshalData(mcb,dsize,iov[i].iov_base);
		datamax -= dsize;
	}
	datamax = (int)dg->DataSize;
	
	e = PutMsg(mcb);

	/* if this is a stream socket, wait for confirmation	*/
	/* that the data got there.				*/
	if( e >= 0 && isstream )
	{
		InitMCB(mcb,0,stream->Reply,NullPort,0);
		e = GetMsg(mcb);
		if( (e & EC_Mask) < EC_Error ) goto again;
	}
done:
	stream->Result2 = e;
	Signal(&stream->Mutex);
	if (mcb != NULL) FreeMsgBuf(mcb);
	return e<0?-1:datamax;
}

extern word RecvMessage(Stream *stream, word flags, ... )
{
	struct msghdr *msg = *(struct msghdr **)(&flags+1);
	struct iovec *iov = msg->msg_iov;
	int msgmax;
	int datasize = 0;
	int i;
	word e = 0;
	MCB *mcb = NULL;
	DataGram *dg;
	byte *data;
		
	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( (stream->Type & ~Type_Flags) != Type_Socket ) return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;
	
	if( stream->Server == NullPort && (e=Bind(stream,NULL,0)) < 0 ) return e;
	
	Wait(&stream->Mutex);

	for( i = 0 ; i < msg->msg_iovlen; i++ ) datasize+=iov[i].iov_len;
	
	msgmax = datasize + 128*3;

	if( msgmax > 65535 )
	{ e = EC_Error|SS_SysLib|EG_WrongSize|EO_Message; goto done; }
	
	mcb = NewMsgBuf(msgmax);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FG_RecvMessage|stream->FnMod);
	MarshalWord(mcb,flags);	
	MarshalWord(mcb,datasize);
	MarshalWord(mcb,stream->Timeout);
	
	e = StreamMsg( mcb, stream );

	if( e < 0 ) goto done;

	dg = (DataGram *)mcb->Control;
	
	if( dg->SourceAddr != -1 ) 
		msg->msg_namelen = (int)UnMarshalStruct(mcb->Data+dg->SourceAddr,msg->msg_name);

	if( dg->AccRights != -1 )
		msg->msg_accrightslen = (int)UnMarshalStruct(mcb->Data+dg->AccRights,msg->msg_accrights);

	data = mcb->Data+dg->Data;
	datasize = (int)dg->DataSize;
	
	for( i = 0 ; i < msg->msg_iovlen && datasize > 0 ; i++ )
	{
		int dsize = iov[i].iov_len;
		if( dsize > datasize ) dsize = datasize;
		memcpy(iov[i].iov_base,data,dsize);
		data += dsize;
		datasize -= dsize;
	}
	datasize = (int)dg->DataSize;
		
done:
	stream->Result2 = e;
	Signal(&stream->Mutex);
	if (mcb != NULL) FreeMsgBuf(mcb);
	return e<0?-1:datasize;
}

extern word GetSocketInfo(Stream *stream, word level, word option, void *optval, word *optlen)
{
	word e = Err_Null;
	MCB *mcb;
	
#ifdef SYSDEB
	SysDebug(stream)("GetSocketInfo(%S,%x,%x,%x,%d)",stream,level,option,optval,*optlen);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( (stream->Type & ~Type_Flags) != Type_Socket) return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;

	if( stream->Server == NullPort && (e=Bind(stream,NULL,0)) < 0 ) return e;
	
	Wait(&stream->Mutex);

	mcb = NewMsgBuf(0);
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_GetInfo|stream->FnMod|FF_SocketInfo);

	MarshalWord(mcb, level );
	MarshalWord(mcb, option );	

	/* a bit of a kludge, if the level is IOCTL and the option	*/
	/* wants to send some data, send it here			*/
	if( level == SOL_IOCTL && (option>>28)==0x4 )
	{
		mcb->MsgHdr.FnRc |= FF_SendInfo;
		MarshalStruct(mcb,optval,*optlen);
	}
		
	e = StreamMsg( mcb, stream );

	if( e >= 0 && mcb->Control[2] != -1 ) 
		*optlen = UnMarshalStruct(mcb->Data+mcb->Control[2],optval);

#ifdef SYSDEB
	SysDebug(stream)("GetSocketInfo: %E",e);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("GetSocketInfo: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	stream->Result2 = e;
	FreeMsgBuf(mcb);
	Signal(&stream->Mutex);
	return e;	
}

extern word SetSocketInfo(Stream *stream, word level, word option, void *optval, word optlen)
{
	word e = Err_Null;
	MCB *mcb;
	
#ifdef SYSDEB
	SysDebug(stream)("SetSocketInfo(%S,%x,%x,%x,%d)",stream,level,option,optval,optlen);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	if( (stream->Type & ~Type_Flags) != Type_Socket) return EC_Error|SS_SysLib|EG_Invalid|EO_Stream;

	if( stream->Server == NullPort && (e=Bind(stream,NULL,0)) < 0 ) return e;
			
	Wait(&stream->Mutex);

	mcb = NewMsgBuf(0);
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_SetInfo|stream->FnMod|FF_SocketInfo);

	MarshalWord(mcb, level );
	MarshalWord(mcb, option );	
	MarshalStruct(mcb, optval, optlen);
	
	e = StreamMsg( mcb, stream );

#ifdef SYSDEB
	SysDebug(stream)("SetSocketInfo: %E",e);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("SetSocketInfo: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	stream->Result2 = e;
	FreeMsgBuf(mcb);
	Signal(&stream->Mutex);
	return e;	
}

/* end of socket.c */
