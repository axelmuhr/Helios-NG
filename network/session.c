/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- session.c								--
--                                                                      --
--	Main module of the Session Manager				--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/session.c,v 1.19 1993/08/12 13:52:06 nickc Exp $*/

/*{{{  version number and history */
static char *VersionNumber = "3.12";
/**
*** History :
***          3.00, initial version
***          3.01, Helios 1.2 beta 0 release
***          3.02, Helios 1.2 beta 1 release
***          3.03, developed at Parsytec, 22.10.90-30.10.90
***          3.04, finishing off for the official 1.2 release
***          3.05, the real 1.2 release
***          3.06, first non-1.2 version
***          3.07, now supports single-processor version of Helios
***          3.08, clean up for fault tolerance
***          3.09, fix relating to TFM startup delays, plus diagnostics
***	     3.10, fixed bug relating to simultaneous logins
***          3.11, password encryption
***          3.12, reduced memory requirements for C40 1.3.1 release
**/
/*}}}*/
/*{{{  header files and compile-time options */
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
#include <process.h>
#include <attrib.h>
#include <pwd.h>
#include <signal.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "session.h"
#include "rmlib.h"
#include "sessaux.h"

	/* Compile-time debugging options	*/
#define Use_Malloc_Debugging		0
#define Use_Objects_Debugging		0
#define ShowHeap_Debugging		0
#define Use_IOC_Debugging		0
#define Default_Diagnostics_Options	0
#define Use_Ports_Debugging		0
/*}}}*/
/*{{{  forward declarations and statics */
/**-----------------------------------------------------------------------------
*** Forward declarations and statics.
***
*** At present the Session Manager /sm contains two types of entry:
***
***   1) there is a /Windows subdirectory containing details of all registered
***      windows, each of which is a WindowNode structure. The Session Manager
***      runs login in all of these windows, via a separate getty thread.
***   2) the top level contains details of all current sessions, each in a
***      SessionNode structure. Note that there is some theoretical overlap
***      between the /Windows entries and the sessions, but this is ignored.
***
*** When passwords are fully supported there will be a third entry,
*** /UserDatabase, which can be accessed in order to examine and manipulate
*** passwords and related information.
**/

static void	do_open(ServInfo *);
static void	do_create(ServInfo *);
static void	do_private(ServInfo *);
static void	do_delete(ServInfo *);
static void	monitor_thread(void);
static void	check_world(void);
static void	show_startup(void);
static void	getty_process(WindowNode *node);
static int	start_tfm(SessionNode *node);

static Object		*NameEntry		= Null(Object);
static Object		*ThisProcessor		= Null(Object);
static Stream		*DiagnosticsStream	= Null(Stream);
static Stream		*DefaultDiagnostics	= Null(Stream);
static DirNode		SessionRoot;
static DirNode		Windows;
static bool		SingleUserMode		= FALSE;
       bool		PasswordChecking	= FALSE;
static int		MonitorInterval;
       int		DebugOptions		= Default_Diagnostics_Options;
static Object		*LoginProgram;
static Object		*TaskforceManager;
static Environ		*my_environ;
       Semaphore	LibraryLock;
       char		**nsrc_environ		= NULL;
       
static DispatchInfo SessionInfo =
	{ &SessionRoot,
	  NullPort,
	  SS_SM,
	  Null(char),
	  { do_private, SM_Stack },
	  {
	  	 { do_open,		SM_Stack	},
	  	 { do_create,		SM_Stack	}, /* Create */
	  	 { DoLocate,		0		},
	  	 { DoObjInfo,		0		},
	  	 { InvalidFn,		0		}, /* ServerInfo */
	  	 { do_delete,		SM_Stack	}, /* Delete */
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
*** 1) check the arguments, to ensure that the Session Manager was started
***    up by the Startns program. 
***
*** 2) fill in the various globals etc.
***
*** 3) initialise the user database, and add the resulting DirEntry to
***    the root data structure. (mostly a no-op in Helios 1.2.x)
***
*** 4) add the Windows subdirectory, for windows added by the newuser
***    command
***
*** 5) if network monitoring is enabled, start up a thread to check
***    every session and window at regular intervals.
***
*** 6) create the name table entry and call the dispatcher
***
*** Note that the Startns utility tries very hard to ensure that this program
*** runs only on the root processor.
**/

int main(int argc, char **argv)
{ char ProcessorName[128];

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif

  signal(SIGPIPE, SIG_IGN);

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

  DiagnosticsStream	= fdstream(1);
  DefaultDiagnostics	= DiagnosticsStream;

  my_environ		= getenviron();
  InitSemaphore(&(LibraryLock), 1);

  if (strcmp(argv[0], "startns-magic"))
   fatal("please use startns to start up the networking software");
  nsrc_environ	= &(argv[4]);
  argc		= argc;
  show_startup();

#ifndef SingleProcessor  
	/* Inform the resource management library who I am */
  RmProgram	= Rm_Session;
  (void) EncodeCapability(argv[1], &RmLib_Cap);
#endif
  
  if (MachineName(ProcessorName) ne Err_Null)
   fatal("MachineName failure");

  SingleUserMode   = (get_config("single_user", nsrc_environ) ne Null(char));
  PasswordChecking = (get_config("password_checking", nsrc_environ) ne Null(char));
  MonitorInterval  = get_int_config("monitor_interval", nsrc_environ);
    
  ThisProcessor = Locate(Null(Object), ProcessorName);
  if (ThisProcessor eq Null(Object))
   fatal("failed to locate own processor");

  LoginProgram = Locate(CurrentDir, LoginName);
  if (LoginProgram eq Null(Object))
   fatal("failed to locate login program %s", LoginName);

  if (get_config("no_taskforce_manager", nsrc_environ) eq Null(char))
   { TaskforceManager = Locate(CurrentDir, TfmName);
     if (TaskforceManager eq Null(Object))
      fatal("failed to locate Taskforce Manager %s", TfmName);
   }
		/* Access to /sm : darwv:rx:ry:rz */
		/* This prevents arbitrary users getting hold of a capability */
		/* for somebody else's Taskforce Manager */
  InitNode((ObjNode *) &SessionRoot, "sm", Type_Directory, 
  	(PasswordChecking) ? RmFlags_PasswordChecking : 0, 0x211109c7);   
  InitList(&(SessionRoot.Entries));
  SessionRoot.Nentries = 0;
  if ((SessionInfo.ReqPort = NewPort()) eq NullPort)
   fatal("unable to allocate a message port");

#if 0
  if (!InitUserDatabase())
   fatal("failed to initialise user database");

	/* Full user database is not yet supported	*/
  Insert(&SessionRoot, &UserDatabase, FALSE);
#endif

  InitNode((ObjNode *) &Windows, "Windows", Type_Directory, 0, 0x211109c7);
  InitList(&(Windows.Entries));
  Windows.Nentries = 0;
  Insert(&SessionRoot, (ObjNode *) &Windows, FALSE);

  if (MonitorInterval eq Invalid_config)
   MonitorInterval = 30;
  elif (MonitorInterval > (30 * 60)) 
   MonitorInterval = 30 * 60;
  elif ((MonitorInterval >= 0) && (MonitorInterval < 20))
   MonitorInterval = 20;

  if (MonitorInterval >= 0)
   if (!Fork(Monitor_Stack, &monitor_thread, 0))
    fatal("not enough memory to initialise session monitoring");

  { NameInfo name;
    name.Port   = SessionInfo.ReqPort;
    name.Flags  = Flags_StripName;
    name.Matrix = 0x21212147;	/* rz: rz : rz : rwvd */
    name.LoadData = Null(WORD);
    NameEntry = Create(ThisProcessor, "sm", Type_Name, sizeof(NameInfo),
      (BYTE *) &name);
    if (NameEntry eq Null(Object))
     fatal("failed to enter name in name table, error code %x",
           Result2(ThisProcessor));
  }

  Dispatch(&SessionInfo);
  
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

  strcpy(output_buffer, init);
  
  for (dest = output_buffer + strlen(output_buffer); *format ne '\0'; format++)
   { if (*format ne '%')
      { *dest++ = *format; continue; }
     switch (*(++format))
      { case	'\0': *dest++ = '%'; format--; break;
        case	'%' : *dest++ = '%'; break;
        case    'c' : *dest++ = (char) va_arg(args, int); break;
        case	's' : { char	*temp = va_arg(args, char *);
			if (temp eq Null(char))
			 { *dest++ = '<'; *dest++ = 'n'; *dest++ = 'u';
			   *dest++ = 'l'; *dest++ = 'l'; *dest++ = '>';
			   break;
			 }
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
  static	char *message = "sm: error is fatal, exiting.\n";
  
  Wait(&(LibraryLock));
  va_start(list, format);
  length = process_format("sm: ", format, list);
  va_end(list);

  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  (void) Write(DiagnosticsStream, message, strlen(message), -1);
  Signal(&LibraryLock);

  if (NameEntry ne Null(Object)) Delete(NameEntry, Null(char));
  Exit(EXIT_FAILURE << 8);
}

void report(char *format, ...)
{ va_list	args;
  int		length;  
  
  va_start(args, format);
  
  Wait(&LibraryLock);
  length = process_format("sm: ", format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  Signal(&LibraryLock);
}

void window_report(Stream *window, char *format, ...)
{ va_list	args;
  int		length;

  va_start(args, format);
  Wait(&LibraryLock);
  length = process_format("sm: ", format, args);
  va_end(args);
  (void) Write(window, output_buffer, length, -1);
  Signal(&LibraryLock);
}

void debug(char *format, ...)
{ va_list	args;
  int		length;
  
  va_start(args, format);
  
  Wait(&LibraryLock);
  length = process_format("sm.debug: ", format, args);
  va_end(args);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
  Signal(&LibraryLock);
}
  
static void show_startup(void)
{ int	length;
  strcpy(output_buffer, "Session Manager version ");
  strcat(output_buffer, VersionNumber);
  strcat(output_buffer, ".\n");
  length = strlen(output_buffer);
  (void) Write(DiagnosticsStream, output_buffer, length, -1);
}
/*}}}*/
/*{{{  do_Create() */
/**-----------------------------------------------------------------------------
*** The Server Library routines.
***
*** do_create() serves two purposes:
***
*** 1) It can be used by the newuser command or the RmRegisterWindow()
***    routine to add an entry to the /sm/Windows directory.
*** 2) It can be used by the login command, the RmCreateSession() routine,
***    or anything else that is interested, to start a new session. Details
***    of the password must usually be supplied.
**/
static void do_create_session(ServInfo *);
static void do_create_window(ServInfo *);

static void do_create(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  IOCCreate	*req		= (IOCCreate *) m->Control;
  DirNode	*d;

  Debug(dbg_Create, ("create request received"));
  
  d = GetTargetDir(servinfo);
  if (d eq Null(DirNode))
   { ErrorMsg(m, Err_Null); return; }

 	/* Two types of Create() are legal : */
	/* 1) Create("/sm/windows", "pc.windows.console", Type_Device, ...); */
	/* 2) Create("/sm", "bart", Type_Session, ...); */

  if ((d eq &SessionRoot) && (req->Type eq Type_Session))
   do_create_session(servinfo);
  elif ((d eq &Windows) && (req->Type eq Type_Device))
   do_create_window(servinfo);
  else
   ErrorMsg(m, EC_Error | EG_WrongFn + EO_Server);
}

	/* Creating a new entry in /sm/Windows.			*/
static void do_create_window(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  ObjNode	*f		= Null(ObjNode);
  char		*pathname 	= servinfo->Pathname;
  IOCCreate	*req 		= (IOCCreate *) m->Control;
  RmWindowInfo	*info;
  MsgBuf	*r 		= Null(MsgBuf);
  WindowNode	*window_node	= Null(WindowNode);

  Debug(dbg_Create, ("registering a window"));

  f = GetTargetObj(servinfo);
  if (f ne Null(ObjNode))	/* window names must be unique */
   { Debug(dbg_Create, ("name already in use"));
     ErrorMsg(m, EC_Error + EG_InUse + EO_Name);
     goto error;
   }
  m->MsgHdr.FnRc = SS_SM;

	/* Allocate space for a new WindowNode, and initialise it.	   */
  window_node = New(WindowNode);
  if (window_node eq Null(WindowNode))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Name); }

  InitNode(&(window_node->ObjNode), objname(pathname), Type_Device, 0,
		 DefFileMatrix);
  InitList(&(window_node->ObjNode.Contents));
  window_node->Window		= Null(Object);
  window_node->WindowStream	= Null(Stream);
  window_node->LoginProgram	= Null(Object);
  window_node->LoginStream	= Null(Stream);
  window_node->WindowServer	= Null(Object);
  window_node->DefaultUser[0]	= '\0';

	/* Extract the information sent by the client		*/
	/* First, the default user name if any.			*/
  info = (RmWindowInfo *) &(m->Data[req->Info]);
  if (info->UserName ne -1)
   { string name = (string) RTOA(info->UserName);
     if (strlen(name) >= NameMax)
      { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Name); goto error; }
     strcpy(window_node->DefaultUser, name);
   }

	/* Check the window that has been supplied.			*/
  Debug(dbg_Create, ("attempting to access this window"));

  window_node->Window = NewObject(RTOA(info->WindowName), &(info->WindowCap));
  if (window_node->Window eq Null(Object))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Window); goto error; }

  window_node->WindowStream = Open(window_node->Window, Null(char), info->Flags & Flags_Mode);
  if (window_node->WindowStream eq Null(Stream))
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Window); goto error; }
  if ((window_node->WindowStream->Flags & Flags_Interactive) eq 0)
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Window); goto error; }
  if (info->Pos > 0)
   (void) Seek(window_node->WindowStream, S_Beginning, info->Pos);

	/* Access the window server supplied, if any.			*/
  if (info->WindowServerName ne MinInt);
   { window_node->WindowServer = 
         NewObject(RTOA(info->WindowServerName), &(info->WindowServerCap));
     if (window_node->WindowServer eq Null(Object))
      { ErrorMsg(m, EC_Error + EG_Invalid + EO_Server); goto error; }
   }

 	/* Allocate the message buffer for FormOpenReply */
  r = New(MsgBuf);
  if( r == Null(MsgBuf) )
   { ErrorMsg(m,EC_Error+EG_NoMemory); goto error; }

 	/* Try to Fork() off the getty process */
  Debug(dbg_Create, ("spawning getty thread"));
  unless(Fork(Getty_Stack, &getty_process, 4, window_node))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Session); goto error; }

  	/* Everything is ready, prepare and send off the reply, insert	*/
  	/* the ObjNode into the /sm/windows/directory, and finished.	*/
  FormOpenReply(r, m, &(window_node->ObjNode), Flags_Server, pathname);
  Insert(&Windows, &(window_node->ObjNode), TRUE);
  PutMsg(&(r->mcb));
  Free(r);
  return;
      
error:
  if (window_node ne Null(WindowNode))
   { if (window_node->Window ne Null(Object))
      Close(window_node->Window);
     if (window_node->WindowStream ne Null(Stream))
      Close(window_node->WindowStream);
     if (window_node->WindowServer ne Null(Object))
      Close(window_node->WindowServer);
     Free(window_node);
   }
}

static void do_create_session(ServInfo *servinfo)
{ static int	sequence_number	= 1;
  MCB		*m		= servinfo->m;
  char		*pathname	= servinfo->Pathname;
  IOCCreate	*req		= (IOCCreate *) m->Control;
  ObjNode	*f;
  SessionNode	*session_node	= Null(SessionNode);
  RmLoginInfo	*info;
  MsgBuf	*r		= Null(MsgBuf);  
#ifdef SingleProcessor
  bool		no_tfm		= TRUE;
#else
  bool		no_tfm		= FALSE;
#endif

  Debug(dbg_Create, ("request to create a session"));
  
	/* Check whether the user is already logged in. If so add	*/
	/* a sequence number to the username and go back to locking	*/
	/* the parent directory.					*/
  f = GetTargetObj(servinfo);
  if (f ne Null(ObjNode))	/* User already logged in */
   { UnLockTarget(servinfo);
     servinfo->Target = (ObjNode *) &SessionRoot;
     LockTarget(servinfo);
     strcat(pathname, ".");
     addint(pathname, sequence_number++);
   }
  else
   m->MsgHdr.FnRc = SS_SM;

	/* On a single-processor system, users may log in with or	*/
	/* without a TFM. This is controlled by a # character in front	*/
	/* of the name. For compatibility this is supported even for	*/
	/* multi-processor systems, but it has no effect since the	*/
	/* default for multi-processor systems is to run a TFM.		*/
  { char	*tmp = objname(pathname);
    char	*tmp2, *tmp3;
    ObjNode	*junk;
    char	tempbuf[NameMax + 1];
    Object	*junk_obj;

    if (*tmp eq '#')
     { no_tfm = FALSE;
       for (tmp2 = tmp3 = tmp; *tmp2 ne '\0'; ) *tmp2++ = *++tmp3;

	/* It is necessary to check for a duplicate name again....	*/
       junk = Lookup( &SessionRoot, tmp, TRUE);
       if (junk ne Null(ObjNode))
	{ strcat(tmp, "."); addint(tmp, sequence_number++); }
     }

	/* Allow for users called helios, root, etc. whose names clash	*/
	/* with existing names. This assumes that server names are	*/
	/* fairly sensible and do not have .123 suffixes.		*/
    tempbuf[0]	= '/';
    strcpy(&(tempbuf[1]), tmp);
    junk_obj = Locate(Null(Object), tempbuf);
    if (junk_obj ne Null(Object))
     { strcat(tmp, "."); addint(tmp, sequence_number++); Close(junk_obj);}
  }

  if (TaskforceManager eq Null(Object)) no_tfm = TRUE;

	/* After these basic checks it is possible to allocate and	*/
	/* initialise a SessionNode structure.				*/
  session_node = New(SessionNode);
  if (session_node eq Null(SessionNode))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Session); goto done; }

						/* access: rwda:r:r:r	*/
  InitNode(&(session_node->ObjNode), objname(pathname), Type_Session, 0,
		0x010101C3);

  session_node->Window			= Null(Object);
  session_node->WindowStream		= Null(Stream);
  session_node->CurrentDirectory	= Null(Object);
  session_node->Program			= Null(Object);
  session_node->TFMProgram		= Null(Object);
  session_node->TFMStream		= Null(Stream);
  session_node->TFM			= Null(Object);

	/* the Create request contains various bits of info, including	*/
	/* window details. For batch jobs this is a batch log file.	*/
  info = (RmLoginInfo *) &(m->Data[req->Info]);

  session_node->Window = NewObject(info->WindowName, &(info->Cap));
  if (session_node->Window eq Null(Object))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Session); goto done; }

  session_node->WindowStream = Open(session_node->Window, Null(char), O_ReadWrite);
  if (session_node->WindowStream eq Null(Stream))
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Window); goto done; }

  if (session_node->WindowStream->Flags & Flags_Interactive)
   session_node->ObjNode.Flags |= Flags_Interactive;

  Debug(dbg_Create, ("validating password"));

	/* Validate the name and password.				*/
  if (!VerifyPassword(info->UserName, info->Password))
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Password); goto done; }

	/* In a single-user mode, there can be only one */
	/* BLV - user database addition			*/
  if (SingleUserMode && (SessionRoot.Nentries ne 1))
   { window_report(session_node->WindowStream, "this is a single-user system");
     ErrorMsg(m, EC_Error + EG_NoResource + EO_Session);
     goto done;
   }

  Debug(dbg_Create, ("obtaining user database information for this session"));
  unless(FillInSessionNode(info->UserName, session_node))
   { window_report(session_node->WindowStream, "invalid entry in user database");
     report("error in user database for name %s", info->UserName);
     ErrorMsg(m, EC_Error + EG_Invalid + EO_Name);
     goto done;
   }

  r = New(MsgBuf);
  if (r eq Null(MsgBuf))
   { ErrorMsg(m, EC_Error + EG_NoMemory + EO_Message); goto done; }

	/* Everything is fine, insert the node into the SessionRoot	*/
	/* directory before releasing the lock on the parent directory. */
  Insert(&SessionRoot, &(session_node->ObjNode), TRUE);

	/* Start the TFM if appropriate					*/
  unless(no_tfm)
   { int rc;

     UnLockTarget(servinfo);
     Debug(dbg_Create, ("attempting to start TFM"));
     rc = start_tfm(session_node);
     LockTarget(servinfo);
     if (rc ne Err_Null)
      { Unlink(&(session_node->ObjNode), TRUE);
	ErrorMsg(m, rc); goto done;
      }
   }

	/* Now construct the reply					*/
  req->Common.Access.Access = AccMask_Full; 
  FormOpenReply(r, m, &(session_node->ObjNode), 0, pathname);
  (void) PutMsg(&(r->mcb));
  Free(r); r = Null(MsgBuf);

	/* Unusually, the do_create thread hangs around waiting for	*/
	/* the TFM to terminate. This means the end of the session, so	*/
	/* some cleaning up is possible. This is only possible if there	*/
	/* is a TFM, of course...					*/
  unless(no_tfm)
   { UnLockTarget(servinfo);
     InitProgramInfo(session_node->TFMStream, PS_Terminate);
     Debug(dbg_Create, ("waiting for TFM termination"));
     GetProgramInfo(session_node->TFMStream, Null(WORD), -1);
     Debug(dbg_Create, ("TFM %s has terminated", session_node->TFMStream->Name));
     LockTarget(servinfo);
     Unlink(&(session_node->ObjNode), TRUE);
   }

done:
  if (r ne Null(MsgBuf)) Free(r);
  if ((session_node ne Null(SessionNode)) && !no_tfm)
   { if (session_node->TFMStream ne Null(Stream))
      { SendSignal(session_node->TFMStream, SIGKILL);
        Close(session_node->TFMStream);
      }
     if (session_node->TFMProgram ne Null(Object))
      { session_node->TFMProgram->FnMod	= 2;
	Delete(session_node->TFMProgram, Null(char));
        Close(session_node->TFMProgram);
      }
     if (session_node->TFM ne Null(Object))
      Close(session_node->TFM);
     if (session_node->Program ne Null(Object))
      Close(session_node->Program);
     if (session_node->CurrentDirectory ne Null(Object))
      Close(session_node->CurrentDirectory);
     if (session_node->WindowStream ne Null(Stream))
      Close(session_node->WindowStream);
     if (session_node->Window ne Null(Object))
      Close(session_node->Window);
     Free(session_node);
   }
}
/*}}}*/
/*{{{  do_Open() */

	/* do_open() serves several purposes:				*/
	/* 1) listing the directories /sm and /sm/Windows		*/
	/* 2) interacting with /sm/UserDatabase in future		*/
	/* 3) getting window details about another user			*/
	/* 4) getting full details about a newly created session	*/
static void do_open(ServInfo *servinfo)
{ MCB		*m          = servinfo->m;
  MsgBuf	*r	    = Null(MsgBuf);
  IOCMsg2	*req        = (IOCMsg2 *)(m->Control);
  char		*pathname   = servinfo->Pathname;
  Port		reqport;
  ObjNode	*f;
  SessionNode	*session;
  byte		*data = m->Data;
  int		bufsize = 0;
  BYTE		*buffer;       

  f = GetTarget(servinfo);
  if (f == Null(ObjNode)) 
   { ErrorMsg(m, EO_Object); return; }

	/* There is no point in opening /sm/windows/pc.window.console */
  if (f->Type eq Type_Device)
   { ErrorMsg(m, EC_Error + EG_WrongFn + EO_Window); return; }

	/* O_Private is used to find out capabilities etc. */
  if ((req->Arg.Mode & Flags_Mode) eq O_Private)
   { unless(CheckMask(req->Common.Access.Access, AccMask_D))
      { ErrorMsg(m, EC_Error + EG_Protected + EO_Object); return; }
   }
  else
   { unless( CheckMask(req->Common.Access.Access,(int)(req->Arg.Mode&Flags_Mode)) ) 
      { ErrorMsg(m,EC_Error+EG_Protected+EO_File); return; }
   }

  r = New(MsgBuf);
  if( r == Null(MsgBuf) )
   { ErrorMsg(m,EC_Error+EG_NoMemory); return; }

  FormOpenReply(r, m, f, Flags_Server | Flags_Closeable, pathname);

  reqport = NewPort();  
  r->mcb.MsgHdr.Reply = reqport;
  PutMsg(&r->mcb);
  Free(r);
  f->Account++;

	/* Accessing either /sm or /sm/windows */  
  if ((f->Type & Type_Flags) eq Type_Directory)
   { DirServer(servinfo, m, reqport); FreePort(reqport); return; }

#if 0
	/* Accessing /sm/UserDatabase, which does not exist in 1.2.x	*/
  if (f eq &UserDatabase)
   { OpenUserDatabase(servinfo, m, reqport); FreePort(reqport); return; }
#endif
	/* The client appears to be trying to access a current session. */     
	/* This may be an arbitrary user attempting to find out about	*/
	/* the login window for that user, or it may be the login	*/
	/* program attempting to get hold of the session's Taskforce	*/
	/* Manager, name+capability. The O_Private open mode is used	*/
	/* for the latter, read-only for the former. A suitable byte	*/
	/* buffer is allocated and filled in, so that GetFileSize()	*/
	/* and Read() are easy.						*/
  session = (SessionNode *) f;

  if ((req->Arg.Mode & Flags_Mode) eq O_Private)
   { RmSessionAux *aux;
     char	  *temp;
     
     bufsize = sizeof(RmSessionAux) + strlen(session->CurrentDirectory->Name) +
     		 strlen(session->Program->Name) + 3;
     if (session->TFM ne  (Object *) NULL)
      bufsize += strlen(session->TFM->Name) + 1;
      
     buffer = (BYTE *) Malloc(bufsize);
     if (buffer eq Null(BYTE)) goto done;
     aux = (RmSessionAux *) buffer;
     strcpy(aux->UserName, session->UserName);
     aux->Uid		= session->Uid;
     aux->Gid		= session->Gid;
     strcpy(aux->Comment, session->Comment);
     aux->CurrentDirCap	= session->CurrentDirectory->Access;
     aux->ProgramCap	= session->Program->Access;
     temp		= &(buffer[sizeof(RmSessionAux)]);
     if (session->TFM eq Null(Object))
      { memset(&(aux->TFMCap), 0, sizeof(Capability)); aux->TFMName = 0; }
     else
      { aux->TFMCap	= session->TFM->Access;
        aux->TFMName	= (word) temp - (word) &(aux->TFMName);
        strcpy(temp, session->TFM->Name);
        temp		+= strlen(temp) + 1;
      }
     strcpy(temp, session->CurrentDirectory->Name);
     aux->CurrentDirName = (word) temp - (word) &(aux->CurrentDirName);     
     temp		+= strlen(temp) + 1;
     strcpy(temp, session->Program->Name);
     aux->ProgramName	 = (word) temp - (word) &(aux->ProgramName);
   }
  else
   { RmLoginWindow	*window;
     bufsize	 = sizeof(RmLoginWindow) + strlen(session->Window->Name);
     buffer	 = (BYTE *) Malloc(bufsize);
     if (buffer eq Null(BYTE)) goto done;

     window	 = (RmLoginWindow *) buffer;
     window->Cap = session->Window->Access;
     strcpy(window->WindowName, session->Window->Name);
   }

  UnLockTarget(servinfo);
  
  forever
   { word e;
     m->MsgHdr.Dest = reqport;
     m->Timeout     = StreamTimeout;
     m->Data        = data;

     e = GetMsg(m);
     if (e == EK_Timeout) break;
     if (e < Err_Null) continue;
     
     Wait(&f->Lock);

     m->MsgHdr.FnRc = 0;

     switch(e & FG_Mask)
      {
      	 case FG_Read  :
      	 		{ WORD	Pos	= m->Control[0];
      	 		  WORD  Size 	= m->Control[1];
      	 		  WORD	rc	= ReadRc_EOF;
      	 		  
      	 		  if ((Pos >= bufsize) || (Pos < 0))
      	 		   { ErrorMsg(m, EC_Error + EG_WrongSize + EO_Window);
      	 		     break;
      	 		   }

			  if ((Pos + Size) > bufsize)
			   { Size = bufsize - Pos;
			     rc   = ReadRc_EOF;
			   }
   			  InitMCB(m, 0, m->MsgHdr.Reply, NullPort, rc);
   			  m->MsgHdr.DataSize = (int) Size;
   			  m->Data = &(buffer[Pos]);
   			  PutMsg(m);
   			  break;
			}
			      	 
      	 case FG_GetSize :
      	 		InitMCB(m, 0, m->MsgHdr.Reply, NullPort, Err_Null);
      	 		MarshalWord(m, bufsize);
      	 		PutMsg(m);
      	 		break;
      	 
         case FG_Close :
                        if (m->MsgHdr.Reply != NullPort) ErrorMsg(m, Err_Null);
                        Signal(&f->Lock);
		        goto done;
         default       :
                        ErrorMsg(m, EC_Error + SS_SM + EG_FnCode + EO_File);
                        break;
       }

     Signal(&f->Lock);
   }

done:
  f->Account--;
  FreePort(reqport);
}

/*}}}*/
/*{{{  do_Delete() */

	/* do_delete() can be used under exceptional circumstances to	*/
	/* abort a newly-created session by sending a kill signal to	*/
	/* the TFM.							*/
static void do_delete(ServInfo *servinfo)
{ MCB		*m		= servinfo->m;
  ObjNode	*o		= GetTarget(servinfo);
  IOCCommon	*req		= (IOCCommon *) m->Control;
  SessionNode	*session_node;

  Debug(dbg_Delete, ("request to abort a session"));

  if (o eq Null(ObjNode))
   { ErrorMsg(m, EC_Error + EG_Unknown + EO_Session); return; }
  if (o->Type ne Type_Session)
   { ErrorMsg(m, EC_Error + EG_Invalid + EO_Session); return; }
  unless(CheckMask(req->Access.Access, AccMask_D))
   { ErrorMsg(m, EC_Error + EG_Protected + EO_Object); return; }

  Debug(dbg_Delete, ("session exists, access rights checked..."));

	/* remove the directory entry, and acknowledge.			*/
  UnLockTarget(servinfo);
  Wait(&SessionRoot.Lock);
  Unlink(o, TRUE);

  ErrorMsg(m, Err_Null);

  Debug(dbg_Delete, ("attempting to destroy a running session"));

	/* If the relevant session involved a TFM, send a signal to	*/
	/* it. This should wake up do_create_session(), which will tidy	*/
	/* up as required.						*/
  session_node = (SessionNode *) o;
  if (session_node->TFMStream ne Null(Stream))
   { Stream *current = session_node->TFMStream;
     Stream *tfm = NewStream(current->Name, &(current->Access), current->Flags);
     SendSignal(tfm, SIGTERM);
     Close(tfm);
   }
	/* If this Session did not involve a TFM, do_delete has to	*/
	/* do the cleaning up.						*/
  else
   { 
     if (session_node->TFMProgram ne Null(Object))
      { session_node->TFMProgram->FnMod	= 2;
	Delete(session_node->TFMProgram, Null(char));
        Close(session_node->TFMProgram);
      }
     if (session_node->TFM ne Null(Object))
      Close(session_node->TFM);
     if (session_node->Program ne Null(Object))
      Close(session_node->Program);
     if (session_node->CurrentDirectory ne Null(Object))
      Close(session_node->CurrentDirectory);
     if (session_node->WindowStream ne Null(Stream))
      Close(session_node->WindowStream);
     if (session_node->Window ne Null(Object))
      Close(session_node->Window);
     Free(session_node);
   }

  Signal(&SessionRoot.Lock); 
}

/*}}}*/
/*{{{  do_Private() */

	/* do_private() is used by the Network Server to indicate that	*/
	/* something has gone wrong in the network. The Network Server	*/
	/* will have sent its capability, which can be compared with	*/
	/* the capability received on start-up. It is also used in	*/
	/* conjunction with diag_sm.					*/
static void do_debug(ServInfo *servinfo);

static void do_private(ServInfo *servinfo)
{ MCB            *m          = servinfo->m;
  ObjNode        *f;

  f = GetTarget(servinfo);
  if (f == Null(ObjNode)) 
   { ErrorMsg(m, EO_Object); return; }

  UnLockTarget(servinfo);
  if ((servinfo->FnCode & FG_Mask) eq FG_NetStatus)
   { Capability *cap	= (Capability *) &(m->Control[5]);
     unless (memcmp(cap, &RmLib_Cap, sizeof(Capability)) eq 0)
      { ErrorMsg(m, EC_Error + EG_Protected + EO_Server); return; }
     ErrorMsg(m, Err_Null);
     check_world();
   }
  elif ((servinfo->FnCode & FG_Mask) eq FG_GetInfo)
   do_debug(servinfo);
  else
   ErrorMsg(m, EC_Error + EG_WrongFn + EO_Object);
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
   { static	char	*message = "sm: output redirected\n";
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
   DebugOptions = (int) req->Arg.Mode;
  if (DebugOptions & dbg_Memory)
   { report("currently %d bytes are free, the heap size is %d", Malloc(-1),
   		Malloc(-3));
     DebugOptions &= ~dbg_Memory;
#if ShowHeap_Debugging
     ShowHeap();
#endif
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
/*{{{  getty process */

/**-----------------------------------------------------------------------------
*** The getty process. This is Fork()'ed off for every window that the Session
*** Manager is supposed to be in charge off. It runs the login program in an
*** infinite loop.
**/
static void getty_process(WindowNode *window_node)
{ Environ	Env;
  char		*Argv[3];
  char		*Envv[1];
  Stream	*Strv[5];
  Object	*Objv[OV_End + 1];
  Stream	*window = window_node->WindowStream;

  window->Flags 	&= ~Flags_CloseOnSend;

	/* The login environment can be built statically		*/    
  Argv[0]		 = LoginArgv0;
  Argv[2]		 = Null(char);
  Envv[0]		 = Null(char);
  Strv[0]		 = window;
  Strv[1]		 = window;
  Strv[2]		 = window;
  Strv[3]		 = my_environ->Strv[3];	/* /logger */
  Strv[4]		 = Null(Stream);
  Objv[OV_Cdir]		 = my_environ->Objv[OV_Cdir];
  Objv[OV_Task]		 = (Object *) MinInt;
  Objv[OV_Code]		 = (Object *) MinInt;
  Objv[OV_Source]	 = LoginProgram;
  Objv[OV_Parent]	 = my_environ->Objv[OV_Task];
  Objv[OV_Home]		 = my_environ->Objv[OV_Home];
  Objv[OV_Console]	 = NewObject(window->Name, &(window->Access));
  Objv[OV_CServer]	 = window_node->WindowServer;
  Objv[OV_Session]	 = (Object *) MinInt;
  Objv[OV_TFM]		 = (Object *) MinInt;
  Objv[OV_TForce]	 = (Object *) MinInt;
  Objv[OV_End]		 = Null(Object);
  Env.Envv		 = Envv;
  Env.Strv		 = Strv;
  Env.Objv		 = Objv;
  Env.Argv		 = Argv;
  
  forever
   { window_node->LoginProgram = Execute(Null(Object), LoginProgram);
     if (window_node->LoginProgram eq Null(Object))
      { report("failed to run login program in window %s", window->Name);
        break;
      }
     Objv[OV_Task]	= window_node->LoginProgram;
     
     window_node->LoginStream = Open(window_node->LoginProgram, Null(char), O_ReadWrite);
     if (window_node->LoginStream eq Null(Stream))
      { report("failed to open login program %s in window %s",
		window_node->LoginProgram->Name, window->Name);
       	break;
      }

     if (window_node->DefaultUser[0] eq '\0')
      Argv[1] = Null(char);
     else
      Argv[1] = window_node->DefaultUser;
     if (SendEnv(window_node->LoginStream->Server, &Env) < Err_Null)
      { report("failed to send environment to login program %s in window %s",
      		 window_node->LoginStream->Name, window->Name);
      	break;
      }
     window_node->DefaultUser[0] = '\0';
     InitProgramInfo(window_node->LoginStream, PS_Terminate);        
     GetProgramInfo( window_node->LoginStream, NULL, -1);

     Close(window_node->LoginStream);
     window_node->LoginStream	= Null(Stream);
     Delete(window_node->LoginProgram, Null(char));
     Close(window_node->LoginProgram);
     window_node->LoginProgram	= Null(Object);

	/* Various checks to ensure that this window still exists	*/
     if (window_node->ObjNode.Parent eq Null(DirNode)) break;
      { Object	*temp = Locate(Null(Object), window_node->Window->Name);
        if (temp eq Null(Object)) 
	 break;
        else
         Close(temp);
      }
   }

  Wait(&Windows.Lock);  
  if (window_node->ObjNode.Parent ne Null(DirNode))
   Unlink(&(window_node->ObjNode), TRUE);
  if (window_node->Window ne Null(Object))
   Close(window_node->Window);
  if (window_node->WindowStream ne Null(Stream))
   Close(window_node->WindowStream);
  if (window_node->WindowServer ne Null(Object))
   Close(window_node->WindowServer);
  if (window_node->LoginStream ne Null(Stream))
   { SendSignal(window_node->LoginStream, SIGKILL);
     Close(window_node->LoginStream);
   }
  if (window_node->LoginProgram ne Null(Object))
   { window_node->LoginProgram->FnMod = 2;
     Delete(window_node->LoginProgram, Null(char));
     Close(window_node->LoginProgram);
   }
  Free(window_node);
  Signal(&Windows.Lock);
}

/*}}}*/
/*{{{  start TFM */

/**-----------------------------------------------------------------------------
*** Starting up a TFM. In the single processor system the TFM always runs
*** locally. Otherwise it is  necessary to obtain a processor from the
*** Network Server.
**/
static int	run_tfm(SessionNode *, Object *processor, RmProcessor);

static int	start_tfm(SessionNode *session_node)
{ word		rc;
  Object	*processor_obj	= Null(Object);
  RmProcessor	obtained = (RmProcessor) NULL;

#ifndef SingleProcessor
  RmProcessor	template = RmNewProcessor();
  int		length;
  char		*attrib;

  if (template eq (RmProcessor) NULL)
   { rc = EC_Error + EG_NoMemory + EO_Processor; goto done; }

  template->ObjNode.Account = session_node->ObjNode.Account;
  length = strlen(session_node->WindowStream->Name) + 6;
  attrib = (char *) Malloc(length);
  if (attrib eq Null(char))
   { rc = EC_Error + EG_NoMemory + EO_Processor; goto done; }

  strcpy(attrib, "NEAR=");
  strcat(attrib, session_node->WindowStream->Name);
  rc = RmAddProcessorAttribute(template, attrib);
  Free(attrib);
  if (rc ne RmE_Success)
   { rc += EC_Error + EG_RmLib; goto done; }

  obtained  = RmObtainProcessor(template);
  if (obtained eq (RmProcessor) NULL)
   { rc = EC_Error + EG_RmLib + RmErrno; goto done; }

  processor_obj = RmMapProcessorToObject(obtained);
#else
	/* On a single-processor system the current processor has to	*/
	/* be used for the TFM.						*/
  processor_obj = Locate(ThisProcessor, Null(char));
#endif

  if (processor_obj eq Null(Object))
   { rc = EC_Error + EG_Invalid + EO_Processor; goto done; }

  rc = run_tfm(session_node, processor_obj, obtained);

done:
  if (processor_obj ne Null(Object))
   Close(processor_obj);

#ifndef SingleProcessor
  if (template ne (RmProcessor) NULL) RmFreeProcessor(template);
  if (obtained ne (RmProcessor) NULL)
   { RmReleaseProcessor(obtained);
     RmFreeProcessor(obtained);
   }
#endif
  return( (int)rc);
}

static int run_tfm(SessionNode *session, Object *processor, RmProcessor obtained)
{ Object	*procman	= Null(Object);
  Object	*tfm		= Null(Object);
  Object	*tfm_program	= Null(Object);
  Stream	*tfm_stream	= Null(Stream);
  Stream	*window_stream	= session->WindowStream;
  Object	*pipe_server	= Null(Object);
  Object	*pipe		= Null(Object);
  Stream	*pipe_stream	= Null(Stream);
  Environ	env;
  Stream	*strv[5];
  Object	*objv[OV_End + 1];
  char		*argv[4];
  char		argv2[NameMax];
  char		pipe_name[NameMax];
  static	int pipe_number = 1;
  word		rc = Err_Null;

  strv[0]		= Null(Stream);
  objv[OV_Console]	= Null(Object);
      
  pipe_server	= Locate(Null(Object), "/pipe");
  if (pipe_server eq Null(Object))
   { window_report(window_stream, "internal error");
     report("error accessing pipe server");
     rc = EC_Error + EG_Broken + EO_Processor;
     goto done;
   }

  strcpy(pipe_name, "sm.");
  addint(pipe_name, pipe_number++);
  pipe = Create(pipe_server, pipe_name, Type_Pipe, 0, Null(BYTE));
  if (pipe eq Null(Object))
   { window_report(window_stream, "internal error");
     report("error creating pipe %s", pipe_name);
     rc = EC_Error + EG_Create + EO_Pipe;
     goto done;
   }

  pipe_stream = PseudoStream(pipe, O_ReadWrite);
  if (pipe_stream eq Null(Stream))
   { window_report(window_stream, "internal error");
     report("error opening pipe %s", pipe->Name);
     rc = EC_Error + EG_Open + EO_Pipe;
     goto done;
   }

  strv[0] = PseudoStream(pipe, O_ReadWrite);
  if (strv[0] eq Null(Stream))
   { window_report(window_stream, "internal error");
     report("error opening pipe %s", pipe->Name);
     rc = EC_Error + EG_Open + EO_Pipe;
     goto done;
   }
    
  procman = Locate(processor, "tasks");        
  if (procman eq Null(Object))
   { window_report(window_stream, "internal error"); 
     report("error contacting Processor Manager %s/tasks, fault %x",
	  processor->Name, Result2(processor));
     rc = EC_Error + EG_Broken + EO_Processor; 
     goto done; 
   }

  tfm_program = Execute(procman, TaskforceManager);
  if (tfm_program eq Null(Object))
   { window_report(window_stream, "internal error");
     report("failed to execute Taskforce Manager, 0x%08x", Result2(procman));
     rc = EC_Error + EG_Create + EO_TFM;
     goto done;
   }
  tfm_stream = Open(tfm_program, Null(char), O_ReadWrite);
  if (tfm_stream eq Null(Stream))
   { window_report(window_stream, "internal error");
     report("failed to access Taskforce Manager %s, %08x", tfm->Name,
       		 Result2(tfm));
     rc = EC_Error + EG_Open + EO_TFM;
     goto done;
   }

  window_stream->Flags &= ~Flags_CloseOnSend;
  objv[OV_Cdir]		= session->CurrentDirectory;
  objv[OV_Task]		= tfm_program;
  objv[OV_Code]		= (Object *) MinInt;
  objv[OV_Source]	= TaskforceManager;
  objv[OV_Parent]	= my_environ->Objv[OV_Task];
  objv[OV_Home]		= session->CurrentDirectory;
  objv[OV_Console]	= NewObject(window_stream->Name, &(window_stream->Access));
  objv[OV_CServer]	= (Object *) MinInt;
  objv[OV_Session]	= (Object *) MinInt;
  objv[OV_TFM]		= (Object *) MinInt;	/* does not yet exist */
  objv[OV_TForce]	= (Object *) MinInt;
  objv[OV_End]		= Null(Object);
  strv[1]		= window_stream;
  strv[2]		= window_stream;
  strv[3]		= my_environ->Strv[3];
  strv[4]		= Null(Stream);
  argv[0]		= "SessionManager";
  argv[1]		= session->ObjNode.Name;
  argv[2]		= argv2;
#ifdef SingleProcessor
  strcpy(argv2, "                ");
#else  
  (void) DecodeCapability(argv2, &RmLib_Cap); 
#endif  
  argv[3]		= Null(char);
  env.Objv		= objv;
  env.Strv		= strv;
  env.Argv		= argv;
  env.Envv		= environ;
    
  if (SendEnv(tfm_stream->Server, &env) < Err_Null)
   { window_report(window_stream, "internal error");
     report("failed to send environment to %s, fault %08x", tfm_stream->Name,
     		Result2(tfm_stream));
     rc = EC_Error + EG_Broken + EO_Program;
     goto done;
   }

  Close(strv[0]); strv[0] = Null(Stream);

#ifndef SingleProcessor
  rc = RmWriteProcessor(pipe_stream, obtained, (RmFilter) NULL);
  if (rc ne RmE_Success)
   { rc |= (EC_Error + EG_RmLib);
     goto done;
   }
#else
  obtained = obtained;
#endif   
  { Capability	cap;
    if (Read(pipe_stream, (BYTE *) &cap, sizeof(Capability), 60 * OneSec) ne sizeof(Capability))
     { rc = EC_Error + EG_Broken + EO_TFM; goto done; }

    Close(pipe_stream); pipe_stream = Null(Stream);
    Delay(OneSec / 2);	/* unfortunately needed */

    tfm = Locate(processor, session->ObjNode.Name);
    if (tfm eq Null(Object))
     { rc = EC_Error + EG_Broken + EO_TFM; goto done; } 
    tfm->Access = cap;
  }
  
  rc = Err_Null;
  
done:
  if (procman ne Null(Object))		Close(procman);
  if (pipe_stream ne Null(Stream))	Close(pipe_stream);
  if (strv[0] ne Null(Stream))		Close(strv[0]);
  if (pipe ne Null(Object))
   { Delete(pipe, Null(char)); Close(pipe); }
  if (pipe_server ne Null(Object))	Close(pipe_server);

  if (rc eq Err_Null)
   { session->TFM		= tfm;
     session->TFMStream		= tfm_stream;
     session->TFMProgram	= tfm_program;
   }
  else
   { if (tfm_stream ne Null(Stream))
      { SendSignal(tfm_stream, SIGKILL); Close(tfm_stream); }
     if (tfm_program ne Null(Object))
      { tfm_program->FnMod = 2; Delete(tfm_program, Null(char)); Close(tfm_program); }
     if (tfm ne Null(Object)) Close(tfm);
   }
  return( (int)rc);
}

/*}}}*/
/*{{{  monitoring and fault-tolerance */

/**-----------------------------------------------------------------------------
*** Fault tolerance support: code to monitor all sessions and windows, and
*** take suitable recovery action when things go wrong.
**/
static void monitor_thread(void)
{ forever
   { Delay(MonitorInterval * OneSec);
     check_world();
   }
}

static void check_world(void)
{ SessionNode	*session;
  Object	*thingy;
  WindowNode	*window;

	/* locking the root prevents any inserts and deletes	*/
  Wait(&SessionRoot.Lock);

  Debug(dbg_Monitor, ("checking the network's consistency"));

	/* Check all the sessions with running TFM's, that the TFM	*/
	/* still exists.						*/
  for ( session = Head_(SessionNode, SessionRoot.Entries); 
        !EndOfList_(session);
	session = Next_(SessionNode, session))
   { if (session->ObjNode.Type ne Type_Session) continue;
     if (session->TFM eq Null(Object)) continue;

     thingy = Locate(Null(Object), session->TFM->Name);
     if (thingy eq Null(Object))
      { Stream *tfm_stream = session->TFMStream;
        window_report(session->WindowStream, "lost contact with TFM %s", tfm_stream->Name);
	report("lost contact with TFM %s", tfm_stream->Name);
        AbortPort(tfm_stream->Reply, EC_Fatal);
      }
     else
      Close(thingy);
   }

  Signal(&SessionRoot.Lock);
  Wait(&Windows.Lock);

  for ( window = Head_(WindowNode, Windows.Entries);
	!EndOfList_(window);
	window = Next_(WindowNode, window))
   { Stream	*login_stream = window->LoginStream;
     if (login_stream eq Null(Stream)) continue;

     thingy = Locate(Null(Object), window->LoginProgram->Name);
     if (thingy eq Null(Object))
      { window_report(window->WindowStream, "lost contact with login program %s, restarting",
			window->LoginProgram->Name);
	AbortPort(login_stream->Reply, EC_Fatal);
	continue;
      }
     else
      Close(thingy);

     thingy = Locate(Null(Object), window->Window->Name);
     if (thingy eq Null(Object))
      { AbortPort(login_stream->Reply, EC_Fatal);
        continue;
      }
     else
      Close(thingy);
   }

  Signal(&Windows.Lock);
}

/*}}}*/

