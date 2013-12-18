/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   S E R V E R S			--
--                     ---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- familiar.c								--
--                                                                      --
--	Program to interact with the Inmos Witch Board, providing	--
--	servers for the various devices supported.			--
--                                                                      --
--	Author:  BLV 22/3/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: familiar.c,v 1.1 1991/03/23 16:32:49 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 

/**
*** The familiar is a program to interact with the Inmos witch board, 
*** providing servers for keyboard, mouse, real-time clock, and battery
*** backed RAM. The witch board runs a piece of occam, because its 32K of
*** RAM is insufficient for Helios. Hence the familiar uses a dumb link
*** protocol similar to the server task.
***
*** The familiar is intended to be economical in its memory use, since it
*** is likely to run on the same processor as X. Hence it is linked with
*** s0.o rather than with c0.o. Do not use printf() or similar routines
*** when debugging.
***
*** At present there is no way of terminating the familiar. Since it
*** provides the keyboard for the whole system, this seems like a good idea.
*** Unfortunately it means rebooting Helios while debugging...
***
*** NvRam support still has to be added.
**/

#pragma -s0	/* Suppress stack checking	*/
#pragma -f0	/* and vector stack		*/
#pragma -g0	/* Leave the names out of the code */

#include <syslib.h>
#include <gsp.h>
#include <root.h>
#include <servlib.h>
#include <sem.h>
#include <nonansi.h>
#include <string.h>
#include <config.h>
#include <link.h>
#include <codes.h>
#include <attrib.h>
#include <ioevents.h>
#include <asm.h>
#include <signal.h>
#include "familiar.h"

#ifndef	SS_Keyboard
#define	SS_Keyboard	0x1A000000
#endif
#ifndef SS_Pointer
#define SS_Pointer	0x1C000000
#endif
#ifndef SS_Clock
#define	SS_Clock	0x1D000000
#endif

#ifndef eq
#define eq	==
#define ne	!=
#endif

/**
*** Boring forward declarations.
**/
static	void	take_over_link(void);	/* and start link guardian */
static	void	parse_args(void);	/* receive environment */
static	void	Keyboard_Open(ServInfo *);
static	void	Mouse_Open(ServInfo *);
static	void	Clock_ObjInfo(ServInfo *);
static	void	Clock_SetDate(ServInfo *);
static	Port	create_name(char *);
static	void	init_mouse(void);
static	void	init_keyboard(void);
static	void	init_clock(void);
static	void	new_mouse(int buttons, int dx, int dy);
static	void	new_keyboard(bool up, int scancode);

static	int		LinkToWitch;
static	ObjNode		Keyboard_Root;
static	ObjNode		Mouse_Root;
static	ObjNode		Clock_Root;
static	Semaphore	LinkAvailable;	/* to write to the witch */
static	Semaphore	Done;		/* signalled to reactivate the guardian */
static	Semaphore	ReplyAvailable;	/* when IOprotocol reply arrives */

static	DispatchInfo	Keyboard_Info = {
	(DirNode *)	&Keyboard_Root,
	NullPort,
	SS_Keyboard,
	Null(char),
	{ NULL,	0 },
	{
		{ Keyboard_Open,	2000 },
		{ InvalidFn,		2000 },	/* Create	*/
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ InvalidFn,		2000 },	/* ServerInfo	*/
		{ InvalidFn,		2000 }, /* Delete	*/
		{ InvalidFn,		2000 }, /* Rename	*/
		{ InvalidFn,		2000 }, /* Link		*/
		{ InvalidFn,		2000 }, /* Protect	*/
		{ InvalidFn,		2000 },	/* SetDate	*/
		{ InvalidFn,		2000 }, /* Refine	*/
		{ InvalidFn,		2000 }, /* CloseObj	*/
		{ InvalidFn,		2000 },	/* Revoke	*/
		{ InvalidFn,		2000 }, /* Reserved1	*/
		{ InvalidFn,		2000 }  /* Reserved2	*/
	}
};

static	DispatchInfo	Mouse_Info = {
	(DirNode *)	&Mouse_Root,
	NullPort,
	SS_Pointer,
	Null(char),
	{ NULL,	0 },
	{
		{ Mouse_Open,		2000 },
		{ InvalidFn,		2000 },	/* Create	*/
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ InvalidFn,		2000 },	/* ServerInfo	*/
		{ InvalidFn,		2000 }, /* Delete	*/
		{ InvalidFn,		2000 }, /* Rename	*/
		{ InvalidFn,		2000 }, /* Link		*/
		{ InvalidFn,		2000 }, /* Protect	*/
		{ InvalidFn,		2000 },	/* SetDate	*/
		{ InvalidFn,		2000 }, /* Refine	*/
		{ InvalidFn,		2000 }, /* CloseObj	*/
		{ InvalidFn,		2000 },	/* Revoke	*/
		{ InvalidFn,		2000 }, /* Reserved1	*/
		{ InvalidFn,		2000 }  /* Reserved2	*/
	}
};

static	DispatchInfo	Clock_Info = {
	(DirNode *)	&Clock_Root,
	NullPort,
	SS_Clock,
	Null(char),
	{ NULL,	0 },
	{
		{ InvalidFn,		2000 },
		{ InvalidFn,		2000 },	/* Create	*/
		{ DoLocate,		2000 },
		{ Clock_ObjInfo,	2000 },
		{ InvalidFn,		2000 },	/* ServerInfo	*/
		{ InvalidFn,		2000 }, /* Delete	*/
		{ InvalidFn,		2000 }, /* Rename	*/
		{ InvalidFn,		2000 }, /* Link		*/
		{ InvalidFn,		2000 }, /* Protect	*/
		{ Clock_SetDate,	2000 },	/* SetDate	*/
		{ InvalidFn,		2000 }, /* Refine	*/
		{ InvalidFn,		2000 }, /* CloseObj	*/
		{ InvalidFn,		2000 },	/* Revoke	*/
		{ InvalidFn,		2000 }, /* Reserved1	*/
		{ InvalidFn,		2000 }  /* Reserved2	*/
	}
};
		
	
/**
*** Main: the familiar is linked with s0.o instead of c0.o, to save space.
*** Hence it needs to do its environment handling, via routine parse_args().
*** Next the link to the witch board is put into dumb mode and a link
*** guardian is started up as a separate process. There is no separate
*** polling process, unlike the Server Task, because there is not much point.
*** Hence the various servers can be started up immediately.
**/

int main(void)
{
  parse_args();

  init_mouse(); init_keyboard(); 
  
  take_over_link();

  init_clock();	/* involves interacting with witch, so afterwards */
  
  InitNode(&Keyboard_Root, "keyboard", Type_File, Flags_Interactive, DefFileMatrix);
  InitList(&Keyboard_Root.Contents);
  Keyboard_Root.Parent	= Null(DirNode);
  Keyboard_Info.ReqPort	= create_name("keyboard");
  Fork(2000, &Dispatch, sizeof(DispatchInfo *), &Keyboard_Info);
  
  InitNode(&Mouse_Root, "mouse", Type_File, Flags_Interactive, DefFileMatrix);
  InitList(&Mouse_Root.Contents);
  Mouse_Root.Parent	= Null(DirNode);
  Mouse_Info.ReqPort	= create_name("mouse");
  Fork(2000, &Dispatch, sizeof(DispatchInfo *), &Mouse_Info);
  
  InitNode(&Clock_Root, "clock", Type_Private, Flags_Interactive, DefFileMatrix);
  InitList(&Clock_Root.Contents);
  Clock_Root.Parent	= Null(DirNode);
  Clock_Info.ReqPort	= create_name("clock");
  Dispatch(&Clock_Info);
}

/**
*** Create an entry in the name table, and return the corresponding message
*** port. This should be called just before the appropriate dispatcher
*** is started/forked off.
**/
static Port create_name(char *name)
{ char		name_buf[IOCDataMax];
  Object	*this_processor;
  Object	*name_entry;
  NameInfo	info;
  Port		result = NullPort;
  
  MachineName(name_buf);
  this_processor = Locate(Null(Object), name_buf);
  if (this_processor eq Null(Object))
   { IOdebug("Familiar: failed to locate own processor");
     Exit(0x100);
   }
   
  info.Port	= result = NewPort();
  info.Flags	= Flags_StripName;
  info.Matrix	= DefNameMatrix;
  info.LoadData	= NULL;
  
  name_entry = Create(this_processor, name, Type_Name, sizeof(NameInfo),
  		(BYTE *) &info);
  Close(this_processor);
  if (name_entry eq Null(Object))
   { IOdebug("Familiar: failed to create name entry for %s", name);
     Exit(0x100);
   }
  Close(name_entry);
  return(result);
}

/**
*** Parse the arguments. At present there should be only one argument,
*** the link number.
**/

	/* Some default link to connect to the witch board. 	*/
	/* This assumes a 2-1 pipeline, with the witch board	*/
	/* doing the booting.					 */
#define	DefaultLink	1

static void parse_args(void)
{ Environ	env;
  char		*temp;
      
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOputs("familiar: failed to receive environment");
     Exit(0x100);
   }

  LinkToWitch	= DefaultLink;
     
  if (env.Argv[0] eq Null(char)) return;
  if ((env.Argv[1] eq Null(char)) || (env.Argv[1] eq (char *) MinInt))
   return;
  temp = env.Argv[1];
  if ((temp[0] < '0') || (temp[0] > '3') || (temp[1] ne '\0'))
   { IOdebug("Familiar: invalid link %s specified", temp);
     Exit(0x100);
   }
  LinkToWitch = temp[0] - '0';
}

/**
*** Take over the link to the witch board. This routine initialises
*** the various semaphores used to control access to the link, sets the
*** link into a sensible mode, and forks off the link guardian.
**/
static	void LinkGuardian(int link);

static	void take_over_link(void)
{ LinkInfo	info;
  LinkConf	conf;
  int		link = LinkToWitch;
  
  InitSemaphore(&LinkAvailable,  1);
  InitSemaphore(&Done,		 0);
  InitSemaphore(&ReplyAvailable, 0);
 
  if (LinkData(link, &info) < Err_Null)
   { IOdebug("familiar: failed to examine link %d to the witch board", link);
     Exit(0x100);
   }

  if (info.Mode eq Link_Mode_Intelligent)
   { IOdebug("familiar: link %d goes to another Helios node", link);
     Exit(0x100);
   }
   
  conf.Flags	= info.Flags;
  conf.Id	= info.Id;
  conf.Mode	= Link_Mode_Dumb;
  conf.State	= Link_State_Dumb;
  
  if (Configure(conf) < Err_Null)
   { IOdebug("familiar: failed to configure link %d to the witch board", link);
     Exit(0x100);
   }
   
  if (AllocLink(link) < Err_Null)
   { IOdebug("familiar: failed to allocate link %d to the witch board", link);
     Exit(0x100);
   }
   
  Fork(2000, &LinkGuardian, sizeof(int), link);  
}

/**
*** The Link Guardian process. This is similar to the one in the
*** server task, but rather simpler.
**/
static	void HandlePollMessage(int link);

static	void LinkGuardian(int link)
{ BYTE	Protocol[4];

  forever
   { if (LinkIn(1, link, Protocol, -1) < 1) continue;
     switch(Protocol[0])
      { case Pro_IOServ	: Signal(&ReplyAvailable);
      			  Wait(&Done);
      			  break;
      	case Pro_Poll	: HandlePollMessage(link);
      			  break;
      
        default		: /* HELP !!!! */
        		  break;
      }
   }
}


static void HandlePollMessage(int link)
{ Head		head;

  if (LinkIn(sizeofHead, link, (BYTE *) &head, -1) < sizeofHead)
   return;

  if (head.FnCode eq Poll_Mouse)
   { witch_mouse	*event = (witch_mouse *) &head;
     new_mouse(event->ButtonChange, (int) event->DX, (int) event->DY);
   }
  elif (head.FnCode eq Poll_RawKeyboard)
   { witch_keyboard	*event = (witch_keyboard *) &head;
     new_keyboard(event->Up, event->Scancode);
   }
  else
   return;
}

/**-----------------------------------------------------------------------------
*** Mouse and keyboard handling. These are all much the same.
*** 1) there is a lock to control access to the appropriate data structure
*** 2) there is a port to keep track of where the event messages should go
*** 3) there is a table to keep track of old events, and variables to
***    handle them.
*** The current version ignores Acknowledge, NegAcknowledge, and the like,
*** although the data is kept in a table so that these can be added.
@!% stylus events!!!
**/

typedef struct my_KeyboardEvent {
	WORD	Type;
	WORD	Counter;
	WORD	Stamp;
	WORD	Key;
	WORD	What;
} my_KeyboardEvent;

typedef struct my_MouseEvent {
	WORD	Type;
	WORD	Counter;
	WORD	Stamp;
	SHORT	X;
	SHORT	Y;
	WORD	Buttons;
} my_MouseEvent;

#define	KeytabSize	32
static	Semaphore	KeyboardLock;
static	Port		KeyboardPort	= NullPort;
static	word		KeyboardCounter = 1;
static	word		KeyboardTail	= 0;
static	word		KeyboardHead	= 0;
static	my_KeyboardEvent	Keytab[KeytabSize];

#define	MousetabSize	32
static	Semaphore	MouseLock;
static	Port		MousePort	= NullPort;
static	word		MouseCounter	= 1;
static	word		MouseTail	= 0;
static	word		MouseHead	= 0;
static	word		Mouse_X		= 16384;
static	word		Mouse_Y		= 16384;
static	my_MouseEvent	Mousetab[MousetabSize];

static	void init_keyboard()
{ int	i;

  InitSemaphore(&KeyboardLock, 1);
  for (i = 0; i < KeytabSize; i++)
   { Keytab[i].Type 	= Event_Keyboard;
     Keytab[i].Stamp	= 0;
   }
}

static void init_mouse()
{ int	i;

  InitSemaphore(&MouseLock, 1);
  for (i = 0; i < MousetabSize; i++)
   { Mousetab[i].Type	= Event_Mouse;
     Mousetab[i].Stamp	= 0;
   }
}

static void new_keyboard(bool up, int scancode)
{ MCB	m;

  Wait(&KeyboardLock);
  if (KeyboardPort eq NullPort) goto done;
  
  Keytab[KeyboardHead].Counter	= KeyboardCounter++;
  Keytab[KeyboardHead].Key	= scancode;
  Keytab[KeyboardHead].What	= up ? Keys_KeyUp : Keys_KeyDown;
  
  InitMCB(&m, MsgHdr_Flags_preserve, KeyboardPort, NullPort, 0);
  m.Data	    = (BYTE *) &(Keytab[KeyboardHead]);
  m.MsgHdr.DataSize = sizeof(my_KeyboardEvent);
  m.Timeout	    = 5 * OneSec;
  (void) PutMsg(&m);

  KeyboardHead = (KeyboardHead + 1) & (KeytabSize - 1);
  if (KeyboardHead eq KeyboardTail)
   KeyboardTail = (KeyboardTail + 1) & (KeytabSize - 1);
   
done:
  Signal(&KeyboardLock);  
}

static void new_mouse(int buttons, int dx, int dy)
{ MCB	m;

  Wait(&MouseLock);
  if (MousePort eq NullPort) goto done;

  Mousetab[MouseHead].Counter= MouseCounter++;

	/* Mouse button conversion. Only a single byte is used	*/
	/* when sending the event, but the formats are similar.	*/
	/* See ioevents.h for more info.			*/
  if ((buttons & 0x080) ne 0)
   { buttons &= 0x07F;
     buttons |= 0x00008000;
   }
  Mousetab[MouseHead].Buttons	= buttons;
  Mouse_X = (Mouse_X + dx) & 0x07FFF;
  Mouse_Y = (Mouse_Y + dy) & 0x07FFF;
  Mousetab[MouseHead].X = Mouse_X;
  Mousetab[MouseHead].Y = Mouse_Y;
       
  InitMCB(&m, MsgHdr_Flags_preserve + MsgHdr_Flags_sacrifice, MousePort,
  	 NullPort, 0);
  m.Data	    = (BYTE *) &(Mousetab[MouseHead]);
  m.MsgHdr.DataSize = sizeof(my_MouseEvent);
  m.Timeout	    = 2 * OneSec;
  (void) PutMsg(&m);

  MouseHead = (MouseHead + 1) & (MousetabSize - 1);
  if (MouseHead eq MouseTail)
   MouseTail = (MouseTail + 1) & (MousetabSize - 1);
   
done:
  Signal(&MouseLock);  
}

/**
*** Server library interfaces for the mouse and keyboard. These are
*** rather trivial.
**/
static void Keyboard_Open(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  MsgBuf	*r;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg2	*req		= (IOCMsg2 *) (m->Control);
  BYTE		*data		= m->Data;
  char		*pathname	= servinfo->Pathname;
  Port		stream_port;
  Port		my_event_port	= NullPort;
  
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  unless (f eq &Keyboard_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }
   
  FormOpenReply(r, m, f, Flags_Closeable | Flags_Interactive, pathname);
  r->mcb.MsgHdr.Reply = stream_port = NewPort();
  PutMsg(&r->mcb);
  Free(r);

  f->Account++;
  UnLockTarget(servinfo);
  forever
   { word	errcode;
   
     m->MsgHdr.Dest	= stream_port;
     m->Timeout		= StreamTimeout;
     m->Data		= data;
     
     errcode = GetMsg(m);
     m->MsgHdr.FnRc	= SS_Keyboard;
     
     if (errcode < Err_Null)
      { 	/* Event streams cannot time out if an event is enabled. */
        if (errcode eq EK_Timeout)
         { Wait(&KeyboardLock);  
           if ((KeyboardPort eq my_event_port) &&
               (KeyboardPort ne NullPort))
            { Signal(&KeyboardLock); continue; }
           Signal(&KeyboardLock);
           break;
         }
        errcode &= EC_Mask;
        if ((errcode eq EC_Error) || (errcode eq EC_Fatal))
         break;
        else
         continue;
      }
      
     if ((errcode & FC_Mask) ne FC_GSP)
      { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream); continue; }
      
     switch(errcode & FG_Mask)
      { case FG_Close	:
      		Wait(&KeyboardLock);
      		if ((KeyboardPort eq my_event_port) &&
      		    (KeyboardPort ne NullPort))
      		 { AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
      		   KeyboardPort = NullPort;
      		 }
      		Signal(&KeyboardLock);
      		if (m->MsgHdr.Reply ne NullPort)
      		 { m->MsgHdr.FnRc	= 0;
      		   ErrorMsg(m, Err_Null);
      		 }
      		FreePort(stream_port);
      		f->Account--;
      		return;
      		
      	case FG_EnableEvents :
 		{ WORD	mask = m->Control[0] & Event_Keyboard;
 		  
 	 	  Wait(&KeyboardLock);
 		  if (mask eq 0)	/* disable */
 		   { if ((KeyboardPort eq my_event_port) &&
 		         (KeyboardPort ne NullPort))
 		      { AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
 		      	KeyboardPort = my_event_port = NullPort;
 		      }
 		     InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
 		     MarshalWord(m, 0);
 		     PutMsg(m);
 		   }
 		  else
 		   { if (KeyboardPort ne NullPort)
 		      AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
 		     KeyboardPort = my_event_port = m->MsgHdr.Reply;
 		     InitMCB(m, MsgHdr_Flags_preserve, m->MsgHdr.Reply, NullPort, Err_Null);
 		     MarshalWord(m, mask);
 		     PutMsg(m);
 		   }
 		  Signal(&KeyboardLock);
 		  break;
 		}
 		
        /* BLV - add Acknowledge and NegAcknowledge sometime */
        default		: ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream);
        		  continue;
      }
   }
   
  f->Account--;
  FreePort(stream_port);
  Wait(&KeyboardLock);
  if ((KeyboardPort eq my_event_port) && (KeyboardPort ne NullPort))
   { AbortPort(KeyboardPort, EC_Error + SS_Keyboard + EG_Broken + EO_Stream);
     KeyboardPort = NullPort;
   }
  Signal(&KeyboardLock);
}

/**
*** And very similar code for the mouse
**/
static void Mouse_Open(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  MsgBuf	*r;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg2	*req		= (IOCMsg2 *) (m->Control);
  BYTE		*data		= m->Data;
  char		*pathname	= servinfo->Pathname;
  Port		stream_port;
  Port		my_event_port	= NullPort;
  
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  unless (f eq &Mouse_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }
   
  FormOpenReply(r, m, f, Flags_Closeable | Flags_Interactive, pathname);
  r->mcb.MsgHdr.Reply = stream_port = NewPort();
  PutMsg(&r->mcb);
  Free(r);

  f->Account++;
  UnLockTarget(servinfo);
  forever
   { word	errcode;
   
     m->MsgHdr.Dest	= stream_port;
     m->Timeout		= StreamTimeout;
     m->Data		= data;
     
     errcode = GetMsg(m);
     m->MsgHdr.FnRc	= SS_Pointer;
     
     if (errcode < Err_Null)
      { 	/* Event streams cannot time out if an event is enabled. */
        if (errcode eq EK_Timeout)
         { Wait(&MouseLock);  
           if ((MousePort eq my_event_port) &&
               (MousePort ne NullPort))
            { Signal(&MouseLock); continue; }
           Signal(&MouseLock);
           break;
         }
        errcode &= EC_Mask;
        if ((errcode eq EC_Error) || (errcode eq EC_Fatal))
         break;
        else
         continue;
      }
      
     if ((errcode & FC_Mask) ne FC_GSP)
      { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream); continue; }
      
     switch(errcode & FG_Mask)
      { case FG_Close	:
      		Wait(&MouseLock);
      		if ((MousePort eq my_event_port) &&
      		    (MousePort ne NullPort))
      		 { AbortPort(MousePort, EC_Error + SS_Pointer + EG_Broken + EO_Stream);
      		   MousePort = NullPort;
      		 }
      		Signal(&MouseLock);
      		if (m->MsgHdr.Reply ne NullPort)
      		 { m->MsgHdr.FnRc	= 0;
      		   ErrorMsg(m, Err_Null);
      		 }
      		FreePort(stream_port);
      		f->Account--;
      		return;
      		
      	case FG_EnableEvents :
 		{ WORD	mask = m->Control[0] & Event_Mouse;
 		  
 	 	  Wait(&MouseLock);
 		  if (mask eq 0)	/* disable */
 		   { if ((MousePort eq my_event_port) &&
 		         (MousePort ne NullPort))
 		      { AbortPort(MousePort, EC_Error + SS_Pointer + EG_Broken + EO_Stream);
 		      	MousePort = my_event_port = NullPort;
 		      }
 		     InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
 		     MarshalWord(m, 0);
 		     PutMsg(m);
 		   }
 		  else
 		   { if (MousePort ne NullPort)
 		      AbortPort(MousePort, EC_Error + SS_Pointer + EG_Broken + EO_Stream);
 		     MousePort = my_event_port = m->MsgHdr.Reply;
 		     InitMCB(m, MsgHdr_Flags_preserve, m->MsgHdr.Reply, NullPort, Err_Null);
 		     MarshalWord(m, mask);
 		     PutMsg(m);
 		   }
 		  Signal(&MouseLock);
 		  break;
 		}
 		
        /* BLV - add Acknowledge and NegAcknowledge sometime */
        default		: ErrorMsg(m, EC_Error + EG_WrongFn + EO_Stream);
        		  continue;
      }
   }
   
  f->Account--;
  FreePort(stream_port);
  Wait(&MouseLock);
  if ((MousePort eq my_event_port) && (MousePort ne NullPort))
   { AbortPort(MousePort, EC_Error + SS_Pointer + EG_Broken + EO_Stream);
     MousePort = NullPort;
   }
  Signal(&MouseLock);
}

/**-----------------------------------------------------------------------------
*** The clock device. This is relatively straightforward.
**/
static	word	startup_time = 0;

static void init_clock()
{ FullHead	Request;
  Head		Reply;
  
  Request.Protocol	= Pro_IOServ;
  Request.FnCode	= Fun_GetClock;
  Request.Extra		= 0;
  Request.HighSize	= 0;
  Request.LowSize	= 0;
  
  Wait(&LinkAvailable);
  (void) LinkOut(sizeofFullHead, LinkToWitch, (BYTE *) &Request, -1);

  Wait(&ReplyAvailable);
  (void) LinkIn(sizeofHead, LinkToWitch, (BYTE *) &Reply, -1);
  if (Reply.FnCode eq Rep_Success)
   (void) LinkIn(sizeof(word), LinkToWitch, (BYTE *) &startup_time, -1);

  Signal(&LinkAvailable);
  Signal(&Done);
}

static	void	Clock_ObjInfo(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg1	*req		= (IOCMsg1 *) (m->Control);
  ObjInfo	info;
    
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  unless(CheckMask(req->Common.Access.Access, AccMask_R))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  unless (f eq &Clock_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  info.DirEntry.Type	= Clock_Root.Type;
  info.DirEntry.Flags	= Clock_Root.Flags;
  info.DirEntry.Matrix	= Clock_Root.Matrix;
  strcpy(info.DirEntry.Name, Clock_Root.Name);
  info.Account		= 0;
  info.Size		= 0;
  info.Dates.Creation	= startup_time;
  
  { FullHead	Request;
    Head	Reply;
    
    Request.Protocol	= Pro_IOServ;
    Request.FnCode	= Fun_GetClock;
    Request.Extra	= 0;
    Request.HighSize	= 0;
    Request.LowSize	= 0;
    
    Wait(&LinkAvailable);
    LinkOut(sizeofFullHead, LinkToWitch, (BYTE *) &Request, -1);
    
    Wait(&ReplyAvailable);
    LinkIn(sizeofHead, LinkToWitch, (BYTE *) &Reply, -1);
    if (Reply.FnCode eq Rep_Success)
     LinkIn(sizeof(word), LinkToWitch, (BYTE *) &info.Dates.Modified, -1);
    else
     info.Dates.Modified = 0;
     
    Signal(&LinkAvailable);
    Signal(&Done);
  }
   
  info.Dates.Access	= info.Dates.Modified;

  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
  m->Data		= (BYTE *) &info;
  m->MsgHdr.DataSize	= sizeof(ObjInfo);
  PutMsg(m);
}

static	void	Clock_SetDate(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  DirNode	*d;
  ObjNode	*f;
  IOCMsg4	*req		= (IOCMsg4 *) (m->Control);
    
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  f = GetTargetObj(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }
   
  unless(CheckMask(req->Common.Access.Access, AccMask_W))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_File); return; }
   
  unless (f eq &Clock_Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }
   
  { FullHead	Request;
    Head	Reply;
    
    Request.Protocol	= Pro_IOServ;
    Request.FnCode	= Fun_SetClock;
    Request.Extra	= 0;
    Request.HighSize	= 0;
    Request.LowSize	= sizeof(Date);

    Wait(&LinkAvailable);
    LinkOut(sizeofFullHead, LinkToWitch, (BYTE *) &Request, -1);
    LinkOut(sizeof(Date), LinkToWitch, (BYTE *) &req->Dates.Modified, -1);
    
    Wait(&ReplyAvailable);
    LinkIn(sizeofHead, LinkToWitch, (BYTE *) &Reply, -1);
    
    Signal(&LinkAvailable);
    Signal(&Done);
  }
  
  m->MsgHdr.FnRc = 0;
  ErrorMsg(m, Err_Null);
}

