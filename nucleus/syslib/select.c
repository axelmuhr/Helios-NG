/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--            Copyright (C) 1987,1990, Perihelion Software Ltd.         --
--                        All Rights Reserved.                          --
--                                                                      --
-- syslib/select.c							--
--                                                                      --
--	System Library, the common program interface to the operating   --
--	system.								--
--                                                                      --
--	Author:  NHG 16/8/87						--
--		 NHG 03/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G%	Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: select.c,v 1.8 1993/07/09 13:36:01 nickc Exp $ */

#include "sys.h"

#define RetryTimeout	(20*OneSec)

word SelectStream(word nstreams, Stream **streams, word *flags, word timeout)
{
	MCB mcb;
	Port *ports = (Port *)Malloc(nstreams * sizeof(Port));
	int i;
	word selected;
	word e;	
	word nfound = 0;
	word delayed = 0;
	
#ifdef SYSDEB
	SysDebug(stream)("Select(%d,%x,%d)",nstreams,streams,timeout);
#endif
	if( ports == NULL ) return EC_Error|SS_SysLib|EG_NoMemory;

again:	
	for( i = 0; i < nstreams; i++ )
	{
		Stream *stream = streams[i];

		ports[i] = NullPort;
		
		if( 	(flags[i]&Flags_Mode) == 0 			||
			CheckStream(stream,C_ReOpen) != Err_Null 	|| 
			(stream->Flags&Flags_Selectable) == 0 
		   ) continue;

		flags[i] &= ~O_Selected;
	
#ifdef SYSDEB
	SysDebug(stream)("Select %x %S",flags[i],stream);
#endif
		/* lock the stream for the duration of the select */
		Wait(&stream->Mutex);
		
		if( stream->Type == Type_Pipe )	ports[i] = PipeSelect(stream,flags[i]);
		else
		{
		retry:
			ports[i] = stream->Reply;
			
			InitMCB(&mcb,MsgHdr_Flags_preserve,stream->Server,ports[i],
				FC_GSP|FG_Select|stream->FnMod|(flags[i]&Flags_Mode));
			
			while( (e = PutMsg( &mcb ) ) != Err_Null ) 
			{
#ifdef SYSDEB
				SysDebug(error)("Select Request Error %E",e);
#endif
				if( (e & EC_Mask) >= EC_Error )
				{ 
					Signal(&stream->Mutex); 
					ports[i] = NullPort; 
					break; 
				}
				if( (e & EC_Mask) == EC_Warn ) 
				{ 
					if( (e = ReOpen(stream)) < Err_Null )
					goto retry; 
				}
			}
		}
	}
	
	/* all the requests are out, get the replies 			*/
	/* first we get all those which have replied immediately 	*/

	e = 0;
	mcb.Timeout = 0;	/* 0 timeout == poll */
	for(;;)
	{
		selected = MultiWait(&mcb,nstreams,ports);
		if( selected == -1 )		/* no selection		*/
		{
			/* When the timeout is zero, we must wait here	*/
			/* a while to avoid a race condition. If we give*/
			/* up too soon, the server has no time to reply.*/
			
			if( timeout != 0 || delayed || nfound > 0 ) break;
			Delay(OneSec/100); /* == 10ms */
			delayed = 1;
			continue;
		}
		if( selected == EK_Timeout) continue;
		if( selected < 0 )		/* error - fail		*/
		{ e = mcb.MsgHdr.FnRc; break; }
		/* set the flags to the bits which were selected	*/
		flags[selected] &= ~Flags_Mode;
		flags[selected] |= O_Selected|(mcb.MsgHdr.FnRc&Flags_Mode);
		nfound++;
	}

	/* now wait for the timeout period or until a reply arrives	*/
	/* but only if no selects have succeded yet.			*/
	if( e == 0 && nfound == 0 && timeout != 0 )
	{
		mcb.Timeout = timeout;
		if( timeout == -1 || mcb.Timeout > RetryTimeout ) mcb.Timeout = RetryTimeout;
		mcb.MsgHdr.FnRc = 0;
		selected = MultiWait(&mcb,nstreams,ports);
		e = mcb.MsgHdr.FnRc;
		if( selected >= 0 && e > 0 ) 
		{
			flags[selected] &= ~Flags_Mode;
			flags[selected] |= O_Selected|(e&Flags_Mode);
			nfound++;
		}
		elif( e == EK_Timeout )
		{
			if( timeout != -1 ) timeout -= mcb.Timeout;
			else e = 0; 
		}
	}
	
	/* free all the ports and unlock the streams */
	for( i = 0; i < nstreams; i++ )
	{
		if( ports[i] != NullPort ) 
		{
			FreePort(ports[i]);
			streams[i]->Reply = NewPort();
			Signal(&(streams[i]->Mutex));
		}
	}

	/* If nothing has been selected, and we still have some	*/
	/* of the timeout to go, go back and do it again.	*/
	
	if( e >= 0 && nfound == 0 && ( timeout > 0 || timeout == -1) ) goto again;
	
	Free(ports);
#ifdef SYSDEB
	SysDebug(stream)("Select: error %x nfound %d",e,nfound);
#endif		
	return e<0?e:nfound;
}
