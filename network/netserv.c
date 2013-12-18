/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netserv.c								--
--                                                                      --
--	The main module of the Network Server				--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/netserv.c,v 1.31 1993/08/12 12:02:43 nickc Exp $*/

/*{{{  version number and history */
static char *VersionNumber = "3.29";
/**
*** History :
*** 	      3.00, initial version
***           3.01, internal development
***           3.02, 1.2 beta 0 release
***           3.03, 1.2 beta 1 release
***           3.04, developed at Parsytec, 22.10.90-30.10.90
***           3.05, finishing off for the official 1.2 release
***           3.06, the real release
***	      3.07, first post 1.2 version
***           3.08, every processor now has a PUID attribute
***           ...
***           3.20, native network support added
***           3.21, cleaned up network agent interaction
***           3.22, cleaned up communication, added support for session ids
***           3.23, fault tolerance work
***           3.24, change of rmlib-server communication to sockets
***           3.25, further changes to netagent comms
***           3.26, C40 changes, plus fixed a memory corruption bug
***           3.27, C40 version now supports IDRom
***           3.28, minor fixes for final C40 release
***           3.29, memory reduction exercise for C40 1.3.1 release
**/
/*}}}*/
/*{{{  headers and compile-time options */
#define	__Netserv_Module
#define __NetworkServer

#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <link.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <root.h>
#include <stdarg.h>
#include <posix.h>
#include <signal.h>
#include <ctype.h>
#include <stddef.h>
#include <nonansi.h>
#include <process.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "netaux.h"

	/* Compile-time debugging options	*/
#define Use_Malloc_Debugging		0
#define Use_Objects_Debugging		0
#define ShowHeap_Debugging		0
#define Use_IOC_Debugging		0
#define Default_Diagnostics_Options	0
#define Use_Ports_Debugging		0
/*}}}*/
/*{{{  forward declarations and statics */
/**
*** Forward declarations.
**/
static void		do_open(ServInfo *);
static void		do_private(ServInfo *);
static void		show_startup(void);
static void		accept_rmlib(void);
static void		abort_socket(int);

/**
*** Variables, static and global ones
**/       
	char		ProcessorName[IOCDataMax];
	Object		*ThisProcessor;
	RmNetwork	Net;
	RmTaskforce	DefaultTaskforce;
	RmProcessor	RootProcessor = RmM_NoProcessor;
	RmProcessor	BootIOProcessor = RmM_NoProcessor;
static	Object		*NameEntry;
	char		NetworkName[NameMax];
	int		NumberProcessors = 0;
	Object		*NetAgent = Null(Object);
	bool		FullReset		= FALSE;
	bool		SilentMode		= FALSE;
static	bool		ReceivedNetwork		= FALSE;
static	bool		NameInstalled		= FALSE;
static	Stream		*DiagnosticsStream	= Null(Stream);
static	Stream		*DefaultDiagnostics	= Null(Stream); 
static	Semaphore	LibraryLock;
static	NetworkFuncs	Functions;
	WORD		DebugOptions		= Default_Diagnostics_Options;
	WORD		LastChange;
	RmProcessor	LastBooted		= RmM_NoProcessor;
static	int		Socket_ctos		= -1;
static	int		Socket_stoc		= -1;
	char		**nsrc_environ		= NULL;
	
static DispatchInfo NetservInfo =
	{ Null(DirNode),
	  NullPort,
	  SS_NetServ,
	  Null(char),
	  { do_private, NS_Stack },
	  {
	  	 { do_open,		NS_Stack	},
	  	 { InvalidFn,		0		},	/* Create */
	  	 { DoLocate,		0		},
	  	 { DoObjInfo,		0		},
	  	 { InvalidFn,		0		}, /* ServerInfo */
	  	 { InvalidFn,		0		}, /* Delete */
	  	 { InvalidFn,		0		}, /* Rename */
	  	 { InvalidFn,		0		}, /* Link   */
	  	 { DoProtect,		0		},
	  	 { InvalidFn,		0		}, /* SetDate */
	  	 { DoRefine,		0		},
	  	 { NullFn,		0		}, /* CloseObj */
	  	 { InvalidFn,		0		}, /* Revoke */
	  	 { InvalidFn,		0		}, /* reserved1 */
	  	 { InvalidFn,		0		}  /* reserved2 */
	  }
};
/*}}}*/
/*{{{  main() */
/**
*** main()
***
*** 1) check the arguments, to ensure that the Network Server was started
***    up by the Startns program.
***
*** 2) fill in the various structures, ready for starting up the server
***    Locate the network agent program
***
*** 3) call the various initialisation routines in the other modules
***
*** 4) Fork off a thread to accept incoming RmLib connections
***
*** 5) create the name table entry, storing the result in a static so that
***    it can be deleted when a terminate request has been received.
***
*** 6) call the dispatcher
***
*** Note that the Startns utility tries very hard to ensure that this program
*** runs only on the root processor.
**/

int main(int argc, char **argv)
{
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

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif

  signal(SIGPIPE, SIG_IGN);
  
  DiagnosticsStream = DefaultDiagnostics = fdstream(1);
  InitSemaphore(&(LibraryLock), 1);
  MRSW_Init();

  LastChange = GetDate();
      
/**
*** Arguments : argv[0] = "startns-magic", just to confuse people
***             argv[1] = "-r"
***		argv[2] = "-q"
***		argv[3] = debugging options
***		argv[4]-> nsrc options
**/   
  argc		= argc;
  if (strcmp(*argv, "startns-magic"))
   { fatal("please use startns to start up the networking software");
     exit(1);
   }
  show_startup();
  
  if (!strcmp(argv[1], "-r")) FullReset		= TRUE;
  if (!strcmp(argv[2], "-q")) SilentMode	= TRUE;
  { char *temp = argv[3];
    for ( ; *temp ne '\0'; temp++)
     switch(*temp)
      { case	'b'	: DebugOptions |= dbg_Boot;	break;
        case	'e'	: DebugOptions |= dbg_Execute;	break;
        case	'a'	: DebugOptions |= dbg_Allocate;	break;
        case	'r'	: DebugOptions |= dbg_Release;	break;
        case	'm'	: DebugOptions |= dbg_Monitor;	break;
        case	'p'	: DebugOptions |= dbg_Problem;	break;
        case	'l'	: DebugOptions |= dbg_Links;	break;
	case	'i'	: DebugOptions |= dbg_Initialise;break;
	case	'n'	: DebugOptions |= dbg_Native;	break;
	case	'c'	: DebugOptions |= dbg_Comms;	break;
      }
  }   
  nsrc_environ = &(argv[4]);

  NetAgent = Locate(CurrentDir, NetAgentCode);
  if (NetAgent eq Null(Object))
   fatal("failed to locate network program %s", NetAgentCode);
  if (get_config("preload_netagent", nsrc_environ) ne Null(char))
   { Object	*loader = Locate(Null(Object), "/loader");
     Object	*link;
     if (loader ne Null(Object))
      { if (Link(loader, objname(NetAgent->Name), NetAgent) >= Err_Null)
         { Stream *tmp = Open(loader, objname(NetAgent->Name), O_ReadOnly);
	   if (tmp ne NULL)
	    { Close(tmp);	/* Sole purpose of Open is to load the code */
	      link = Locate(loader, objname(NetAgent->Name));
              if (link ne Null(Object))
               { 	/* Now that the code is loaded it can be protected */
	         Protect(link, NULL, 0x05050585);	/* rea:re;re:re */
	         Close(NetAgent);
                 NetAgent = link;
               }
            }
	 }
        Close(loader);
      }
   }
   
  Net = RmNewNetwork();
  if (Net eq (RmNetwork)NULL)
   fatal("insufficient memory to initialise");
  strcpy(Net->DirNode.Name, "ns");
  NetservInfo.Root = (DirNode *) Net;
  NetworkName[0] = '\0';
  
  if (MachineName(ProcessorName) ne Err_Null)
   fatal("MachineName failure");
   
  ThisProcessor = Locate(Null(Object), ProcessorName);
  if (ThisProcessor eq Null(Object))
   fatal("failed to locate own processor");

  if ((NetservInfo.ReqPort = NewPort()) eq NullPort)
   fatal("unable to allocate a message port");

	/* Initialise the constant table of network functions	*/
  Functions.report		= (VoidFnPtr) &report;
  Functions.fatal		= (VoidFnPtr) &fatal;
  Functions.LookupProcessor	= (WordFnPtr) &RmLookupProcessor;
  Functions.rexec		= (WordFnPtr) &rexec;
  Functions.BuildConfig		= (WordFnPtr) &BuildTRANConfig;
  Functions.StartNetworkAgent	= (WordFnPtr) &StartNetworkAgent;
  Functions.StopNetworkAgent	= (WordFnPtr) &StopNetworkAgent;
  Functions.XchNetworkAgent	= (WordFnPtr) &XchNetworkAgent;
  
	/*  Call initialisation code in the other modules.	*/
  InitBootstrap();
  InitMonitor();
  InitAlloc();
  InitNative();
  InitMisc();

	/* Prepare to receive incoming rmlib requests.		*/
  unless(Fork(AcceptRmLib_Stack, &accept_rmlib, 0))
   fatal("failed to spawn thread to handle incoming connections");
  
	/* Enter the name in the name table */
  { NameInfo name;
    name.Port   = NetservInfo.ReqPort;
    name.Flags  = Flags_StripName;
    name.Matrix = DefDirMatrix;		/* for now - will get updated later */
    name.LoadData = Null(WORD);
    NameEntry = Create(ThisProcessor, "ns", Type_Name, sizeof(NameInfo),
      (BYTE *) &name);
    if (NameEntry eq Null(Object))
     fatal("failed to enter name in name table, error code %x",
           Result2(ThisProcessor));
  }

  NameInstalled = TRUE;
  
  Dispatch(&NetservInfo);

	/* Dispatch returns when the name table entry is deleted by	*/
	/* TerminateNetworkServer() below.				*/	
  forever Delay(30 * 60 * OneSec);
  
  return(0);
}
/*}}}*/
/*{{{  diagnostics routines */
/*----------------------------------------------------------------------------*/
/**
*** Usual diagnostics routines.
**/
static char	output_buffer[256];
static char	*int_to_string(char *buffer, int x);

static int	process_format(char *init, char *format, va_list args)
{ char	*dest;
  char	*null_str= "<null>";

  strcpy(output_buffer, init);
  
  for (dest = output_buffer + strlen(output_buffer); *format ne '\0'; format++)
   { if (*format ne '%')
      { *dest++ = *format; continue; }
     switch (*(++format))
      { case	'\0': *dest++ = '%'; format--; break;
        case	'%' : *dest++ = '%'; break;
        case    'c' : *dest++ = (char) va_arg(args, int); break;

	case	'W' :	/* C40 string in Global memory, for device drivers.		*/
#ifdef __C40		/* Assumes netserv and driver are loaded into the same strobe	*/
		      { char		buf[80], *temp;
			int		format_mp, code_mp;

			format_mp   = (int)  va_arg(args, char *);
			format_mp >>= 2;
			format_mp  += (int) _DataToFuncConvert(NULL);
			code_mp     = (int) &process_format;
			code_mp    &= 0xC0000000;
			format_mp  |= code_mp;
			MP_GetData(buf, format_mp, 0, 80 / sizeof(int));
			for (temp = buf; *temp ne '\0'; *dest++ = *temp++);
			break;
		      }
#endif	/* for non-C40, 'W' == 's' so just drop through	*/

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

	case	'L' : { int   temp = va_arg(args, int);
			char *str;
			switch(temp)
			 { case RmL_NotConnected : str = "NotConnected"; break;
			   case RmL_Dumb	 : str = "Dumb"; break;
			   case RmL_Intelligent  : str = "Running"; break;
			   case RmL_Pending	 : str = "Pending"; break;
			   case RmL_Dead	 : str = "Dead"; break;
			   default		 : str = "<unknown>";
			 }
			while (*str ne '\0') *dest++ = *str++;
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
  static	char *message = "ns: error is fatal, exiting.\n";
  
  Wait(&(LibraryLock));
  va_start(list, format);
  length = process_format("ns: ", format, list);
  va_end(list);

  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  (void) Write(DiagnosticsStream, message, strlen(message), -1);
  Signal(&LibraryLock);

  if (NameInstalled) (void) Delete(NameEntry, Null(char));

	/* If the sockets have been created, get rid of them	*/
  if (Socket_ctos >= 0) abort_socket(Socket_ctos);
      
  Exit(EXIT_FAILURE << 8);
}

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

  memset(&cap, 0, sizeof(Capability));
  (void) write(sock_ctos, (BYTE *) &cap, sizeof(Capability));
  len	= -1;
  (void) write(sock_ctos, (BYTE *) &len, sizeof(len));

done:
  if (sock_ctos >= 0) close(sock_ctos);
  if (sock_stoc >= 0) close(sock_stoc);
}

void report(char *format, ...)
{ va_list	args;
  int		length;  
  
  va_start(args, format);
  
  Wait(&LibraryLock);
  length = process_format("ns: ", format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  Signal(&LibraryLock);
}

void debug(char *format, ...)
{ va_list	args;
  int		length;
  
  va_start(args, format);
  
  Wait(&LibraryLock);
  length = process_format("ns.debug: ", format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  Signal(&LibraryLock);
}
  
static void show_startup(void)
{ int	length;
  strcpy(output_buffer, "Network Server version ");
  strcat(output_buffer, VersionNumber);
  strcat(output_buffer, ".\n");
  length = strlen(output_buffer);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  strcpy(output_buffer, "Resource Management library version ");
  strcat(output_buffer, RmVersionNumber);
  strcat(output_buffer, ".\n");
  length = strlen(output_buffer);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
}
/*}}}*/
/*{{{  Terminate the Network Server */
/**
*** Terminate the Network Server cleanly. This is used mainly in joinnet,
*** when the subnet has been integrated and the subnet's Network Server
*** should exit. It is important to destroy
*** the ctos and stoc sockets as soon as possible, to prevent further
*** clients connecting to this server. 
**/
void	TerminateNetworkServer(void)
{
  if (Socket_ctos >= 0)
   { struct sockaddr_un	 address;
     int		 sock_ctos	= -1;
     int		 sock_stoc	= -1;
     Stream		*real_socket	= fdstream(Socket_ctos);
     int		 len;
     Capability		 cap;

     sock_ctos = socket(AF_UNIX, SOCK_STREAM, 0);
     if (sock_ctos < 0) goto done;
     sock_stoc = socket(AF_UNIX, SOCK_STREAM, 0);
     if (sock_stoc < 0) goto done;

     address.sun_family	= AF_UNIX;
     strcpy(address.sun_path, objname(real_socket->Name));
     len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

     if (connect(sock_ctos, (struct sockaddr *) &address, len) < 0) goto done;

     address.sun_family = AF_UNIX;
     strcpy(address.sun_path, objname(real_socket->Name));
     len = strlen(address.sun_path);
     address.sun_path[len - 1] = 'c';
     address.sun_path[len - 4] = 's';
     len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

     if (connect(sock_stoc, (struct sockaddr *) &address, len) < 0) goto done;

     memset(&cap, 0, sizeof(Capability));
     (void) write(sock_ctos, (BYTE *) &cap, sizeof(Capability));
     len = -1;
     (void) write(sock_ctos, (BYTE *) &len, sizeof(len));

done:
     if (sock_ctos >= 0) close(sock_ctos);
     if (sock_stoc >= 0) close(sock_stoc);
   }

  Delete(NameEntry, Null(char));
  FreePort(NetservInfo.ReqPort);
  Delay(15 * OneSec);	/* to let the netagents finish */
  Exit(0);  
}
/*}}}*/
/*{{{  Kick the Session Manager when a problem has been detected */
	/* A little utility to inform the Session Manager when it	*/
	/* should check its world, e.g. after a processor crash.	*/
	/* This sends a private protocol IOC message to /sm, with a	*/
	/* capability for the Network Server. The Session Manager can	*/
	/* use this capability to validate the source of the message.	*/
void KickSessionManager(void)
{ MsgBuf	*m = New(MsgBuf);
  Object	*session_manager = Locate(Null(Object), "/sm/Windows");
  Capability	cap;

  if ((m eq Null(MsgBuf)) || (session_manager eq Null(Object)))
   goto done;

  Debug(dbg_Problem, ("reporting problem to the Session Manager"));

  m->mcb.Data		= m->data;
  m->mcb.Control	= m->control;

  InitMCB(&(m->mcb), MsgHdr_Flags_preserve, NullPort, NullPort,
		FC_GSP | SS_NetServ | FG_NetStatus);
  MarshalCommon(&m->mcb, session_manager, Null(char));
  NewCap(&cap, (ObjNode *) &(Net->DirNode),
		AccMask_R + AccMask_W + AccMask_D + AccMask_A);
  MarshalCap(&m->mcb, &cap);
  SendIOC(&m->mcb);

done:
  if (session_manager ne Null(Object)) Close(session_manager);
  if (m ne Null(MsgBuf)) Free(m);
}
/*}}}*/

/*{{{  do_open() */
static void do_open(ServInfo *servinfo)
{ MCB           *m          = servinfo->m;
  MsgBuf        *r;
  char          *pathname   = servinfo->Pathname;
  Port          reqport;
  ObjNode       *f;

	/* do_open() can only be used to read directories. No special	*/
	/* access rights are required.					*/
  f = GetTarget(servinfo);
  if (f eq Null(ObjNode))
   { ErrorMsg(m, EO_Object); return; }
    
  if ((f->Type & Type_Flags) ne Type_Directory)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  r = New(MsgBuf);
  if( r eq Null(MsgBuf))
   { ErrorMsg(m,EC_Error+EG_NoMemory); return; }

  FormOpenReply(r, m, f, Flags_Server | Flags_Closeable, pathname);
  reqport = NewPort();  
  r->mcb.MsgHdr.Reply = reqport;
  PutMsg(&r->mcb);
  Free(r);

  MRSW_GetRead();
  DirServer(servinfo, m, reqport);
  MRSW_FreeRead();
  FreePort(reqport);
}
/*}}}*/
/*{{{  do_private() - debugging options */
static void do_debug(ServInfo *servinfo);
static void do_private(ServInfo *servinfo)
{ MCB            *m          = servinfo->m;
  ObjNode        *f;

  f = GetTarget(servinfo);
  if (f eq Null(ObjNode)) 
   { ErrorMsg(m, Err_Null); return; }

	/* BLV access should be checked here, to limit debugging	*/
	/* to the system administrator.					*/

	/* All private messages must refer to the root object */
  if (f ne (ObjNode *) servinfo->DispatchInfo->Root)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  UnLockTarget(servinfo);

  if ((servinfo->FnCode & FG_Mask) eq FG_NetStatus)
   { HandleLinkChange(servinfo); return; }
   
  if ((servinfo->FnCode & FG_Mask) eq FG_GetInfo)
   { do_debug(servinfo); return; }
   
  if ((servinfo->FnCode & FG_Mask) ne FG_Terminate)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object); return; }

  if (get_config("single_user", nsrc_environ) eq Null(char))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Server); return; }

  ErrorMsg(m, Err_Null);

  TerminateNetworkServer();  
}

static void	do_debug(ServInfo *servinfo)
{ MCB		*m	= servinfo->m;
  IOCMsg2	*req	= (IOCMsg2 *) m->Control;
  
  if (req->Arg.Mode eq dbg_Revert)
   { DiagnosticsStream = DefaultDiagnostics;
     m->MsgHdr.FnRc = 0;
     ErrorMsg(m, Err_Null);
     return;
   }
  
  if (req->Arg.Mode eq dbg_Redirect)
   { static	char	*message = "ns: output redirected\n";
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

/*{{{  accept RmLib connection */
/**-----------------------------------------------------------------------------
*** The main purpose of the Network Server is to support requests generated
*** by the Resource Management library. This is done via Unix-domain sockets.
*** The Network Server has a thread running continuously accepting new
*** connections from clients, and this will start up a ConnectionGuardian thread
*** per connection.
**/
static	void	connection_guardian(NsConn);

static	void	accept_rmlib(void)
{ struct sockaddr	address;
  int			next_connection = 0;
  int			len;
  Capability		cap;
  NsConn		new_conn;

  Socket_ctos	= socket(AF_UNIX, SOCK_STREAM, 0);
  Socket_stoc	= socket(AF_UNIX, SOCK_STREAM, 0);
  if ((Socket_ctos < 0) || (Socket_stoc < 0))
   fatal("unable to set up sockets");

  address.sa_family	= AF_UNIX;
  strcpy(address.sa_data, ".NS_ctos");
  if (bind(Socket_ctos, &address, sizeof(address)) ne 0)
   fatal("failed to bind socket name");

  address.sa_family	= AF_UNIX;
  strcpy(address.sa_data, ".NS_stoc");
  if (bind(Socket_stoc, &address, sizeof(address)) ne 0)
   fatal("failed to bind socket name");

  listen(Socket_ctos, SOMAXCONN);
  listen(Socket_stoc, SOMAXCONN);

  forever
   { new_conn = Malloc(sizeof(NsConnStruct));
     if (new_conn eq (NsConn) NULL)
      { Delay(2 * OneSec); continue; }

     InitSemaphore(&(new_conn->WriteLock), 1);
     InitList(&(new_conn->Processors));
     new_conn->Id = next_connection++;

     while ((new_conn->Socket_ctos = accept(Socket_ctos, NULL, NULL)) < 0)
      Delay(5 * OneSec);

	/* Pray that the client has not crashed in between...	*/
     new_conn->Socket_stoc = accept(Socket_stoc, NULL, NULL);

	/* Perform some initial communication to ensure that things	*/
	/* are working.							*/
     if (read(new_conn->Socket_ctos, (BYTE *) &cap, sizeof(Capability))
		ne sizeof(Capability))
      goto fail;

	/* Check that a capability has been supplied, that it is a	*/
	/* valid capability, and that it gives the required access.	*/
     new_conn->FullAccess	= FALSE;
     if (cap.Access ne 0)
      if (GetAccess(&cap, Net->DirNode.Key))
       if (cap.Access & AccMask_D)
        new_conn->FullAccess	= TRUE;

     if (read(new_conn->Socket_ctos, (BYTE *) &(new_conn->Program), sizeof(int)) ne sizeof(int))
      goto fail;

     if (new_conn->Program eq -1)
      { close(new_conn->Socket_ctos);
        close(new_conn->Socket_stoc);
        goto terminate;
      }

     len = 0;
     if (write(new_conn->Socket_stoc, (BYTE *) &len, sizeof(int)) ne sizeof(int))
      goto fail;

	/* System library calls are currently preferred, because	*/
	/* timeouts are necessary for fault tolerance.			*/
     new_conn->Pipe_ctos = fdstream(new_conn->Socket_ctos);
     new_conn->Pipe_stoc = fdstream(new_conn->Socket_stoc);

	/* Spawn a connection guardian and accept the next connection	*/
     if (Fork(ConnectionGuardian_Stack, &connection_guardian, sizeof(NsConn), new_conn))
      continue;

fail:
     close(new_conn->Socket_ctos);
     close(new_conn->Socket_stoc);
     Free(new_conn);
   }  

terminate:
  close(Socket_ctos);
  close(Socket_stoc);
}
/*}}}*/
/*{{{  the ConnectionGuardian thread */

/**
*** The ConnectionGuardian process. Essentially this is just another
*** dispatcher. It reads two integers from the pipe, a job id and
*** a request code.
**/

typedef void NsRequestHandler(NsConn, int, RmRequest *, RmReply *);
typedef struct	NsRequestTableEntry {
	int			FnCode;
	bool			Protected;
	bool			WriteLock;
	NsRequestHandler	*handler;
} NsRequestTableEntry;
	
static void	HandleStartns		(NsConn, int, RmRequest *, RmReply *);
static void	HandleRegisterTfm	(NsConn, int, RmRequest *, RmReply *);
static void	HandleGetId		(NsConn, int, RmRequest *, RmReply *);
static NsConn	FindConnection		(RmProcessor);

NsRequestTableEntry	request_handlers[] =
{
 { RmC_Startns,			FALSE,	TRUE,	HandleStartns		},
 { RmC_GetNetwork,		FALSE,	FALSE,	HandleGetNetwork	},
 { RmC_GetNetworkHardware,	FALSE,	FALSE,	HandleGetNetwork	},
 { RmC_GetHierarchy,		FALSE,	FALSE,	HandleGetNetwork	},
 { RmC_ObtainProcessor,		TRUE,	FALSE,	HandleObtainProcessor	},
 { RmC_ReleaseProcessor,	TRUE,	FALSE,	HandleReleaseProcessor	},
 { RmC_LastChange,		FALSE,	FALSE,	HandleLastChange	},
 { RmC_ObtainExactNetwork,	TRUE,	FALSE,	HandleObtainNetwork	},
 { RmC_ObtainNetwork,		TRUE,	FALSE,	HandleObtainNetwork	},
 { RmC_ReleaseNetwork,		TRUE,	FALSE,	HandleReleaseNetwork	},
 { RmC_GetLinkMode,		FALSE,	FALSE,	HandleGetLinkMode	},
 { RmC_SetLinkMode,	/* ? */	FALSE,	FALSE,	HandleSetLinkMode	},
#if Joinnet_Supported
 { RmC_AcceptNetwork,		FALSE,	FALSE,	HandleAcceptNetwork	},
 { RmC_JoinNetwork,		FALSE,	FALSE,	HandleJoinNetwork	},
#endif
 { RmC_IsProcessorFree,		FALSE,	FALSE,	HandleIsProcessorFree	},
#if Native_Supported
 { RmC_SetNative,		TRUE,	TRUE,	HandleNative		},
 { RmC_ResetProcessors,		TRUE,	FALSE,	HandleNative		},
 { RmC_Reboot,			TRUE,	TRUE,	HandleNative		},
 { RmC_TestConnections,		FALSE,	TRUE,	HandleConnections	},
 { RmC_MakeConnections,		TRUE,	TRUE,	HandleConnections	},
 { RmC_Revert,			TRUE,	TRUE,	HandleConnections	},
#endif
 { RmC_GetId,			TRUE,	FALSE,	HandleGetId		},
 { RmC_RegisterTfm,		TRUE,	FALSE,	HandleRegisterTfm	},
 { RmC_ReportProcessor,		TRUE,	FALSE,	HandleReportProcessor	},
 { -1,				FALSE,	FALSE,	NULL			}
};

 
static void connection_guardian(NsConn connection)
{ int		job_id;
  RmRequest	request;
  RmReply	reply;
  Stream	*pipe	= connection->Pipe_ctos;
  bool		broken	= FALSE;
  int		i;
  word		rc;

  while (!broken)
   { 
     Clear(request); Clear(reply);
	 
     rc = Read(pipe, (BYTE *) &job_id, sizeof(int), 70 * OneSec);
     if (rc <= 0)
      { if (connection->Program eq Rm_Session)
	 report("warning, Session Manager has terminated");
	goto broken;
      }

     if ((job_id & RmR_Private) ne 0)
      { if ((job_id & ~RmR_Private) eq RmR_Synch) continue;
        IOdebug("Netserv : private protocol received down pipe");
        continue;
      }

     rc = FullRead(pipe, (BYTE *) &request, sizeof(RmRequest), -1);
     if (rc ne sizeof(RmRequest)) goto broken;

     if ((request.Network ne (RmNetwork) NULL) ||
         (request.Taskforce ne (RmTaskforce) NULL))
      if ((rc = RmReadStream(pipe, &(request.Network), &(request.Taskforce)))
          ne RmE_Success)
       goto broken;

     if (request.Processor ne (RmProcessor) NULL)
      if ((rc = RmReadProcessor(pipe, &(request.Processor), FALSE))
          ne RmE_Success)
       goto broken;

     if (request.Task ne (RmTask) NULL)
      if ((rc = RmReadTask(pipe, &(request.Task), FALSE))
          ne RmE_Success)
       goto broken;

     if (request.VariableSize > 0)
      { request.VariableData = Malloc(request.VariableSize);
        if (request.VariableData eq NULL)
         goto broken;
        if (FullRead(pipe, request.VariableData, request.VariableSize, -1) < 0)
	 goto broken;
      }

     if (DebugOptions & dbg_Comms)
      debug("request %x, jobid %x, Network %N, \n          Taskforce %T, Processor %P, Task %T, size %d",
		request.FnRc, job_id, request.Network, request.Taskforce, 
		request.Processor, request.Task,
		request.VariableSize);

     for (i = 0; request_handlers[i].handler ne NULL; i++)
      if (request.FnRc eq request_handlers[i].FnCode)
       break;
     if (request_handlers[i].handler ne NULL)
      { if (request_handlers[i].Protected && !(connection->FullAccess))
	 { reply.FnRc	= RmE_NoAccess;
	   ReplyRmLib(connection, job_id, &reply);
         }
        else
	 { if (request_handlers[i].WriteLock)
	    MRSW_GetWrite();
	   else
	    MRSW_GetRead();
           (*request_handlers[i].handler)(connection, job_id, &request, &reply);
	   if (request_handlers[i].WriteLock)
	    MRSW_FreeWrite();
	   else
	    MRSW_FreeRead();
         }
      }
     else
      { report("unexpected request %x", request.FnRc);
	broken = TRUE;
      }

     if (request.Network ne (RmNetwork) NULL) RmFreeNetwork(request.Network);
     if (request.Taskforce ne (RmTaskforce) NULL) RmFreeTaskforce(request.Taskforce);
     if (request.Processor ne (RmProcessor) NULL) RmFreeProcessor(request.Processor);
     if (request.Task ne (RmTask) NULL) RmFreeTask(request.Task);
     if (request.VariableData ne NULL) Free(request.VariableData);

	/* the connection may have been broken synchronously or	*/
	/* asynchronously, via ReplyRmLib() or as a result of a	*/
	/* detected crash and a call to RemConnection().	*/
     if (connection->Pipe_ctos eq Null(Stream)) goto done;
   }

broken:
	/* Write()s do not appear to timeout, this should avoid deadlocks */
	/* at the risk of Abort(NULL), I think.				  */
  Abort(connection->Pipe_stoc);
  MRSW_GetRead();	/* Necessary to manipulate owned processors	*/
  AbortConnection(connection);
  MRSW_FreeRead();

done:
  Free(connection);
}

/*}}}*/
/*{{{  ReplyRmLib() - send a reply back to a client */
/**
*** This is called from a request handler called by the above
*** ConnectionGuardian() to send a reply back to the client. It will
*** be called with at least a read lock.
**/
int ReplyRmLib(NsConn connection, int job_id, RmReply *reply)
{ int		rc 	= RmE_CommsBreakdown;
  Stream	*pipe	= connection->Pipe_stoc;

  if (DebugOptions & dbg_Comms)
   debug("reply %x, jobid %x, Network %N,\n          Taskforce %T, Processor %P, Task %T, size %d",
	reply->FnRc, job_id, reply->Network, reply->Taskforce,
	reply->Processor, reply->Task, reply->VariableSize);

  Wait(&(connection->WriteLock));
  if (Write(pipe, (BYTE *) &job_id, sizeof(int), -1) ne sizeof(int))
   goto fail;

  if (Write(pipe, (BYTE *) reply, sizeof(RmReply), -1) ne sizeof(RmReply))
   goto fail;

  if ((reply->Network ne (RmNetwork) NULL) ||
      (reply->Taskforce ne (RmTaskforce) NULL))
   if ((rc = RmWriteStream(pipe, reply->Network, reply->Taskforce, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->Processor ne (RmProcessor) NULL)
   if ((rc = RmWriteProcessor(pipe, reply->Processor, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->Task ne (RmTask) NULL)
   if ((rc = RmWriteTask(pipe, reply->Task, reply->Filter))
       ne RmE_Success)
    goto fail;

  if (reply->VariableSize > 0)
   if (Write(pipe, reply->VariableData, reply->VariableSize, -1)
       ne reply->VariableSize)
    { rc = RmE_CommsBreakdown; goto fail; }

  Signal(&(connection->WriteLock));
  return(RmE_Success);

fail:
  Signal(&(connection->WriteLock));
  AbortConnection(connection);
  return(rc);
}
/*}}}*/
/*{{{  connection-related exception handling */
/**
*** If a processor disappears from the network, e.g. because an
*** external subnet has disconnected, and this processor is owned
*** then the processor has to be removed from the connection and
*** the owner has to be informed. This routine will be called
*** with a write lock.
***
*** The routine may also be called if the network server decides to
*** reclaim a processor from its current owner. The reason can be one
*** of RmR_Crashed or RmR_Reclaimed.
***
*** Locking status: this routine is called with a write lock so there
*** can be no processor allocation, freeing, or modification happening.
*** If the processor is currently owned by a connection then it will
*** have a session id other than -1. Hence this provides a safe check.
***
*** Care has to be taken if the connection is breaking or broken...
**/

void RemConnection(RmProcessor processor, int reason)
{ ProcessorEntry	*proc_entry;
  NsConn		connection;

  Debug(dbg_Problem, ("checking ownership of %P", processor));

  connection = FindConnection(processor);
  if (connection eq (NsConn) NULL) return;

	/* Now inform the owner about what has happened. This is only	*/
	/* possible if there is still an owner...			*/
	/* If this processor is running the session's TFM, ouch !!!	*/
  if (processor->ObjNode.Flags & NsFlags_TfmProcessor)
   { Debug(dbg_Problem, ("TFM processor has gone, aborting connection"));
     AbortConnection(connection);
     return;
   }

  if (connection->Pipe_stoc ne Null(Stream))
   { InformConnection(processor, reason);
     processor->SessionId	= -1;
     proc_entry = GetProcEntry(processor);
     Remove(&(proc_entry->Connection));
   }
}

void InformConnection(RmProcessor processor, int reason)
{ NsConn	connection = FindConnection(processor);
  int		x;
  Stream	*pipe;

  if (connection eq (NsConn) NULL) return;
  if (connection->Pipe_stoc eq Null(Stream)) return;

  Debug(dbg_Problem, ("informing owning client about %P", processor));

  Wait(&(connection->WriteLock));
  pipe = connection->Pipe_stoc;
  reason |= RmR_Private;
  if (Write(pipe, (BYTE *) &reason, sizeof(word), 10 * OneSec) ne sizeof(word)) goto fail;
  x = sizeof(word);	/* data size */
  if (Write(pipe, (BYTE *) &x, sizeof(x), 10 * OneSec) ne sizeof(x)) goto fail;
  if (Write(pipe, (BYTE *) &processor->Uid, sizeof(RmUid), 10 * OneSec) ne sizeof(RmUid)) goto fail;
  Signal(&(connection->WriteLock));
  return;

fail:
  Signal(&(connection->WriteLock));
  Debug(dbg_Problem, ("failed to write to owning client"));
}

/**
*** This routine aborts a connection if there is a communication failure
*** with the relevant client. It may be called from the ConnectionGuardian
*** or ReplyRmLib() with a read lock, or from RemConnection() if the TFM
*** processor has crashed, with a write lock. 
**/
void AbortConnection(NsConn connection)
{
  if (connection->Pipe_stoc ne Null(Stream))
   { connection->Pipe_stoc	= Null(Stream);
     connection->Pipe_ctos	= Null(Stream);
     Abort(fdstream(connection->Socket_stoc));
     Abort(fdstream(connection->Socket_ctos));
     close(connection->Socket_ctos);
     close(connection->Socket_stoc);
   }
  unless(EmptyList_(connection->Processors))
   { Debug(dbg_Problem, ("AbortConnection, releasing processors"));
     WalkList(&(connection->Processors), &AutomaticRelease);
   }
}

/**
*** utility to find the connection owning this processor, if any.
**/
static NsConn	FindConnection(RmProcessor processor)
{ NsConn		connection;
  Node			*node;
  ProcessorEntry	*proc_entry;
  int			i = NumberProcessors + 1;	/* safety */

  if (processor->SessionId eq -1) return((NsConn) NULL);

  proc_entry = GetProcEntry(processor);
  node = &(proc_entry->Connection);
  while ((Prev_(Node, node) ne Null(Node)) && (i-- > 0))
    node = Prev_(Node, node);
  if (i <= 0) return((NsConn) NULL);

  connection = (NsConn) (((BYTE *) node) - offsetof(NsConnStruct, Processors));

  return(connection);
}
/*}}}*/

/*{{{  HandleGetId() */
/**----------------------------------------------------------------------------
*** GetId(), return the Id corresponding to this connection.
**/
static void HandleGetId(NsConn connection, int job_id,
			RmRequest *request, RmReply *reply)
{
  reply->FnRc	= RmE_Success;
  reply->Reply1	= connection->Id;
  ReplyRmLib(connection, job_id, reply);
  request	= request;
}
/*}}}*/
/*{{{  HandleRegisterTFM() */
/**----------------------------------------------------------------------------
*** RegisterTfm(), allow a TFM to set the session Id of the processor
*** obtained by the Session Manager to run the TFM. Ownership of the 
*** processor is transferred from the Session Manager's connection to
*** the TFM's connection. Hence if the TFM's connection is broken this
*** processor will be cleaned up along with all the other processors owned,
*** without worrying the Session Manager. Note that the Session Manager
*** monitors all TFM's and login windows, so that it can restart a login
*** if necessary.
**/
static void HandleRegisterTfm(NsConn connection, int job_id,
			RmRequest *request, RmReply *reply)
{ RmProcessor		processor;
  int			rc = RmE_Success;
  ProcessorEntry	*proc_entry;

  if (connection->Program ne Rm_TFM)
   { rc = RmE_NoAccess; goto done; }

  processor = RmFindProcessor(Net, request->Uid);
  if (processor eq (RmProcessor) NULL)
   { rc = RmE_BadProcessor; goto done; }
  processor->SessionId		 = connection->Id;
  processor->ObjNode.Flags	|= NsFlags_TfmProcessor;

  proc_entry = GetProcEntry(processor);
  Remove(&(proc_entry->Connection));
  AddHead(&(connection->Processors),  &(proc_entry->Connection));

done:
  reply->FnRc	= rc;
  reply->Reply1 = NumberProcessors;
  ReplyRmLib(connection, job_id, reply);
}
/*}}}*/
/*{{{  HandleStartns() */

/**----------------------------------------------------------------------------
***
*** HandleStartns(). The client side will send a complete Network and
*** Taskforce, including hardware facilities. If a Network has been
*** received already then things have gone somewhat wrong. Otherwise
*** the network is processed.
*** The reply consists of the JobID, as usual, followed by a success code
*** and a capability for the Network Server.
*** N.B. The incoming network and taskforce have to be cleared out of
*** the incoming request message, or the ConnectionGuardian will free them.
**/

static void StartnsAux(RmNetwork);

static void HandleStartns(NsConn connection, int job_id, 
		RmRequest *request, RmReply *reply)
{ RmNetwork	network;

  if (ReceivedNetwork)
   { reply->FnRc	= RmE_InUse;
     ReplyRmLib(connection, job_id, reply);
     return;
   }

  network		= request->Network;
  DefaultTaskforce	= request->Taskforce;
  request->Network	= (RmNetwork) NULL;
  request->Taskforce	= (RmTaskforce) NULL;

  StartnsAux(network);   /* Sort out the data */

  reply->FnRc		= RmE_Success;
  NewCap( (Capability *) &(reply->Reply1), (ObjNode *) &(network->DirNode), 
    	AccMask_R + AccMask_W + AccMask_D + AccMask_A);
  ReplyRmLib(connection, job_id, reply);
  return;
}

/**
*** Once a network has been read in there is a lot of work to be done.
*** 1) put the network into the directory structure. This means
***    remembering the network name and making the network the root
***    directory. The old root can now be freed.
*** 2) figure out which of the processors in the network structure the
***    network server is running on.
*** 3) modify every network and processor structure, in particular fill in
***    the key, matrix and other fields. Also check every hardware facility
***    in the network, loading the device drivers, and for every processor
***    work out how many facilities affect the processor. Add a PUID
***    attribute to every processor.
*** 4) create a ProcessorEntry field for every processor. This includes
***    details of all the hardware facilities.
*** 5) call all the device driver initialisation routines.
*** 6) copy all the current links into the default links held in the
***    proc_entry field.
*** 7) count the number of processors
*** 8) work out what is to be done with the new network, by calling a
***    routine in the netboot.c module.
**/
static int		StartnsAux2(RmNetwork);
static int		StartnsAux3(RmProcessor, ...);
static int		StartnsAux4(RmHardwareFacility *, ...);
static int		StartnsAux5(RmProcessor, ...);
static int		StartnsAux6(RmNetwork, ...);
static int		StartnsAux7(RmProcessor, ...);
static int		StartnsAux8(RmHardwareFacility *, ...);
static int		StartnsAux9(RmProcessor, ...);
static RmProcessor	DetermineRoot(RmNetwork);
static RmProcessor	DetermineIO(RmProcessor);

static void	StartnsAux(RmNetwork network)
{ 
  strcpy(NetworkName, network->DirNode.Name);
  RmRootName = NetworkName;
  strcpy(network->DirNode.Name, "ns");  
  { RmNetwork old_root = (RmNetwork) NetservInfo.Root;
    Net = (RmNetwork) (NetservInfo.Root = &(network->DirNode));
    RmFreeNetwork(old_root);
  }

  NumberProcessors = RmCountProcessors(network);
  { extern void PreallocMsgBufs(int);
    PreallocMsgBufs(NumberProcessors + 4);
  }
  RootProcessor		= DetermineRoot(network);
  LastBooted		= RootProcessor;
  BootIOProcessor	= DetermineIO(RootProcessor);
  (void) StartnsAux2(network);
  (void) RmApplyProcessors(network, &StartnsAux5);
  (void) StartnsAux6(network);
  (void) RmApplyProcessors(network, &StartnsAux9);
  if ((get_config("single_user", nsrc_environ) eq Null(char)) ||
      (get_config("share_root_processor", nsrc_environ) eq Null(char)))
   RootProcessor->ObjNode.Account = RmO_System;
   
  StartBootstrap();
}

static int  StartnsAux2(RmNetwork network)
{ 
  network->DirNode.Matrix = 0x21212147;	/* rz : rz : rz : rwvd */
  network->DirNode.Key	  = NewKey() + _cputime() + (int) network;
  network->DirNode.Dates.Creation =
  network->DirNode.Dates.Modified =
  network->DirNode.Dates.Access   = GetDate();
  network->DirNode.Account	  = 0;

	/* Rearrange things so that subnets are at the back */
  if (network->NoSubnets > 0)
   { RmProcessor	current, next;
     int		i;
     next = RmFirstProcessor(network);
     for (i = 0; i < network->DirNode.Nentries; i++)
      { current = next;
        next    = RmNextProcessor(current);
        if (RmIsNetwork(current))
         { (void) Remove(&(current->ObjNode.Node));
           (void) AddTail(&(network->DirNode.Entries), &(current->ObjNode.Node));
         }
      }
   }
   
  (void) RmApplyNetwork(network, &StartnsAux3);
  (void) RmApplyHardwareFacilities(network, &StartnsAux4);
  return(0);
}

static void add_puid_attribute(RmProcessor);

static int  StartnsAux3(RmProcessor processor, ...)
{
  if (RmIsNetwork(processor)) return(StartnsAux2((RmNetwork) processor));

  processor->ObjNode.Matrix = 0x01010143;	/* r:r:r:drw */
  processor->ObjNode.Key    = GetDate() + _cputime() + (int) processor;
  processor->ObjNode.Dates.Creation = GetDate();
  processor->ObjNode.Dates.Modified = 0;
  processor->ObjNode.Dates.Access   = 0;
  if ((RmGetProcessorPurpose(processor) & RmP_System) eq 0)
   processor->ObjNode.Account = RmO_FreePool;
  else
   processor->ObjNode.Account = RmO_System;

  processor->StructType	= RmL_New;	/* ensure that the Network Server */
  					/* can fiddle with the structures */
  processor->ObjNode.Size	= 0;	/* State currently undefined.	  */
  RmSetProcessorPrivate(processor, 0);
  processor->SessionId		= -1;
  processor->ApplicationId	= -1;
  add_puid_attribute(processor);  
  return(0);
}

/**
*** Add the puid attribute in a separate routine, because it involves using
*** a lot of stack.
**/
static void add_puid_attribute(RmProcessor processor)
{ char namebuf[IOCDataMax];
  strcpy(namebuf, "PUID=");
  BuildName(&(namebuf[5]), processor);
  RmAddObjectAttribute((RmObject) processor, namebuf, TRUE);
}

/**
*** Part of the hardware initialisation. Load every device driver needed
*** by this network. Update every processor affected by this hardware
*** facility.
**/
static	void StartnsAux4_resetcommand(RmHardwareFacility *);

static int StartnsAux4(RmHardwareFacility *hardware, ...)
{ int i;

  if ((hardware->Type eq RmH_ResetDriver) || 
      (hardware->Type eq RmH_ConfigureDriver))
   { NetworkDCB	*network_dcb = New(NetworkDCB);
     if (network_dcb eq Null(NetworkDCB))
      fatal("out of memory when initialising network data structures");
     network_dcb->NetworkName		= NetworkName;
     network_dcb->Net			= (RmNetwork) NetservInfo.Root;
     network_dcb->RootProcessor		= RootProcessor;
     network_dcb->HardwareFacility	= hardware;
     network_dcb->Functions		= &Functions;
     hardware->Device = (void *) OpenDevice(hardware->Name, network_dcb);
     if (hardware->Device eq Null(void))
      fatal("failed to load device driver %s", hardware->Name);
   }

  if (hardware->Type eq RmH_ResetCommand)
   StartnsAux4_resetcommand(hardware);
   
  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor processor = hardware->Processors[i];
     int temp = RmGetProcessorPrivate(processor);
     RmSetProcessorPrivate(processor, temp + 1);
   }

  return(0);
}

/**
*** Cope with reset commands. These have the following characteristics:
*** 1) hardware->Option contains the name of a processor, e.g. /Cluster/00
*** 2) hardware->Name contains a command that must be parsed,
***    e.g. -e /helios/bin/pa_reset pa_reset 3
**/

static void StartnsAux4_resetcommand(RmHardwareFacility *hardware)
{ RmTask	task = RmNewTask();
  Object	*program = Null(Object);
  RmProcessor	processor;
  char		*progname;
  char		*temp;
  int		arg = 0;
      
  processor = RmLookupProcessor(Net, hardware->Option);
  if (processor eq (RmProcessor) NULL)
   fatal("control processor %s for reset command is not in the network",
   		hardware->Option);

  hardware->Essential = processor;

  if (task eq (RmTask) NULL)
   fatal("out of memory when initialising reset command");
  hardware->Device = task;

  temp = hardware->Name;
  if ((temp[0] eq '-') && 
      ((temp[1] eq 'e') || (temp[1] eq 'E')) &&
      isspace(temp[2]))
   { RmSetTaskId(task, "1");
     temp = &(temp[3]);
   }
  else
   RmSetTaskId(task, "0");
   
  while (isspace(*temp)) temp++;
  progname = temp;
  until ((*temp eq '\0') || isspace(*temp)) temp++;
  if (*temp eq '\0') goto done_args;
  
  *temp++ = '\0';
  until (*temp eq '\0')
   { char	*argname;
     char	junk;
     
     while (isspace(*temp)) temp++;
     if (*temp eq '\0') goto done_args;
     argname = temp;
     until ((*temp eq '\0') || isspace(*temp)) temp++;
     junk = *temp;
     *temp = '\0';
     if (arg ne 0) RmAddTaskArgument(task, arg++, argname);
     arg++;
     *temp = junk;
   }
   
done_args:

  if (progname[0] eq '/')
   program = Locate(Null(Object), progname);
  else 
   { Object	*heliosbin = Locate(Null(Object), "/helios/bin");
     if (heliosbin ne Null(Object))
      { program = Locate(heliosbin, progname);
        Close(heliosbin);
      }
   }

  if (program eq Null(Object))
   fatal("failed to locate reset command %s", progname);
  else
   RmSetTaskCode(task, program->Name);
  Close(program);
}      

/**
*** For every processor in the network allocate and initialise a
*** ProcessorEntry structure. This will contain all the information
*** needed by the Network Server to maintain this processor, after
*** the remaining initialisation.
**/
static int	StartnsAux5(RmProcessor processor, ...)
{ int			hardware_count;
  int			number_links;
  int			size;
  ProcessorEntry	*proc_entry;
    
  hardware_count = RmGetProcessorPrivate(processor);
  number_links	 = RmCountLinks(processor);
  size = (hardware_count * sizeof(DriverEntry)) + 
  	 (number_links * sizeof(RmLink)) +
  	 sizeof(ProcessorEntry);
  proc_entry = (ProcessorEntry *) Malloc(size);  	 
  if (proc_entry eq Null(ProcessorEntry))
   fatal("out of memory when initialising processor data structures");
  memset((void *) proc_entry, 0, size);

  proc_entry->Incarnation	= 1;
  proc_entry->NumberDrivers	= hardware_count;
  proc_entry->DriverEntry	= (DriverEntry *)
  				((BYTE *) proc_entry + sizeof(ProcessorEntry));
  proc_entry->StandardLinks	= (RmLink *) ((BYTE *)proc_entry->DriverEntry 
                                  + (hardware_count * sizeof(DriverEntry)));
  proc_entry->Purpose		= RmGetProcessorPurpose(processor);
  proc_entry->Netagent		= Null(Stream);
  proc_entry->NetagentDate	= 0;
  proc_entry->BeingBooted	= FALSE;
  proc_entry->CommandDate	= GetDate();
  InitSemaphore(&(proc_entry->NetagentLock), 1);
  proc_entry->Processor		= processor;
  proc_entry->WindowServer	= Null(Object);
  proc_entry->ConsoleWindow	= Null(Object);
  RmSetProcessorPrivate(processor, (int) proc_entry);
  return(0);
}

/**
*** Call the initialisation routines for all the device drivers.
**/
static int  StartnsAux6(RmNetwork network, ...)
{ (void) RmApplyHardwareFacilities(network, &StartnsAux8);
  (void) RmApplyNetwork(network, &StartnsAux7);
  return(0);
}

static int  StartnsAux7(RmProcessor processor, ...)
{ if (RmIsNetwork(processor))
   return(StartnsAux6((RmNetwork) processor));
  else
   return(0);
}

static int  StartnsAux8(RmHardwareFacility *hardware, ...)
{ DriverRequest	request;
  DCB		*device;
  int		i, j;

  for (i = 0; i < hardware->NumberProcessors; i++)
   { RmProcessor	processor = hardware->Processors[i];
     ProcessorEntry	*proc_entry;
     proc_entry = GetProcEntry(processor);
     
     for (j = 0; j < proc_entry->NumberDrivers; j++)
      { DriverEntry	*driver_entry = &(proc_entry->DriverEntry[j]);
        if (driver_entry->Hardware ne Null(RmHardwareFacility)) continue;
        driver_entry->Hardware = hardware;
        switch(hardware->Type)
         { case RmH_ResetDriver : 
         	driver_entry->Flags = DriverFlags_ResetDriver; break;
           case RmH_ConfigureDriver :
           	driver_entry->Flags = DriverFlags_ConfigureDriver; break;
           case RmH_ResetCommand :
           	driver_entry->Flags = DriverFlags_ResetCommand; break;
         }
        break;
      }
   }

  if (hardware->Type eq RmH_ResetDriver)
   { device			= (DCB *) hardware->Device;
     request.FnRc		= ND_Initialise;
     request.NumberProcessors	= 0;
     (*(device->Operate))(device, &request);
     if (request.FnRc ne Err_Null)
      report("warning, failed to initialise driver %s, fault %x",
      		hardware->Name, request.FnRc);
   }
  if (hardware->Type eq RmH_ConfigureDriver)
   { device			= (DCB *) hardware->Device;
     request.FnRc		= ND_Initialise;
     request.NumberProcessors	= 0;
     (*(device->Operate))(device, &request);
     if (request.FnRc ne Err_Null)
      { report("warning, failed to initialise driver %s, fault %x",
      		hardware->Name, request.FnRc);
      	goto done;
      }

     request.FnRc		= ND_ObtainProcessors;
     request.NumberProcessors	= -1;
     (*(device->Operate))(device, &request);
     if (request.FnRc ne Err_Null)
      { report("warning, driver %s failed to obtain its processors, fault %x",
      		hardware->Name, request.FnRc);
      	goto done;
      }
     
     request.FnRc		= ND_MakeInitialLinks;
     request.NumberProcessors	= -1;
     (*(device->Operate))(device, &request);
     if (request.FnRc ne Err_Null)
      report("warning, driver %s failed to make its initial links, fault %x",
      		hardware->Name, request.FnRc);
   }

done:
  return(0);
}

/**
*** For every processor in the network, make a copy of the current links in
*** the ProcessorEntry structure. This allows the Network Server to restore
*** the default links when required. Also work out the amount of control
*** provided by the hardware drivers.
***
*** NB some device drivers may have to change the saved links, if the
*** initialisation phase has to be delayed. For example, on Telmat machines
*** the final machine configuration is not known until the initial
*** connections are made.
**/
static int	StartnsAux9(RmProcessor processor, ...)
{ ProcessorEntry	*proc_entry;
  int			number_links;
  int			i;
  int			control = RmC_FixedMapping + RmC_FixedLinks;
  int			purpose;

  proc_entry = GetProcEntry(processor);
  number_links = RmCountLinks(processor);
  for (i = 0; i < number_links; i++)
   { RmLink	*link = RmFindLink(processor, i);
     memcpy((void *) &(proc_entry->StandardLinks[i]), (void *) link,
     		sizeof(RmLink));
   }

  purpose = RmGetProcessorPurpose(processor) & RmP_Mask;
  if ((purpose eq RmP_IO) || (purpose eq RmP_Router))
   { processor->Control = RmC_FixedMapping + RmC_FixedLinks;
     return(0);
   }
   
  for (i = 0; i < proc_entry->NumberDrivers; i++)
   { DriverEntry	*driver_entry = &(proc_entry->DriverEntry[i]);
     if (driver_entry->Flags eq DriverFlags_ConfigureDriver)
      { int i, number_links, flags;
        number_links = RmCountLinks(processor);
        for (i = 0; i < number_links; i++)
         { flags = RmGetLinkFlags(processor, i);
           if (flags & RmF_Configurable)
      	    { control &=  ~RmC_FixedLinks; break; }
      	 }
      }
     if (driver_entry->Flags & DriverFlags_PossibleReset)
      control |= RmC_PossibleReset;
     if (driver_entry->Flags & DriverFlags_DefiniteReset)
      control |= RmC_Reset;
     if (driver_entry->Flags & DriverFlags_NativePossible)
      control |= RmC_Native;
     if (driver_entry->Flags & DriverFlags_MappingFlexible)
      control &= ~RmC_FixedMapping;
     if (driver_entry->Flags & DriverFlags_Reclaim)
      control |= RmC_Reclaim;
   }
  processor->Control = control;
     
  return(0);
}
/**
*** Figure out which of the processors in the network map the program
*** is running on.
*** 1) I can work out the current processor name by a system call. This
***    could be /Cluster/00, /00, or something silly.
*** 2) If the current name is /Cluster/00 then I can assume that the
***    processor has been booted up with the full network name, or that
***    a previous incarnation of the network server produced the full
***    network name. Hence I look up the name, and if the result is sensible
***    everything is happy.
*** 3) If the name is /00 or something like that matters become more difficult.
***    The network is searched for all entries that match the name.
***    If there is no match or exactly one match life is easy. If there is
***    more than one match, some heuristics have to be used. The routine
***    simply sees if any of the candidates have a root at the top level,
***    and if so that candidate is chosen. Otherwise an error message is given.
***    If there are problems the user can always resolve them via the nsrc file.
**/

static int		DetermineAux1(RmProcessor, ...);

static RmProcessor	DetermineRoot(RmNetwork network)
{ char	*temp = ProcessorName + strlen(ProcessorName);
  RmProcessor	*candidates = Null(RmProcessor);
  int		next_candidate = 0;

	/* Distinguish between /00 and /Cluster/00 */  
  for ( ; (temp > ProcessorName) && (*temp ne '/'); temp--);
  
  if (temp ne ProcessorName)
   {	/* The current processor name is already something like /Cluster/00 */
   	/* Hence I can lookup the real processor.			    */
     RmProcessor result;

     result = RmLookupProcessor(network, ProcessorName);
     if (result eq (RmProcessor) NULL)
      fatal("cannot find processor %s in the resource map", ProcessorName);
     if (RmIsProcessor(result)) return(result);
     fatal("in the resource map %s is not a processor", ProcessorName);
   }

	/* The current name is /00 or similar. Search the whole network */   
  (void) RmApplyProcessors(network, &DetermineAux1, ++temp, &candidates,
  			 &next_candidate);

  	/* If there are no candidates then something is very wrong. */
  if (next_candidate eq 0)
   fatal("none of the processors in the resource map correspond to %s",
   		ProcessorName);
   		
   	/* If there is exactly one candidate then life is easy */
  if (next_candidate eq 1)
   { RmProcessor result = candidates[0];
     Free(candidates);
     return(result);
   }

  { int	i;
    for (i = 0; i < next_candidate; i++)
     { RmProcessor processor	 = candidates[i];
       if (RmParentNetwork(processor) eq RmRootNetwork(processor))
        { Free(candidates);
          return(processor);
        }
     }
  }

  fatal("internal error determining the root of the network");
  return(NULL);
}

static int DetermineAux1(RmProcessor processor, ...)
{ va_list	args;
  char		*proc_name;
  RmProcessor	**candidates;
  int		*next_candidate;
  RmProcessor	*new_candidates;
  
  va_start(args, processor);
  proc_name		= va_arg(args, char *);
  candidates		= va_arg(args, RmProcessor **);
  next_candidate	= va_arg(args, int *);
  va_end(args);
  
	/* If the name does not match then everything is easy */
  if (strcmp(Procname(processor), proc_name)) return(0);

	/* Expand the table of candidates */
  new_candidates = (RmProcessor *)
  		Malloc((1 + (word) *next_candidate) * sizeof(RmProcessor));
  if (new_candidates eq Null(RmProcessor))
  	fatal("out of memory when determining root processor");

  if (*next_candidate ne 0)
   memcpy(new_candidates, *candidates, sizeof(RmProcessor) * *next_candidate);
  new_candidates[*next_candidate] = processor;
  *next_candidate += 1;
  if (candidates ne NULL) Free(*candidates);
  *candidates = new_candidates;
  return(0);
}

/**
*** Given the root processor, determine the I/O processor if any.
*** This must satisfy the following restrictions:
*** 1) it must be of type I/O
*** 2) the link flags for that link must have the Link_Flags_parent bit set
***
*** It is perfectly possible for there to be no I/O boot processor, if
*** the system is rom-booted.
**/
static RmProcessor	DetermineIO(RmProcessor root_processor)
{ LinkInfo	info;
  int		number_links = RmCountLinks(root_processor);
  int		i;
  
  for (i = 0; i < number_links; i++)
   { if (LinkData(i, &info) < Err_Null) continue;
     if ((info.Flags & Link_Flags_parent) && 
         (info.Mode eq Link_Mode_Intelligent))
      { RmProcessor	neighbour;
        int		destlink;
	int		purpose;
	
        neighbour = RmFollowLink(root_processor, i, &destlink);
        if (neighbour eq RmM_NoProcessor)
         { report("root processor %P was booted from link %d",
                   root_processor, i);
           fatal("the resource map has no processor on that link");
         }
	purpose = RmGetProcessorPurpose(neighbour);
	if ((purpose & RmP_Mask) eq RmP_IO) return(neighbour);
	if ((purpose & RmP_Mask) eq RmP_Native) return(RmM_NoProcessor);
	report("root processor %P was booted from link %d",
		root_processor, i);
	fatal("the resource map has an invalid processor on that link");
      }
   }
  return(RmM_NoProcessor);
}

/*}}}*/
