/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netagent.c								--
--                                                                      --
--	This program is run by the Network Server on the various	--
--	processors in order to perform various operations, such as	--
--	enabling/disabling links.					--
--                                                                      --
--	Author:  BLV 14/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/netagent.c,v 1.39 1993/08/12 11:56:00 nickc Exp $*/

/**
*** The netagent can be compiled to work in two different ways, affecting
*** the way in which it communicates with the Network Server. The first
*** option involves a pipe created by the Network Server, supplied in an
*** environment. The second involves the netagent installing itself as
*** as a server and performing low-level message transactions. This
*** compile-time option determines the communication.
**/
#define PIPEIO 0

/*{{{  Header files and administration */

#include <stdio.h>
#include <stddef.h>
#include <syslib.h>
#include <task.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include <root.h>
#include <module.h>
#include <signal.h>
#include <servlib.h>
#include <process.h>
#include <c40.h>
#include "private.h"
#include "rmlib.h"
#include "netaux.h"

#ifdef __TRAN
#include <asm.h>		/* needed for resetch */
#endif

#ifndef eq
#define eq ==
#define ne !=
#endif

/**
*** Ensure that the C library (plus posix and fplib) are not loaded
**/
static void _stack_error(Proc *p)
{ VoidFnPtr junk;
  junk = &_stack_error;
  IOdebug("Netagent: stack overflow in %s at %x",p->Name,&p);
  Exit(0x0080 | SIGSTAK);
}
/*}}}*/
/*{{{  Prototypes etc. */
/**
*** These routines support the various requests. The Network Server sends
*** NA_Message structures to the netagent which include a function code.
*** The structure and the various function codes are defined in netaux.h
*** There is a separate handler for every function code. Particular
*** processors may be unable to cope with particular function codes.
**/
static void	do_SetLinkMode(NA_Message *);
static void	do_Protect(NA_Message *);
static void	do_Revoke(NA_Message *);
static void	do_Cupdate(NA_Message *);
static void	do_Clean(NA_Message *);
static void	do_ClearNames(NA_Message *);
static void	do_UpdateIO(NA_Message *);
static void	do_GetLinkMode(NA_Message *);
static void	do_Terminate(NA_Message *);
#ifdef __TRAN
static void	do_TransputerBoot(NA_Message *, bool);
static void	do_ParsytecReset(NA_Message *);
static void	Parsytec_Reset(int link);
#endif
#ifdef __C40
static void	do_C40Boot(NA_Message *);
#endif

static char	ProcessorName[IOCDataMax];

#define	MainReadTimeout		(30 * OneSec)
#define ExtraReadTimeout	(10 * OneSec)
#define ReplyTimeout		(60 * OneSec)
/*}}}*/
#if PIPEIO
/*{{{  main() : receive requests from the Network Server via a pipe */
/**
*** Get the environment and the standard streams, which are a pipe from and
*** to the network server. The same pipe is used bi-directionally.
*** Then loop for ever reading request packets and invoke suitable handling
*** routines.
**/
static Stream	*Stdin;
static Stream	*Stdout;
static BYTE	Datavec[IOCDataMax];
static void	CommunicationsBreakdown(int);

int main(void)
{ Environ	env;
  NA_Message	message;
  word		rc;

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif
  
  if (GetEnv(MyTask->Port, &env) < Err_Null)
   { IOdebug("netagent: failed to receive environment");
     Exit(0x100);
   }
  if ((env.Strv[0] eq Null(Stream)) || (env.Strv[0] eq (Stream *) MinInt))
   { IOdebug("netagent: failed to get communication streams");
     Exit(0x100);
   }
  Stdin		= env.Strv[0];
  Stdout    	= env.Strv[0];
  MachineName(ProcessorName);

	/* Initial communication */
  if (Read(Stdin, (BYTE *) &rc, sizeof(word), 5 * OneSec) ne sizeof(word))
   { IOdebug("netagent(%s): failed to get initial data", ProcessorName);
     Exit(0x100);
   }
  if (Write(Stdin, (BYTE *) &rc, sizeof(word), 5 * OneSec) ne sizeof(word))
   { IOdebug("netagent(%s): failed to respond to initial data", ProcessorName);
     Exit(0x100);
   }

  forever
   { if (Read(Stdin, (BYTE *) &message, sizeof(NA_Message), MainReadTimeout) ne
   		 sizeof(NA_Message))
	CommunicationsBreakdown(1);

     message.Data = Datavec;
     if (message.Size > 0)
      if (Read(Stdin, message.Data, message.Size, ExtraReadTimeout) ne message.Size)
       CommunicationsBreakdown(2);
    
    switch(message.FnRc)
     {	case NA_Quit		: Exit(0);
	case NA_Noop		: /* Just keep running */	break;
	case NA_SetLinkMode	: do_SetLinkMode(&message);	break;
	case NA_Protect		: do_Protect(&message);		break;
	case NA_Revoke		: do_Revoke(&message);		break;
	case NA_Cupdate		: do_Cupdate(&message);		break;
	case NA_Clean		: do_Clean(&message);		break;
	case NA_ClearNames	: do_ClearNames(&message);	break;
	case NA_UpdateIO	: do_UpdateIO(&message);	break;
	case NA_GetLinkMode	: do_GetLinkMode(&message);	break;
	case NA_Terminate	: do_Terminate(&message);	break;

#ifdef __TRAN
	case NA_TransputerBoot	: do_TransputerBoot(&message, FALSE);	break;
	case NA_ParsytecBoot	: do_TransputerBoot(&message, TRUE); 	break;
	case NA_ParsytecReset	: do_ParsytecReset(&message);		break;
#endif

#ifdef __C40
	case NA_C40Boot		: do_C40Boot(&message);		break;
#endif

       default : IOdebug("Netagent(%s): unexpected request 0x%x", ProcessorName,
       				message.FnRc);
		 Exit(0x100);
     }
   }
}

/**
*** If there are processor failures then the netagent may lose communication
*** with the Network Server. The netagent should fail silently, as the
*** Network Server contains recovery code to restart the agent etc.
**/
static void	CommunicationsBreakdown(int x)
{ /* IOdebug("netagent(%s): comms breakdown %d", ProcessorName, x); */
  Exit(0x100 | x);
}

/**
*** Send a reply message
**/
static void	Reply(NA_Message *message, int rc, int size, BYTE *data)
{ message = message;
  if (Write(Stdout, (BYTE *) &rc, sizeof(WORD), ReplyTimeout) ne sizeof(WORD))
   CommunicationsBreakdown(3);
  if (size > 0)
   if (Write(Stdout, data, size, ReplyTimeout) ne size)
    CommunicationsBreakdown(4);
}
/*}}}*/
#else
/*{{{  main() : receive requests via message passing */
/*{{{  forward declarations and statics */
	/* This variable is used to keep track of the last received	*/
	/* message. If no message is received for a certain period of	*/
	/* time (30 seconds) the netagent aborts.			*/
static word		LastMessage;

static void		do_open(ServInfo *servinfo);

static ObjNode		Netagent_Root;
static DispatchInfo Netagent_Info = {
	(DirNode *)	&Netagent_Root,
	NullPort,
	SS_NetServ,
	Null(char),
	{ NULL, 0},
	{
		{ do_open,		3000 },
		{ InvalidFn,		2000 },	/* Create	*/
		{ DoLocate,		2000 },
		{ DoObjInfo,		2000 },
		{ InvalidFn,		2000 },	/* ServerInfo	*/
		{ InvalidFn,		2000 },	/* Delete	*/
		{ InvalidFn,		2000 },	/* Rename	*/
		{ InvalidFn,		2000 },	/* Link		*/
		{ InvalidFn,		2000 },	/* Protect	*/
		{ InvalidFn,		2000 },	/* SetDate	*/
		{ InvalidFn,		2000 }, /* Refine	*/
		{ InvalidFn,		2000 },	/* CloseObj	*/
		{ InvalidFn,		2000 },	/* Revoke	*/
		{ InvalidFn,		2000 },	/* Reserved1	*/
		{ InvalidFn,		2000 },	/* Reserved2	*/
	}
};
/*}}}*/
/*{{{  monitor thread, to abort the netagent if necessary */
static void netagent_monitor(void)
{ LastMessage	= GetDate();

  forever
   { Delay(5 * OneSec);
     if ((GetDate() - LastMessage) > 30)
      { AbortPort(Netagent_Info.ReqPort, EC_Fatal);
        break;
      }
   }
}
/*}}}*/
/*{{{  do_open() */
static void do_open(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  MsgBuf	*r;
  ObjNode	*f;
  Port		 stream_port;
  NA_Message	*message= (NA_Message *) m->Control;
  BYTE		*data	= m->Data;

  f = GetTarget(servinfo);
  if (f == Null(ObjNode))
   { ErrorMsg(m, EO_File); return; }

  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); return; }

  FormOpenReply(r, m, f, Flags_Closeable, servinfo->Pathname);
  r->mcb.MsgHdr.Reply	= stream_port	= NewPort();
  PutMsg(&r->mcb);
  Free(r);

  f->Account++;
  UnLockTarget(servinfo);

  forever
   { word	errcode;

     m->MsgHdr.Dest	= stream_port;
     m->Timeout		= 30 * OneSec;
     m->Data		= data;
     m->Control		= (WORD *) message;
     errcode		= GetMsg(m);
     m->MsgHdr.FnRc	= SS_NetServ;

     if (errcode < Err_Null) break;	/* exit on any error */
     if ((errcode & FC_Mask) eq FC_GSP)
      { if ((errcode & FG_Mask) eq FG_Close)
         { if (m->MsgHdr.Reply ne NullPort)
            { m->MsgHdr.FnRc	= 0;
	      ErrorMsg(m, Err_Null);
	    }
	 }
	else
	 ErrorMsg(m, EC_Error + EG_Invalid + EO_Message);
	break;
      }

     message		= (NA_Message *) m->Control;
     message->Data	= m->Data;
     LastMessage	= GetDate();

     switch(message->FnRc)
      {	case NA_Quit		: /* wait for the close */
	case NA_Noop		: /* Just keep running */	break;
	case NA_SetLinkMode	: do_SetLinkMode(message);	break;
	case NA_Protect		: do_Protect(message);		break;
	case NA_Revoke		: do_Revoke(message);		break;
	case NA_Cupdate		: do_Cupdate(message);		break;
	case NA_Clean		: do_Clean(message);		break;
	case NA_ClearNames	: do_ClearNames(message);	break;
	case NA_UpdateIO	: do_UpdateIO(message);		break;
	case NA_GetLinkMode	: do_GetLinkMode(message);	break;
	case NA_Terminate	: do_Terminate(message);	break;

#ifdef __TRAN
	case NA_TransputerBoot	: do_TransputerBoot(message, FALSE);	break;
	case NA_ParsytecBoot	: do_TransputerBoot(message, TRUE); 	break;
	case NA_ParsytecReset	: do_ParsytecReset(message);		break;
#endif

#ifdef __C40
	case NA_C40Boot		: do_C40Boot(message);		break;
#endif

        default : IOdebug("Netagent(%s): unexpected request 0x%x", ProcessorName,
       				message->FnRc);
		 Exit(0x100);
      }
   }

  if (--(f->Account) eq 0)
   AbortPort(Netagent_Info.ReqPort, EC_Fatal);
}
/*}}}*/
/*{{{  Reply() */
static void Reply(NA_Message *message, int fnrc, int size, BYTE *data)
{ MCB	*m;

		/* The NA_Message is the control vector. I can step	*/
		/* back to get the MCB.					*/
  m = (MCB *) ((BYTE *) message - sizeof(MCB));
  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, fnrc);
  m->MsgHdr.DataSize	= size;
  m->Data		= data;
  PutMsg(m);
}
/*}}}*/

int main(void)
{ Object	*nametable_entry;

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif

  { extern int SafetySize; /* in the server library */
    SafetySize = 0;
  }
  InitNode(&Netagent_Root, ".netagent", Type_File, 0, DefFileMatrix);
  InitList(&Netagent_Root.Contents);
  Netagent_Root.Parent	= Null(DirNode);

  MachineName(ProcessorName);

  { NameInfo	 info;
    Object	*this_processor = Locate(Null(Object), ProcessorName);
    info.Port		= Netagent_Info.ReqPort	= NewPort();
    info.Flags		= Flags_StripName;
    info.Matrix		= DefNameMatrix;
    info.LoadData	= NULL;    
    nametable_entry	= Create(this_processor, ".netagent", Type_Name,
		sizeof(NameInfo), (BYTE *) &info);
    Close(this_processor);
  }

  if (nametable_entry ne Null(Object))
   { Fork(2000, &netagent_monitor, 0);
     Dispatch(&Netagent_Info);
     Delete(nametable_entry, Null(char));  /* after an AbortPort() */
   }

  Exit(0);
}
/*}}}*/
#endif
/*{{{  Set link mode */

/**
*** Set the link specified by Arg1 to the mode specified by Arg2. This mode
*** may be 0, 1, 2, or 3. For now ignore any errors.
**/
/*{{{  hardware link resets */

#ifdef __TRAN
	/* perform resetch_()'s on a particular link		*/
static void reset_link(int link)
{ int	x;
  int	*links	= (int *) MinInt;

  if ((x = resetch_(links[link])) != MinInt) runp_(x);
  if ((x = resetch_(links[link + 4])) != MinInt) runp_(x);
}
#else
#ifndef __C40
static void reset_link(int link)
{ link = link;
}
#endif
#endif

/*}}}*/

static void	do_SetLinkMode(NA_Message *message)
{ word		link = message->Arg1;
  word		mode = message->Arg2;
  LinkInfo	info;
  LinkConf	conf;
  
  if (LinkData(link, &info) < Err_Null) return;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags | Link_Flags_report;
   
	/* A dumb link that has been allocated must not be changed. */
  if ((info.Mode eq Link_Mode_Dumb) && (info.State eq Link_State_Running))
   goto done;

	/* If the link was not connected or dumb, reset the hardware now*/
#if 0
	/* BLV - this causes bootstrap failures during the cross-link	*/
	/* phase. This is highly suspicious, as the link guardian	*/
	/* should not be touching the link at the moment.		*/
  if (info.Mode ne Link_Mode_Intelligent)
   reset_link(link);
#endif

  switch(mode)
   { case RmL_NotConnected :
   		    conf.Mode	= Link_Mode_Null;
   		    conf.State	= Link_State_Null;
   		    break;

     case RmL_Dumb :
     		    conf.Mode	= Link_Mode_Dumb;
     		    conf.State	= Link_State_Dumb;
     		    break;
     		    
     case RmL_Running :
       		    conf.Mode	= Link_Mode_Intelligent;
     		    conf.State	= Link_State_Running;
     		    break;
     
     case RmL_Pending :
     			/* Unfortunately the Network Server does not	*/
     			/* currently keep track of which links were used*/
     			/* for bootstrap. Hence it could try to set	*/
     			/* bootlinks back to pending mode...		*/
     		    conf.Mode	= Link_Mode_Intelligent;
     		    if ((info.Mode eq Link_Mode_Intelligent) &&
     		    	(info.State eq Link_State_Running))
     		     conf.State = Link_State_Running;
     		    else
		     conf.State	= Link_State_Dead;
     		    break;

     case RmL_Dead :
		    conf.Mode	= Link_Mode_Intelligent;
		    conf.State	= Link_State_Crashed;
		    break;

     default	  : return;
   }

	/* Configure the link. Then possibly enable the link or reset	*/
	/* the link hardware.						*/
  if (Configure(conf) < Err_Null)
   IOdebug("netagent(%s): failed to configure link %d", ProcessorName, link);
  elif ((mode eq RmL_Running) &&
        ((info.Mode ne Link_Mode_Intelligent) || (info.State ne Link_State_Running)))
   EnableLink(link);
#if 0
	/* BLV - this causes reboots to fail. I suspect that the link	*/
	/* guardian is still timing out, i.e. the configure does not	*/
	/* block until the link guardian has setttled down.		*/
   elif ((mode ne RmL_Running) && (mode ne RmL_Pending))
      reset_link(link);
#endif  

done:

  Reply(message, RmE_Success, 0, NULL);
}

/*}}}*/
/*{{{  Get link mode */

/**
*** Get the current link mode
**/
static void	do_GetLinkMode(NA_Message *message)
{ word		link = message->Arg1;
  int		mode;
  LinkInfo	info;

  if (LinkData(link, &info) < Err_Null)
   { Reply(message, RmE_BadLink, 0, NULL); return; }

  switch(info.Mode)
   { case	Link_Mode_Null : mode = RmL_NotConnected; break;
     case	Link_Mode_Dumb : mode = RmL_Dumb; break;
     case	Link_Mode_Intelligent :
			     	if (info.State eq Link_State_Running)
			     	 mode = RmL_Intelligent;
			     	elif (info.State eq Link_State_Dead)
			     	 mode = RmL_Pending;
		     		else
		     		 mode = RmL_Dead;
				break;

     default :	Reply(message, RmE_BadLink, 0, NULL);
		return;
   }

  Reply(message, RmE_Success, sizeof(WORD), (BYTE *) &mode);
}

/*}}}*/
/*{{{  Protect processor */

/**
*** do_Protect(). This is used to set up the protection needed. The routine
*** should return three capabilities. The first is the owner capability,
*** which is passed on to the Taskforce Manager when the processor gets
*** allocated. The second is read-only, which may or may not be handed out.
*** The third is the full capability, which only the Network
*** Server needs. This full capability is the only capability in the whole
*** network which allows the access matrix to be altered again, and hence
*** it may be needed by this routine.
**/
static void	do_Protect(NA_Message *message)
{ Matrix	matrix		= (Matrix) message->Arg1;
  Capability	*full_cap	= (Capability *) message->Data;
  Capability	result[3];
  Object	*processor;
  word		*temp;

  memset((void *) result, 0, 3 * sizeof(Capability));
  message->FnRc = Err_Null;

  temp = (word *) full_cap;
  
  if ((temp[0] eq 0) && (temp[1] eq 0))
   {	/* the processor has just been booted, and this is the first	*/
   	/* time that protection is being handled.			*/
	/* First, obtain a default object for the processor. This	*/
	/* includes alter access, courtesy of the Processor Manager.	*/
     processor = Locate(Null(Object), ProcessorName);
     if (processor eq Null(Object))
      { message->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Processor;
        goto done;
      }
   }
  else
   {	/* The protection is being reset. This means that the existing	*/
   	/* capability must be used in order to have alter access.	*/
     processor = NewObject(ProcessorName, full_cap);
     if (processor eq Null(Object))
      { message->FnRc = EC_Error + SS_NetServ + EG_NoMemory + EO_Processor;
        goto done;
      }
   }

	/* Remember the full capability, including alter access		*/
  result[2] = processor->Access;

	/* Now refine the access to produce a capability for the owner	*/
  if (Refine(processor, ~AccMask_A) < Err_Null)
   { 
     message->FnRc = Result2(processor);
     goto done;
   }
  result[0] = processor->Access;

	/* Go back to alter access, and produce a read-only capability */
  processor->Access = result[2];
  if (Refine(processor, AccMask_R | AccMask_Z) < Err_Null)
   {
     message->FnRc = Result2(processor);
     goto done;
   }
  result[1] = processor->Access;
  
	/* Go back to alter access, and protect the processor */
  processor->Access = result[2];
  if (Protect(processor, Null(char), matrix) < Err_Null) 
   { 
     message->FnRc = Result2(processor);
     goto done;
   }

done:
  Reply(message, (int) message->FnRc,
	(message->FnRc eq Err_Null) ? 3 * sizeof(Capability) : 0,
	(BYTE *) result);
}

/*}}}*/
/*{{{  Revoke capabilities */
/**
*** Currently a no-op
**/
static void	do_Revoke(NA_Message *message)
{ message = message;
}
/*}}}*/
/*{{{  Update processor's name */

/**
*** Update the name of a processor. For example, if the current name is /00
*** change it to /Net/00. Arg1 contains the length of the new network name.
**/
static void	do_Cupdate(NA_Message *message)
{ Object	*processor	= Null(Object);
  WORD	rc;
  
  processor	= Locate(Null(Object), ProcessorName);
  if (processor eq Null(Object))
   { rc = EC_Error + SS_NetServ + EG_Unknown + EO_Processor; goto done; }

  rc = Rename(processor, Null(char), message->Data);
  
done:
  Reply(message, (int) rc, 0, NULL);
  if (processor ne Null(Object)) Close(processor);
}

/*}}}*/
/*{{{  Update neighbour's (I/O processor) name */

/**
*** Update the name of an I/O processor at the other end of the specified
*** link. For example, change it from /IO to /Net/IO.
**/
static void	do_UpdateIO(NA_Message *message)
{ word		link = message->Arg1;
  char		linkbuf[10];	/* link.267\0 */
  Object	*processor	= Null(Object);
  word		rc;
    
  processor	= Locate(Null(Object), ProcessorName);
  if (processor eq Null(Object))
   { rc = EC_Error + SS_NetServ + EG_Unknown + EO_Processor; goto done; }
   
  strcpy(linkbuf, "link.");
  if (link eq 0)
   strcat(linkbuf, "0");	/* addint() does not cope with 0 */
  else
   addint(linkbuf, link);
   
  rc = Rename(processor, linkbuf, message->Data);

done:
  Reply(message, (int) rc, 0, NULL);
  if (processor ne Null(Object)) Close(processor);  
}

/*}}}*/
/*{{{  Clean out processor */

/**-----------------------------------------------------------------------------
*** This routine should clean out the processor.
*** Essentially it performs several walkdirs.
*** 3) two WalkDirs of /tasks to find any programs that should not be running.
***    There are occasional problems killing off programs so the final
***    kill_task is not done until the end, after the reply is sent.
*** 2) a WalkDir of /loader to delete any unused libraries
*** 3) possibly a WalkDir of /pipe if the pipe server is running
*** 4) ditto for /fifo
*** 5) ditto for /ram
**/
static word	WalkDir(Object *x, WordFnPtr fn);
static bool	pipe_running	= FALSE;
static bool	fifo_running	= FALSE;
static bool	ram_running	= FALSE;
static word	kill_task(Object *task);
static word	kill_something(Object *whatever);
static word	cleaning_date;

static void	do_Clean(NA_Message *message)
{ Object	*procman	= Locate(Null(Object), "/tasks");
  Object	*loader		= Locate(Null(Object), "/loader");
  word		 running_tasks	= 0;

  cleaning_date = message->Arg1;

  if (procman ne Null(Object))
   { running_tasks = WalkDir(procman, &kill_task);   /* generates SIGINT */
     if (running_tasks > 0)
      { Delay(running_tasks * (OneSec / 10));
        WalkDir(procman, &kill_task);	/* generates SIGKILL	*/
      }
   }

  if (loader ne Null(Object))
   { (void) WalkDir(loader, &kill_something); Close(loader); }

  if (pipe_running)
   { Object	*pipe_server = Locate(Null(Object), "/pipe");
     if (pipe_server ne Null(Object))
      { (void) WalkDir(pipe_server, &kill_something);
        Close(pipe_server);
      }
   }
   
  if (fifo_running)
   { Object	*fifo_server = Locate(Null(Object), "/fifo");
     if (fifo_server ne Null(Object))
      { (void) WalkDir(fifo_server, &kill_something);
        Close(fifo_server);
      }
   }
   
  if (ram_running)
   { Object	*ram_server = Locate(Null(Object), "/ram");
     if (ram_server ne Null(Object))
      { (void) WalkDir(ram_server, &kill_something);
        Close(ram_server);
      }
   }

  Reply(message, (int) Err_Null, 0, NULL);   

  if (running_tasks > 0)
   { (void)WalkDir(procman, &kill_task);	/* performs KillTask(), or dies	*/
     Close(procman);
   }
}

static word WalkDir(Object *dir, WordFnPtr fn)
{ Stream  	*s;
  WORD		size, i;
  DirEntry	*entry, *cur;
  Object	*item;
  word		 result = 0;
  
  if ((dir->Type & Type_Flags) eq Type_Stream) return(0);
   
  s = Open(dir, Null(char), O_ReadOnly);
  if (s eq Null(Stream))
   { IOdebug("netagent: error cleaning %s (%x)", dir->Name, Result2(dir));
     return(0); 
   }

  size = GetFileSize(s);
  if (size eq 0) return(0);

  entry = (DirEntry *) Malloc(size);
  if (entry == Null(DirEntry)) 
   { IOdebug("netagent: out of memory cleaning %s", dir->Name);
     Close(s); 
     return(0);
   }
     
  if ((size = Read(s, (BYTE *) entry, size, -1)) < 0)
   { IOdebug("netagent: read error cleaning %s", dir->Name);
     Close(s);
     return(0);
   }

  Close(s);
      
  cur = entry;
  for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
   { if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
      continue;

     item = Locate(dir, cur->Name);
     if (item ne Null(Object))
      { result += (*fn)(item);
        Close(item);
      }
   }

  Free(entry);
  return(result);
}

	/* delete something from a ram-disk or similar */
static word	kill_something(Object *x)
{ if ((x->Type & Type_Flags) eq Type_Directory)
   WalkDir(x, &kill_something);
  Delete(x, Null(char));
  return(0);
}

	/* kill a task. Several tasks are exempt from this. 		*/
	/* Also, commands started by the Network Server are exempt	*/
static	word kill_task(Object *task)
{ char		*name = objname(task->Name);
  ObjInfo	info;
      
  if (	!strncmp(name, "netagent.", 9)	||
	!strncmp(name, "login.", 6)	||
	!strncmp(name, "Null.", 5)	||
	!strncmp(name, "Loader.", 7)	||
  	!strncmp(name, "ProcMan.", 8))
   return(0);

  if (!strncmp(name, "Fifo.", 5)) { fifo_running = TRUE; return(0); }
  if (!strncmp(name, "Pipe.", 5)) { pipe_running = TRUE; return(0); }
  if (!strncmp(name, "Ram.", 4))  { ram_running = TRUE;  return(0); }
  
  if ((ObjectInfo(task, Null(char), (BYTE *) &info) < Err_Null) || 
      (info.Dates.Creation <= cleaning_date))
   return(0);

  (void) Delete(task, Null(char));
  return(1);
}

/*}}}*/
/*{{{  Clear name table */
/**
*** Clearing the names involves sending a message to the processor manager.
*** Currently no special capability is required. The network server does not
*** expect a reply.
**/
static void	do_ClearNames(NA_Message *message)
{ Object	*procman = Null(Object);
  word		rc = Err_Null;
  MsgBuf	*r = New(MsgBuf);

  if (r eq Null(MsgBuf))
   { rc = EC_Error + SS_NetServ + EG_NoMemory + EO_Message; goto done; }

  procman = Locate(Null(Object), "/tasks");  
  if (procman eq Null(Object))
   { rc = EC_Error + SS_NetServ + EG_Unknown + EO_ProcMan; goto done; }

  InitMCB(&(r->mcb), MsgHdr_Flags_preserve, NullPort, NullPort, 
          FC_GSP + FG_Reconfigure);
  r->mcb.Control = r->control;
  r->mcb.Data	 = r->data;
  MarshalCommon(&(r->mcb), procman, Null(char));
  SendIOC(&(r->mcb));

done:
  if (r ne Null(MsgBuf)) Free(r);
  if (procman ne Null(Object)) Close(procman);
  message = message;
}
/*}}}*/
/*{{{  Terminate processor */
/**
*** Terminate(). This is slightly nasty because the pipe has to be
*** closed first.
**/
static	void	do_Terminate(NA_Message *message)
{
#if PIPEIO
  Close(Stdin);
	/* This delay appears to be needed with the 1.2.2 nucleus */
  Delay(OneSec / 10);
#endif

  Terminate();
  message = message;
}
/*}}}*/
#ifdef __TRAN
/*{{{  Parsytec-style reset */
/**
*** Perform a Parsytec reset down the link specified by Arg1 of the message.
*** Do not bother to send back a reply.
**/
static void	do_ParsytecReset(NA_Message *message)
{ int link = message->Arg1;
  Parsytec_Reset(link);
}
/*}}}*/
/*{{{  Transputer bootstrap */

/**-----------------------------------------------------------------------------
*** do_TransputerBoot(). This code should be kept in step with rboot.c
*** There is a second argument to specify a Parsytec reset. Arg1 of the message
*** is the link number. Arg2 is the size of the remaining data. This
*** consists of a nucleus string, possibly empty to indicate that the same
*** nucleus should be used, but always aligned to a four byte boundary.
*** This is followed by the configuration vector.
***
*** The routine returns a similar result to the system BootLink() routine,
*** but all the work is done the hard way
***
*** NOTE: in Helios 1.3 the option to use the system's BootLink() routine has
*** been removed, as it was never enabled. Also, the various delays involved
*** have been removed as there is no known justification for them. The
*** only hardware for which a delay during the bootstrap is required, as
*** far as I know, is a Parsytec SuperCluster where a delay is needed after
*** asserting the reset to allow the memory parity hardware to settle down.
*** This delay has been moved to Parsytec_Reset().
***
*** The netagent without delays has been tested extensively on a T.Node and
*** on a MultiCluster. Bootstrap appears to have become more reliable.
**/

static MPtr	determine_image(char *, word *);
static void	transputer_init_link(int);
static void	transputer_set_link(int);
static int	transputer_check_link(int);
static int	transputer_BootLink(word link, MPtr image, Config *config, word csize);

static void	do_TransputerBoot(NA_Message *message, bool parsytec)
{ BYTE		*nucleus_string = message->Data;
  Config	*config;
  int		link = message->Arg1;
  int		length;
  word		rc = Err_Null;
  MPtr		system_image;

  length = strlen(nucleus_string) + 1;
  length = (length + 3) & ~3;
  config = (Config *) &(nucleus_string[length]);
  length = message->Size - length;	/* now length of config vector */

  system_image = determine_image(nucleus_string, &rc);
  if ( MNull_(system_image) ) goto done;

  config->ImageSize = MWord_(system_image,0);
  config->Date	    = GetDate();

  transputer_init_link(link);
  
  if (parsytec) Parsytec_Reset(link);

  rc = transputer_BootLink(link, system_image, config, length);
  if (rc eq Err_Null)
   { transputer_set_link(link);
     rc = transputer_check_link(link);
   }
   
done:
  if ((strlen(nucleus_string) > 0) && (!MNull_(system_image)))
   Free(MtoC_(system_image));

  Reply(message, rc, 0, NULL);
}

/**
*** Figure out which nucleus to use. If the string provided is empty
*** then use the current nucleus. Otherwise try to locate and read in
*** the specified nucleus.
**/
static	MPtr determine_image(char *name, word *rc)
{ Object	*nuc;
  ObjInfo	info;
  MPtr		image = GetSysBase();
  Stream	*s;
  BYTE		*buffer;
  
  if (strlen(name) eq 0) return(image);
  
  nuc = Locate(Null(Object), name);
  if (nuc eq Null(Object))
   { *rc = EC_Error + SS_NetServ + EG_Unknown + EO_File; 
     return NullMPtr;
   }

  if ((nuc->Type & Type_Flags) ne Type_Stream)
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_Invalid + EO_File;
     return NullMPtr;
   }
   
  if (ObjectInfo(nuc, Null(char), (BYTE *) &info) < Err_Null)   
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_Broken + EO_File;
     return NullMPtr;
   }
   
  if (info.Size <= 0)
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_WrongSize + EO_File;
     return NullMPtr;
   }

  buffer = Malloc(info.Size);
  if (buffer eq Null(BYTE))
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_NoMemory + EO_File;
     return NullMPtr;
   }

  s = Open(nuc, Null(char), O_ReadOnly);
  Close(nuc);
  if (s eq Null(Stream))
   { Free(buffer);
     *rc = EC_Error + SS_NetServ + EG_Open + EO_File;
     return NullMPtr;
   }
   
  if (Read(s, buffer, info.Size, -1) ne info.Size)
   { Free(buffer);
     buffer = Null(BYTE);
     *rc = EC_Error + SS_NetServ + EG_Broken + EO_Stream;
   }
  Close(s);
  return(CtoM_(buffer));
}

/**
*** If I am about to attempt a bootstrap down a link, that link had better
*** be in a sensible state.
**/
static void transputer_init_link(int link)
{ LinkInfo info;
  LinkConf conf;

  if (LinkData(link, &info) ne Err_Null) return;

  if (info.Mode eq Link_Mode_Dumb) return;
  conf.Mode 	= Link_Mode_Dumb;
  conf.State	= Link_State_Dumb;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  (void) Configure(conf);

  reset_link(link);
}

/**
*** After a bootstrap, ensure that the link is in a sensible state.
*** Note that the newly-booted processor almost immediately sends
*** an info exchange, so the booting processor must not send its own
*** info. Hence the correct state is pending, not intelligent.
**/
static void transputer_set_link(int link)
{ LinkInfo	info;
  LinkConf	conf;
  
  if (LinkData(link, &info) ne Err_Null) return;

  if ((info.Mode eq Link_Mode_Intelligent) &&
      (info.State eq Link_State_Running))
   return;
  conf.Mode 	= Link_Mode_Intelligent;
  conf.State	= Link_State_Dead;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  (void) Configure(conf);
}

/**
*** The network agent checks the booting link. If the remote processor has come
*** up then it will have enabled the link by now, and it can be used by the
*** networking software. If not then the netagent polls for a while, waiting
*** for the link.
**/
static	int	transputer_check_link(int link)
{ LinkInfo	info;
  int		rc;
  int		i;

  for (i = 0; i < 100; i++)    
   { if ((rc = LinkData(link, &info)) ne Err_Null) return(rc);
     if ((info.Mode eq Link_Mode_Intelligent) &&
         (info.State eq Link_State_Running))
      break;
     else
      Delay(10000); /* 10 milliseconds * 100 loops */
   }

  if ((info.Mode ne Link_Mode_Intelligent) ||
      (info.State ne Link_State_Running))
   return(EC_Error + SS_NetServ + EG_Boot + EO_Processor);

  return(Err_Null);
}

/**
*** Perform a Parsytec-style reset on the specified link.
**/
#define Reset_Address	0x000000C0
static void Parsytec_Reset(int link)
{ uword *reg = (uword *) Reset_Address;
  *reg = 0;
  *reg = 1;
  *reg = 2;
  *reg = 3;
  *reg = 1 << link;
  Delay(5000);
  *reg = 0;
  Delay(10000);
}

/**
*** Manual bootstrap. This involves the following stages.
*** 1) send in nboot.i, held in a slot in the system image
*** 2) send in a control byte to nboot.i to read in the system image
*** 3) send in the whole system image
*** 4) send in the configuration vector
**/
static int transputer_BootLink(word link, MPtr image, Config *config, word confsize)
{ UBYTE	temp[4];
  MPtr  nboot;
  word  nboot_size;
  word	image_size;
  int	rc;

  nboot		= MRTOA_(MInc_(image,IVecBootStrap*sizeof(WORD)));
  nboot_size	= MWord_(image,IVecProcMan*sizeof(WORD))
		  - MWord_(image,IVecBootStrap*sizeof(WORD)) + 4;


  temp[0] = (UBYTE) nboot_size;
  if ((rc = LinkOut(1, link, temp, 2 * OneSec)) ne Err_Null)
   return(Boot_BootstrapSize | EG_Timeout | EC_Error | SS_NetServ);

  if (LinkOut(nboot_size, link, MtoC_(nboot), 2 *OneSec) ne Err_Null)
   return(Boot_BootstrapCode | EG_Timeout | EC_Error | SS_NetServ);

	/* There should be a short delay between the nboot start-up and	*/
	/* sending in the nucleus, because nboot performs a resetch.	*/
	/* Delays during this time should be safe because nboot knows	*/
	/* the bootlink and hence cannot be fould up by other		*/
	/* processors...						*/
  for (rc = 0; rc < 1024; rc++)
   temp[0] += rc;

  temp[0] = 4;			/* bootstrap command */
  if (LinkOut(1, link, temp, 2 * OneSec) ne Err_Null)
   return(Boot_ControlByte | EG_Timeout | EC_Error | SS_NetServ);

  image_size = MWord_(image,0);
  if (LinkOut(image_size, link, MtoC_(image), 3 * OneSec) ne Err_Null)
   return(Boot_Image | EG_Timeout | EC_Error | SS_NetServ);

  if (LinkOut(sizeof(WORD), link, (BYTE *) &confsize, 2 * OneSec) ne Err_Null)
   return(Boot_ConfigSize | EG_Timeout | EC_Error | SS_NetServ);

  if (LinkOut(confsize, link, (BYTE *) config, 2 * OneSec) ne Err_Null)
   return(Boot_ConfigVector | EG_Timeout | EC_Error | SS_NetServ);
  return(Err_Null);
}

/*}}}*/
#endif
#ifdef __C40
/*{{{  C40 bootstrap */

/**
*** C40 bootstrap. Arg1 of the message is the link number that should be
*** used. Arg2 is the size of the remaining data. This consists of a
*** nucleus string, possibly empty to indicate that the current nucleus
*** should be used, but always aligned to a four-byte boundary. This
*** is followed by the configuration vector. The code used at present is
*** very similar to the transputer version.
**/
static MPtr	determine_image(char *, word *);
static void	C40_init_link(int);
static void	C40_set_link(int);
static word	C40_check_link(word);
static word	C40_BootLink(word link, MPtr image, C40_Bootstrap *);

static void do_C40Boot(NA_Message *message)
{ C40_Bootstrap *boot_info;
  word		 link = message->Arg1;
  word		 rc = Err_Null;
  MPtr		system_image;

  boot_info	= (C40_Bootstrap *) message->Data;
  if (boot_info->Nucleus ne 0)
   system_image = determine_image(RTOA(boot_info->Nucleus), &rc);
  else
   system_image = determine_image(NULL, &rc);
  if ( MNull_(system_image) ) goto done;

  boot_info->Config.ImageSize	= MWord_(system_image, 0);
  boot_info->Config.Date	= GetDate();

  C40_init_link((int) link);
  rc = C40_BootLink(link, system_image, boot_info);
  if (rc eq Err_Null)
   { C40_set_link((int)link);
     rc = C40_check_link(link);
   }

done:
  if ((boot_info->Nucleus ne 0) && !MNull_(system_image))
   Free(MtoC_(system_image));

  Reply(message, (int) rc, 0, NULL);
}
/**
*** Figure out which nucleus to use. If the string provided is empty
*** then use the current nucleus. Otherwise try to locate and read in
*** the specified nucleus.
**/
static	MPtr determine_image(char *name, word *rc)
{ Object	*nuc;
  ObjInfo	info;
  MPtr		image = GetSysBase();
  Stream	*s;
  BYTE		*buffer;
  
  if ((name eq NULL) || (strlen(name) eq 0)) return(image);
  
  nuc = Locate(Null(Object), name);
  if (nuc eq Null(Object))
   { *rc = EC_Error + SS_NetServ + EG_Unknown + EO_File; 
     return(NullMPtr);
   }

  if ((nuc->Type & Type_Flags) ne Type_Stream)
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_Invalid + EO_File;
     return(NullMPtr);
   }
   
  if (ObjectInfo(nuc, Null(char), (BYTE *) &info) < Err_Null)   
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_Broken + EO_File;
     return(NullMPtr);
   }
   
  if (info.Size <= 0)
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_WrongSize + EO_File;
     return(NullMPtr);
   }

  buffer = Malloc(info.Size);
  if (buffer eq Null(BYTE))
   { Close(nuc);
     *rc = EC_Error + SS_NetServ + EG_NoMemory + EO_File;
     return(NullMPtr);
   }

  s = Open(nuc, Null(char), O_ReadOnly);
  Close(nuc);
  if (s eq Null(Stream))
   { Free(buffer);
     *rc = EC_Error + SS_NetServ + EG_Open + EO_File;
     return(NullMPtr);
   }
   
  if (Read(s, buffer, info.Size, -1) ne info.Size)
   { Free(buffer);
     buffer = Null(BYTE);
     *rc = EC_Error + SS_NetServ + EG_Broken + EO_Stream;
   }
  Close(s);
  return(CtoM_(buffer));
}

/**
*** If I am about to attempt a bootstrap down a link, that link had better
*** be in a sensible state.
**/
static void C40_init_link(int link)
{ LinkInfo info;
  LinkConf conf;

  if (LinkData(link, &info) ne Err_Null) return;

  if (info.Mode eq Link_Mode_Dumb) return;

  conf.Mode 	= Link_Mode_Dumb;
  conf.State	= Link_State_Dumb;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  (void) Configure(conf);
}

/**
*** After a bootstrap, ensure that the link is in a sensible state.
*** Note that the newly-booted processor almost immediately sends
*** an info exchange, so the booting processor must not send its own
*** info. Hence the correct state is pending, not intelligent.
**/
static void C40_set_link(int link)
{ LinkInfo	info;
  LinkConf	conf;
  
  if (LinkData(link, &info) ne Err_Null) return;

  if ((info.Mode eq Link_Mode_Intelligent) &&
      (info.State eq Link_State_Running))
   return;

  conf.Mode 	= Link_Mode_Intelligent;
  conf.State	= Link_State_Dead;
  conf.Id	= info.Id;
  conf.Flags	= info.Flags;
  (void) Configure(conf);
}

/**
*** The network agent checks the booting link. If the remote processor has come
*** up then it will have enabled the link by now, and it can be used by the
*** networking software. If not then the netagent polls for a while, waiting
*** for the link.
**/
static	word	C40_check_link(word link)
{ LinkInfo	info;
  word		rc;
  int		i;

  for (i = 0; i < 100; i++)    
   { if ((rc = LinkData(link, &info)) ne Err_Null) return(rc);
     if ((info.Mode eq Link_Mode_Intelligent) &&
         (info.State eq Link_State_Running))
      break;
     else
      Delay(10000); /* 10 milliseconds * 100 loops */
   }

  if ((info.Mode ne Link_Mode_Intelligent) ||
      (info.State ne Link_State_Running))
   return(EC_Error + SS_NetServ + EG_Boot + EO_Processor);

  return(Err_Null);
}

/**
*** Manual bootstrap. This involves the following stages.
*** 1) Send in magic numbers for the memory
*** 2) Send in C40boot.i, held in a slot in the system image
*** 3) Send in more magic numbers
*** 4) Receive bootstrap executing acknowledgement
*** 5) Send hardware configuration flags to booter
*** 6) If appropriate, send in the supplied IDROM
*** 7) Send in the whole system image
*** 8) Send in the configuration vector
**/
static word C40_BootLink(word link, MPtr image, C40_Bootstrap *boot_info)
{ WORD	temp[4];
  MPtr	c40boot;
  word  c40boot_size;
  word	image_size;
  word	rc;

	/* BLV - should load the bootstrap if a special one has been	*/
	/* specified in the C40_Bootstrap structure.			*/
  c40boot	= MRTOA_(MInc_(image,IVecBootStrap*sizeof(WORD)));
  c40boot_size	= MWord_(image,IVecProcMan*sizeof(WORD))
		  - MWord_(image,IVecBootStrap*sizeof(WORD)) + 4;

	/* If an IDROM is specified then the bus control registers from	*/
	/* the IDROM are used rather than the defaults.			*/
  if (boot_info->Hwconfig & (HW_PseudoIDROM | HW_ReplaceIDROM))
   { temp[0] = boot_info->Idrom.GBCR;
     temp[1] = boot_info->Idrom.LBCR;
   }
  else
   { temp[0] = 0x3e39fff0;		/* Global bus memory control word */
     temp[1] = 0x3e39fff0;		/* Local  bus memory control word */
   }
  temp[2] = c40boot_size / sizeof(WORD);/* Block size			  */
  temp[3] = 0x002ffc00;			/* Load address			  */

  /* Send C40 boot protocol header */
  if ((rc = LinkOut(16, link, temp, 2 * OneSec)) ne Err_Null)
   return(Boot_BootstrapSize | EG_Timeout | EC_Error | SS_NetServ);

  /* Send Helios-C40 bootstrap */
  if (MP_LinkOut(c40boot_size/sizeof(WORD), link, c40boot, 2 *OneSec) ne Err_Null)
   return(Boot_BootstrapCode | EG_Timeout | EC_Error | SS_NetServ);

  temp[0] = 0;				/* terminator	*/
  temp[1] = 0;				/* IVTP		*/
  temp[2] = 0;				/* TVTP		*/
  temp[3] = 0x00300000;			/* IACK		*/

  /* Send C40 boot protocol tail */
  if ((rc = LinkOut(16, link, temp, 2 * OneSec)) ne Err_Null)
   return(Boot_ProtocolTail | EG_Timeout | EC_Error | SS_NetServ);

  /* Receive bootstrap acknowledgement from bootstrap code */
  if ((rc = LinkIn(4, link, temp, 2 * OneSec)) ne Err_Null)
   return(Boot_Acknowledgement | EG_Timeout | EC_Error | SS_NetServ);
  if (temp[0] != 1) /* bootstrap ack is always 1 */
   return(Boot_Acknowledgement2 | EG_Timeout | EC_Error | SS_NetServ);

  /* Send hardware configuration flags to bootstrap code */
  if ((rc = LinkOut(4, link, (BYTE *) &(boot_info->Hwconfig), 2 * OneSec)) ne Err_Null)
   return(Boot_Hwconfig | EG_Timeout | EC_Error | SS_NetServ);

  /* Send the IDRom if appropriate			*/
  if (boot_info->Hwconfig & (HW_PseudoIDROM | HW_ReplaceIDROM))
   if ((rc = LinkOut(sizeof(IDROM), link, (BYTE *) &(boot_info->Idrom), 2 * OneSec)) ne Err_Null)
    return(Boot_Idrom | EG_Timeout | EC_Error | SS_NetServ);

  /* Send Helios-C40 nucleus */
  image_size = MWord_(image,0);
  if (MP_LinkOut(image_size/sizeof(WORD), link, image, 2 * OneSec) ne Err_Null)
   return(Boot_Image | EG_Timeout | EC_Error | SS_NetServ);
  
  /* Sending size of config */
  if (LinkOut(sizeof(WORD), link, (BYTE *) &(boot_info->ConfigSize), 2 * OneSec) ne Err_Null)
   return(Boot_ConfigSize | EG_Timeout | EC_Error | SS_NetServ);

  /* Sending config */
  if (LinkOut(boot_info->ConfigSize, link, (BYTE *) &(boot_info->Config), 2 * OneSec) ne Err_Null)
   return(Boot_ConfigVector | EG_Timeout | EC_Error | SS_NetServ);
  return(Err_Null);
}

/*}}}*/
#endif

