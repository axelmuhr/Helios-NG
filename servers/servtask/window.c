/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   S E R V E R  T A S K			--
--                 ----------------------------------		        --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      window.c                                                        --
--                                                                      --
--  Author:  BLV 5/9/88                                                 --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: window.c,v 1.1 1990/10/17 13:44:37 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 
/* SccsId: 1.5 3/5/90  Copyright (C) 1988, Perihelion Software Ltd. */

#include <helios.h>
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

#define GLOBAL extern
#include "servtask.h"
#include "window.h"
#include "keymap.h"
#include "version.h"

PRIVATE DirNode Logger_Root;
PRIVATE void Logger_Open(ServInfo *);
PRIVATE void Logger_Write(MCB *);

PRIVATE void Console_Open(ServInfo *);
PRIVATE void Console_Read(MCB *, Window *);
PRIVATE void Console_Write(MCB *, Window *);

#ifdef NEVER
DispatchInfo Console_Info = {
	Null(DirNode),
	NullPort,
	SS_IOProc,
	Machine_Name,
	{ NULL, 0 },
	{
		{ Console_Open,		4000 },
		{ InvalidFn,		2000 },
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ NullFn,		2000 },
		{ InvalidFn,		2000 },
		{ DoRename,		2000 },
		{ DoLink,		2000 },
		{ DoProtect,		2000 },
		{ DoSetDate,		2000 },
		{ DoRefine,		2000 },
		{ NullFn,		2000 }
	}
};
#endif

PRIVATE DirNode Window_Root;
PRIVATE void Window_Create(ServInfo *);
PRIVATE void Window_Delete(ServInfo *);

DispatchInfo Window_Info = {
	&Window_Root,
	NullPort,
	SS_IOProc,
	Machine_Name,
	{ NULL, 0},
	{
		{ Console_Open,		4000 },  /* useable */
		{ Window_Create,	2000 },
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ NullFn,		2000 },
		{ Window_Delete,	2000 },
		{ DoRename,		2000 },
		{ DoLink,		2000 },
		{ DoProtect,		2000 },
		{ DoSetDate,		2000 },
		{ DoRefine,		2000 },
		{ NullFn,		2000 }
	}
};
	
DispatchInfo Logger_Info = {
	&Logger_Root,
	NullPort,
	SS_IOProc,
	Machine_Name,
	{ NULL, 0 },
	{
		{ Logger_Open,		4000 },
		{ InvalidFn,		2000 },
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ NullFn,		2000 },
		{ InvalidFn,		2000 },
		{ DoRename,		2000 },
		{ DoLink,		2000 },
		{ DoProtect,		2000 },
		{ DoSetDate,		2000 },
		{ DoRefine,		2000 },
		{ NullFn,		2000 }
	}
};


PRIVATE Window ServerWindow;
PRIVATE Window *active_window;
PRIVATE WORD init_window(Window *);
PRIVATE void outputch(WORD, Window *);
PRIVATE WORD Init_Ansi(Window *, int, int);
PRIVATE void Ansi_Out(string, Window *);
PRIVATE void Tidy_Ansi(Screen *);
void write_to_log(string x);
PRIVATE void redraw_screen(Window *);
#define WINDOW_FORWARDS	   1
#define WINDOW_BACK        -1
#define WINDOW_REFRESH     0
void switch_window(int);
PRIVATE Semaphore windowing_lock;

/**
*** The main windowing initialisation routine.
**/
void InitialiseWindowing()
{
  InitSemaphore(&windowing_lock, 1);
  
  InitNode(&ServerWindow.Node, "Server Window", Type_File, 0, DefFileMatrix);
#ifdef NEVER
  Console_Info.Root = (DirNode *) &ServerWindow.Node;
#endif
  
  InitNode((ObjNode *) &Window_Root, "window", Type_Directory, 0, DefDirMatrix);
  InitList(&(Window_Root.Entries));
  Window_Root.Nentries = 0;
  
  (void) init_window(&ServerWindow);
  
  active_window = &ServerWindow;
  
  Ansi_Out("Helios ", &ServerWindow);
  Ansi_Out((Host eq DP2) ? "DP2" : "PC", &ServerWindow);
  Ansi_Out(" MiniServer ", &ServerWindow);
  Ansi_Out(version, &ServerWindow);
  Ansi_Out("\r\nCopyright (C) Perihelion Software Ltd. 1989\r\nAll rights reserved.\r\n", &ServerWindow);
  
}

/**
***
*** This routine initialises the windowing parts of a
*** Window structure. It does not worry about the ObjNode
*** sub-structure.
**/
PRIVATE word init_window(Window *window)
{ window->XOFF   = 0;
  window->head   = 0;
  window->tail   = 0;

  memset(&(window->attr), 0, sizeof(Attributes));
  AddAttribute   (&(window->attr), ConsoleEcho);
  RemoveAttribute(&(window->attr), ConsoleIgnoreBreak);
  AddAttribute   (&(window->attr), ConsoleBreakInterrupt);
  AddAttribute   (&(window->attr), ConsolePause);
  RemoveAttribute(&(window->attr), ConsoleRawInput);
  RemoveAttribute(&(window->attr), ConsoleRawOutput);
  window->attr.Min  = (Host eq PC) ? 25 : 20;
  window->attr.Time = 80;
  window->break_handler.port    = NullPort;
  window->break_handler.ownedby = 0L;
  window->stream_count          = 0;   
  window->cooker.count          = 0;
  InitSemaphore(&window->in_lock, 1);
  InitSemaphore(&window->out_lock,1);
  InitSemaphore(&window->cooker.lock, 1);
  InitSemaphore(&window->break_handler.lock, 1);
  
  return( Init_Ansi(window, (Host eq PC) ? 25 : 20, 80) );
}

/**
*** This routine creates a new window and adds it to a /window directory
**/
PRIVATE ObjNode *CreateWindow(MCB *mcb, DirNode *parent, string fullname)
{ Window  *window = Malloc(sizeof(Window));
  char    *name = fullname + strlen(fullname);
  ObjNode *f;
  static  int counter = 1;
  
  while (*name ne '/') name--; name++;
  
  f = Lookup(parent, name, true);
  if (f != NULL)
   { strcat(name, ".");
     addint(name, counter++);
   }
      
  if (window eq Null(Window))
   { mcb->MsgHdr.FnRc |= EC_Error + EG_NoMemory + EO_Window;
     return(Null(ObjNode));
   }
   
  if (!init_window(window))
   { Free(window);
     mcb->MsgHdr.FnRc |= EC_Error + EG_NoMemory + EO_Window;
     return(Null(ObjNode));
   }
   
  InitNode(&(window->Node), name, Type_Stream, Flags_Interactive, DefFileMatrix);
  
  Insert(parent, &(window->Node), TRUE);
  
  { Wait(&windowing_lock);
    active_window = window;
    switch_window(WINDOW_REFRESH);
    Signal(&windowing_lock);
  }  
   
  return(&(window->Node));
}

PRIVATE void delete_window(Window *window)
{ 
  Unlink(&(window->Node), TRUE);
  
  { Wait(&windowing_lock);
    if (window eq active_window)
     switch_window(WINDOW_BACK);
    Signal(&windowing_lock);
  }

  Tidy_Ansi(&window->screen);
  Free(window);
}

PRIVATE int DP2map[] = {
	Home,
	PageUp,
	End,
	PageDown,
	CursorLeft,
	CursorRight,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	ShiftF1,
	ShiftF2,
	ShiftF3,
	ShiftF4,
	ShiftF5,
	ShiftF6,
	ShiftF7,
	ShiftF8,
	ShiftF9,
	ShiftF10,
	AltShiftF1,
	AltShiftF2,
	AltShiftF3,
	AltShiftF4,
	AltShiftF5,
	AltShiftF6,
	AltShiftF7,
	AltShiftF8,
	AltShiftF9,
	AltShiftF10,
	ShiftCursorUp,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown2,
	Unknown3,
	Unknown4,
	ShiftCursorDown,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	AltF1,
	AltF2,
	AltF3,
	AltF4,
	AltF5,
	AltF6,
	AltF7,
	AltF8,
	AltF9,
	AltF10,
	CursorUp,
	Unknown5,
	Unknown6,
	Help,
	Unknown7,
	Unknown8,
	Unknown9,
	CursorDown
};

PRIVATE int PCmap[] = {
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	Unknown1,
	Unknown1,
	Home,
	CursorUp,
	PageUp,
	Unknown1,
	CursorLeft,
	Unknown1,
	CursorRight,
	Unknown1,
	End,
	CursorDown,
	PageDown,
	InsertKey,
	Delete,
	ShiftF1,
	ShiftF2,
	ShiftF3,
	ShiftF4,
	ShiftF5,
	ShiftF6,
	ShiftF7,
	ShiftF8,
	ShiftF9,
	ShiftF10,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	Unknown1,
	AltF1,
	AltF2,
	AltF3,
	AltF4,
	AltF5,
	AltF6,
	AltF7,
	AltF8,
	AltF9,
	AltF10
};

PRIVATE void ConsoleAddch(Window *window, BYTE ch);
PRIVATE void SendBreak(Port port);

void ConsoleNewchar(BYTE ch)
{ Window *window;

  Wait(&windowing_lock);

  window = active_window;
  Wait(&window->in_lock);
  
  if ((Host eq DP2) && (ch eq 0))
    ch = 0x03;
  
  if ((ch & 0x80) ne 0)
   { int  x;
     char *ptr;
     if (Host eq DP2)
      x = DP2map[ch - 0x00B1];
     else
      x = PCmap[ch - 0x0080];
      
     if (x eq AltF1)
      { switch_window(WINDOW_FORWARDS); goto done; }
     elif (x eq AltF2)
      { switch_window(WINDOW_BACK); goto done; }
     elif (x eq AltF3)
      { switch_window(WINDOW_REFRESH); goto done; }
     elif (x eq AltF5)
      { extern void do_sleep(void);
        Signal(&window->in_lock);
        Signal(&windowing_lock);
        Fork(1000, do_sleep, 0);
        return;
      }
 
     ptr = keymap[x];
     for ( ; *ptr ne '\0'; ptr++)
      ConsoleAddch(window, *ptr);
     goto done;
   }
   
  if ((ch eq 0x0013) || (ch eq 0x0011))   /* Deal with ctrl-S/ctrl-Q */
   { if (!IsAnAttribute(&window->attr, ConsolePause))
      goto addch;
     if (ch eq 0x0013)
       window->XOFF = TRUE;
     else
       window->XOFF = FALSE;
     goto done;
   }

  if (ch eq 0x0003)			/* Deal with ctrl-C */
   { if (IsAnAttribute(&window->attr, ConsoleIgnoreBreak))
       goto done;
     if (!IsAnAttribute(&window->attr, ConsoleBreakInterrupt))
       goto addch;
     if (window->break_handler.port ne NullPort)
	(void) Fork(Stacksize, SendBreak, 4, window->break_handler.port);
     goto done;
   }

addch:
/**
*** at this point I may have to do console key conversions.
**/
  ConsoleAddch(window, ch);

done:
  Signal(&window->in_lock);
  Signal(&windowing_lock);
}

PRIVATE void ConsoleAddch(Window *window, BYTE ch)
{ 
  window->Table[window->head] = (UBYTE) ch;
  window->head = (window->head + 1) & (Console_limit - 1);
  if (window->head eq window->tail)  /* buffer overflow */
   window->head = (window->head - 1) & (Console_limit - 1);
}

PRIVATE WORD ConsoleGetchar(Window *window)
{ WORD ret = 0;

  Wait(&window->in_lock);

  if (window->head ne window->tail)
   { ret = window->Table[window->tail]; 
     window->tail = (window->tail + 1) & (Console_limit - 1);
   }

  Signal(&window->in_lock);

  return(ret);
}

PRIVATE void SendBreak(Port port)
{ MCB     mcb;
  IOEvent myevent;

  mcb.MsgHdr.Flags	= MsgHdr_Flags_preserve;
  mcb.MsgHdr.ContSize	= 0;
  mcb.MsgHdr.DataSize	= sizeof(IOEvent);
  mcb.MsgHdr.FnRc	= EventRc_IgnoreLost;
  mcb.MsgHdr.Dest	= port;
  mcb.MsgHdr.Reply	= NullPort;
  mcb.Control		= Null(WORD);
  mcb.Data		= (BYTE *) &myevent;
  mcb.Timeout		= LongTimeout;
  myevent.Type		= Event_Break;
  myevent.Counter	= 0;
  myevent.Stamp		= GetDate();

  (void) PutMsg(&mcb);
}

void switch_window(int direction)
{
  switch(direction)
   { case WINDOW_REFRESH  : break;
   
     case WINDOW_FORWARDS : if (active_window eq &ServerWindow)
     			     { unless(EmptyList_(Window_Root.Entries))
                                active_window = Head_(Window, Window_Root.Entries);
                             }
                            else
                             { if (active_window eq Tail_(Window, Window_Root.Entries))
                                active_window = &ServerWindow;
                               else
                                active_window = Next_(Window, &active_window->Node.Node);
                             }
                            break;
     
     case WINDOW_BACK     : if (active_window eq &ServerWindow)
     			     { unless(EmptyList_(Window_Root.Entries))
     			        active_window = Tail_(Window, Window_Root.Entries);
     			     }
     			    else
     			     { if (active_window eq Head_(Window, Window_Root.Entries))
     			        active_window = &ServerWindow;
     			       else
     			        active_window = Prev_(Window, &active_window->Node.Node);
     			     }
     			    break;
   }

  redraw_screen(active_window);
}

/**
*** The console routines
**/

PRIVATE void Console_Open(ServInfo *servinfo)
{ WORD OwnCtrlC = 0;
  Window *window;
  MCB    *mcb   = servinfo->m;
  MsgBuf *r;
  DirNode *d;
  ObjNode *f;
  IOCMsg2 *req = (IOCMsg2 *) (mcb->Control);
  Port    StreamPort;
  BYTE    *data = mcb->Data;
  char    *pathname = servinfo->Pathname;
  
  d = (DirNode *) GetTargetDir(servinfo);
  if (d == Null(DirNode))
   { ErrorMsg(mcb, EO_Directory);
     return;
   }
   
  f = GetTargetObj(servinfo);
  if (f == Null(ObjNode))
   { if ( (d == (DirNode *) &ServerWindow) ||  /* running /console */
          ( (req->Arg.Mode & O_Create) == 0))  /* create bit clear */
      { ErrorMsg(mcb, EO_File);
        return;
      }
     mcb->MsgHdr.FnRc &= ~(EC_Mask + EG_Mask + EO_Mask);
     
     unless(CheckMask(req->Common.Access.Access, AccMask_W))
      { ErrorMsg(mcb, EC_Error + EG_Protected + EO_Directory);
        return;
      }
      
     if ((f = CreateWindow(mcb, d, pathname)) == Null(ObjNode))
      { ErrorMsg(mcb, 0);
        return;
      }
   }
  
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode) )
  { ErrorMsg(mcb, EC_Error+EG_Protected + EO_File);
    return;
  }  

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { ErrorMsg(mcb, EC_Error + EG_NoMemory);
     return;
   }

  FormOpenReply(r, mcb, f, Flags_Closeable | Flags_Interactive, pathname);
  if ((StreamPort = NewPort())  eq NullPort)
   { ErrorMsg(mcb, EC_Error + EG_Congested + EO_Port);
     return;
   }
  r->mcb.MsgHdr.Reply = StreamPort;
  
  PutMsg(&r->mcb);
  Free(r);

  if (f->Type eq Type_Directory)
   { DirServer(servinfo, mcb, StreamPort);
     FreePort(StreamPort);
     return;
   }
   
  f->Account++;
  window = (Window *) f;
    
  UnLockTarget(servinfo);

  forever
   { WORD errcode;
     mcb->MsgHdr.Dest	= StreamPort;
     mcb->Timeout	= StreamTimeout;
     mcb->Data		= data;

     errcode = GetMsg(mcb);

     if (errcode eq EK_Timeout)
      { 
        if (OwnCtrlC)
	 continue;
	else
	 break;
      }

     if (errcode < Err_Null) { continue; }
     
     if ((errcode & FC_Mask) ne FC_GSP)
      { 
        SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
        continue;
      }

     switch( errcode & FG_Mask )
      { case FG_Read	: Console_Read(mcb, window); break;

        case FG_Write	: Console_Write(mcb, window); break;

        case FG_Close	: 
                          if (OwnCtrlC)
			   { Wait(&window->break_handler.lock);
			     window->break_handler.ownedby = Null(WORD);
			     window->break_handler.port  = NullPort;
			     Signal(&window->break_handler.lock);
			   }
			  if (mcb->MsgHdr.Reply ne NullPort)
			   Return(mcb, ReplyOK, 0, 0, preserve);
			  FreePort(StreamPort);
			  f->Account--;
			  return;

	case FG_Seek	: mcb->Control[0] = 0;
			  Return(mcb, ReplyOK, 1, 0, preserve);
			  break;

	case FG_GetSize	: mcb->Control[0] = 0;
			  Return(mcb, ReplyOK, 1, 0, preserve);
			  break;

	case FG_SetSize	: Return(mcb, ReplyOK, 0, 0, preserve);
			  break;

	case FG_GetInfo	: Wait(&window->in_lock);
			  memcpy(mcb->Data, &window->attr, sizeof(Attributes));
			  Return(mcb, ReplyOK, 0, sizeof(Attributes), preserve);
			  Signal(&window->in_lock);
			  break;

	case FG_SetInfo	: Wait(&window->in_lock);
			  memcpy(&window->attr, mcb->Data, sizeof(Attributes));
			  Return(mcb, ReplyOK, 0, 0, preserve);
			  if (!IsAnAttribute(&window->attr, ConsolePause))
			   if (window->XOFF)
			    window->XOFF = FALSE;
			  Signal(&window->in_lock);
			  break;

	case FG_EnableEvents	:
			  { WORD mask = mcb->Control[0] & Event_Break;

			    Wait(&window->break_handler.lock);

			    if (mask eq 0)	/* Disabling ? */
			     { if (OwnCtrlC)	/* Yes, owner ? */
				{ window->break_handler.port    = NullPort;
				  window->break_handler.ownedby = Null(WORD);
				  OwnCtrlC = FALSE;
				}
			     }
			    else		/* No, so enabling */
			     { if (window->break_handler.ownedby ne Null(WORD))
				{ *(window->break_handler.ownedby) = 0;
				  FreePort(window->break_handler.port);
				}
			       window->break_handler.ownedby     = &OwnCtrlC;
			       OwnCtrlC                          = TRUE;
			       window->break_handler.port        = mcb->MsgHdr.Reply;
			     }
			    Signal(&window->break_handler.lock);

			    mcb->Control[0] = mask;
			    mcb->MsgHdr.Flags	 = (mask eq 0) ? 0 : 
							MsgHdr_Flags_preserve;
			    mcb->MsgHdr.ContSize = 1;
			    mcb->MsgHdr.DataSize = 0;
			    mcb->MsgHdr.Dest	 = mcb->MsgHdr.Reply;
			    mcb->MsgHdr.Reply	 = NullPort;
			    mcb->MsgHdr.FnRc	 = ReplyOK;
			    mcb->Timeout	 = 5 * OneSec;
			   (void) PutMsg(mcb);
			  }
			 break;

	case FG_Acknowledge	:	/* These two do nothing */
	case FG_NegAcknowledge	: break;

	default			: 
	                          SendError(mcb, EC_Error + SS_IOProc + 
					 EG_WrongFn + EO_File, preserve);
      }
   }

  f->Account--;
  FreePort(StreamPort);
}


PRIVATE void Window_Create(ServInfo *servinfo)
{ MCB		*m = servinfo->m;
  MsgBuf	*r;
  DirNode	*d;
  ObjNode	*f;
  IOCCreate	*req = (IOCCreate *) m->Control;
  char		*pathname = servinfo->Pathname;
    
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }
   
  unless( CheckMask(req->Common.Access.Access, AccMask_W))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Directory);
     return;
   }
   
  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Window);
     return;
   }

#ifdef never
  if ((f = GetTargetObj(servinfo)) != Null(ObjNode))
   { ErrorMsg(m, EC_Error + EG_Create + EO_Window);
     Free(r);
     return;
   }
#else
  f = GetTargetObj(servinfo);
#endif
      
  f = CreateWindow(m, d, pathname);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, 0);
     Free(r);
     return;
   }
   
  FormOpenReply(r, m, f, 0, pathname);
  
  PutMsg(&r->mcb);
  
  Free(r);
}

PRIVATE void Window_Delete(ServInfo *servinfo)
{ MCB *m	 = servinfo->m;
  ObjNode *f;
  IOCCommon *req = (IOCCommon *) m->Control;
  
  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_Window);
     return;
   }
   
  unless(CheckMask(req->Access.Access, AccMask_D))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Window);
     return;
   }
  
  if (f->Type ne  Type_Stream)
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Window);
     return;
   }
   
  if (f->Account > 0)
   { ErrorMsg(m, EC_Error + EG_InUse + EO_Window);
     return;
   }
   
  delete_window((Window *) f);
  ErrorMsg(m, Err_Null);
}

/**
*** Console_Read() extracts the read parameters from the message and from the
*** current attributes and calls a suitable routine to deal with the request.
*** This leaves most of the work to be done by raw and cooked_console_read().
**/

PRIVATE void raw_Console_read(MCB *, Window *, WORD, WORD);
PRIVATE void cooked_Console_read(MCB *, Window *, WORD, WORD);
PRIVATE int  empty_buffer(Window *);

PRIVATE void Console_Read(MCB *mcb, Window *window)
{ ReadWrite *readwrite = (ReadWrite *) &(mcb->Control[0]);
  WORD count, timelimit;

  count = readwrite->Size;
  
  if (count eq 0L)		/* special case */
    { Return(mcb, ReadRc_EOD, 0L, 0L, preserve);
      return;
    }
		/* I want a sensible limit on the amount read, 1 Kbyte reads */
		/* from a console are silly.				     */
  if (count >= IOCDataMax) count = IOCDataMax - 1;

  if (readwrite->Timeout eq -1L)	/* And work out the timelimit */
    timelimit = MaxInt;
  else
    timelimit = GetDate() + (readwrite->Timeout / (2 * OneSec));

  if (IsAnAttribute(&window->attr, ConsoleRawInput))
    raw_Console_read(mcb, window, count, timelimit);
  else
    cooked_Console_read(mcb, window, count, timelimit);
}
  
/**
*** In raw input mode, I read as many characters as I am asked for
*** or as there are available within the time limit. I perform no
*** editing of any sort, and echo all characters if the echo attribute is set.
**/

PRIVATE void raw_Console_read(MCB *mcb, Window *window, WORD count, WORD timelimit)
{ BYTE *buff = mcb->Data;
  WORD echo  = IsAnAttribute(&window->attr, ConsoleEcho);
  WORD i, temp;
        
  for (i = 0; i < count; )
   { if ((temp = empty_buffer(window)) ne 0)
      buff[i++] = (BYTE) temp;
     else
      break;
   }
  
  for (i = 0; i < count; )
   {                
       if ((temp = ConsoleGetchar(window)) ne 0)
        { buff[i++] = temp; 
          if (echo)       /* Should I echo the character ? */
            outputch(temp, window);
         }
       else
        { Delay(OneSec / 40);
          if (timelimit < GetDate())       /* Time limit exceeded ? */
           { 
             if (i > 0)                    /* Read any characters at all ? */
              { count = i; break; }
             else                           /* No, so return timeout. */
              { Return(mcb, EC_Recover + SS_IOProc + EG_Timeout + EO_Stream,
                               0, 0, preserve);
                return;
              }
           }
        }
     }
  Return(mcb, ReadRc_EOD, 0, count, preserve);
}

/**
***   Cooked input is rather more complicated. I am responsible
*** for all input processing, including cursor keys, backspace
*** and delete. Data cannot be sent until a carriage return has
*** been detected, because it might be erased. However, I must
*** echo it anyway so that the user can see what he is typing.
*** This means reading the data into a static buffer until
*** it is ready to be sent. To avoid horrible problems, I make sure that
*** there can be only one cooked read going on at any one time.
***
*** As the data is read in and processed I store it in cooked_buffer, and
*** I use cooked_count to keep track of my current position in this buffer.
*** Routine fill_buffer() is responsible for actually getting and processing
*** the keys, returning one of Cooked_EOF, _EOF or _Done. There is a hideous
*** problem in that a particular cooked read may have extracted and processed
*** lots of data without satisfying the request, leaving the data in the
*** buffer. If flag outstanding is set then this is the case. The data still
*** in the buffer may satisfy the current Read request, leaving some data still
*** in the buffer or completely emptying the buffer.
***
*** If the Read has not yet been satisfied by now, I start to extract data from
*** the buffer and process it. If no data is available the coroutine must
*** suspend itself, partly to release the buffer and partly in order to let
*** poll_the_devices() be called because that is the routine which will put
*** more data into the buffer. As for the rest of the code, it is too hideous
*** to describe.
**/

PRIVATE int  fill_buffer(Window *, WORD);

PRIVATE void cooked_Console_read(MCB *mcb, Window *window, WORD count, WORD timelimit)
{ WORD echo  = IsAnAttribute(&window->attr, ConsoleEcho);
  WORD temp;
  BYTE *buff = mcb->Data;
  Microwave *cooker = &window->cooker;
         
  Wait(&cooker->lock);

                   /* is there a newline already in the buffer ? */
  for (temp = 0; temp < cooker->count; temp++)
   if (cooker->buffer[temp] eq '\n')
    {     
      if (count >= temp+1)    /* should we send all of the data ? */
       { memcpy(buff, cooker->buffer, temp+1);
         Return(mcb, ReadRc_EOD, 0, temp+1, preserve);
         cooker->count -= (temp+1);
         if (cooker->count > 0)
          memmove(cooker->buffer, &(cooker->buffer[temp+1]),cooker->count);
         goto done;
       }
      else
       { memcpy(buff, cooker->buffer, count);
         Return(mcb, ReadRc_EOD, 0, count, preserve);
         memmove(cooker->buffer, &(cooker->buffer[count]),
                cooker->count - count); 
         cooker->count -= count;
         goto done;
       }    	
    }

		/* If I get here then there is not a complete line in the */
		/* buffer, so I have to do some more buffer filling.	  */
  forever
    { if (timelimit < GetDate())	/* Has time limit been exceeded ? */
       { 
         Return(mcb, EC_Recover + SS_IOProc + EG_Timeout + EO_Stream,
                        0, 0, preserve);
         break;
       }

      temp = fill_buffer(window, echo);       /* fill_buffer does the editing */
      
      if (temp eq Cooked_EOF)		/* Has ctrl-D been detected ? */
       {				/* Yes,send whatever data is buffered */
         if (cooker->count <= count)
          { memcpy(buff, cooker->buffer, cooker->count);
            Return(mcb, ReadRc_EOF, 0, cooker->count, preserve);
            cooker->count = 0;		        /* buffer now empty */
            break;
          }
         else
          { memcpy(buff, cooker->buffer, count);
            cooker->count -= count;
	    memmove(cooker->buffer, &(cooker->buffer[count]), cooker->count);
            Return(mcb, ReadRc_EOD, 0, count, preserve);
            break;
          }
       }

      if (temp eq Cooked_Done)             /* was there a carriage return ? */
        { 				   /* Yes */
          if (cooker->count <= count)
           { memcpy(buff, cooker->buffer, cooker->count); 
             Return(mcb, ReadRc_EOD, 0, cooker->count, preserve);
             cooker->count = 0;
             break;
           }
          else
           { memcpy(buff, cooker->buffer, count);
	     cooker->count -= count;
	     memmove(cooker->buffer, &(cooker->buffer[count]), cooker->count);
             Return(mcb, ReadRc_EOD, 0, count, preserve);
             break;
           }
        }
      else
	  Delay(OneSec / 40);	/* Wait for more data to arrive */
    }
done:
   Signal(&cooker->lock);
}


PRIVATE int empty_buffer(Window *window)
{ Microwave *cooker = &window->cooker;
  int result;
  
  if (cooker->count <= 0) return(0);
  result = cooker->buffer[0];
  if (--(cooker->count) > 0)
   memmove(cooker->buffer, &(cooker->buffer[1]), cooker->count);
  return(result);
}

           /* Routine fill_buffer takes data from the main console circular */
           /* buffer and performs local editing.                            */
PRIVATE int fill_buffer(window, echo)
Window *window;
WORD echo;
{ int curr_ch;
  Microwave *cooker = &(window->cooker);
  
  while ((curr_ch = ConsoleGetchar(window)) ne 0)
    switch(curr_ch)
      { case 0x0A :    /* treat carriage return and newline the same way... */
        case 0x0D : if (cooker->count eq Console_limit)
                     { if (echo)
                        Ansi_Out("\b\b \b\b", window);
                       cooker->count--;
                     }
                    cooker->buffer[cooker->count++] = '\n';
                    if (echo) Ansi_Out("\r\n", window);
                    return(Cooked_Done);

        case 0x04 :                     /*ctrl-D, EOF */
                    return(Cooked_EOF);

        case 0x08 : if (cooker->count)              /* backspace if possible */
                     { cooker->count--;
                       if (echo)
                         Ansi_Out("\b \b", window);   /* do the erase */
                     }
                   break;

        case 0x7F : cooker->count = 0;             /* erase line */
                    if (echo)
                     { PRIVATE BYTE clearline[4] =
			 { '\r', 0x9B, 0x4B, '\0' };
                       Ansi_Out(clearline, window);
                     }
                    break;

                   /* worry about the cursor keys later */

        default : if ((0x20 <= curr_ch) && (curr_ch < 0x007F) )
                   { if (cooker->count eq Console_limit)
                      { if (echo)
                         Ansi_Out("\b \b", window);
                        cooker->count--;
                      }
                     cooker->buffer[cooker->count++] = curr_ch;
                     if (echo) outputch(curr_ch, window);
                   }
      }

  return(Cooked_EOD);        /* no more characters in the buffer, so return */
}


/**
*** Write data to the console.
***
***  This is ridiculously complicated. The first problem is that there
*** are two types of write requests, single writes and multiple writes.
*** Console_Write takes care of this bit of the protocol, sending its
*** data to routine write_out. The routine also takes care of XON/XOFF,
*** and hence it is responsible for all the timing.
***
***  Data arrives at write_out() in fairly small lumps, so that the
*** server is not locked up when handling large writes to the console :
*** Console_write() suspends itself between lumps. There would be another
*** problem here if I allowed multiple messages, but I ensure that I only
*** ever get one data message. I just hope that nobody tries to write
*** more than 64K to the screen at any one time.
***
***  Writeout() checks which output mode the console is in at the moment
*** and performs raw/cooked editing. The data is then output via routine
*** Ansi_Out. Output() is used elsewhere to write data to the screen.
*** There is another routine outputch() to output a single character, and
*** this is used mainly for echoing.
***
***  Incidentally, the system would get extremely confused if anybody
***tried to output '\0', or if writes were interleaved in the wrong way.
**/


PRIVATE void write_out(BYTE *, WORD, bool, Window *window);

PRIVATE void Console_Write(MCB *mcb, Window *window)	/* handle a write to screen request */
{ bool rawmode = IsAnAttribute(&window->attr, ConsoleRawOutput);
  ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  WORD timeout, timelimit;
  WORD count, written, temp;
  BYTE *buff, *ptr;
  Port myport    = mcb->MsgHdr.Dest;
  Port itsport   = mcb->MsgHdr.Reply;
  bool ownbuf	 = FALSE;

  if ((count = readwrite->Size) eq 0L)		/* special case */
   { mcb->Control[0] = 0;
     Return(mcb, WriteRc_Done, 1, 0, preserve);
     return;
   }

  timeout = readwrite->Timeout;

  if (mcb->MsgHdr.DataSize ne 0)	/* Has the data arrived already ? */
    buff = mcb->Data;
  else					/* No, so I have to get the data */
    { WORD fetched   = 0;

      buff = Malloc(count);		/* Get hold of a buffer for it */
      if (buff eq (BYTE *) NULL)
       { SendError(mcb, EC_Error + SS_IOProc + EG_NoMemory + EO_Server, 
		   preserve);
         return;
       }

      ownbuf = TRUE;      

		/* The data is read in in lumps of upto MaxData, */
		/* and put into the same buffer. Once the buffer */
		/* has been filled I start printing it.          */

		/* Start by sending the initial message, giving sizes */
      mcb->Control[0] = (count > Message_Limit) ? Message_Limit : count;
      mcb->Control[1] = Message_Limit;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      mcb->MsgHdr.Dest  = itsport;
      mcb->MsgHdr.Reply = NullPort;
      mcb->MsgHdr.FnRc  = WriteRc_Sizes;
      mcb->MsgHdr.ContSize = 2;
      mcb->MsgHdr.DataSize = 0;
      mcb->Timeout	   = timeout;
      (void) PutMsg(mcb);

      ptr = buff;

		/* Now wait for all the data */
      while (fetched < count)
       { mcb->MsgHdr.Dest = myport;
	 mcb->Data	  = ptr;
	 mcb->Timeout	  = timeout;

	 if (GetMsg(mcb) < 0)	/* Help !!!!!!!! */
	  { if (ownbuf)		/* Just go back to waiting for GSP request */
	     Free(buff);
	    return;
	  }
	 
         fetched   += (word) mcb->MsgHdr.DataSize;
         ptr       += mcb->MsgHdr.DataSize;           /* next bit of buffer */
       }

    }

        /* When I get here, all the data has arrived from the	*/
        /* transputer. I need to display count characters,	*/
        /* starting at location buff				*/
  mcb->Control[0]   = count;
  mcb->MsgHdr.Reply = itsport;

  Return(mcb, WriteRc_Done, 1, 0, preserve);

        /* I have a large outer loop, and each time around my loop */
        /* I check for XOFF and display 40 chars.                  */
        /* I use pointer currptr to keep track of how far I am in  */
        /* the buffer, and word written contains how much written  */

   ptr = buff; written = 0L; timelimit = GetDate() + (timeout / OneSec);

   Wait(&window->out_lock);
   while (written < count)
    {                                       /* Start by worrying about XOFF */
      while (window->XOFF)
       Delay(OneSec);

     temp = ((count - written) < Console_Max) ? (count - written) : Console_Max;
     write_out(ptr, temp, rawmode, window);
     ptr = ptr + temp;
     written += temp;
   }

               /* tidy up everything */
  if (ownbuf)
    Free(buff);
  Signal(&window->out_lock);
               /* Finished !!! */
}     


    /* Routine write_out() takes a fairly small amount of data, processes */
    /* it as per the current output mode (raw/cooked), and outputs it.    */
PRIVATE void write_out(BYTE *data, WORD amount, bool rawmode, Window *window)
{ BYTE *newdata, tempbuf[120];		/* Allow three chars for every one */
					/* supplied */
  newdata = &(tempbuf[0]);

  if (rawmode)
    while (amount--) *newdata++ = *data++;
  else
    { 
      while (amount--)
       { if (((32 <= *data) && (*data <= 126)) ||    /* printable chars */
             (( 0x07 <= *data) && (*data <= 0x0D)))  /* usual control chars */
           *newdata++ = *data;
         if (*data eq 0x0A) *newdata++ = 0x0D;	/* '\n' -> "\n\r" */
         data++;
       }
    }

  *newdata = '\0';
  Ansi_Out(&(tempbuf[0]), window);
}

            /* Output a single character, used mainly for echoing */
            /* I want to go via Ansi_Out, to handle ANSI-VT52     */
            /* conversions etc, even though it seems a little bit */
            /* silly.                                             */
PRIVATE void outputch(WORD ch, Window *window)
{ BYTE temp[4];

  temp[0] = ch; temp[1] = '\0';
  Ansi_Out(&temp[0], window);
}

/**
*** The error logger. Under the miniserver/servertask system at present this
*** is a write-only device, and the output always goes to the server window.
*** At some future stage this may be upgraded to support logging to file,
*** reading the logfile, etc. However, that does not make much sense until
*** I have multiple windows and facilities to switch between windows and
*** enable debugging etc.
**/

PRIVATE void Logger_Open(ServInfo *servinfo)
{ MCB    *mcb   = servinfo->m;
  MsgBuf *r;
  DirNode *d;
  ObjNode *f;
  IOCMsg2 *req = (IOCMsg2 *) (mcb->Control);
  Port    StreamPort;
  BYTE    *data = mcb->Data;
  char    *pathname = servinfo->Pathname;
  
  d = (DirNode *) GetTargetDir(servinfo);
  if (d == Null(DirNode))
   { ErrorMsg(mcb, EO_Directory);
     return;
   }
   
  f = GetTargetObj(servinfo);
  if (f == Null(ObjNode))
  { ErrorMsg(mcb, EO_File);
    return;
  }
  
  unless(CheckMask(req->Common.Access.Access, req->Arg.Mode & Flags_Mode) )
  { ErrorMsg(mcb, EC_Error+EG_Protected + EO_File);
    return;
  }  

  r = New(MsgBuf);
  if (r == Null(MsgBuf))
   { ErrorMsg(mcb, EC_Error + EG_NoMemory);
     return;
   }

  FormOpenReply(r, mcb, f, Flags_Closeable | Flags_Interactive | Flags_MSdos,
                pathname);
  if ((StreamPort = NewPort())  eq NullPort)
     { ErrorMsg(mcb, EC_Error + EG_Congested + EO_Port);
     return;
   }
  r->mcb.MsgHdr.Reply = StreamPort;
  
  PutMsg(&r->mcb);
  Free(r);

  f->Account++;
    
  UnLockTarget(servinfo);
  
  forever
   { WORD errcode;
     mcb->MsgHdr.Dest	= StreamPort;
     mcb->Timeout	= StreamTimeout;
     mcb->Data		= data;

     errcode = GetMsg(mcb);

     if (errcode eq EK_Timeout)
      break;

     if (errcode < Err_Null) { continue; }
     
     if ((errcode & FC_Mask) ne FC_GSP)
      { 
        SendError(mcb, EC_Error + SS_IOProc + EG_WrongFn + EO_Stream, preserve);
        continue;
      }

     switch( errcode & FG_Mask )
      { case FG_Read	: Return(mcb, ReadRc_EOF, 0, 0, preserve);
                          break;

        case FG_Write	: Logger_Write(mcb); break;

        case FG_Close	: 
			  if (mcb->MsgHdr.Reply ne NullPort)
			   Return(mcb, ReplyOK, 0, 0, preserve);
			  FreePort(StreamPort);
			  f->Account--;
			  return;

	case FG_Seek	: mcb->Control[0] = 0;
			  Return(mcb, ReplyOK, 1, 0, preserve);
			  break;

	case FG_GetSize	: mcb->Control[0] = 0;
			  Return(mcb, ReplyOK, 1, 0, preserve);
			  break;

	case FG_SetSize	: Return(mcb, ReplyOK, 0, 0, preserve);
			  break;

	case FG_Acknowledge	:	/* These two do nothing */
	case FG_NegAcknowledge	: break;

	case FG_GetInfo	:
	case FG_SetInfo	:
	case FG_EnableEvents	: 
	default			: 
	                          SendError(mcb, EC_Error + SS_IOProc + 
					 EG_WrongFn + EO_File, preserve);
      }
   }

  f->Account--;
  FreePort(StreamPort);
}

PRIVATE void Logger_Write(MCB *mcb)
{ ReadWrite *readwrite = (ReadWrite *) mcb->Control;
  BYTE *buffer, *ptr;
  bool ownbuf	= FALSE;
  Port itsport	= mcb->MsgHdr.Reply, myport = mcb->MsgHdr.Dest;
  WORD timeout, count, written, temp;

  if (readwrite->Size eq 0)
    { mcb->Control[0] = 0;
      Return(mcb, ReplyOK, 1, 0, preserve);
      return;
    }
  else
   count = readwrite->Size;

  timeout = readwrite->Timeout;

  if (mcb->MsgHdr.DataSize ne 0)
   buffer = mcb->Data;
  elif ((buffer = (BYTE *) Malloc(count+1)) eq Null(BYTE))
    { SendError(mcb, EC_Warn + SS_IOProc + EG_NoMemory + EO_Server, preserve);
      return;
    }
  else
    { WORD fetched   = 0;
      ownbuf = TRUE;      

		/* The data is read in in lumps of upto Message_Limit, */
		/* and put into the same buffer. Once the buffer */
		/* has been filled I can start writing it.       */

		/* Start by sending the initial message, giving sizes */
      mcb->Control[0] = (count > Message_Limit) ? Message_Limit : count;
      mcb->Control[1] = Message_Limit;
      mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
      mcb->MsgHdr.Dest  = itsport;
      mcb->MsgHdr.Reply = NullPort;
      mcb->MsgHdr.FnRc  = WriteRc_Sizes;
      mcb->MsgHdr.ContSize = 2;
      mcb->MsgHdr.DataSize = 0;
      mcb->Timeout	   = timeout;
      (void) PutMsg(mcb);

      ptr = buffer;

		/* Now wait for all the data */
      while (fetched < count)
       { mcb->MsgHdr.Dest = myport;
	 mcb->Data	  = ptr;
	 mcb->Timeout	  = timeout;

	 if (GetMsg(mcb) < 0)	/* Help !!!!!!!! */
	  { if (ownbuf)		/* Just go back to waiting for GSP request */
	     Free(buffer);
	    return;
	  }
	 
         fetched   += (word) mcb->MsgHdr.DataSize;
         ptr       += mcb->MsgHdr.DataSize;           /* next bit of buffer */
       }
    }

   ptr = buffer; written = 0;
   
   while (written < count)
    { int x;
      temp = ((count - written) < Console_Max) ? (count - written) : Console_Max;
      x = ptr[temp]; ptr[temp] = '\0';
      write_to_log(ptr);
      ptr[temp] = x;
      ptr = ptr + temp;
      written += temp;
   }

                /* When I get here, I have written WRITTEN */
                /* chars within the time available */
                /* So inform the transputer. */
  mcb->Control[0] = written;
  mcb->MsgHdr.Reply = itsport;

  Return(mcb, WriteRc_Done, 1, 0, preserve);

  if (ownbuf) Free(buffer);
}

void write_to_log(string x)
{ Ansi_Out(x, &ServerWindow);
}

/**
*** A general purpose ANSI terminal emulator, which can be easily modified
*** to deal with particular terminals.
**/

/**
*** Internal variables used to keep track of the screen :
***
*** map is an array used to store what the screen is supposed to look like. 
*** It can be indexed by screen[row][col].
***
*** Rows and Cols give the size of the screen, assumed constant. e.g. 25 and 80.
***
*** Cur_x and Cur_Y give the current position, in the range 0 <= Cur_x < Cols,
*** and 0 <= Cur_y < Rows. This means that internally my top left corner is at
*** ( 0, 0) whereas ANSI expects ( 1, 1)...
***
*** Screen_mode holds the current display mode.
***/

PRIVATE BYTE **map;
PRIVATE Cur_x, Cur_y, LastRow, LastCol;
#define Mode_Plain	0
#define Mode_Bold	0x0001
#define Mode_Italic	0x0002
#define Mode_Underline	0x0004
#define Mode_Inverse	0x0008
PRIVATE int Screen_mode;
PRIVATE Window *current_window;
PRIVATE Screen *current_screen;
PRIVATE int terminal_wraps = FALSE;

/**
*** Here are the routines corresponding to the ANSI sequences.
**/
PRIVATE void add_ch(int ch);
PRIVATE void insert_chars(int count);
PRIVATE void cursor_up(int count);
PRIVATE void cursor_down(int count);
PRIVATE void cursor_right(int count);
PRIVATE void cursor_left(int count);
PRIVATE void cursor_next(int count);
PRIVATE void cursor_prev(int count);
PRIVATE void move_cursor(int y, int x);
PRIVATE void erase_endofscreen(void);
PRIVATE void erase_endofline(void);
PRIVATE void insert_line(void);
PRIVATE void delete_line(void);
PRIVATE void delete_char(int count);
PRIVATE void scroll_up(int count);
PRIVATE void scroll_down(int count);
PRIVATE void set_rendition(void);

/**
*** These are the functions which have to do the work. They are described below.
**/
PRIVATE void clear_screen(void);
PRIVATE void clear_eol(void);
PRIVATE void move_to(int y, int x);
PRIVATE void refresh_screen(int y, int x);
PRIVATE void backspace(void);
PRIVATE void carriage_return(void);
PRIVATE void linefeed(void);
PRIVATE void set_mark(void);
PRIVATE void use_mark(void);
PRIVATE void foreground(int colour);
PRIVATE void background(int colour);
PRIVATE void set_bold(int flag);
PRIVATE void set_italic(int flag);
PRIVATE void set_underline(int flag);
PRIVATE void set_inverse(int flag);
PRIVATE void ring_bell(void);

/**
*** These routines are responsible for actually sending characters to the output
*** device. They do some buffering.
**/
PRIVATE void send_ch(int ch);
PRIVATE void flush(void);

/**
*** Init_Ansi() should be called during the initialisation process. Note that
*** it clears the screen, so that I know exactly what is on the screen when I
*** start up.
**/

PRIVATE void reset_escape(Screen *);

PRIVATE WORD Init_Ansi(window, No_Rows, No_Cols)
Window *window;
int No_Rows, No_Cols;
{ int i;
  BYTE **map, *whole_screen;
  Screen *screen = &(window->screen);
  
  if ((map = (BYTE **) Malloc(No_Rows * sizeof(BYTE *))) eq (BYTE **) NULL)
    return(FALSE);

  if ((whole_screen = (BYTE *) Malloc(No_Rows * No_Cols)) eq (BYTE *) NULL)
   { Free(map); return(FALSE); }
   
  map[0] = whole_screen;
  for (i = 1; i < No_Rows; i++)
   map[i] = map[i-1] + No_Cols;
   
  screen->map		= map;
  screen->whole_screen	= whole_screen;
  screen->Rows		= No_Rows;
  screen->Cols		= No_Cols;
  screen->mode		= Mode_Plain;
  reset_escape(screen);
  
  { static char cls[2] = { 0x0C, 0x00 };
    Ansi_Out(&(cls[0]), window);
  }
  
  return(TRUE);
}

PRIVATE void Tidy_Ansi(screen)
Screen *screen;
{ 
  Free(screen->whole_screen); Free(screen->map);
}


PRIVATE void reset_escape(Screen *screen)
{ screen->flags &= ~(ANSI_escape_found | ANSI_in_escape);
  screen->flags |= ANSI_firstdigit;
  screen->args[0] = 0; screen->args[1] = 0; screen->args[2] = 0;
  screen->args[3] = 0; screen->args[4] = 0;
  screen->args_ptr = &(screen->args[0]); screen->gotdigits = 0;
}

extern void write_to_screen(string, int *);

PRIVATE void Ansi_Out(itsstr, window)
STRING itsstr;
Window *window;
{ Screen *screen;
  UBYTE  *str = (UBYTE *) itsstr;
  
  Wait(&windowing_lock);
  
  screen	 = &(window->screen);
  Cur_x          = screen->Cur_x;
  Cur_y          = screen->Cur_y;
  map            = screen->map;
  LastRow        = screen->Rows - 1;
  LastCol        = screen->Cols - 1;
  Screen_mode    = screen->mode;
  current_window = window;
  current_screen = screen;

  if (current_window ne active_window)
   if (current_window eq &ServerWindow & !windows_nopop)
    { active_window = &ServerWindow;
      redraw_screen(&ServerWindow);
    }
    
  for ( ; *str ne '\0'; str++)
   { if (screen->flags & ANSI_in_escape)
      switch(*str)
       {		/* If it is a digit, it is part of the current arg */
	 case '0': case '1': case '2': case '3': case '4':
	 case '5': case '6': case '7': case '8': case '9':
		*(screen->args_ptr) = (10 * *(screen->args_ptr)) + (*str - '0');
		if (screen->flags & ANSI_firstdigit)
		  { screen->gotdigits++;
		    screen->flags &= ~ANSI_firstdigit;
		  }
		break;

		/* A semicolon marks the end of the current arg.	*/
	 case ';': screen->args_ptr++;
	           screen->flags |= ANSI_firstdigit;
	           break;

	 case '@':	insert_chars((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'A':	cursor_up((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'B':	cursor_down((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'C':	cursor_right((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'D':	cursor_left((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'E':	cursor_next((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'F':	cursor_prev((screen->gotdigits) ? screen->args[0] : 1);
	                goto end;

	 case 'H':	move_cursor( (screen->gotdigits > 0) ? (screen->args[0] - 1) : 0,
				     (screen->gotdigits > 1) ? (screen->args[1] - 1) : 0);
		        goto end;

	 case 'J':	erase_endofscreen();	goto end;

	 case 'K':	erase_endofline();	goto end;

	 case 'L':	insert_line();		goto end;

	 case 'M':	delete_line();		goto end;

	 case 'P':	delete_char((screen->gotdigits) ? screen->args[0] : 1); goto end;

	 case 'S':	scroll_up((screen->gotdigits) ? screen->args[0] : 1);   goto end;

	 case 'T':	scroll_down((screen->gotdigits) ? screen->args[0] : 1); goto end;

	 case 'm':	set_rendition();
	 default :

end:
		   reset_escape(screen);
       }
     else	/* Not in escape sequence... yet */
      { if ((screen->flags & ANSI_escape_found) && (*str eq '['))
         { screen->flags |= ANSI_in_escape;
           screen->flags &= ~ANSI_escape_found;
           continue;
         }

        screen->flags &= ~ANSI_escape_found;
        switch(*str)
         {		/* Line feed only : record terminator should have */
         		/* expanded already.				  */
	   case '\n' :	linefeed(); break;

			/* Single carriage return */
	   case '\r' :	carriage_return(); break;

			/* Bell */
	   case 0x07 :  ring_bell(); break;
	   
			/* Form Feed */
	   case 0x0C :	clear_screen(); break;

			/* Backspace */
	   case 0x08 :	backspace(); break;

			/* Vertical tab */
	   case 0x0B :	cursor_up(1); break;

			/* CSI */
	   case 0x009B :
           case 0xFF9B : screen->flags |= ANSI_in_escape;
                         break;

			/* possibly part of CSI */
	   case 0x1B : screen->flags |= ANSI_escape_found; break;

			/* Horizontal tab */
	   case 0x09 : { int i = 8 - (Cur_x % 8);
                         if ((Cur_x + i) > LastCol)
                          { carriage_return(); linefeed(); }
                         else
			  move_to(Cur_y, Cur_x + i);
			 break;
		       }

			/* Something else - is it printable ? */
	  default    : if (*str >= ' ') add_ch(*str);
        }
      }
   }

  flush();
  screen->Cur_x = Cur_x;
  screen->Cur_y = Cur_y;
  screen->mode  = Screen_mode;
  Signal(&windowing_lock);
}

/**
***
**/
PRIVATE void insert_chars(count)
int count;
{ register int i;

  if ((Cur_x + count) > LastCol)  /* wipe to eol */
   { clear_eol();
     for (i = Cur_x; i <= LastCol; i++)
      map[Cur_y][i] = ' ';
     return;
   }

  for (i = LastCol; i >= (Cur_x + count); i--)
    map[Cur_y][i] = map[Cur_y][i - count];
  for (i = 0; i < count; i++)
    map[Cur_y][Cur_x+i] = ' ';

  set_mark();

  for (i = 0; i < count; i++)
    send_ch(' ');

  if (terminal_wraps && (Cur_y eq LastRow))
   { for (i = Cur_x; i < (LastCol - count); i++)
      send_ch(map[Cur_y][i]);
     current_screen->flags |= ANSI_dirty;
   }
  else
   for (i = Cur_x; i < (LastCol + 1 - count); i++)
    send_ch(map[Cur_y][i]);

  use_mark();
}

PRIVATE void cursor_up(count)
int count;
{ Cur_y -= count;
  if (Cur_y < 0) Cur_y = 0;
  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_down(count)
int count;
{ 
  Cur_y += count;
  if (Cur_y > LastRow) Cur_y = LastRow;

  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_right(count)
int count;
{
  Cur_x += count;
  if (Cur_x > LastCol) Cur_x = LastCol;

  move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_left(count)
int count;
{ Cur_x -= count;
  if (Cur_x < 0) Cur_x = 0;

  if (Cur_x eq 0)
   carriage_return();
  else
   move_to(Cur_y, Cur_x);
}

PRIVATE void cursor_next(count)
int count;
{ register int i;
  carriage_return();
  for (i = 0; i < count; i++) linefeed();
}

PRIVATE void cursor_prev(count)
int count;
{ Cur_y -= count; Cur_x = 0;
  if (Cur_y < 0) Cur_y = 0;
  move_to(Cur_y, Cur_x);
}

PRIVATE void move_cursor(y, x)
int y,x;
{ 
  if (x < 0) x = 0;
  if (x > LastCol) x = LastCol;
  if (y < 0) y = 0;
  if (y > LastRow) y = LastRow;
  move_to(y, x);
}

PRIVATE void erase_endofscreen()
{ register int i, j;

  set_mark();

  for (i = Cur_y; i <= LastRow; i++)
   for (j = (i eq Cur_y) ? Cur_x : 0 ; j <= LastCol; j++)
    map[i][j] = ' ';

  if ((Cur_x eq 0) && (Cur_y eq 0))
    clear_screen();
  else
    refresh_screen(Cur_y, LastRow);

  use_mark();
}

PRIVATE void erase_endofline()
{ register int i;

  for (i = Cur_x; i <= LastCol; i++)
   map[Cur_y][i] = ' ';

  clear_eol();
}

PRIVATE void insert_line()
{ register int i;
  register BYTE *petr = map[LastRow];

  set_mark();

  for (i = LastRow; i > Cur_y; i--)
   map[i] = map[i-1];
  map[Cur_y] = petr;
  for (i = 0; i <= LastCol; i++) petr[i] = ' ';  
    
  refresh_screen(Cur_y, LastRow);  

  use_mark();
}

PRIVATE void delete_line()
{ register BYTE *petr = map[Cur_y];
  register int i;

  set_mark();
  
  for (i = Cur_y; i < LastRow; i++)
   map[i] = map[i+1];

  map[LastRow] = petr;

  for (i = 0; i <= LastCol; i++) petr[i] = ' ';

  refresh_screen(Cur_y, LastRow);

  use_mark();
}

PRIVATE void delete_char(count)
int count;
{ register int i;
  register char ch;

  set_mark();

  if ((Cur_x + count) > (LastCol + 1) ) count = LastCol + 1 - Cur_x;
  
  for (i = Cur_x; i < (LastCol + 1 - count); i++)
   { ch = map[Cur_y][i + count];
     send_ch(ch); map[Cur_y][i] = ch;
   }

  if (terminal_wraps && (Cur_y eq LastRow))
   { for (i = 0; i < (count - 1); i++)
      { send_ch(' '); map[LastRow][LastCol + i - count] = ' '; }
     map[LastRow][LastCol] = ' ';
         /* no need for dirty : it is blank anyway */
     current_screen->flags &= ~ANSI_dirty;
   }
  else   
   for (i = 0; i < count; i++)
    { send_ch(' '); map[Cur_y][LastCol - i] = ' '; }
   
  use_mark();
}

PRIVATE void scroll_up(count)
int count;
{ int i;

  set_mark();
  if (count > LastRow) { clear_screen(); use_mark(); return; }

  move_to(LastRow, 0);
  for (i = 0; i < count; i++) linefeed();
  use_mark();
}

PRIVATE void scroll_down(count)
int count;
{ int i;
  register int j;
  register BYTE *tempptr;
  
  set_mark();
  
  if (count > LastRow)
   clear_screen();
  else
   { for (i = 0; i < count; i++)
      { tempptr = map[LastRow];
        for (j = LastRow; j > 0; j--)
          map[j] = map[j-1];
        map[0] = tempptr;
      }
      
     for (i = 0; i < count; i++)
       for (j = 0; j <= LastCol; j++)
         map[i][j] = ' ';
     refresh_screen(0, LastRow);
   }
   
  use_mark();
}

PRIVATE void set_rendition()
{ int i;
  Screen *screen = current_screen;
  
  for (i = 0; i < screen->gotdigits; i++)
   { 
     if ( (screen->args[i] >= 30) && (screen->args[i] <= 37))
      foreground(screen->args[i] - 30);
     elif ( (screen->args[i] >= 40) && (screen->args[i] <= 47))
      background(screen->args[i] - 30);
     elif (screen->args[i] eq 7)
      { if (!(Screen_mode & Mode_Inverse))
         { set_inverse(1); Screen_mode |= Mode_Inverse; }
      }
     elif (screen->args[i] eq 4)
      { if (!(Screen_mode & Mode_Underline))
         { set_underline(1); Screen_mode |= Mode_Underline; }
      }
     elif (screen->args[i] eq 3)
      { if (!(Screen_mode & Mode_Italic))
         { set_italic(1); Screen_mode |= Mode_Italic; }
      }
     elif (screen->args[i] eq 1)
      { if (!(Screen_mode & Mode_Bold))
         { set_bold(1); Screen_mode |= Mode_Bold; }
      }
     elif (screen->args[i] eq 0)
      { if (Screen_mode & Mode_Bold)
         { set_bold(0); Screen_mode &= ~Mode_Bold; }
        if (Screen_mode & Mode_Italic)
         { set_italic(0); Screen_mode &= ~Mode_Italic; }
        if (Screen_mode & Mode_Underline)
         { set_underline(0); Screen_mode &= ~Mode_Underline; }
        if (Screen_mode & Mode_Inverse)
         { set_inverse(0); Screen_mode &= ~Mode_Inverse; }
      }
   }
}

PRIVATE void add_ch(ch)
int ch;
{ if (terminal_wraps && (Cur_x eq LastCol))
   { if (Cur_y eq LastRow)
      { map[Cur_y][Cur_x] = (BYTE) ch;
        current_screen->flags |= ANSI_dirty;
        Cur_x++;
        return;
      }
     map[Cur_y][Cur_x] = (BYTE) ch;
     send_ch(ch);
     move_to(Cur_y, LastCol);
     Cur_x++;
     return; 
   }

  if (Cur_x > LastCol)     /* Implicit wrap-around */
   { carriage_return(); linefeed(); }

  map[Cur_y][Cur_x] = (BYTE) ch; send_ch(ch); Cur_x++;
}


/**
*** Here are the terminal-dependant routines. Essentially these are for
*** VT52, which the PC interprets.
**/

/**
*** This routine should clear the entire screen and move the cursor to the
*** top-left corner.
**/

PRIVATE void clear_screen()
{ int i, j;
  for (i = 0; i <= LastRow; i++)
   for (j = 0; j <= LastCol; j++)
    map[i][j] = ' ';
  Cur_x = 0; Cur_y = 0;

  send_ch(0x0C);	/* Form Feed */
}

/**
*** This routine should clear the rest of the current line, including the
*** cursor position, leaving the cursor position unchanged.
**/
PRIVATE void clear_eol()
{ int a, b, i;

  a = Cur_x; b = Cur_y;
  for (i = Cur_x; i <= LastCol; i++)
   send_ch(' ');
  move_to(b, a);
}

/**
*** This routine should move the cursor to the position specified.
**/

PRIVATE void move_to(y, x)
int y, x;
{ Cur_x = x; Cur_y = y;

  send_ch(0x1B); send_ch('Y'); send_ch(y + 0x20); send_ch(x + 0x20);
}

/**
*** This routine should refresh the screen between the rows specified.
**/

PRIVATE void refresh_screen(start_y, end_y)
int start_y, end_y;
{  int i, j;
   int x, y;
   x = Cur_x; y = Cur_y;
   
   move_to(start_y, 0);
   
   for (i = start_y; i <= end_y; i++)
    { 
      for (j = 0; j <= LastCol; j++)
       send_ch(map[i][j]);
       
      if (i < LastRow) {send_ch('\n'); send_ch('\r'); }
    }  

   Cur_y = y; Cur_x = x;
}

/**
*** This routine should cause an audio or visual alert of some sort.
**/
PRIVATE void ring_bell()
{
  send_ch(0x07);
}

/**
*** This routine should move the cursor left one position, without erasing the
*** character.
**/
PRIVATE void backspace()
{ if (Cur_x eq 0) return;
  Cur_x --;

  send_ch(0x08);
}

/**
*** This routine should move the cursor to the first column of the current row.
**/

PRIVATE void carriage_return()
{ Cur_x = 0;

  send_ch(0x0D);
}

/**
*** This routine should move the cursor down one row. If the cursor is on the
*** bottom row the screen should scroll up one line, and the actual cursor
*** position will remain unchanged. The cursor should NOT move to the beginning
*** of the line.
**/

PRIVATE void linefeed()
{ if (Cur_y >= LastRow)
   { int i; BYTE *ptr = map[0];	/* Scroll up */
     for (i = 0; i < LastRow; i++)
      map[i] = map[i+1];
     for (i = 0; i <= LastCol; i++)
      ptr[i] = ' ';
     map[LastRow] = ptr;
     Cur_y = LastRow;
   }
  else
   Cur_y++;

   send_ch(0x0A);
}

/**
*** set_mark() and use_mark() are used to remember and restore the current
*** screen position. Many terminal emulators provide a built-in facility
*** for this.
**/

PRIVATE int mark_x, mark_y;

PRIVATE void set_mark()
{
   mark_x = Cur_x; mark_y = Cur_y;
}

PRIVATE void use_mark()
{
  move_to(mark_y, mark_x);
}

/**
*** Colours: these can be one of the following values :
***
*** 0) Black
*** 1) Red
*** 2) Green
*** 3) Yellow
*** 4) Blue
*** 5) Magenta
*** 6) Cyan
*** 7) White
**/

PRIVATE void foreground(colour)
int colour;
{ colour = colour;
}

PRIVATE void background(colour)
int colour;
{ colour = colour;
}

PRIVATE void set_inverse(flag)
int flag;
{ send_ch(0x1B); send_ch( flag ? 'P' : 'U');
}

PRIVATE void set_bold(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_underline(flag)
int flag;
{ flag = flag;
}

PRIVATE void set_italic(flag)
int flag;
{ flag = flag;
}

/**
*** redraw the screen, following a window switch. This must lock the
*** ANSI output semaphore to protect the output buffer below.
**/

PRIVATE void redraw_screen(Window *window)
{ Screen *screen = &(window->screen);
  char   *text   = window->Node.Name;
  BYTE   **map   = window->screen.map;
  int    i, j, k;
  
  current_window = window;
  
  send_ch(0x0C);	/* clear the screen */
  for (i = 0; i < screen->Rows; i++)
   { for (j = screen->Cols - 1; j>= 0; j--)
      if (map[i][j] ne ' ') break;
     for (k = 0; k <= j; k++)
      send_ch(map[i][k]);

     if (i < screen->Rows - 1)
      { send_ch('\r'); send_ch('\n'); }
   }
   
  move_to(0, screen->Cols - strlen(text));
  set_inverse(1);
  while (*text ne '\0')
   send_ch(*text++);
  set_inverse(0);
  move_to(screen->Cur_y, screen->Cur_x);
  flush();
}

/**
*** These routines are responsible for sending converted terminal output to
*** the screen, by one means or another.
**/
#define Flush_limit 200
PRIVATE BYTE output_buffer[Flush_limit + 5];
PRIVATE int  output_count = 0; 

PRIVATE void send_ch(ch)
int ch;
{
  output_buffer[output_count++] = ch;
  if (output_count >= Flush_limit)
    flush();
}

PRIVATE void flush()
{ int junk;
 
  if ((Mode eq Mode_Background) || (current_window ne active_window))
   { output_count = 0;
     return;
   }

  output_buffer[output_count] = '\0';
  output_count = 0;
  write_to_screen(&(output_buffer[0]), &junk);
}
