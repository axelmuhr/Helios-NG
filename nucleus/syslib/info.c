/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/info.c							--
--                                                                      --
--	Routines to get and set info on a stream, and to manipulate	--
--	attributes.							--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: info.c,v 1.3 1992/05/01 16:27:28 nickc Exp $ */

#define _in_info

#include "sys.h"

#include <attrib.h>
#include <root.h>

/*--------------------------------------------------------
-- GetInfo						--
--							--
-- Get any saved info about an open object.		--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD GetInfo(Stream *stream, byte *info)
{
	word size = 0;
	word e = Err_Null;
	MCB m;

#ifdef SYSDEB
	SysDebug(stream)("GetInfo(%S,%P)",stream,info);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_GetInfo|stream->FnMod);

	m.Data = info;
		
	if ( (e = StreamMsg(&m,stream)) >= Err_Null ) size = m.MsgHdr.DataSize;


#ifdef SYSDEB
	SysDebug(stream)("GetInfo: %d error %E",size,e);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("GetInfo: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	if( m.MsgHdr.Reply != NullPort ) FreePort(m.MsgHdr.Reply);
	stream->Result2 = e;
	Signal(&stream->Mutex);
	return size;

}

/*--------------------------------------------------------
-- SetInfo						--
--							--
-- Set saved info about an open object/stream. Mostly	--
-- used to set terminal characteristics.		--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD SetInfo(Stream *stream, byte *info, word size)
{
	word e = Err_Null;
	MCB m;
	
#ifdef SYSDEB
	SysDebug(stream)("SetInfo(%S,%P,%d)",stream,info,size);
#endif

	if( (e = CheckStream(stream,C_ReOpen)) != Err_Null ) return e;

	Wait(&stream->Mutex);

	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_SetInfo|stream->FnMod);

	m.Data = info;
	m.MsgHdr.DataSize = (unsigned short)size;

	e = StreamMsg( &m, stream );

#ifdef SYSDEB
	SysDebug(stream)("SetInfo: %E",e);
	if( m.MsgHdr.Reply != NullPort ) SysDebug(error)("SetInfo: Non-Null Reply port %x",m.MsgHdr.Reply);
#endif
	if( m.MsgHdr.Reply != NullPort ) FreePort(m.MsgHdr.Reply);
	stream->Result2 = e;
	Signal(&stream->Mutex);
	return e;

}

/*--------------------------------------------------------
-- GetAttributes					--
-- SetAttributes					--
-- etc.							--
--							--
-- Attribute manipulation routines			--
-- 							--
--------------------------------------------------------*/

PUBLIC WORD GetAttributes(Stream *stream, Attributes *attr)
{
	return GetInfo(stream,(byte *)attr);
}


PUBLIC WORD SetAttributes(Stream *stream, Attributes *attr)
{
	return SetInfo(stream,(byte *)attr,sizeof(Attributes));
}


/* In an Attribute, the bottom two bits indicate which word */
/* in the Attributes structure we are interested in, and    */
/* the rest of the word specifies a single bit.		    */

PUBLIC WORD IsAnAttribute(Attributes *attr, Attribute item)
{ WORD *ptr = (WORD *) attr + (item & 0x00000003);
  item     &= 0xFFFFFFFC;
  return( *ptr & item);
}

PUBLIC void AddAttribute(Attributes *attr, Attribute item)
{ WORD *ptr = (WORD *) attr + (item & 0x00000003);
  item     &= 0xFFFFFFFC;
  *ptr     |= item;
}

PUBLIC void RemoveAttribute(Attributes *attr, Attribute item)
{ WORD *ptr = (WORD *) attr + (item & 0x00000003);
  item     &= 0xFFFFFFFC;
  *ptr     &= ~item;
}


/* The input and output speeds of a stream are kept in	*/
/* the bottom byte of the relevant Attributes field,	*/
/* thus allowing upto 256 different speeds.		*/

PUBLIC WORD GetInputSpeed(Attributes *attr)
{ return((attr->Input) & 0x000000FF);
}

PUBLIC WORD GetOutputSpeed(Attributes *attr)
{ return((attr->Output) & 0x000000FF);
}

PUBLIC void SetInputSpeed(Attributes *attr, WORD speed)
{ attr->Input  &= 0xFFFFFF00;	/* clear bottom byte */
  speed        &= 0x000000FF;	/* prevent nasty crash if speed invalid */
  attr->Input  |= speed;
}

PUBLIC void SetOutputSpeed(Attributes *attr, WORD speed)
{ attr->Output &= 0xFFFFFF00;	/* clear bottom byte */
  speed        &= 0x000000FF;	/* prevent nasty crash if speed invalid */
  attr->Output |= speed;
}

/*--------------------------------------------------------
-- MachineName						--
--							--
-- Get this machine's name from the IOC.		--
--							--
--------------------------------------------------------*/

PUBLIC WORD MachineName(byte *name)
{
	word rc = Err_Null;
	MCB *mcb;
	Port reply = NewPort();

#ifdef SYSDEB
	SysDebug(process)("MachineName(%P)",name);
#endif

	mcb = NewMsgBuf(0);

	InitMCB(mcb,MsgHdr_Flags_preserve,
		MyTask->IOCPort,reply,FC_Private|FG_MachineName);
	
	rc = IOCMsg(mcb,name);

	FreePort(reply);
#ifdef SYSDEB
	SysDebug(process)("MachineName: %E name %N",rc,name);
	if( mcb->MsgHdr.Reply != NullPort ) SysDebug(error)("MachineName: Non-Null Reply port %x",mcb->MsgHdr.Reply);
#endif
	if( mcb->MsgHdr.Reply != NullPort ) FreePort(mcb->MsgHdr.Reply);

	FreeMsgBuf(mcb);

	return rc;
}

/*--------------------------------------------------------
-- GetDate						--
--							--
-- Get the current system date and time.		--
-- 							--
--------------------------------------------------------*/

PUBLIC Date GetDate()
{
	return (Date)(GetRoot()->Time);
}

/* end of info.c */
