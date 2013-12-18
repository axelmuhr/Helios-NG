/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/ioc.c								--
--                                                                      --
--	Passive handle operations which are all sent to the IOC.	--
--									--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: ioc.c,v 1.14 1993/07/09 13:36:54 nickc Exp $ */

#define _in_ioc

#include "sys.h"

/*--------------------------------------------------------
-- SetupStream						--
--							--
-- Initialize the stream structure from the given 	--
-- open reply message. This routine has to be a little	--
-- careful since it may be used to re-initialize an	--
-- existing stream in ReOpen, Bind, Connect etc.	--
--							--
--------------------------------------------------------*/

static bool SetupStream( Stream *stream, MCB *mcb )
{
	IOCReply1 *rep = (IOCReply1 *)mcb->Control;
	Port server = mcb->MsgHdr.Reply;

	if( (mcb->MsgHdr.ContSize*sizeof(word)) < (sizeof(IOCReply1)-sizeof(word)) ) return false;
		
	stream->Type = rep->Type;
	stream->Flags = Flags_Stream|rep->Flags;

	if ( server == NullPort )
	{
		FreePort( stream->Server );
		stream->Server = rep->Object;
	}
	else 
	{
		if( stream->Server != server ) FreePort(stream->Server);
		stream->Server = server;
		stream->Flags |= Flags_Server;

		/* pipes do not have a server despite returning a port on open */
		if( stream->Type == Type_Pipe ) stream->Flags ^= Flags_Server;

		if( (server & Port_Flags_Remote) != 0 )
			stream->Flags |= Flags_Remote;
	}

	stream->Access = rep->Access;
	stream->Reply = mcb->MsgHdr.Dest;
	if(mcb->MsgHdr.FnRc >= Err_Null ) stream->FnMod = mcb->MsgHdr.FnRc;
	stream->Timeout = stream->Flags&Flags_Fast?mcb->Timeout:IOCTimeout;


	strcpy(stream->Name,&mcb->Data[rep->Pathname]);
	
	if( (stream->Type == Type_Pseudo) && (stream->Flags & Flags_OpenOnGet) )
		ReOpen(stream);	

	return true;
}

/*--------------------------------------------------------
-- Open							--
--							--
-- Open a Stream to the named object.			--
--							--
--------------------------------------------------------*/

PUBLIC Stream *
Open(
     Object *	object,
     string	name,
     word	mode )
{
	word rc = Err_Null;
	Stream *stream = NULL;
	MCB *mcb;
	IOCReply1 *rep;
	word stlen;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Open(%O,%N,%x)",object,name,mode);
#endif

	if( CheckObject(object,C_Locate) != Err_Null ) return Null(Stream);

	reply = NewPort();

	mcb = NewMsgBuf(0);
	rep = (IOCReply1 *)mcb->Control;

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP|FG_Open|object->FnMod);

	MarshalCommon(mcb,object,name);

	MarshalWord(mcb,mode);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	stlen = sizeof(Stream) + (word)strlen(mcb->Data+rep->Pathname) + SafetyMargin;

	stream = (Stream *)Malloc(stlen);

	if( stream == NULL ) 
	{
		rc = EC_Error|SS_SysLib|EG_NoMemory|EO_Stream;
		goto Done;
	}
	else memclr( (void *)stream, (int)stlen );

	if( SetupStream( stream, mcb ) )
	{
		stream->Flags |= mode&Flags_SaveMode;
		InitSemaphore( &stream->Mutex, 1 );
		stream->Pos = 0;
	}

	AddStream( stream );	

	rc = Err_Null;
	
	if( mode & Flags_Append ) Seek(stream, S_End, 0);
    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Open: %E stream: %S",rc,stream);
#endif
	FreeMsgBuf(mcb);

	if( rc < Err_Null ) FreePort(reply);

	object->Result2 = rc;
	return stream;
}

/*--------------------------------------------------------
-- Locate						--
--							--
-- Find the named object and allocate an object struct	--
-- for it.						--
--							--
--------------------------------------------------------*/

PUBLIC Object *
Locate(
       Object *	object,
       STRING	name )
{
	word rc = Err_Null;
	Object *obj = Null(Object);
	MCB *mcb;
	IOCReply1 *rep;
	word oblen;
	Port reply;
	word fnmod = 0;


#ifdef SYSDEB
	SysDebug(ioc)("Locate(%O,%N)",object,name);
#endif
	/* Locate can be called with a null object pointer */
	
	if( object != NULL ) 
	{	
		if( CheckObject(object,C_Locate) != Err_Null )
		  {
		    return NULL;
		  }
		
		fnmod = object->FnMod;
	}

	reply = NewPort();

	mcb = NewMsgBuf(0);
	rep = (IOCReply1 *)mcb->Control;

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP|FG_Locate|fnmod);

	MarshalCommon(mcb,object,name);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null )
	  {
	    goto Done;
	  }

	oblen = sizeof(Object) + (word)strlen(mcb->Data+rep->Pathname) + SafetyMargin;

	obj = (Object *)Malloc(oblen);
	
	if( obj == NULL )
	{
		rc = EC_Error|SS_SysLib|EG_NoMemory|EO_Object;

		goto Done;
	}
	else memclr( (void *)obj, (int)oblen );

	obj->Type    = rep->Type;
	obj->Flags   = rep->Flags;
	obj->Access  = rep->Access;
	obj->Reply   = reply;
	obj->FnMod   = rc & SS_Mask;
	obj->Timeout = IOCTimeout;
	
	strcpy(obj->Name,mcb->Data+rep->Pathname);

	AddObject( obj );

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Locate: %E object: %O",rc,obj);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Locate: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	if( object != Null(Object) ) object->Result2 = rc;

	if( rc < Err_Null ) FreePort(reply);

	return obj;
}

/*--------------------------------------------------------
-- Create						--
--							--
-- General purpose interface to the create function	--
-- may be used to create files, name table names, load	--
-- code, or perform any other "creation" type operation.--
-- The function creates a named object of the given type--
-- using the data structure of the given size.		--
--							--
--------------------------------------------------------*/

PUBLIC Object *
Create(
       Object *	object,
       string	name,
       word	type,
       word	size,
       byte *	data )
{
	word rc = Err_Null;
	Object *obj = Null(Object);
	MCB *mcb;
	IOCReply1 *rep;
	word oblen;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Create(%O,%N,%T,%d,%P)",object,name,type,size,data);
#endif

	if ( CheckObject(object,C_Locate) != Err_Null )
	  {
	    return NULL;
	  }

	reply = NewPort();

	mcb = NewMsgBuf(0);
	rep = (IOCReply1 *)mcb->Control;
	
	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP|FG_Create|object->FnMod);

	MarshalCommon(mcb,object,name);

	MarshalWord(mcb,type);
	MarshalWord(mcb,size);
	MarshalOffset(mcb);
	MarshalData(mcb,size,data);

	mcb->Timeout = object->Timeout;
	
	/* IOdebug( "Create: sending message" ); */
	
	if ( (rc = IOCMsg(mcb, NULL)) < Err_Null )
	  {
	    /* IOdebug( "Create: message send failed" ); */
	    
	    goto Done;
	  }
	
	/* IOdebug( "Create: message sent" ); */
	
	oblen = sizeof(Object) + (word)strlen(mcb->Data+rep->Pathname) + SafetyMargin;

	obj = (Object *)Malloc(oblen);
	
	if ( obj == NULL )
	  {
	    rc = EC_Error|SS_SysLib|EG_NoMemory|EO_Object;
		
	    goto Done;
	  }	
	else memclr( (void *)obj, (int)oblen );

	obj->Type    = rep->Type;
	obj->Flags   = rep->Flags;
	obj->Access  = rep->Access;
	obj->Reply   = reply;
	obj->FnMod   = rc & SS_Mask;
	obj->Timeout = IOCTimeout;
	
	strcpy(obj->Name,mcb->Data+rep->Pathname);

	AddObject( obj );

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Create: %E object: %O",rc,obj);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Create: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	if( rc < Err_Null ) FreePort(reply);

	object->Result2 = rc;
	
	return obj;	
}

/*--------------------------------------------------------
-- ObjectInfo						--
--							--
-- Obtain any information stored about this object	--
--							--
--------------------------------------------------------*/

PUBLIC WORD
ObjectInfo(
	   Object *	object,
	   STRING	name,
	   byte *	info )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("ObjectInfo(%O,%N,%P)",object,name,info);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;
	
	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_ObjectInfo|object->FnMod);

	MarshalCommon(mcb,object,name);

	if( (rc = IOCMsg(mcb,info)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("ObjectInfo: %E infosize %d",rc,mcb->MsgHdr.DataSize);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("ObjectInfo: Non-Null Reply port %x",mcb->MsgHdr.Reply);	
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
	
}

/*--------------------------------------------------------
-- ServerInfo						--
--							--
-- Obtain any information from the server, the object   --
-- serves merely to identify the server.		--
--							--
--------------------------------------------------------*/

PUBLIC WORD
ServerInfo(
	   Object *	object,
	   byte *	info )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;
#ifdef SYSDEB
	SysDebug(ioc)("ServerInfo(%O,%P)",object,info);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;
	
	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_ServerInfo|object->FnMod);

	MarshalCommon(mcb,object,NULL);

	if( (rc = IOCMsg(mcb,info)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("ServerInfo: %E infosize %d",rc,mcb->MsgHdr.DataSize);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("ServerInfo: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Link							--
--							--
-- Establish a link to the object "to" under the	--
-- name "name".						--
--							--
--------------------------------------------------------*/

PUBLIC WORD
Link(
     Object *	object,
     string	name,
     Object *	to )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;
#ifdef SYSDEB
	SysDebug(ioc)("Link(%O,%N,%O)",object,name,to);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Link|object->FnMod);

	MarshalCommon(mcb,object,name);
	MarshalString(mcb,to->Name);
	MarshalCap(mcb,&to->Access);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Link: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Link: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- SetDate						--
--							--
-- Set the datestamp on the named object to the given   --
-- value.						--
-- At present this routine only sets the Modified date	--
-- on the object.					--
--							--
--------------------------------------------------------*/

PUBLIC WORD
SetDate(
	Object *	object,
	STRING		name,
	DateSet *	dates )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("SetDate(%O,%N)",object,name);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_SetDate|object->FnMod);

	MarshalCommon(mcb,object,name);
	MarshalDate(mcb,dates->Creation);
	MarshalDate(mcb,dates->Access);
	MarshalDate(mcb,dates->Modified);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("SetDate: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("SetDate: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Protect						--
--							--
-- Set the access matrix of the the named object.	--
--							--
--------------------------------------------------------*/

PUBLIC WORD
Protect(
	Object *	object,
	STRING		name,
	Matrix		matrix )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Protect(%O,%N,%X)",object,name,matrix);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Protect|object->FnMod);

	MarshalCommon(mcb,object,name);
	MarshalWord(mcb,matrix);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Protect: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Protect: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Revoke						--
--							--
-- Revoke all capabilities for the given object, the	--
-- object structure will be re-initialized with a new	--
-- capability for the given object.			--
--							--
--------------------------------------------------------*/

PUBLIC WORD
Revoke( Object * object )
{
	word rc = Err_Null;
	MCB *mcb;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Revoke(%O)",object);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Revoke|object->FnMod);

	MarshalCommon(mcb,object,NULL);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;
	object->Access = *(Capability *)mcb->Control;

    Done:
#ifdef SYSDEB
	SysDebug(ioc)("Revoke: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Revoke: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Delete						--
--							--
-- Delete the named object.				--
--							--
--------------------------------------------------------*/

PUBLIC WORD
Delete(
       Object *	object,
       STRING	name )
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Delete(%O,%N)",object,name);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Delete|object->FnMod);

	MarshalCommon(mcb,object,name);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:

#ifdef SYSDEB
	SysDebug(ioc)("Delete: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Delete: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Rename						--
--							--
-- Rename the object called "from" relative to object   --
-- as "to" relative to object. Both paths must be on    --
-- the same server. In general this only works for	--
-- filing systems.					--
--							--
--------------------------------------------------------*/

PUBLIC word Rename(Object *object, string from, string to)
{
	word rc = Err_Null;
	MCB *mcb ;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Rename(%O,%N,%N)",object,from,to);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Rename|object->FnMod);

	MarshalCommon(mcb,object,from);
	MarshalString(mcb,to);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rc = Err_Null;

    Done:

#ifdef SYSDEB
	SysDebug(ioc)("Rename: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Rename: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

/*--------------------------------------------------------
-- Refine						--
--							--
-- Refine the access allowed by the object's capability --
--							--
--------------------------------------------------------*/

PUBLIC WORD Refine(Object *object, AccMask mask)
{
	word rc = Err_Null;
	MCB *mcb ;
	IOCReply2 *rep;
	Port reply;

#ifdef SYSDEB
	SysDebug(ioc)("Refine(%O,%A)",object,mask);
#endif

	if( (rc = CheckObject(object,C_Locate)) != Err_Null ) return rc;

	reply = object->Reply;
	
	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_GSP+FG_Refine|object->FnMod);

	MarshalCommon(mcb,object,NULL);
	MarshalWord(mcb,mask);

	if( (rc = IOCMsg(mcb,NULL)) < Err_Null ) goto Done;

	rep = (IOCReply2 *)mcb->Control;

	object->Access = rep->Cap;	/* copy new capability in */

	rc = Err_Null;

    Done:

#ifdef SYSDEB
	SysDebug(ioc)("Refine: %E",rc);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("Refine: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);
	FreeMsgBuf(mcb);

	object->Result2 = rc;
	
	return rc;
}

