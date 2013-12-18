/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- tfm.c								--
--                                                                      --
--	Main module of the Taskforce Manager				--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/tfm.c,v 1.38 1994/03/01 12:35:21 nickc Exp $*/

/*{{{  version Number and history */
static char *VersionNumber = "3.30";
/**
*** History :
***           3.00, initial version
***           3.01, internal development
***           3.02, 1.2 beta 0 release
***           3.03, 1.2 beta 1 release
***           3.04, developed at Parsytec, 22.10.90-30.10.90
***           3.05, finishing off for the official 1.2 release
***           3.06, the real 1.2 release
***           3.07, first post 1.2 version, shipped with 1.2.1
***           3.08, changed the environment handling
***           3.09, added support for Tiny Helios
***           3.10, now generates errors if SendEnv/GetEnv fails
***           3.11, works around timing problems with the environments
***           ...
***           3.20, includes native network support
***           3.21, cleaned up the communication, added support for ids
***           3.22, minor bug fixes 30.10.91, absolute component names and
***                 shell scripts
***           3.23, change of communication to use sockets
***           3.24, extensions for fault tolerance
***	      3.25, added RmLib execute support
***           3.26, misc. bug fixes
***           3.27, C40 work
***           3.28, support for parallel libraries (TaskToTaskforce)
***           3.29, bug fixes
***           3.30, reduced memory requirements for C40 1.3.1 release
**/
/*}}}*/
/*{{{  headers and compile-time options */
#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <ctype.h>
#include <nonansi.h>
#include <attrib.h>
#include <pwd.h>
#include <signal.h>
#include <module.h>
#include <root.h>
#include <process.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "tfmaux.h"

	/* Compile-time debugging options	*/
#define Use_Malloc_Debugging		0
#define Use_Objects_Debugging		0
#define ShowHeap_Debugging		0
#define Use_IOC_Debugging		0
#define Redirect_To_Logger		0
#define Default_Diagnostics_Options	0
#define Use_Ports_Debugging		0
/*}}}*/
/*{{{  forward declarations and statics */
/*----------------------------------------------------------------------------*/
/**
*** forward declarations and statics. The /<username> directory contains two
*** different entries: domain and tfm. These are responsible for the processor
*** network and for the executing task forces respectively.
**/

static void		init_signals(void);
static void		show_startup(char *name);
static void		init_domain(void);
static void		do_open(ServInfo *);
static void		do_create(ServInfo *);
static void		do_delete(ServInfo *);
static void		do_private(ServInfo *);
static void		CheckWindow(void);
static void		abort_socket(int);

static	Object		*ThisProcessor;
	char		ProcessorName[IOCDataMax];
static	Object		*NameEntry;
	char		NetworkName[NameMax];
	DirNode		Root;
	DirNode		TFM;
	RmNetwork	Domain;
	Semaphore	LibraryLock;
	int		TaskSequenceNumber = 1;
	WORD		DebugOptions = Default_Diagnostics_Options;
static	Stream		*DiagnosticsStream;
static	Stream		*DefaultDiagnostics;
	RmProcessor	TfmProcessor;
	WORD		LastChange;
	Object		*PipeCode = Null(Object);

static DispatchInfo TFMInfo =
	{ &Root,
	  NullPort,
	  SS_TFM,
	  ProcessorName,
	  { do_private,	TFM_Stack },
	  {
	  	 { do_open,		TFM_Stack * 2	},
	  	 { do_create,		TFM_Stack	}, /* Create */
	  	 { DoLocate,		0		},
	  	 { DoObjInfo,		0		},
	  	 { InvalidFn,		0		}, /* ServerInfo */
	  	 { do_delete,		TFM_Stack	}, /* Delete */
	  	 { InvalidFn,		0		}, /* Rename */
	  	 { InvalidFn,		0		}, /* Link   */
	  	 { DoProtect,		0		},
	  	 { InvalidFn,		0		}, /* SetDate */
	  	 { DoRefine,		0		},
	  	 { NullFn,		0		}, /* CloseObj */
	  	 { InvalidFn,		0		}, /* Revoke */
	  	 { InvalidFn,		0		}, /* Reserved1 */
	  	 { InvalidFn,		0		}  /* Reserved2 */
	  }
};
/*}}}*/
/*{{{  main() */
/*----------------------------------------------------------------------------*/
/**
*** main()
***
*** argv[0] must be the string SessionManager, because only the Session
*** Manager is allowed to start up Taskforce Managers. argv[1]
*** corresponds to the name of the session, e.g. bart or bart.3
**/

int main(int argc, char **argv)
{
#ifndef __TRAN
  SetPriority(HighServerPri);
#endif
	
#if Use_Malloc_Debugging
  PatchMalloc();
#endif
#if Use_Objects_Debugging
  PatchObjects();
#endif
#if Use_IOC_Debugging
  PatchIOC(1);
#endif
#if Use_Ports_Debugging
  PatchPorts();
#endif
  
  DiagnosticsStream = DefaultDiagnostics = fdstream(1);
#if Redirect_To_Logger
  DiagnosticsStream = Open(cdobj(), "/logger", O_WriteOnly);
#endif

  LastChange = GetDate();

  signal(SIGPIPE, SIG_IGN);    
  InitSemaphore(&(LibraryLock), 1);
  RmProgram = Rm_TFM;
  MRSW_Init();
    
  if ((argc ne 3) || strcmp(argv[0], "SessionManager"))
    fatal("only the Session Manager can start up a Taskforce Manager");

  show_startup(argv[1]);
  
  EncodeCapability(argv[2], &RmLib_Cap);

	/* Initialise the other modules */
  InitJobs();
  InitMap();
  InitRun();
  InitMisc();
  
 	/* Initialise static data */
  if (MachineName(ProcessorName) ne Err_Null)
   fatal("MachineName failure");

  ThisProcessor = Locate(Null(Object), ProcessorName);
  if (ThisProcessor eq Null(Object))
   fatal("failed to locate own processor");
     
  InitNode((ObjNode *) &Root, argv[1], Type_Directory, 0, 0x211109c7);
  InitList(&(Root.Entries));
  InitNode((ObjNode *) &TFM, "tfm", Type_Directory, 0, 0x211109c7);
  InitList(&(TFM.Entries));
  Insert(&Root, (ObjNode *) &TFM, FALSE);

	/* Initialise the domain. This requires very different action	*/
	/* depending on whether or not this is a single-processor	*/
	/* system.							*/
  init_domain();

	/* Allow some signals to be handled asynchronously, since the	*/
	/* Posix library is rarely called.				*/
  init_signals();

	/* Start a process which will terminate the TFM if the current	*/
	/* window disappears.						*/
  if (MonitorDelay >= 0)
	  (void) Fork(CheckWindow_Stack, &CheckWindow, 0);

	/* Load the pipe server into memory	*/
  { Object	*junk = Locate(ThisProcessor, "pipe/xyz");
    if (junk ne Null(Object)) Close(junk);
    PipeCode = Locate(ThisProcessor, "loader/pipe");
    if (PipeCode eq Null(Object))
     report("warning, failed to load pipe server into memory");
  }

	/* Start up a thread to handle incoming RmLib connections	*/
  unless(Fork(AcceptConnections_Stack, &AcceptConnections, 0))
   fatal("out of memory spawning a thread to handle RmLib connections");

	/* Create a name-table entry */
  if ((TFMInfo.ReqPort = NewPort()) eq NullPort)
   fatal("unable to allocate a message port.");

  { NameInfo name;
    name.Port		= TFMInfo.ReqPort;
    name.Flags		= Flags_StripName;
    name.Matrix		= 0x21212147;	/* rz : rz : rz : rwvd */
    name.LoadData	= Null(WORD);   
    NameEntry = Create(ThisProcessor, argv[1], Type_Name, sizeof(NameInfo),
    		(BYTE *) &name);
    if (NameEntry eq Null(Object))
     fatal("failed to enter name in name table, error code %x",
     		Result2(ThisProcessor));
  }
	/* Send back a reply with a capability for the TFM. This ends	*/
	/* up in the environment of the login shell.			*/
  { Capability	cap;
    NewCap(&cap, (ObjNode *) &Root, AccMask_Full);
/* AccMask_R+AccMask_W+AccMask_D+AccMask_V);*/
    if (Write(fdstream(0), (BYTE *) &cap, sizeof(Capability), 5 * OneSec)
    		ne sizeof(Capability))
     fatal("failed to send back a capability for this Taskforce Manager");
  }
  close(0);
  Dispatch(&TFMInfo);
  forever Delay(30 * 60 * OneSec);
  return(0); 
}
/*}}}*/
/*{{{  initialising the domain */
/**-----------------------------------------------------------------------------
*** Initialising the domain. In a multi-processor system the TFM is started up
*** by the Session Manager, and it can get network details from the Network
*** Server. In addition the Session Manager will send it details of the
*** processor running the TFM, since SM obtained this processor to run the
*** TFM.
**/

#ifndef SingleProcessor
/*{{{  usual multi-processor case */
static void init_domain()
{ int i, rc;

  for (i = 0; i < 5; i++)
   { Domain = RmGetNetworkHierarchy();
     if (Domain ne (RmNetwork) NULL) break;
     switch(RmErrno)
      { case	RmE_CommsBreakdown :
       	  report("communications problems when interacting with network server");
     	  break;
        case	RmE_NoMemory :
       	  report("tfm out of memory"); break;
        case	RmE_ServerMemory :
       	  report("network server out of memory"); break;
        case	RmE_BadArgument :
       	  report("problem contacting network server"); break;
        default			:
          report("unexpected resource management library problem %s",
        	RmMapErrorToString(RmErrno));
      }
     report("retrying");
     Delay(OneSec);
   }

  if (Domain eq (RmNetwork) NULL)
   fatal("failed to initialise network hierarchy");

  strcpy(NetworkName, RmGetNetworkId(Domain));
  RmRootName = NetworkName;
  strcpy(Domain->DirNode.Name, "domain");
  Insert(&Root,  (ObjNode *) &(Domain->DirNode), FALSE);

  rc = RmReadProcessor(fdstream(0), &TfmProcessor, FALSE);
  if (rc ne RmE_Success)
    fatal(
    "failed to get processor details from Session Manager, RmLib error %s",
    		RmMapErrorToString(rc));
  else
   { DomainEntry *domain_entry;
   
     TfmProcessor->ObjNode.Key		= NewKey() + _cputime();
     TfmProcessor->AllocationFlags	= RmF_TfmProcessor | RmF_Permanent;
     RmStartSearchHere			= TfmProcessor->Uid;
     unless(AddDomainEntry(TfmProcessor))
      fatal("out of memory when initialising tfm's own processor");
     domain_entry = GetDomainEntry(TfmProcessor);
     domain_entry->NumberUsers = 1;
     if (RmInsertProcessor(Domain, TfmProcessor) eq (RmProcessor) NULL)
      fatal("failed to insert tfm processor into domain");
   }

	/* Install a handler for exceptions generated by the Network	*/
	/* Server via the Resource Management library			*/
  RmExceptionHandler = (VoidFnPtr) Tfm_ExceptionHandler;
	
	/* Tell the Network Server that the TFM's processor is owned	*/
	/* by this session, and not by the Session Manager.		*/
  { RmRequest	request;
    RmReply	reply;

    Clear(request); Clear(reply);
    request.FnRc	= RmC_RegisterTfm;
    request.Uid		= TfmProcessor->Uid;
    (void) RmXch(&RmParent, &request, &reply);
#ifndef __I860
    { extern void PreallocMsgBufs(int);
      PreallocMsgBufs(reply.Reply1 + 4);
    }
#endif
    TfmProcessor->SessionId = RmGetSession();
  }
}
/*}}}*/
#else
/*{{{  single-processor system, i.e. Tiny Helios */
	/* In a single processor system most of the information has to	*/
	/* be invented. There is definitely no Network Server. There	*/
	/* may or may not be a Session Manager.				*/
static void init_domain()
{ 
  Domain = RmNewNetwork();
  if (Domain eq (RmNetwork) NULL)
   fatal("failed to initialise domain");
   
  strcpy(Domain->DirNode.Name, "domain");
  NetworkName[0] = '\0';
  RmRootName	 = NetworkName;
  Insert(&Root, (ObjNode *) &(Domain->DirNode), FALSE);
    
  TfmProcessor = RmNewProcessor();
  if (TfmProcessor eq (RmProcessor) NULL)
   fatal("failed to initialise TFM processor");

  TfmProcessor->ObjNode.Key	= NewKey() + _cputime();
  TfmProcessor->AllocationFlags	= RmF_TfmProcessor | RmF_Permanent;
  TfmProcessor->ObjNode.Size	= RmS_Running + RmP_Helios;
  TfmProcessor->Connections	= 0;
  RmStartSearchHere		= TfmProcessor->Uid;
  unless(AddDomainEntry(TfmProcessor))
   fatal("out of memory when initialising tfm's own processor");
  strcpy(TfmProcessor->ObjNode.Name, objname(ThisProcessor->Name));

  { ProcStats	*stats = (ProcStats *)Malloc(sizeof(ProcStats) + 
  			(4 * (sizeof(LinkConf) + 3 * sizeof(WORD))) +
  			IOCDataMax);
    if (stats ne Null(ProcStats))
     { if (ServerInfo(ThisProcessor, (BYTE *) stats) eq Err_Null)
        { word	memory = stats->MemMax;
          memory = memory + (256 * 1024) - 1;
          memory &= ~((256 * 1024) - 1);
          TfmProcessor->MemorySize = memory;
          
          switch(stats->Type)
           { case	 800 :
	     case	 805 :
	     case	 801 : TfmProcessor->Type = RmT_T800; break;
             case	 414 : TfmProcessor->Type = RmT_T414; break;
             case	 425 : TfmProcessor->Type = RmT_T425; break;
             case	 400 : TfmProcessor->Type = RmT_T400; break;
             case	0xA3 : TfmProcessor->Type = RmT_Arm; break;
             case   0x320C40 : TfmProcessor->Type = RmT_C40; break;
	     case	0x86 : TfmProcessor->Type = RmT_i860; break;

             default : report("warning, unknown processor type %d", 
             		stats->Type);
           }
        }
       Free(stats);
     }
  }
        
  if (RmAddtailProcessor(Domain, TfmProcessor) eq (RmProcessor) NULL)
   fatal("failed to insert tfm processor into domain");

  TfmProcessor->StructType	= RmL_Obtained;
  Domain->StructType		= RmL_Obtained;
}
/*}}}*/
#endif
/*}}}*/
/*{{{  termination */
/*----------------------------------------------------------------------------*/
/**
*** Terminate the Taskforce Manager. This routine is called in a separate
*** process when the first task to be created, i.e. the shell, terminates.
*** All outstanding tasks and taskforces are aborted. Reclaiming the
*** processors is left to the Network Server, when it detects that the
*** connection has disappeared. Then the Taskforce Manager itself terminates
*** which should signal the Session Manager to unlink the entry for the user.
***
*** This code may also be called as a result of signals.
**/
void TerminateTFM(void)
{ int		number_thingies = 0;
  ObjNode	*node;
  static	bool already_called = FALSE;

	/* Step 1, lock the /tfm directory to avoid terminating	*/
	/* forces screwing up the directory as I walk through	*/
	/* it.							*/
  Wait(&(TFM.Lock));
  if (already_called)
   { Signal(&(TFM.Lock)); forever Delay(30 * 60 * OneSec); }
  else
   already_called = TRUE;

 	/* Step 2, walk through the /tfm directory killing	*/
 	/* off everything. A SIGKILL signal should do the job.	*/
  for (node = Head_(ObjNode, TFM.Entries);
       !EndOfList_(node);
       node = Next_(ObjNode, node))
   { 
     if (node->Type eq Type_Taskforce)
      taskforce_DoSignal((RmTaskforce) node, SIGKILL);
     elif (node->Type eq Type_Task)
      task_DoSignal((RmTask) node, SIGKILL);
     number_thingies++;
   }

	/* Step 3, unlock the directory again to allow things	*/
	/* to terminate completely.				*/
  Signal(&(TFM.Lock));

 	/* Step 4, wait a while to let the thingies die.	*/
  Delay(number_thingies * OneSec);

 	/* Step 5, clean out and exit.				*/ 	
  Delete(NameEntry, Null(char));

	/* At this point various other bits of the system could	*/
	/* start sending signals to the TFM which might foul	*/
	/* up the shutting down of the sockets.			*/
  signal(SIGTERM, SIG_IGN);

  if (Socket_ctos >= 0) abort_socket(Socket_ctos);

  Exit(0);
}

	/* Aborting a socket is unbelievably gruesome, it involves	*/
	/* connecting to that socket and sending it a shutdown message.	*/
static void abort_socket(int fd)
{ struct sockaddr_un	address;
  int			sock_ctos = -1;
  int			sock_stoc = -1;
  Stream		*real_socket	= fdstream(fd);
  int			len;
  Capability		cap;

  sock_ctos = socket(AF_UNIX, SOCK_STREAM, 0); 
  if (sock_ctos < 0) goto done;
  sock_stoc = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_stoc < 0) goto done;

  address.sun_family	= AF_UNIX;
  strcpy(address.sun_path, objname(real_socket->Name));
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

  if (connect(sock_ctos, (struct sockaddr *) &address, len) < 0) goto done;

  address.sun_family	= AF_UNIX;
  strcpy(address.sun_path, objname(real_socket->Name));
  len = strlen(address.sun_path);
  address.sun_path[len - 1] = 'c';
  address.sun_path[len - 4] = 's';
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;
  
  if (connect(sock_stoc, (struct sockaddr *) &address, len) < 0) goto done;

  NewCap(&cap, (ObjNode *) &TFM, AccMask_Full);
  (void) write(sock_ctos, (BYTE *) &cap, sizeof(Capability));
  len	= -1;
  (void) write(sock_ctos, (BYTE *) &len, sizeof(len));
  (void) read(sock_stoc, (BYTE *) &len, sizeof(len));

done:
	/* BLV - closing the various sockets at this point seems to be	*/
	/* a bad idea, there are all kinds of weird deadlock problems	*/
  len = 0;
}
/*}}}*/
/*{{{  signal handling */
/*----------------------------------------------------------------------------*/
/** Signal handling. Abort signals should cause the TFM to go through its
*** termination routine.
**/
static void	mysignalhandler(int);

static void init_signals(void)
{ struct sigaction	temp;
  if (sigaction(SIGINT, Null(struct sigaction), &temp) ne 0)
   { report("warning, failed to access signal handling facilities.");
     return;
   }
  temp.sa_handler	= &mysignalhandler;
  temp.sa_flags	|= SA_ASYNC;
  if (sigaction(SIGINT, &temp, Null(struct sigaction)) ne 0)
   { report("warning, failed to modify signal handling facilities.");
     return;
   }

  if (sigaction(SIGTERM, Null(struct sigaction), &temp) ne 0)
   { report("warning, failed to access signal handling facilities.");
     return;
   }

  temp.sa_handler	 = &mysignalhandler;
  temp.sa_flags		|= SA_ASYNC;
  if (sigaction(SIGTERM, &temp, Null(struct sigaction)) ne 0)
   { report("warning, failed to modify signal handling facilities.");
     return;
   }

  signal(SIGPIPE, SIG_IGN);
}

static void mysignalhandler(int x)
{ x = x;
  TerminateTFM();
}
/*}}}*/
/*{{{  checking the current world's consistency */
/*----------------------------------------------------------------------------*/
/**
*** This routine checks the current diagnostics stream at regular interval.s
*** If the stream has gone away then the TFM aborts.
**/
static	void CheckWindow(void)
{
  forever
   { Object	*x;
     Delay(30 * OneSec);
     Wait(&LibraryLock);
     x = Locate(Null(Object), DiagnosticsStream->Name);
     Signal(&LibraryLock);
     if (x eq Null(Object))
      TerminateTFM();
     Close(x);
   }     
}
/*}}}*/
/*{{{  diagnostics routines */
/*----------------------------------------------------------------------------*/
/**
*** Usual diagnostics routines.
**/
static char	output_buffer[256];
static char	*int_to_string(char *buffer, int x);

static int	process_format(char *format, va_list args)
{ char	*dest;
  char	*null_str= "<null>";

  strcpy(output_buffer, "tfm: ");
  
  for (dest = &(output_buffer[5]) ; *format ne '\0'; format++)
   { if (*format ne '%')
      { *dest++ = *format; continue; }
     switch (*(++format))
      { case	'\0': *dest++ = '%'; format--; break;
        case	'%' : *dest++ = '%'; break;
        case    'c' : *dest++ = (char) va_arg(args, int); break;
        case	's' : { char	*temp = va_arg(args, char *);
			if (temp eq Null(char)) temp = null_str;
                        while (*temp ne '\0') *dest++ = *temp++;
                        break;
                      }
	case	'S' : { Stream	*x = va_arg(args, Stream *);
			char	*temp;
			if (x eq NULL) 
			 temp = null_str;
			else
			 temp = x->Name;
                        while (*temp ne '\0') *dest++ = *temp++;
                        break;
                      }
	case	'O' : { Object *x = va_arg(args, Object *);
			char	*temp;
			if (x eq NULL) 
			 temp = null_str;
			else
			 temp = x->Name;
                        while (*temp ne '\0') *dest++ = *temp++;
                        break;
                      }
	case	'P' :
	case	'N' :
	case	'T' : { RmProcessor x = va_arg(args, RmProcessor);
			char	*temp;
			if (x eq NULL) 
			 temp = null_str;
			else
			 temp = x->ObjNode.Name;
                        while (*temp ne '\0') *dest++ = *temp++;
                        break;
                      }
        case	'x' : { int	x = va_arg(args, int);
        		int	shift;
        		*dest++ = '0'; *dest++ = 'x';
        		for (shift = 28; shift >= 0; shift -= 4)
        		 { int temp = (x >> shift) & 0x0F;
        		   if (temp <= 9)
        		    *dest++ = '0' + temp;
        		   else 
        		    *dest++ = 'a' + temp - 10;
        		 }
        		break;
        	      }
	case	'd' : { int	temp = va_arg(args, int);
	   		dest = int_to_string(dest, temp);
	   		break;
		      }  

	default	    : *dest++ = '%'; *dest++ = *format; break;
      }
    }
  if (DiagnosticsStream->Flags & Flags_Interactive)
   *dest++ = '\r';
  *dest++ = '\n';
  return(dest - output_buffer);
}

static char	*int_to_string_aux(char *buffer, unsigned int i)
{ if (i > 9) buffer = int_to_string_aux(buffer, i / 10);
  *buffer++	= (i % 10) + '0';
  return(buffer);
}

static char	*int_to_string(char *buffer, int x)
{ if (x < 0) { x = -x; *buffer++ = '-'; }
  return(int_to_string_aux(buffer, (unsigned int ) x));
}

void fatal(char *format, ...)
{ va_list	list;
  int		length;
  static	char *message = "TFM : error is fatal, exiting.\n";
  
  Wait(&(LibraryLock));
  va_start(list, format);
  length = process_format(format, list);
  va_end(list);

  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  (void) Write(DiagnosticsStream, message, strlen(message), -1);
  Signal(&LibraryLock);
    
  Exit(EXIT_FAILURE << 8);
}

void report(char *format, ...)
{ va_list	args;
  int		length;  
  
  va_start(args, format);
  
  Wait(&LibraryLock);
  length = process_format(format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  Signal(&LibraryLock);
}

static void show_startup_aux(char *format, ...)
{ va_list args;
  int	  length;
  
  va_start(args, format);
  length = process_format(format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, &(output_buffer[5]), (word)length - 5, -1);
}
  
static void show_startup(char *session_name)
{ Object	*hushlogin = Locate(CurrentDir, ".hushlogin");
  
  if (hushlogin ne Null(Object))
   { Close(hushlogin); return; }
    
  show_startup_aux("\r\n\
\t\tTaskforce Manager version %s, session %s\r\n",
	VersionNumber, session_name);
}
/*}}}*/
/*{{{  main server library routines */

/*{{{  do_open */

/*----------------------------------------------------------------------------*/
/**
*** do_open(). This routine can be called in the following contexts.
*** 1) simply to look at the various directories
*** 2) to open a task or taskforce, in order to send it an environment etc.
**/
static void	do_open_task(ServInfo *, MCB *, Port);
static void	do_open_taskforce(ServInfo *, MCB *, Port);

static void	do_open(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  MsgBuf	*r		= Null(MsgBuf);
  IOCMsg2	*req		= (IOCMsg2 *) m->Control;
  ObjNode	*f;
  char		*pathname	= servinfo->Pathname;
  Port		reqport;

  	/* The target object must exist, the O_Create bit is not supported */
  f = GetTarget(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, Err_Null); return; }
  
  unless( CheckMask(req->Common.Access.Access, (int)(req->Arg.Mode & Flags_Mode)))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Object); return; }
   
	/**
	*** Only the following may be opened:
	*** 1) any directory, including root, domain, tfm, networks, and
	***    taskforces
	*** 2) any task at the tfm top level
	**/
  if ((f->Type & Type_Flags) ne Type_Directory)
   if ((f->Type ne Type_Task) || (f->Parent ne &TFM))
    { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Stream); return; }
   
  FormOpenReply(r, m, f, Flags_Closeable, pathname);
  reqport = NewPort();
  r->mcb.MsgHdr.Reply = reqport;
  PutMsg(&(r->mcb));
  Free(r);

  if (((f->Type & Type_Flags) eq Type_Directory) &&
      (req->Arg.Mode eq O_ReadOnly))
   { DirServer(servinfo, m, reqport); FreePort(reqport); return; }

  if (f->Type eq Type_Task)
   do_open_task(servinfo, m, reqport);
  else
   do_open_taskforce(servinfo, m, reqport);
  FreePort(reqport);
}

/*}}}*/
/*{{{  do_create */

/*----------------------------------------------------------------------------*/
/**
*** do_create() should be called for only two things: creating tasks
*** and creating taskforces that are compatible with the processor manager
*** and the old TFM. It is only legal to create objects inside the /tfm
*** subdirectory, so this must be the context.
**/
static void do_create_task(ServInfo *, Object *, int size, int ptype);
static void do_create_taskforce(ServInfo *, Stream *, ImageHdr *, int size);

static void	do_create(ServInfo *servinfo)
{ MCB		*m 		= servinfo->m;
  IOCCreate	*req		= (IOCCreate *) m->Control;
  TaskInfo	*info		= (TaskInfo *) &(m->Data[req->Info]);
  DirNode	*d;
  Object	*o		= Null(Object);
  Stream	*s		= Null(Stream);
  int		ItsAProgram	= 0;
  int		Ptype;

  Debug(dbg_Create, ("Create request received"));

	/* NHG's Execute() is funny. It does not give the name of the	*/
	/* object to create. This has to be extracted from the program	*/
  d = (DirNode *) GetTarget(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, EO_Directory); return; }

  unless (d eq &TFM)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Directory); return; }

  unless(CheckMask(req->Common.Access.Access, AccMask_W))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Directory); return; }

	/* The access mask currently refers to the parent. It must be	*/
	/* modified to refer to the actual task.			*/
  req->Common.Access.Access = AccMask_Full;

	/* Find out what the program is supposed to be */   
  o = NewObject(RTOA(info->Name), &(info->Cap));  
  if (o eq Null(Object))
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Program); return; }

  Debug(dbg_Create, ("testing program %O", o));
  { ImageHdr	Hdr;
    int		size;

    s = Open(o, Null(char), O_ReadOnly);      
    if (s eq Null(Stream))
     { ErrorMsg(m, EC_Error + EG_Invalid + EO_Program); goto done; }

    size = (int) GetFileSize(s);
    if (size < (int) sizeof(ImageHdr))
     { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Program); goto done; }
     
    if (Read(s, (BYTE *) &Hdr, sizeof(ImageHdr), -1) ne sizeof(ImageHdr))
     { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Program); goto done; }
    

    switch(Hdr.Magic)
     { case	Image_Magic :		/* standard program */
	 if (Hdr.Size <= 0)
	  { ErrorMsg(m, EC_Error + EG_Invalid + EO_Program); goto done; }
	 else
	  ItsAProgram	= (int) Hdr.Size;
         Ptype		= (int)((Hdr.Flags eq 0) ? RmT_Default : Hdr.Flags);
         do_create_task(servinfo, o, ItsAProgram, Ptype);
         break;

       case Taskforce_Magic :
       case RmLib_Magic	    :       	  
       	  ItsAProgram	= 0;
	  do_create_taskforce(servinfo, s, &Hdr, size);
	  s = Null(Stream);	/* closed automatically */
       	  break;

       default :
          ErrorMsg(m, EC_Error + EG_Invalid + EO_Program);
          goto done;
     }
  }     
    
done:
  if (s ne Null(Stream)) Close(s);
  if (o ne Null(Object)) Close(o);
}

/*}}}*/
/*{{{  do_delete */
/*----------------------------------------------------------------------------*/
/**
*** do_delete() may be applied either to a Taskforce or a task, provided it is
*** at the top level. The action to take depends on the KillState held
*** in the TaskEntry structure.
**/
static void do_delete_task(ServInfo *servinfo);
static void do_delete_taskforce(ServInfo *servinfo);

static void do_delete(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  ObjNode	*f;
  IOCCommon	*req		= (IOCCommon *) m->Control;

  Debug(dbg_Delete, ("delete request received"));
    
  f = GetTarget(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_Task); return; }

  unless(CheckMask(req->Access.Access, AccMask_D))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Task); return; }
 
  unless(f->Parent eq &TFM) 
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  if (f->Type eq Type_Task)
   do_delete_task(servinfo);
  elif (f->Type eq Type_Taskforce)
   do_delete_taskforce(servinfo);
  else
   ErrorMsg(m, EC_Error + EG_Unknown + EO_Object);
}
/*}}}*/
/*{{{  do_private, debugging options */
/*----------------------------------------------------------------------------*/
/**
*** do_private(). This is used for enabling/disabling debugging.
**/
static void	do_private(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  ObjNode	*f;
  IOCMsg2	*req	= (IOCMsg2 *) m->Control;
  
  f = GetTarget(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, Err_Null); return; }

  if ((f ne (ObjNode *) &TFM) || ((servinfo->FnCode & FG_Mask) ne FG_GetInfo))
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  if (req->Arg.Mode eq dbg_Revert)
   { DiagnosticsStream = DefaultDiagnostics;
     m->MsgHdr.FnRc = 0;
     ErrorMsg(m, Err_Null);
     return;
   }

  UnLockTarget(servinfo);
  
  if (req->Arg.Mode eq dbg_Redirect)
   { static	char	*message = "tfm: output redirected\n";
     int	length	= strlen(message);
     Stream	*stream;
     WORD	index	= m->Control[6];
     StrDesc	*desc	= (StrDesc *) &(m->Data[index]);
     stream		= NewStream(desc->Name, &(desc->Cap), desc->Mode);
     stream->Pos	= desc->Pos;
     
     if (Write(stream, (BYTE *) message, length, -1) ne length)
      { ErrorMsg(m, EC_Error + EG_Open + EO_Stream);
        Close(stream);
        return;
      }
     if (DiagnosticsStream ne DefaultDiagnostics) Close(DiagnosticsStream);
     DiagnosticsStream = stream;
     m->MsgHdr.FnRc = 0;
     ErrorMsg(m, Err_Null);
     return;
   } 
   
  if (req->Arg.Mode ne dbg_Inquire)   
   DebugOptions = req->Arg.Mode;
  if (DebugOptions & dbg_Memory)
   { report("currently %d bytes are free, the heap size is %d", Malloc(-1),
   		Malloc(-3));
     DebugOptions &= ~dbg_Memory;
#if ShowHeap_Debugging
     ShowHeap();
#endif
   }
  if (DebugOptions & dbg_Lock)
   { MRSW_Info	buf;
     MRSW_GetInfo(&buf);
     report("lock status: ar %d, rr %d, aw %d, rw %d, sw %d",
		buf.ar, buf.rr, buf.aw, buf.rw, buf.sw);
     DebugOptions &= ~dbg_Lock;
   }
#if Use_IOC_Debugging
  if (DebugOptions & dbg_IOC)
   { ShowIOC();
     DebugOptions &= ~dbg_IOC;
   }
#endif

  m->MsgHdr.FnRc = DebugOptions;
  ErrorMsg(m, Err_Null);  
}
/*}}}*/

/*}}}*/

	/* These routines are called from the generic do_create() etc.	*/
	/* and do the real work.					*/
/*{{{  do_create_task() */

/*----------------------------------------------------------------------------*/
/**
*** This code deals with requests for individual tasks, e.g. /bart/tfm/shell.1
***
*** 1) do_create_task()
***   Given a Create request and an Object for the binary task,
***   together with some information about the target program,
***   start the task running.
***
***   a) obtain and fill in an RmTask structure. Eventually this will be
***      added to the /tfm directory.
***   b) try to get a suitable processor for running this program.
***   c) execute and run the program. Store the required information in the
***      RmTask structure (Object pointer and Stream).
***   d) retry this a number of times, in case the processor chosen
***      was short of memory
***   e) add the task to the /tfm subdirectory
***   f) all done. Construct the reply and send it.
**/

static void do_create_task(ServInfo *servinfo, Object *itsprogram,
			   int progsize, int ptype)
{ RmProcessor	processor	= (RmProcessor) NULL;
  RmTask	task		= (RmTask) NULL;
  MCB		*m		= servinfo->m;
  bool		success		= FALSE;
  MsgBuf	*r		= Null(MsgBuf);
  char		*pathname	= servinfo->Pathname;
  int		retries		= TaskRetries;
  TaskEntry	*task_entry;
  Object	*program	= CopyObject(itsprogram);
  word		rc		= EC_Error + EG_Create + EO_Program;

  Debug(dbg_Create, ("creating task"));

  UnLockTarget(servinfo);

  if (program eq Null(Object))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); goto done; }
  r = New(MsgBuf);
  
  if (r eq Null(MsgBuf))
   { Close(program);
     ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); 
     goto done; 
   }

  task = RmNewTask();             
  if (task eq (RmTask) NULL)
   { Close(program); 
     ErrorMsg(m, EC_Error + EG_NoMemory + EO_Task); 
     goto done; 
   }

  if (task_AddTaskEntry(task) ne RmE_Success)
   { Close(program); 
     ErrorMsg(m, EC_Error + EG_Invalid + EO_Taskforce); 
     goto done; 
   }
  task_entry		  = GetTaskEntry(task);
  task_entry->ProgramSize = progsize;
  task_entry->Program	  = program;
  
  if ((RmSetTaskId(task, objname(program->Name)) ne RmE_Success) ||
      (RmSetTaskType(task, ptype) ne RmE_Success) ||
      (RmSetTaskMemory(task, (3 * (unsigned long)progsize)) ne RmE_Success))
   { ErrorMsg(m, EC_Error + EG_Broken + EO_TFM); goto done; }

	/* Sort out the name.					*/
  	/* Allow five digits, . and the terminator		*/
  if (strlen(task->ObjNode.Name) > (NameMax - (5 + 1 + 1)))
   { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Name); goto done; }
  strcat(&(task->ObjNode.Name[0]), ".");
  Wait(&LibraryLock);		/* TaskSequenceNumber must be locked */
  if (TaskSequenceNumber eq 1)
   task->ObjNode.Flags |= TfmFlags_FirstTask;
  addint(&(task->ObjNode.Name[0]), TaskSequenceNumber++);
  Signal(&LibraryLock);
  pathcat(pathname, task->ObjNode.Name);


  MRSW_GetWrite();

  for (retries = TaskRetries; retries > 0; retries--)
   { 
     Debug(dbg_Create, ("attempting mapping for %T", task));

     if (!domain_MapTask(task))
      { rc = EC_Error + EG_NoResource + EO_Processor; break; }

     processor = RmFollowTaskMapping(Domain, task);
     rc = task_Run(processor, task, program);
     if (rc eq Err_Null) 
      { success = TRUE; break; }

     Debug(dbg_Create, ("failed to execute task %T", task));
          
     if ((rc & EG_Mask) eq EG_NoMemory) break;
      
     MarkProcessor(processor);	/* This processor is suspicious */
     domain_UnmapTask(task);
   }

  if (success)  
   {	/* Add the task to the directory, and send back a reply */
     Debug(dbg_Create, ("task %T is running", task));
     Insert(&(TFM), &(task->ObjNode), FALSE);
     FormOpenReply(r, m, &(task->ObjNode), 0, pathname);
     PutMsg(&(r->mcb));
   }
  else
   ErrorMsg(m, rc);

  MRSW_FreeWrite();
   
done:
  if (!success && (task ne (RmTask) NULL))
   { MRSW_GetWrite();
     task_Destroy(task);
     MRSW_FreeWrite();
   }
  if (r ne Null(MsgBuf)) Free(r);
}

/*}}}*/
/*{{{  do_open_task() */

/**
*** 2) do_open_task().
***    This is used for the stream operations on a single
***    executing task. The current target in the servinfo structure is
***    an RmTask structure which is already being executed and monitored.
***    The Open() request has been acknowledged, so all this routine does
***    is accept requests and forward them down the appropriate stream.
**/

static void do_open_task(ServInfo *servinfo, MCB *m, Port reqport)
{ RmTask	task		= (RmTask) servinfo->Target;
  TaskEntry	*task_entry	= GetTaskEntry(task);
  BYTE		*Data		= m->Data;
  WORD		*Control	= m->Control;
  WORD		e;

  Debug(dbg_Create, ("open request for %T", task));
  
  task_entry->UseCount++;
  task->ObjNode.Account += 10000;

  UnLockTarget(servinfo);
  
  forever
   { m->MsgHdr.Dest	= reqport;
     m->Data		= Data;
     m->Control		= Control;
     m->Timeout		= StreamTimeout;

     e = GetMsg(m);
     if ((e < Err_Null) && (e ne EK_Timeout)) continue;
     Wait(&(task->ObjNode.Lock));
     task_entry = GetTaskEntry(task);

     if (e eq EK_Timeout)
      { unless(RmGetTaskState(task) & RmS_Running) task_GenProgInfo(task);
        break;
      }

     switch (m->MsgHdr.FnRc & FG_Mask)
      { case FG_SendEnv		:
		{ Environ received;
		  Port	  reply = tfm_GetEnv(reqport, m, &received);
		  word	  rc;
		  if (reply eq NullPort) break; /* tfm_GetEnv sent error */

		  MRSW_GetRead();
		  rc = task_HandleEnv(task, &received);
		  InitMCB(m, 0, reply, NullPort, rc);
		  PutMsg(m);
		  tfm_FreeEnv(&received);
		  if (rc < Err_Null)
		   task_DoSignal(task, SIGKILL);
		  MRSW_FreeRead();
		  break;
		}
      
        case FG_Signal 		:
		MRSW_GetRead();
        	task->ObjNode.Account += 10;
        	m->MsgHdr.FnRc = SS_TFM;
        	ErrorMsg(m, 0);
        	task_DoSignal(task, m->Control[0]);
        	task->ObjNode.Account += 10;
		MRSW_FreeRead();
        	break;
			        
        case FG_ProgramInfo	:
		{ Port	port		 = m->MsgHdr.Reply;
		  word	mask		 = m->Control[0];

		  MRSW_GetRead();
		  mask			&= PS_Terminate;
		  FreePort(task_entry->ProgInfoPort);
		  task->ObjNode.Account	+= 100;
		  task_entry->ProgInfoPort = (mask == 0) ? NullPort : port;
		  task_entry->ProgInfoMask = (int) mask;
		  InitMCB(m , (mask eq 0) ? 0 : MsgHdr_Flags_preserve,
		  	port, NullPort, Err_Null);
		  MarshalWord(m, mask);
		  MarshalWord(m, 1);
		  PutMsg(m);
		  task->ObjNode.Account += 100;
		  unless (RmGetTaskState(task) & RmS_Running)
		   task_GenProgInfo(task);
		  MRSW_FreeRead();
		  break;
		}

        case FG_Close		:
        	if (m->MsgHdr.Reply ne NullPort) ErrorMsg(m, 0);
        	m->MsgHdr.FnRc = SS_TFM;
        	goto done;
       
        default			:
        	ErrorMsg(m, EC_Error + EG_FnCode + EO_Task);
        	break;
      }
     
     Signal(&(task->ObjNode.Lock));
   }

done:
  task_entry->UseCount--;
  task->ObjNode.Account -= 10000;
  if (task_entry->UseCount eq 0)
   { MRSW_GetWrite();
     task_Destroy(task);
     MRSW_FreeWrite();
   }
  else
   Signal(&(task->ObjNode.Lock));
}

/*}}}*/
/*{{{  do_delete_task() */
/**
*** 3) do_delete_task().
***    As per the processor manager.
**/

static void	do_delete_task(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  RmTask	task 		= (RmTask) servinfo->Target;
  TaskEntry	*task_entry	= GetTaskEntry(task);

  MRSW_GetRead();
  
  if (task_entry eq Null(TaskEntry))
   { ErrorMsg(m, EC_Error + EG_Broken + EO_Task); goto done; }
  if (task_entry->ProgramObject eq Null(Object))
   { ErrorMsg(m, EC_Error + EG_Broken + EO_Task); goto done; }

  Debug(dbg_Delete, ("deleting task %T, kill state %d", \
	  	task, task_entry->KillState));
  		
  switch(task_entry->KillState++)
   { case	0 : task_DoSignal(task, SIGINT); break;
     case	1 : task_DoSignal(task, SIGKILL); break;
     case	2 : task_Exterminate(task); break;
   }

  ErrorMsg(m, Err_Null);
done:
  MRSW_FreeRead();
}
/*}}}*/
/*{{{  do_create_taskforce() */
/*----------------------------------------------------------------------------*/
/**
*** Similar routines to the above, but for handling taskforces. Most
*** of these routines are similar to the routines for handling
*** single tasks, but tend to walk down the taskforce applying the operation
*** to every component.
***
*** 1) do_create_taskforce()
*/

static int  update_mapping(RmTask, ...);

static void do_create_taskforce(ServInfo *servinfo, Stream *CDLBinary, ImageHdr *hdr, int size)
{ RmTaskforce	taskforce	= (RmTaskforce) NULL;
  RmNetwork	network		= (RmNetwork) NULL;
  MCB		*m		= servinfo->m;
  bool		success		= FALSE;
  MsgBuf	*r		= Null(MsgBuf);
  char		*pathname	= servinfo->Pathname;
  int		retries;
  int		rc;
  TaskEntry	*task_entry	= Null(TaskEntry);
  
  Debug(dbg_Create, ("creating taskforce"));

  UnLockTarget(servinfo);
            
  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); goto done; }

	/* The binary object may be an old-style CDL binary or a binary	*/
	/* produced via the Resource Management library			*/
  if (hdr->Magic eq Taskforce_Magic)
   taskforce = RmReadCDL(CDLBinary, hdr, size);
  else
   { int *junk = (int *)hdr;
     rc = RmE_Success;
     if (junk[1])
      rc = RmReadNetwork(CDLBinary, &network, FALSE);
     if (junk[2] && (rc eq RmE_Success))
      rc = RmReadTaskforce(CDLBinary, &taskforce, FALSE);
     if ((network ne (RmNetwork) NULL) && (taskforce ne (RmTaskforce) NULL))
      (void) RmApplyTasks(taskforce, &update_mapping, network);
     Close(CDLBinary);
   }
  if (taskforce eq (RmTaskforce) NULL)
   { Debug(dbg_Create, ("error reading file"));
     ErrorMsg(m, EC_Error + EG_Invalid + EO_Taskforce); 
     goto done; 
   }

  if (task_AddTaskEntry((RmTask) taskforce) ne RmE_Success)
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Taskforce); goto done; }
  task_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);

  rc = RmSearchTasks(taskforce, &task_AddTaskEntry);

  if (rc eq RmE_ServerMemory)
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Taskforce); goto done; }
  elif (rc eq RmE_NotFound)
   { ErrorMsg(m, EC_Error + EG_Unknown + EO_Program); goto done; }
  elif (rc ne RmE_Success)
   { ErrorMsg(m, EC_Error + EG_Broken + EO_Program); goto done; }
   
	/* Allow five digits, . and the terminator */
  if (strlen(taskforce->DirNode.Name) > (NameMax - (5 + 1 + 1)))
   { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Name); goto done; }
  strcat(&(taskforce->DirNode.Name[0]), ".");
  Wait(&LibraryLock);		/* TaskSequenceNumber must be locked */
  if (TaskSequenceNumber eq 1)
   taskforce->DirNode.Flags |= TfmFlags_FirstTask;
  addint(&(taskforce->DirNode.Name[0]), TaskSequenceNumber++);
  Signal(&LibraryLock);
  pathcat(pathname, taskforce->DirNode.Name);

  for (retries = TaskforceRetries; retries > 0; retries--)
   { word e;

     Debug(dbg_Create, ("attempting to map taskforce %T", taskforce));

     MRSW_GetWrite();
     e = domain_MapTaskforce(network, taskforce);
     MRSW_FreeWrite();

     unless(e)
      { ErrorMsg(m, EC_Error + EG_NoResource + EO_Network); goto done; }
     task_entry->Mapped = TRUE;

     Debug(dbg_Create, ("attempting to start taskforce %T", taskforce));

     MRSW_GetRead();     	
     e = taskforce_Start(taskforce);
     MRSW_FreeRead();

     if (e eq Err_Null)   	/* The Taskforce is now up and running */
      { success = TRUE; break; }
     
     if ((e & EG_Mask) eq EG_NoMemory) break;

     MRSW_GetWrite();          
     domain_UnmapTaskforce(taskforce);
     MRSW_FreeWrite();

     task_entry->Mapped = FALSE;
   }
   
  if (success)      	
   {	/* Add it to the /tfm directory and send back a reply */
     Debug(dbg_Create, ("taskforce %T is running", taskforce));
     Insert(&(TFM), (ObjNode *) &(taskforce->DirNode),  FALSE);
     FormOpenReply(r, m, (ObjNode *) &(taskforce->DirNode), 0, pathname);
     PutMsg(&(r->mcb));
   }
  else
   ErrorMsg(m, EC_Error + EG_Create + EO_Taskforce);
      
done:
  if (!success && (taskforce ne (RmTaskforce) NULL))
   { MRSW_GetWrite();
     taskforce_Destroy(taskforce);
     MRSW_FreeWrite();
   }
  if (r ne Null(MsgBuf)) Free(r);
  if (network ne (RmNetwork) NULL)
   RmFreeNetwork(network);
}

	/* Because the network and taskforce are read separately rather	*/
	/* than via RmReadStream(), the task <-> processor mapping may	*/
	/* be screwed up. This code sorts out the mess.			*/
static int update_mapping(RmTask task, ...)
{ va_list	args;
  RmNetwork	network;
  RmProcessor	processor;

  va_start(args, task);
  network = va_arg(args, RmNetwork);
  va_end(args);

  if (task->MappedTo ne RmL_NoUid)
   { processor = RmFindProcessor(network, task->MappedTo);
     task->MappedTo = RmL_NoUid;
     if (processor ne (RmProcessor) NULL)
      RmMapTask(processor, task);
   }
  return(0);
}
/*}}}*/
/*{{{  do_open_taskforce() */

/*----------------------------------------------------------------------------*/
/**
*** 2) do_open_taskforce().
***    This is used for the stream operations on a single
***    executing taskforce. The current target in the servinfo structure is
***    an RmTaskforce structure which is already being executed and monitored.
***    The Open() request has been acknowledged, so all this routine does
***    is accept requests and forward them down the appropriate stream.
**/

static void do_open_taskforce(ServInfo *servinfo, MCB *m, Port reqport)
{ RmTaskforce	taskforce	= (RmTaskforce) servinfo->Target;
  TaskEntry	*task_entry	= (TaskEntry *) RmGetTaskforcePrivate(taskforce);
  BYTE		*Data		= m->Data;
  WORD		*Control	= m->Control;
  WORD		e;

  Debug(dbg_Create, ("open request for taskforce %T", taskforce));
  		
  task_entry->UseCount++;
  taskforce->DirNode.Account += 10000;

  UnLockTarget(servinfo);

  forever
   { m->MsgHdr.Dest	= reqport;
     m->Data		= Data;
     m->Control		= Control;
     m->Timeout		= StreamTimeout;

     e = GetMsg(m);

     if ((e < Err_Null) && (e ne EK_Timeout)) continue;
     Wait(&(taskforce->DirNode.Lock));
     task_entry = (TaskEntry *) RmGetTaskforcePrivate(taskforce);

     if (e eq EK_Timeout)
      { unless(RmGetTaskforceState(taskforce) & RmS_Running)
	 taskforce_GenProgInfo(taskforce);
        break;
      }
     
     m->MsgHdr.FnRc = 0;
     switch (e & FG_Mask)
      { case FG_SendEnv		:
		{ Environ	received;
		  Port		reply;
		  word		rc;

		  m->MsgHdr.FnRc = e;	/* needed by Tfm_GetEnv */
		  reply = tfm_GetEnv(reqport, m, &received);
		  if (reply eq NullPort) break; /* tfm_GetEnv sent error */

		  if (taskforce->DirNode.Flags & TfmFlags_GotEnviron)
		   { m->MsgHdr.FnRc = SS_TFM;
		     ErrorMsg(m, EC_Error + EG_InUse + EO_Stream);
		     break;
		   }
			/* Send back an initial message to the client	*/
			/* requesting a long timeout.			*/
		  InitMCB(m, MsgHdr_Flags_preserve, reply, NullPort,
				EC_Recover + EG_NewTimeout + 300);
		  PutMsg(m);

		  MRSW_GetRead();
		  rc = taskforce_HandleEnv(taskforce, &received);
		  InitMCB(m, 0, reply, NullPort, rc);
		  PutMsg(m);
		  tfm_FreeEnv(&received);
		  if (rc < Err_Null)
      		   taskforce_DoSignal(taskforce, SIGKILL);
                  taskforce->DirNode.Flags |= TfmFlags_GotEnviron;
		  MRSW_FreeRead();
      		  break;
		}
      
        case FG_Signal 		:
		MRSW_GetRead();
        	taskforce->DirNode.Account += 10;
        	m->MsgHdr.FnRc = SS_TFM;
        	ErrorMsg(m, 0);
        	taskforce_DoSignal(taskforce, m->Control[0]);
        	taskforce->DirNode.Account += 10;
		MRSW_FreeRead();
        	break;
			        
        case FG_ProgramInfo	:
		{ Port	port		 = m->MsgHdr.Reply;
		  word	mask		 = m->Control[0];
		  MRSW_GetRead();
		  mask			&= PS_Terminate;
		  FreePort(task_entry->ProgInfoPort);
		  taskforce->DirNode.Account	+= 100;
		  task_entry->ProgInfoPort = (mask == 0) ? NullPort : port;
		  task_entry->ProgInfoMask = (int) mask;
		  InitMCB(m , (mask eq 0) ? 0 : MsgHdr_Flags_preserve,
		  	port, NullPort, Err_Null);
		  MarshalWord(m, mask);
		  MarshalWord(m, 1);
		  PutMsg(m);
		  taskforce->DirNode.Account += 100;
		  unless(RmGetTaskforceState(taskforce) & RmS_Running)
                   taskforce_GenProgInfo(taskforce);

		  MRSW_FreeRead();
		  break;
		}

        case FG_Close		:
		Debug(dbg_Create, ("close request"));
        	if (m->MsgHdr.Reply ne NullPort) ErrorMsg(m, 0);
        	m->MsgHdr.FnRc = SS_TFM;
        	goto done;
       
        default			:
        	ErrorMsg(m, EC_Error + EG_FnCode + EO_Task);
        	break;
      }
     Signal(&(taskforce->DirNode.Lock));
   }

done:
  task_entry->UseCount--;
  taskforce->DirNode.Account -= 10000;
  if (task_entry->UseCount eq 0)
   { MRSW_GetWrite();
     taskforce_Destroy(taskforce);
     MRSW_FreeWrite();
   }
  else
   Signal(&(taskforce->DirNode.Lock));
}

/*}}}*/
/*{{{  do_delete_taskforce() */
/**
*** 3) do_delete_taskforce()
**/
static int	do_delete_taskforce_aux(RmTask task, ...);

static void	do_delete_taskforce(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  RmTaskforce	Taskforce	= (RmTaskforce) servinfo->Target;
  TaskEntry	*task_entry	= (TaskEntry *) RmGetTaskforcePrivate(Taskforce);

  MRSW_GetRead();
  
  if (task_entry eq Null(TaskEntry))
   { ErrorMsg(m, EC_Error + EG_Broken + EO_Task); goto done; }

  Debug(dbg_Delete, ("deleting taskforce %T, current kill state is %d",\
	  	Taskforce, task_entry->KillState));
  	
  switch(task_entry->KillState++)
   { case	0 : taskforce_DoSignal(Taskforce, SIGINT); break;
     case	1 : taskforce_DoSignal(Taskforce, SIGKILL); break;
     case	2 : 
      (void) RmApplyTasks(Taskforce, &do_delete_taskforce_aux);
   }
  ErrorMsg(m, Err_Null);
done:
  MRSW_FreeRead();
}

static int do_delete_taskforce_aux(RmTask task, ...)
{ 
  task_Exterminate(task);
  return(0);
}
/*}}}*/





