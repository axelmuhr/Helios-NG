/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990 - 1993, Perihelion Software Ltd.      --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmlib2.c								--
--                                                                      --
--	The interaction module of the Resource Management library.	--
--	This module is responsible for communication between the 	--
--	application and the various servers.				--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/rmlib2.c,v 1.27 1994/03/10 17:13:40 nickc Exp $*/

#define in_rmlib	1

/*{{{  Headers */
#if defined __SUN4 || defined RS6000
#include </hsrc/include/memory.h>
#include </hsrc/include/link.h>
#define _link_h
#endif
#include <stddef.h>
#include <syslib.h>
#include <stdarg.h>
#include <string.h>
#include <root.h>
#include <posix.h>
#include <gsp.h>
#include <nonansi.h>
#include <process.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pwd.h>
#include "exports.h"
#include "private.h"
#include "rmlib.h"

#ifdef Malloc		/* courtesy of servlib.h */
#undef Malloc
#endif

#ifdef RS6000
extern Environ * getenviron( void );
#endif
/*}}}*/
/*{{{  Compile time options */

#ifdef __TRAN
#pragma -f0		/* 0 == disable vector stack			*/
#pragma -g0		/* remove names from code			*/
#endif

#ifdef STACKCHECK
#pragma	-s0
#else
#pragma -s1
#endif

#ifdef STACKEXTENSION
#define RmPipeGuardian_Stack	1500
#define RmDoSynch_Stack		750
#else
#define RmPipeGuardian_Stack	3000
#define RmDoSynch_Stack		1000
#endif

/*}}}*/
/*{{{  Miscellaneous utilities */

/*{{{  FullRead */

/**
*** A little utility routine to cope with the fact that pipe reads do
*** not necessarily return the amount of data requested.
**/
int FullRead(Stream *pipe, BYTE *buffer, int amount, word timeout)
{ int	read = 0;
  int	temp;

  forever  
  { temp = (int) Read(pipe, &(buffer[read]), ((word)amount - (word)read), timeout);
    if ((temp < 0) || ((temp eq 0) && (timeout eq -1)))
     return((read eq 0) ? temp : read);
    read += temp;
    if (read >= (word)amount) return(read);
    if (timeout ne -1) return(read);
  }
}

/*}}}*/
/*{{{  RmLookupProcessor() */
/**
*** Given a network, look up the processor. 
*** Arguments : Network, the root structure or a subnet
***             name, something like Cluster/00
*** 
*** The routine determines the last bit of the name, e.g. 00, and
*** searches the network. When a processor is reached whose ID matches
*** this last bit of the name, the search goes back up the tree trying
*** to match all the parents.
**/
static int	LookupAux1(RmProcessor, ...);

RmProcessor	RmLookupProcessor(RmNetwork Network, char *name)
{ char		*temp = name + strlen(name);

  if (*name eq '/') name++;  
  for ( ; (temp >= name) && (*temp ne '/'); temp--);

  return((RmProcessor) RmSearchProcessors(Network, &LookupAux1, name, ++temp));
}

static int LookupAux1(RmProcessor Processor, ...)
{ va_list	args;
  char		*name;
  char		*last_name;
  RmNetwork	current;
  RmNetwork	root_net;
  int		amount;
      
  va_start(args, Processor);
  name		= va_arg(args, char *);
  last_name	= va_arg(args, char *);
  va_end(args);

	/* Unless the last bit matches, do not bother to check */  
  if (strcmp(RmGetProcessorId(Processor), last_name)) return(0);

  current	= (RmNetwork) Processor;
  root_net	= RmRootNetwork(Processor);
  
  while (last_name > name)	/* If name is 00, match is immediate	*/
   { last_name--; last_name--;  /* Skip the / and get to last char	*/
     for ( amount = 0; (last_name >= name) && (*last_name ne '/'); 
           last_name--, amount++);
     last_name++;		/* should now be Cluster */
     current = RmParentNetwork((RmProcessor) current);
     if (current eq (RmNetwork) NULL) return(0);

     if ((current eq root_net) && (RmRootName ne NULL))
      { if (strncmp(RmRootName, last_name, amount)) return(0);
      }
     else
      { if (strncmp(current->DirNode.Name, last_name, amount)) return(0); 
      }
   }
  return((int) Processor);
}
/*}}}*/
#ifdef __HELIOS
/*{{{  RmBuildProcessorName() and RmMapProcessorToObject() */

/**
*** 1) RmBuildProcessorName(): given a suitable buffer of IOCDataMax bytes, 
***    and a processor that is part of a network, construct the full pathname
***    of the processor
**/
char *RmBuildProcessorName(char *buffer, RmProcessor processor)
{ const char *name;

  if ((processor eq NULL) ||
      ((processor->ObjNode.Type ne Type_Processor) && (processor->ObjNode.Type ne Type_Network)))
   { RmErrno = RmE_NotProcessor; return(NULL); }

  if (processor eq (RmProcessor) RmRootNetwork(processor))
   { *buffer++ = '/';
     if (RmRootName ne NULL)
      strcpy(buffer, RmRootName);
     else
      strcpy(buffer, processor->ObjNode.Name);
     return(&(buffer[strlen(buffer)]));
   }
  else
   { buffer = RmBuildProcessorName(buffer, (RmProcessor) RmParentNetwork(processor));
     *buffer++ = '/';
     for (name = processor->ObjNode.Name; *name ne '\0'; ) *buffer++ = *name++;
     *buffer = '\0';
     return(buffer);
   }
}

/**
*** 2) RmMapProcessorToObject(): given a processor that is part of a real
***    network, return a Helios Object structure corresponding to this
***    processor and containing the appropriate capability.
**/
Object	*RmMapProcessorToObject(RmProcessor processor)
{ char		*buf; 
  Object	*result;
  Capability	*cap;
  bool		newbuf = FALSE;

  CheckProcessorFail(processor, Null(Object));  

  if ((processor->StructType ne RmL_Obtained) &&
      (processor->StructType ne RmL_Existing))
   return(RmErrno = processor->Errno = RmE_NoAccess, Null(Object));
  cap = &(processor->RealCap);

  if (processor->ObjNode.Parent ne Null(DirNode))
   {   /* This processor is in an RmNetwork, build the name using the network */
     buf = (char *) Malloc(IOCDataMax);
     if (buf eq Null(char))
      return(RmErrno = processor->Errno = RmE_NoMemory, Null(Object));
     newbuf = TRUE;
     (void) RmBuildProcessorName(buf, processor);
   }
  else
   { 
     buf = RmGetObjectAttribute((RmObject) processor, "PUID", TRUE);
     if (buf eq Null(char))
      return(RmErrno = processor->Errno = RmE_NotNetwork, Null(Object));
   }
  if (*((int *) cap) eq 0)
   result = Locate(Null(Object), buf);
  else
   result = NewObject(buf, cap);

  if (newbuf) Free(buf);
  if (result eq Null(Object))
   RmErrno = processor->Errno = RmE_NotFound;
  return(result);
}

/*}}}*/
/*{{{  my_objname */
	/* Why is this in the Server Library !"@$%^&*	*/
char	*my_objname(char *name)
{ char	*tmp = name + strlen(name);
  until( (*tmp eq c_dirchar) || (tmp < name)) tmp--;
  return(tmp + 1);
}
/*}}}*/
/*{{{  Name and user id handling */
/**
*** RmWhoAmI() and RmWhoIs(). These are used to examine the user id's
*** returned by the previous routine, and map onto posix calls -
*** eventually.
**/
int RmWhoAmI(void)
{ Environ	*env		= getenviron();
  Object	*session;
  int		i;
  char		username[NameMax];
  char		*temp;
  struct passwd	*passwd;
  
	/* First, get hold of the environment and the Session object */    
  if (env eq Null(Environ)) return(-1);
  if (env->Objv eq Null(Object *)) return(-1);
  for (i = 0; i <= OV_Session; i++)
   if (env->Objv[i] eq Null(Object))
    return(0);
 
  session = env->Objv[OV_Session];
  if (session eq (Object *) MinInt) return(-1);

	/* Get the last bit of the session name, which should be the username */
  temp = session->Name + strlen(session->Name);
  for ( ; *temp ne '/'; temp--);
  strcpy(username, ++temp);

	/* Strip out .12 at the end, if present */
  for (temp = username + strlen(username); temp > username; temp--)
   if (*temp eq '.')
    { *temp = '\0'; break; }

  passwd =  getpwnam(username);
  if (passwd eq Null(struct passwd))
   return(0);

  return(passwd->pw_uid);
}

const char *RmWhoIs(int uid)
{ struct passwd	*passwd;

  if (uid eq RmO_SystemPool)	return("free pool");
  if (uid eq RmO_System)	return("system");
  if (uid eq RmO_Cleaners)	return("cleaners");
  if (uid eq RmO_Graveyard)	return("graveyard");

  passwd = getpwuid(uid);
  if (passwd eq Null(struct passwd))
   return("<unknown>");
  else
   return(passwd->pw_name);
}
/*}}}*/
/*{{{  Error handling */
/**-----------------------------------------------------------------------------
*** Error manipulation, this is about as bad a place as any to have this routine
**/
const	char	*RmMapErrorToString(int err)
{ switch(err)
   { case RmE_Success		: return("success");
     case RmE_NotProcessor	: return("NotProcessor, argument is not a valid processor");
     case RmE_NotTask		: return("NotTask, argument is not a valid task");
     case RmE_NotNetwork	: return("NotNetwork, argument is not a network");
     case RmE_NotTaskforce	: return("NotTaskforce, argument is not a taskforce");
     case RmE_WrongNetwork	: return("WrongNetwork, this network is inappropriate");
     case RmE_WrongTaskforce	: return("WrongTaskforce, this taskforce is inappropriate");
     case RmE_InUse		: return("InUse, some object is currently in use");
     case RmE_Corruption	: return("Corruption, there appears to be memory corruption");
     case RmE_ReadOnly		: return("ReadOnly, the object cannot be modified");
     case RmE_BadArgument	: return("BadArgument, one of the arguments was invalid");
     case RmE_NoMemory		: return("NoMemory, there is not enough memory on the local processor");
     case RmE_NotFound		: return("NotFound, a search failed");
     case RmE_TooLong		: return("TooLong, a string argument was too large");
     case RmE_NotRootNetwork	: return("NotRootNetwork, the operation cannot be performed on a subnet");
     case RmE_NoAccess		: return("NoAccess, the application has not obtained access");
     case RmE_OldStyle		: return("OldStyle, please recompile your resource map");
     case RmE_BadFile		: return("BadFile, the file contains invalid data");
     case RmE_CommsBreakdown	: return("CommsBreakdown, the application failed to communicate properly with a server");
     case RmE_Skip		: return("Skip, a large container for refuse etc.");
     case RmE_NotRootTaskforce	: return("NotRootTaskforce, the operation cannot be performed on a sub-taskforce");
     case RmE_MissingServer	: return("MissingServer, an essential server could not be found");
     case RmE_PartialSuccess	: return("PartialSuccess, the operation did not succeed completely");
     case RmE_BadLink		: return("BadLink, the specified link does not exist");
     case RmE_BadProcessor	: return("BadProcessor, this processor is currently unusable");
     case RmE_BadChannel	: return("BadChannel, the specified channel does not exist");
     case RmE_YouMustBeJoking	: return("YouMustBeJoking, not currently implemented");
     case RmE_ServerMemory	: return("ServerMemory, a server ran out of memory");
     case RmE_NotPossible	: return("NotPossible, hardware limitation");
     case RmE_NoResource	: return("NoResource, not enough free resources");
     default			: return("<unknown error>");
   }
}
/*}}}*/
#endif

/*}}}*/
#ifdef __HELIOS
/*{{{  Client-server communication */

     static bool RmOpenDefaultServer(RmServer *);
     static void RmPipeGuardian(RmServer);

/*{{{  General description */
/**
*** Communication requirements for the networking software. Bi-directional
*** communication is required for the following:
***
*** Taskforce Manager	<-> Network Server
*** Session Manager	<-> Network Server
*** Application		<-> Network Server (read-only, all clever jobs have)
***			   		   (to be validated by the TFM     )
*** Application		<-> Taskforce Manager
*** Application		<-> Session Manager
*** Special		<-> e.g. one Network Server to another for joinnet
***
*** The data to be transmitted is very variable. The most common data is
*** networks, processors, taskforces, and tasks, which are best transmitted
*** using fifo's or pipes. Other data includes timestamps, task termination
*** details, exception handling for crashed processors and aborted tasks,
*** and so on.
***
*** For future portability and for reliability Unix domain sockets are used
*** for all communication. This is not particularly efficient, for example
*** it involves an extra thread running in both the server and the client,
*** but that is a penalty I am willing to pay for now. For Helios 2.x it
*** may be desirable to switch to virtual channels, as the software is
*** careful not to rely on select() or similar features. The actual
*** communication is via Syslib Reads and Writes, to get timeouts.
***
*** For now, any application including the networking servers typically
*** has upto two streams open : one to the Network Server, read-only,
*** to implement routines such as RmGetNetwork(); and one to the ``parent''. 
*** For the Taskforce Manager and the Session Manager this parent is the
*** Network Server. For everything else this parent is the session's 
*** Taskforce Manager. The library multiplexes all interactions down 
*** these stream. In future it may be necessary/desirable to include the
*** Session Manager and Batch Server as programs using this facility.
***
*** The following routines are used for the communication :
***  1) RmJob	RmNewJob(server);
*** 	 If the connection has not yet been opened, this routine does so. It
***      returns a pointer to a suitable structure, useful for other routines.
***  2) Stream	*RmLockWrite(RmJob);
***	 This routine gets exclusive access to the stream, and returns it
***	 for whatever the caller wants to do, e.g. write a network structure.
***  3) void	RmUnlockWrite(RmJob);
***      This routine releases the exclusive access
***  4) Stream	*RmLockRead(RmConn);
***	 This routine is used only inside the servers, to wait for data
***	 arriving on the stream.
***  5) Stream	*RmSwitchLockRead(RmJob);
***	 This routine releases the exclusive write access as before and
***	 waits on a semaphore for the reply to arrive. It returns the
***	 Stream pointer so that the caller can read all the reply data.
***  6) void	RmUnlockRead(RmJob);
***	 This implies that the read is finished.
***  7) void	RmFinishJob(RmJob);
***	 This routine is called once the operation has finished. It does
***	 not close the Stream to the server, because that stream may be
***	 shared with other jobs.
***  8) int	RmTx(RmJob, RmRequest);
***	 Send a fairly arbitrary request
***  9) int	RmRx(RmJob, RmReply);
***	 Receive a fairly arbitrary reply
*** 10) int RmXch(server, RmRequest *, RmReply)
***	 Create a new job, send a request, receive a reply, finish the job
***
*** The implementation involves link guardian-style processes waiting at
*** both ends of the connection for incoming packets. Every packet starts
*** with a unique identifier, indicating the particular job.
*** There are special synchronisation packets to ensure
*** that both sides are still alive.
***
*** The servers can associate resources with stream connections. For example,
*** if an application has obtained a network of processors and exits without
*** releasing them, the resulting close request generated by the system
*** library will cause the server to reclaim the resources.
***
*** In addition to RmNewJob(), there is a routine RmOpenServer().
*** This opens the stream to the specified server. It is used by special
*** programs such as mergenet, that need to interact with a specific Network
*** Server. The corresponding routine, RmCloseServer(), is used to terminate.
***
*** Consider a vaguely typical piece of code: RmGetNetwork(). This would
*** perform the following steps.
***	1) RmNewJob(&RmNetworkServer);
***	2) RmLockWrite(Job);
***	3) send the request code for RmGetNetwork()
***	4) RmSwitchLockRead(Job)
***		(this suspends the routine until the reply arrives)
***	5) read the reply, success or failure
***	6) RmUnlockRead(Job)
***	7) RmFinishedJob(Job)
**/
/*}}}*/
/*{{{  Basic job handling */

/**
*** RmNewJob(). Given an open RmServer to the Network Server, Session
*** Manager, or Parent, or to some application-defined Server, this routine
*** allocates a new RmConn structure and adds it to the list of known
*** connections using this server. For the three default servers the
*** stream may not have been opened yet. Hence I have to pass the address
*** of the RmServer pointer, not the pointer itself, so that I can compare
*** this with the three basic addresses. This allows me to OpenServer()
*** the right thingy.
**/

int	RmNewJob(RmServer *server, RmJob *job_ptr)
{ RmJob	new_job	= (RmJob)Malloc(sizeof(RmJobStruct));

  if (new_job eq (RmJob)NULL)
   return(RmErrno = RmE_NoMemory);
  
  if (*server eq (RmServer)NULL)
   unless(RmOpenDefaultServer(server))
    { Free(new_job); return(RmErrno = RmE_MissingServer); }
 
  Wait(&((*server)->StructLock));
  new_job->Id		= (*server)->MaxId++;
  new_job->Server	= *server;
  new_job->WriteLocked	= FALSE;
  new_job->ReadLocked	= FALSE;
  new_job->KeepLocked	= FALSE;  
  InitSemaphore(&(new_job->Wait), 0);
#ifdef SYSDEB
  new_job->Node.Next = new_job->Node.Prev = &new_job->Node;
#endif
  AddTail(&((*server)->Jobs), &(new_job->Node));
  Signal(&((*server)->StructLock));
  *job_ptr = new_job;
  return(RmE_Success);
}

/**
*** Undo the effects of a NewJob. Very easy, just remove the
*** thingy from its linked list after suitable locking and free it.
**/
void	RmFinishedJob(RmJob job)
{ RmServer	server = job->Server;

  if (job->WriteLocked)
   { IOdebug("RmLib Job Error : id %d, write still locked", job->Id);
     Signal(&(server->WriteLock));
   }
  if (job->ReadLocked)
   { IOdebug("RmLib Job Error : id %d, read still locked", job->Id);
     Signal(&(server->PipeGuardian));
   }
  Wait(&(server->StructLock));
  Remove(&(job->Node));
  Signal(&(server->StructLock));
  Free(job);  
}

/**
*** The various locking operations. These are all very basic.
**/
Stream	*RmLockWrite(RmJob job)
{ RmServer	server = job->Server;
  Stream	*pipe;

  Wait(&(server->WriteLock));
  job->WriteLocked	= TRUE;
  pipe			= server->Pipe_ctos;
  		
	/* The connection ID is sent automatically, the application does */
	/* not have to worry about it.	*/
  if (Write(pipe, (BYTE *) &(job->Id), sizeof(int), -1) ne sizeof(int))
   return(Null(Stream));

  return(pipe);
}

void	RmUnlockWrite(RmJob job)
{ RmServer	server = job->Server;
  Signal(&(server->WriteLock));
  job->WriteLocked = FALSE;
}

Stream	*RmLockRead(RmJob job)
{ RmServer	server = job->Server;

  Wait(&(job->Wait));		/* Triggered by RmPipeGuardian */
  job->ReadLocked = FALSE;
  return(server->Pipe_stoc);
}

Stream	*RmSwitchLockRead(RmJob job)
{ RmServer	server = job->Server;

  if (!job->WriteLocked)
   IOdebug("RmLib job error : switching lock while write unlocked");
  else   
   { Signal(&(server->WriteLock));
     job->WriteLocked = FALSE;
   }
   
  Wait(&(job->Wait));
  job->ReadLocked = TRUE;
  return(server->Pipe_stoc);
}

void	RmUnlockRead(RmJob job)
{ RmServer	server = job->Server;
  if (!job->ReadLocked)
   IOdebug("RmLib job error, unlocking read lock that has not been set");
  Signal(&(server->PipeGuardian));
  job->ReadLocked = FALSE;
}

/*}}}*/
/*{{{  Connecting to servers */

/**
*** RmOpenServer(). This attempts to set up the basic connection
*** between a client and a networking server. All communication between that
*** client and that server will share this connection.
***
*** An RmServerStruct structure is allocated and initialised. Then an attempt
*** is made to connect to the named socket, which should be something like
*** /.NS_ctos. If successful the name is manipulated to the
*** complement stream, e.g. /.NS_stoc, to get a stream for the
*** other direction. To support fault tolerance timed-out communication is
*** required, hence the Helios Streams are extracted and stored as well.
***
*** Following the connections there is some initial communication to ensure
*** that the sockets really have been set up. If this works a PipeGuardian
*** thread is spawned off to handle messages sent by the server.
**/

int	RmOpenServer(char *sockserv, char *name, Capability *cap, RmServer *server_ptr)
{ RmServer 		server	= (RmServer)NULL;
  struct sockaddr_un	address;
  Stream		*sock;
  int			len;
  Capability		local_cap;
  int			rc = RmE_Success;

  if ((name eq Null(char)) || (server_ptr eq Null(RmServer)))
   return(RmErrno = RmE_MissingServer);

  if (sockserv eq Null(char)) sockserv = "/.socket";

	/* The RmServer structure, allocate and initialise */
  server		= (RmServer)Malloc(sizeof(RmServerStruct));
  if (server eq (RmServer) NULL) return(RmErrno = RmE_NoMemory);

  InitSemaphore(&(server->WriteLock), 1);
  server->MaxId		= 1;
  InitSemaphore(&(server->StructLock), 1);
  InitSemaphore(&(server->PipeGuardian), 0);
  InitList(&(server->Jobs));
  server->Socket_ctos	= Null(Stream);
  server->Socket_stoc	= Null(Stream);
  server->Pipe_ctos	= Null(Stream);
  server->Pipe_stoc	= Null(Stream);

  sock = Socket(sockserv, SOCK_STREAM, 0);
  if (sock eq Null(Stream))
   { rc = RmE_NoMemory; goto fail; }

  address.sun_family	= AF_UNIX;
  strcpy(address.sun_path, name);
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

  if (Connect(sock, (BYTE *) &address, len) < Err_Null)
   { rc = RmE_MissingServer; goto fail; }

  server->Socket_ctos	= sock;
  server->Pipe_ctos	= sock;

	/* hack the _ctos to _stoc	*/
  strcpy(address.sun_path, name);  
  len = strlen(address.sun_path);
  address.sun_path[len - 1]	= 'c';
  address.sun_path[len - 4]	= 's';
  address.sun_family	= AF_UNIX;
  len = sizeof(address.sun_family) + strlen(address.sun_path) + 1;

	/* create another socket for the reverse direction	*/
  sock = Socket(sockserv, SOCK_STREAM, 0);
  if (sock eq Null(Stream))
   { rc = RmE_NoMemory; goto fail; }

  if (Connect(sock, (BYTE *) &address, len) < Err_Null)
   { rc = RmE_NoMemory; goto fail; }

  server->Socket_stoc	= sock;
  server->Pipe_stoc	= sock;

  if (cap eq Null(Capability))
   { memset(&local_cap, 0, sizeof(Capability)); cap = &local_cap; }

  if (Write(server->Pipe_ctos, (BYTE *) cap, sizeof(Capability), 10 * OneSec)
	ne sizeof(Capability))
   { rc = RmE_CommsBreakdown; goto fail; }

  if (Write(server->Pipe_ctos, (BYTE *) &RmProgram, sizeof(int), 10 * OneSec)
	ne sizeof(int))
   { rc = RmE_CommsBreakdown; goto fail; }

  if (Read(server->Pipe_stoc, (BYTE *) &len, sizeof(int), 10 * OneSec)
	ne sizeof(int))
   { rc = RmE_CommsBreakdown; goto fail; }

	/* Fork() off the pipe guardian process, to wait for data */
  unless(Fork(RmPipeGuardian_Stack, RmPipeGuardian, 4, server))
   { rc = RmE_NoMemory; goto fail; }

  *server_ptr = server;
  return(RmE_Success);

fail:
  if (server ne (RmServer) NULL)
   { if (server->Socket_ctos ne Null(Stream))
      Close(server->Socket_ctos);
     if (server->Socket_stoc ne Null(Stream))
      Close(server->Socket_stoc);
     Free(server);
   }

  return(rc);
}

/**
*** RmCloseServer(). This does the inverse operation to RmOpenServer()
*** above. It is legal only iff there are no outstanding connections
*** using this RmServer. Closing the RmServer requires terminating
*** the PipeGuardian process, which is non-trivial. It can be handled
*** by close'ing the Stream originally Open'ed to the server, which causes
*** the server to send a terminate message down the pipe and the terminate
*** message is interpreted by the PipeGuardian. This assumes that the
*** Close always gets through...
**/

int	RmCloseServer(RmServer server)
{ 
  if (server eq (RmServer) NULL) return(RmE_Success);
  Wait(&(server->StructLock));
  unless(EmptyList_(server->Jobs))
   { Signal(&(server->StructLock)); return(RmErrno = RmE_InUse); }

  server->Pipe_stoc	= Null(Stream);
  server->Pipe_ctos	= Null(Stream);
  if (server->Socket_stoc ne Null(Stream))
   { Close(server->Socket_stoc);
     server->Socket_stoc	= Null(Stream);
   }
  if (server->Socket_ctos ne Null(Stream))
   { Close(server->Socket_ctos);
     server->Socket_ctos	= Null(Stream);
   }
  Signal(&(server->StructLock));
  Delay(2 * OneSec);
  return(RmE_Success);
}

/**
*** This routine is invoked automatically when the application attempts
*** to connect to one of the default servers for the first time. These
*** default servers can be the Network Server, the Session Manager, and
*** the ``parent''. For the first two no special authority is required.
*** The parent is trickier. The Network Server does not have a parent.
*** The parent for the Session Manager and for a Taskforce Manager is the
*** Network Server, and a suitable capability should have been installed.
*** The parent for user applications is the Taskforce
*** Manager, and there should be an Object entry in the environment.
**/
static bool	RmOpenDefaultServer(RmServer *server_ptr)
{ char		*name;
  char		*path	= Null(char);
  Capability	*cap;
  char		path_buf[109];	/* size <- <sys/un.h> */
  char		name_buf[NameMax + 8];

  if (server_ptr eq &RmNetworkServer)
   { name = ".NS_ctos";
     if ((RmProgram eq Rm_Session) || (RmProgram eq Rm_TFM))
      cap = &RmLib_Cap;
     else
      cap = Null(Capability);
   }
  elif (server_ptr eq &RmSessionManager)
   { name = ".SM_ctos"; cap = Null(Capability); }
  elif (server_ptr eq &RmParent)
   switch (RmProgram)
    { case Rm_Netserv	: return(FALSE);

      case Rm_Session	:
      case Rm_TFM	: if (RmNetworkServer ne (RmServer) NULL)
			   { RmParent = RmNetworkServer; return(TRUE); }
			  name = ".NS_ctos";
			  cap  = &RmLib_Cap;
			  break;
			  
      case Rm_User	:
      default		:
      			  { Environ	*env = getenviron();
			    char	*temp;
			    int		i;

			    if (env eq Null(Environ))		return(FALSE);
			    if (env->Objv eq Null(Object *))	return(FALSE);
			    for (i = 0; i <= OV_TFM; i++)
			     if (env->Objv[i] eq Null(Object))
			      return(FALSE);
			    if (env->Objv[OV_TFM] ne (Object *) MinInt)
			     cap = &(env->Objv[OV_TFM]->Access);
			    else
			     cap = NULL;
			    path = path_buf; name = name_buf;

				/* /Cluster/01/bart.1/tfm		 */
				/* -> /Cluster/01/.socket, .bart.1._ctos */
			    strcpy(path, env->Objv[OV_TFM]->Name);
			    name[0] = '.';
			    temp = objname(path);
			    *--temp = '\0';
			    temp = objname(path);
			    strcpy(&(name[1]), temp);
			    strcpy(temp, ".socket");
			    strcat(name, "_ctos");
				/* -> /Cluster/01/.socket, .bart.1_ctos	*/
			  }
			  break;
    }
  else
   return(FALSE);

  if (RmOpenServer(path, name, cap, server_ptr) eq RmE_Success)
    return(TRUE);
  else
   return(FALSE);
}

/*}}}*/
/*{{{  The Pipe Guardian */

/**
*** The PipeGuardian(). This is a little process Fork()'ed off when Opening
*** a Stream to a Server. It waits on the pipe for incoming packets. These
*** may be messages aimed at the PipeGuardian itself, such as synchronisation,
*** termination, or exceptions. Alternatively they may be aimed at some
*** other process that has an open connection using this stream. A search
*** is made to find the connection, and that connection is Signalled to
*** wake up the process. The pipe guardian now releases the pipe stream to
*** that process, and waits for that process to read all the data it is
*** expecting.
**/
static	int	RmMatchJob(RmJob job, int id);
static  int	RmAbortJob(RmJob job);
static	void	RmDoSynch(RmServer);
static	void	RmDoDup2(RmServer);

static	void RmPipeGuardian(RmServer server)
{ Stream *pipe	= server->Pipe_stoc;
  int	 id;

#ifndef __TRAN
  SetPriority(HighServerPri);
#endif

  forever
   { word result;

     result = FullRead(pipe, (BYTE *) &id, sizeof(int), 30 * OneSec);

     if (result eq 4)   /* Success */
      { 
        if (id & RmR_Private)
         { 
	   if ((id & ~RmR_Private) eq RmR_Terminate)
            break;
           elif ((id & ~RmR_Private) eq RmR_Synch)
            continue;
	   elif ((id & ~RmR_Private) eq RmR_Dup2)
	    RmDoDup2(server);
           else
            { int	size;
	      BYTE	*data;
	      result = FullRead(pipe, (BYTE *) &size, sizeof(int), 30 * OneSec);
	      if (result ne 4) break;
	      data = (BYTE *)Malloc(size);
	      if (data eq NULL) break;
	      result = FullRead(pipe, data, size, 30 * OneSec);
	      if (result != size) break;
	      if (RmExceptionHandler ne (VoidFnPtr) NULL)
	       { unless(Fork(RmExceptionStack, RmExceptionHandler,
			12, id, size, data))
		  Free(data);
	       }
	      else
	       Free(data);
	    }
         }
        else
         { RmJob	job;
           Wait(&(server->StructLock));
	   job = (RmJob)
	      SearchList(&(server->Jobs), (WordFnPtr) &RmMatchJob, id);
	   Signal(&(server->StructLock));
	   if (job eq (RmJob) NULL)
	    break;
	   Signal(&(job->Wait));
	   Wait(&(server->PipeGuardian));
	 } 
      }
     elif (result eq 0)		/* timeout */
      { if (server->Pipe_ctos eq Null(Stream)) break;
      	(void) Fork(RmDoSynch_Stack, &RmDoSynch, 4, server);
      }
     else
      { word rc2 = Result2(pipe);
	if ((rc2 eq ReadRc_EOF) || ((rc2 & EC_Mask) > EC_Warn))
	 break;
      }
   }

 (void) WalkList(&(server->Jobs), (WordFnPtr) &RmAbortJob);
  RmCloseServer(server);
  Free(server);
}

static	int	RmMatchJob(RmJob job, int id)
{ if (job->Id eq id)
   return(1);
  else
   return(0);
}

static int	RmAbortJob(RmJob job)
{ Signal(&(job->Wait));
  return(0);
}

static void RmDoSynch(RmServer server)
{ int	rc = RmR_Private + RmR_Synch;

  Wait(&(server->WriteLock));
  (void) Write(server->Pipe_ctos, (BYTE *) &rc, sizeof(int), 10 * OneSec);
  Signal(&(server->WriteLock));
}

/**
*** This code is intended to support parallel libraries such as the
*** farm library. The TFM has received a request to update a taskforce
*** or convert a task to a taskforce. As part of handling this request
*** it has to make this task open additional channels. Any requests that
*** may require this should set the RmC_KeepLocked flag to ensure that
*** no other requests are sent to the TFM at the same time.
**/
static void RmDoDup2(RmServer server)
{ Dup2Details	 details;
  Stream	*stream;
  bool		 success	= FALSE;
  int		 fd;

	/* Receive the file descriptor that this task should		*/
	/* open, followed by the name of the stream. The latter		*/
	/* incorporates a capability.					*/
  Read(server->Pipe_stoc, (BYTE *) &details, sizeof(Dup2Details), -1);
  stream = Open(cdobj(), details.Name, O_ReadWrite);
  if (stream eq NULL) goto done;

	/* Now turn the Stream into a file descriptor. If necessary use	*/
	/* dup2() to ensure that the right file descriptor is used.	*/
  fd = sopen(stream);
  if (fd < 0)
   { Close(stream); goto done; }
  if (fd ne details.FileDescriptor)
   { int res = dup2(fd, details.FileDescriptor);
     close(fd);
     if (res eq details.FileDescriptor)
      success = TRUE;
    }
   else
    success = TRUE;

done:
  Write(server->Pipe_ctos, (BYTE *) &success, sizeof(bool), -1);
}

/*}}}*/
/*{{{  RmTx, RmRx, RmXch */

/**
*** General Rmlib->Server Communication routines
**/
int RmTx(RmJob job, RmRequest *request)
{ Stream	*pipe = RmLockWrite(job);
  int		rc;

#if 0
IOdebug("RmTx: FnRc %x, Network %x, Taskforce %x, Processor %x, Task %x, size %d",
		request->FnRc, request->Network, request->Taskforce,
		request->Processor, request->Task,
		request->VariableSize);
#endif

  if (request->FnRc & RmC_KeepLocked)
   { job->KeepLocked = TRUE; request->FnRc &= ~RmC_KeepLocked; }

  if (Write(pipe, (BYTE *) request, sizeof(RmRequest), -1) ne sizeof(RmRequest))
   { rc = RmE_CommsBreakdown; goto fail; }

  if ((request->Network ne (RmNetwork) NULL) ||
      (request->Taskforce ne (RmTaskforce) NULL))
   if ((rc = RmWriteStream(pipe, request->Network, request->Taskforce,
			request->Filter)) ne RmE_Success)
    goto fail;

  if (request->Processor ne (RmProcessor) NULL)
   if ((rc = RmWriteProcessor(pipe, request->Processor, FALSE)) ne RmE_Success)
    goto fail;

  if (request->Task ne (RmTask) NULL)
   if ((rc = RmWriteTask(pipe, request->Task, FALSE)) ne RmE_Success)
    goto fail;

  if (request->VariableSize > 0)
   if (Write(pipe, request->VariableData, request->VariableSize, -1)
	ne request->VariableSize)
    { rc = RmE_CommsBreakdown; goto fail; }

  return(RmE_Success);

fail:
#ifdef SYSDEB
  IOdebug("RmTx: failed to send request down pipe %s", pipe->Name);
#endif
  RmUnlockWrite(job);
  return(rc);
}

int RmRx(RmJob job, RmReply *reply)
{ Stream	*pipe;
  int		rc;
  BYTE		*vardata = reply->VariableData;

  if (job->KeepLocked)
   pipe = RmLockRead(job);
  else
   pipe = RmSwitchLockRead(job);

  if (FullRead(pipe, (BYTE *) reply, sizeof(RmReply), -1) ne sizeof(RmReply))
   { rc = RmE_CommsBreakdown; goto fail; }

#if 0
IOdebug("RmRx: FnRc %x, Network %x, Taskforce %x, Processor %x, Task %x, size %d",
		reply->FnRc, reply->Network, reply->Taskforce,
		reply->Processor, reply->Task, reply->VariableSize);
#endif

  if ((reply->Network ne (RmNetwork) NULL) ||
      (reply->Taskforce ne (RmTaskforce) NULL))
   if ((rc = RmReadStream(pipe, &(reply->Network), &(reply->Taskforce)))
       ne RmE_Success)
    goto fail;

  if (reply->Processor ne (RmProcessor) NULL)
   if ((rc = RmReadProcessor(pipe, &(reply->Processor), FALSE))
       ne RmE_Success)
    goto fail;

  if (reply->Task ne (RmTask) NULL)
   if ((rc = RmReadTask(pipe, &(reply->Task), FALSE))
       ne RmE_Success)
    goto fail;

  if (reply->VariableSize > 0)
   { if (vardata eq NULL)
      { vardata = reply->VariableData = (char *)Malloc(reply->VariableSize);
        if (vardata eq NULL)
         { rc = RmE_NoMemory; goto fail; }
      }
     if (FullRead(pipe, vardata, reply->VariableSize, -1) ne reply->VariableSize)
      { rc = RmE_CommsBreakdown; goto fail; }
   }

  if (job->KeepLocked) RmUnlockWrite(job);
  RmUnlockRead(job);
  return(reply->FnRc);

fail:
#ifdef SYSDEB
  IOdebug("RmRx: failed to receive reply from pipe %s", pipe->Name);
#endif
  if (job->KeepLocked) RmUnlockWrite(job);
  RmUnlockRead(job);
  return(rc);
}

int RmXch(RmServer *Server, RmRequest *request, RmReply *reply)
{ RmJob	job;
  int	rc = RmNewJob(Server, &job);

  if (rc ne RmE_Success) return(rc);
  rc = RmTx(job, request);
  if (rc eq RmE_Success)
   rc = RmRx(job, reply);
  RmFinishedJob(job);
  return(rc);
}

/*}}}*/

/*}}}*/
#endif
/*{{{  Stream I/O */

/**-----------------------------------------------------------------------------
*** Stream I/O. The ``user'' routines are RmRead() and RmWrite(),
*** which take filenames. The routines attempt to open the actual files,
*** and call lower-level routines RmReadStream() and RmWriteStream().
*** These are implemented using yet lower level routines. All of these
*** are accessible but not documented.
**/
/*{{{  byte swapping, if needed */
#ifndef HOSTISBIGENDIAN
#define SWAP(a)
#else
int swap(byte *x)
{ union { int x; byte y[4]; } a;

  a.y[3] = *x++;
  a.y[2] = *x++;
  a.y[1] = *x++;
  a.y[0] = *x;
  return(a.x);
}

#define SWAP(a) (*(int *)(&(a))) = swap((byte *) &(a));
#endif
/*}}}*/
/*{{{  RmRead(), RmWrite() */
int RmRead(char *filename, RmNetwork *Network, RmTaskforce *Taskforce)
{ Stream 	*stream;
  Object 	*file;
  int	 	rc;
  Environ	*env = getenviron();
  
  if (filename eq Null(char))
   return(RmErrno = RmE_BadArgument);
  if ((Network eq Null(RmNetwork)) && (Taskforce eq Null(RmTaskforce)))
   return(RmErrno = RmE_BadArgument);
  file = Locate(env->Objv[0], filename);
  if (file eq Null(Object))
   { word rc = Result2(env->Objv[0]);
     if ((rc & EG_Mask) eq EG_Protected)
      return(RmErrno = RmE_NoAccess);
     elif ((rc & EG_Mask) eq EG_NoMemory)
      return(RmErrno = RmE_NoMemory);
     else
      return(RmErrno = RmE_NotFound);
   }      
  stream = Open(file, Null(char), O_ReadOnly);
  (void) Close(file);
  if (stream eq Null(Stream))
   return(RmErrno = RmE_NoMemory);
  rc = RmReadStream(stream, Network, Taskforce);
  (void) Close(stream);
  return(rc);  
}

int RmWrite(char *filename, RmNetwork Network, RmTaskforce Taskforce)
{ Stream	*stream;
  int		rc;
  RmFilterStruct filter;
  Environ	*env = getenviron();
    
  filter.Network	= NULL;
  filter.Processor	= NULL;  
  filter.Taskforce	= NULL;
  filter.Task		= NULL;
  filter.SendHardware	= TRUE;
  
  if (filename eq Null(char))
   return(RmErrno = RmE_BadArgument);
  if ((Network eq (RmNetwork)NULL) && (Taskforce eq (RmTaskforce)NULL))
   return(RmErrno = RmE_BadArgument);
  if (Network ne (RmNetwork) NULL)
   if (Network->DirNode.Type ne Type_Network)
    return(RmErrno = RmE_NotNetwork);
  if (Taskforce ne (RmTaskforce) NULL)
   if (Taskforce->DirNode.Type ne Type_Taskforce)
    return(RmErrno = RmE_NotTaskforce);    

  stream = Open(env->Objv[0], filename, O_WriteOnly + O_Create + O_Truncate);
  if (stream eq Null(Stream))
   { word res = Result2(env->Objv[0]);
     if ((res & EG_Mask) eq EG_NoMemory)
      return(RmErrno = RmE_NoMemory);
     elif ((res & EG_Mask) eq EG_Protected)
      return(RmErrno = RmE_NoAccess);
     else
      return(RmErrno = RmE_BadFile);
   }
  rc = RmWriteStream(stream, Network, Taskforce, &filter);
  (void) Close(stream);
  return(rc);
}
/*}}}*/
/*{{{  RmReadStream(), RmWriteStream() */
/**
*** The next two routines, RmReadStream() and RmWriteStream(), are called
*** mostly from inside Helios software. Hence there is no checking.
***
*** The first three words of a saved file contain the following:
*** a version number, a bool to indicate whether or not there is a network,
*** and a bool to indicate whether or not there is a taskforce. These
*** 12 bytes are read in one go. If this test is passed calls are made to
*** yet lower level routines to do the real work.
**/
static	int	UpdateMapping(RmTask, ...);

int RmReadStream(Stream *stream, RmNetwork *Network, RmTaskforce *Taskforce)
{ int		junk[3];
  RmNetwork	LocalNetwork;
  RmTaskforce	LocalTaskforce;
  int		rc;

  if (FullRead(stream, (BYTE *) junk, 3 * sizeof(int), -1) ne (3 * sizeof(int)))
   return(RmErrno = RmE_BadFile);
  SWAP(junk[0]) SWAP(junk[1]) SWAP(junk[2])

  if (junk[0] eq 0x06)	/* old-style resource map */
   return(RmErrno = RmE_OldStyle);
	/* 7 was used in Helios 1.2.2 for resource map binaries */
  if ((junk[0] ne 0x07)	&& (junk[0] ne RmLib_Magic))
   return(RmErrno = RmE_BadFile);
   
  if (junk[1])		/* There is a Network in the file */
   { if (Network eq Null(RmNetwork))	/* but it should be discarded */
      { rc = RmReadNetwork(stream, &LocalNetwork, FALSE);
        if (rc eq RmE_Success)
         (void) RmFreeNetwork(LocalNetwork);
        else
         return(rc);
      }
     else
      { rc = RmReadNetwork(stream, Network, FALSE);
        if (rc ne RmE_Success)
         return(rc);
      }
   }
  else	/* No network, but the user expected one */
   { if (Network ne Null(RmNetwork))
      *Network = (RmNetwork)NULL;
   }
      
  if (junk[2])	/* Similarly for the Taskforce */
   { if (Taskforce eq Null(RmTaskforce))
      { rc = RmReadTaskforce(stream, &LocalTaskforce, FALSE);
        if (rc eq RmE_Success)
         (void) RmFreeTaskforce(LocalTaskforce);
        else
         return(rc);
      }
     else
      { rc = RmReadTaskforce(stream, Taskforce, FALSE);
        if (rc ne RmE_Success) return(rc);
        (void) RmApplyTasks(*Taskforce, &UpdateMapping, *Network);
      }
   }
  else
   { if (Taskforce ne Null(RmTaskforce))
      *Taskforce = (RmTaskforce) NULL;
   }
   
  return(RmE_Success);
}

int RmWriteStream(Stream *stream, RmNetwork Network, RmTaskforce Taskforce,
		RmFilter filter)
{ int	junk[3];
  int	rc;
  
  junk[0] = (int) RmLib_Magic;
  junk[1] = (Network ne (RmNetwork) NULL);
  junk[2] = (Taskforce ne (RmTaskforce) NULL);

  SWAP(junk[0]) SWAP(junk[1]) SWAP(junk[2])

  if (Write(stream, (BYTE *) junk, 3 * sizeof(int), -1) ne (3 * sizeof(int)))
   return(RmErrno = RmE_BadFile);
   
  if (Network ne (RmNetwork) NULL)
   if ((rc = RmWriteNetwork(stream, Network, filter)) ne RmE_Success)
    return(rc);

  if (Taskforce ne (RmTaskforce) NULL)
   if ((rc = RmWriteTaskforce(stream, Taskforce, filter)) ne RmE_Success)
    return(rc);
    
  return(RmE_Success);
}

/**
*** Update the task->processor mapping. If a taskforce is part of the
*** file then every component task has to be updated. If there is no
*** network then the task cannot be mapped. Otherwise the task must be
*** mapped onto the right processor.
**/
static	int	UpdateMapping(RmTask task, ...)
{ va_list	args;
  RmNetwork	network;
  RmProcessor	processor;

  va_start(args, task);
  network = va_arg(args, RmNetwork);
  va_end(args);
  
  if (network eq (RmNetwork) NULL) return(0);
   
  if (task->MappedTo ne RmL_NoUid)
   { processor = RmFindProcessor(network, task->MappedTo);
     task->MappedTo = RmL_NoUid;
     if (processor ne (RmProcessor) NULL)
      if (RmMapTask(processor, task) ne RmE_Success)
       { IOdebug("RmLib: internal error, failed to map task onto processor");
       }
   }
  return(0);
}
/*}}}*/
/*{{{  RmReadNetwork() */

/**
*** Every object in the file is preceeded by a type, in this  case
*** Type_Network or Type_File. This allows RmReadProcessor below to
*** read a single word, check the type, and if necessary call 
*** RmReadNetwork() recursively. Unfortunately the type has now been
*** read, and I do not want to Seek() back because that will fail on
*** pipes. Hence I use a third argument to say whether or not this
*** routine has been called recursively.
**/
static	int	RmReadRoot(RmSet);
static	int	RmReadResets(Stream *, RmNetwork, int);
static  void	RmTidyNetworkRead(RmNetwork);

int RmReadNetwork(Stream *stream, RmNetwork *NetworkPtr, bool recursive)
{ RmNetwork	Network;
  RmSet		root;
  int		rc;
  int		i;
  int		resets;

  if (!recursive)
   { int type;
     if (FullRead(stream, (BYTE *) &type, sizeof(int), -1) ne sizeof(int))
      return(RmErrno = RmE_BadFile);
     SWAP(type)
     if (type ne Type_Network)
      return(RmErrno = RmE_NotNetwork);
   }

  Network = *NetworkPtr = (RmNetwork) Malloc(sizeof(RmNetworkStruct));
  if (Network eq (RmNetwork) NULL)
   return(RmErrno = RmE_NoMemory);
  if (FullRead(stream, (BYTE *) Network, sizeof(RmNetworkStruct), -1) ne
  		sizeof(RmNetworkStruct))
   { Free(Network); *NetworkPtr = NULL; return(RmErrno = RmE_BadFile); }

  SWAP(Network->DirNode.Type)
  SWAP(Network->DirNode.Flags)
  SWAP(Network->DirNode.Key)
  SWAP(Network->DirNode.Dates.Creation)
  SWAP(Network->DirNode.Dates.Access)
  SWAP(Network->DirNode.Dates.Modified)
  SWAP(Network->DirNode.Account)
  SWAP(Network->DirNode.Nentries)
  SWAP(Network->Private)
  SWAP(Network->NoTables)
  SWAP(Network->Private2)
  SWAP(Network->Errno)

  resets			= (int) Network->Hardware.Head;
  SWAP(resets)
  InitList(&(Network->Hardware));
  root				= Network->Root;
  Network->Root			= NULL;
  Network->DirNode.Node.Next	= NULL;
  Network->DirNode.Node.Prev	= NULL;
  Network->DirNode.Parent	= NULL;
  InitSemaphore(&(Network->DirNode.Lock), 1);
  InitList(&(Network->DirNode.Entries));

  if (resets > 0)
   if ((rc = RmReadResets(stream, Network, resets)) ne RmE_Success)
    goto error;
    
	/* When writing the root, the pointer is set to NULL */
	/* There is also some special work to be done to sort out the Uid */
	/* tables. */
  if (root eq (RmSet) NULL)
   { Network->Root = (RmSet) Network;
     if ((rc = RmReadRoot((RmSet) Network)) ne RmE_Success)
      { Free(Network); *NetworkPtr = NULL; return(rc); }
   }

  for (i = 0; i < Network->DirNode.Nentries; i++)
   { int type;
     if (FullRead(stream, (BYTE *) &type, sizeof(int), -1) ne sizeof(int))
      { rc = RmE_BadFile; Network->DirNode.Nentries = i; goto error; }
     SWAP(type)
     if (type eq Type_Network)
      { RmNetwork	Subnet;
        if ((rc = RmReadNetwork(stream, &Subnet, TRUE)) ne RmE_Success)
         { rc = RmE_BadFile; Network->DirNode.Nentries = i; goto error; }
        AddTail(&(Network->DirNode.Entries), &(Subnet->DirNode.Node));
        Subnet->DirNode.Parent = &(Network->DirNode);
      }
     elif (type eq Type_Processor)
      { RmProcessor	Processor;
        if ((rc = RmReadProcessor(stream, &Processor, TRUE)) ne RmE_Success)
         { Network->DirNode.Nentries = i; goto error; }
        AddTail(&(Network->DirNode.Entries), &(Processor->ObjNode.Node));
        Processor->ObjNode.Parent = &(Network->DirNode);
      }
     else
      { rc = RmE_BadFile; Network->DirNode.Nentries = i; goto error; }
   }

  if (Network->Root eq (RmSet) Network)
   RmTidyNetworkRead(Network);
   
  return(RmE_Success);
  
error:
  RmFreeNetwork(Network);
  *NetworkPtr = NULL;
  RmErrno = rc;
  return(rc);
}

/**
*** For now I do not bother to ship the UID tables with the root structures,
*** because they do not appear to be needed. Hence all this routine does is
*** allocate space for the tables.
**/
static int RmReadRoot(RmSet Set)
{ int 	NoTables	= Set->NoTables;
  Set->NoTables		= 0;
  Set->Tables		= Null(RmUidTableEntry *);
  
  while(NoTables-- > 0)
   unless(RmExtendFreeQ(Set))
    return(RmErrno = RmE_NoMemory);
  return(RmE_Success);
}

/**
*** The file contains count RmHardwareFacility structures, each followed by
*** a suitable table. The table contains Uid's rather than pointers, a
*** problem that is resolved later. The AddHardwareFacility() routine
***is used, which makes a duplicate of the RmHardwareFacility structure.
**/

static int	RmReadResets(Stream *stream, RmNetwork Network, int count)
{ RmHardwareFacility	Reset;
  int			*table;
  int			rc;
      
  while(count-- > 0)
   { if (FullRead(stream, (BYTE *) &Reset, sizeof(RmHardwareFacility), -1) ne
   	 sizeof(RmHardwareFacility))
      return(RmErrno = RmE_BadFile);
     SWAP(Reset.Type)
     SWAP(Reset.NumberProcessors)
     SWAP(Reset.Essential)
     table = (int *) Malloc(sizeof(int) * (word) Reset.NumberProcessors);
     if (table eq Null(int))
      return(RmErrno = RmE_NoMemory);
     if (FullRead(stream, (BYTE *) table, sizeof(int) * Reset.NumberProcessors, -1)
              ne (sizeof(int) * Reset.NumberProcessors))
      { Free(table); return(RmErrno = RmE_BadFile); }
     Reset.Processors = (RmProcessor *) table;
#ifdef HOSTISBIGENDIAN
     { int i;
       for (i = 0; i < Reset.NumberProcessors; i++)
        SWAP(table[i])
     }
#endif
     rc = RmAddHardwareFacility(Network, &Reset);
     Free(table);
     if (rc ne RmE_Success)
      return(RmErrno = rc);
   }
  return(RmE_Success);
}

/**
*** Once the whole network has been read in, some tidying up is required.
*** In particular, all objects need to have the Root set; all processors
*** should install their pointers in the Uid tables; and all reset and
*** configuration drivers should have their Uid's converted to pointers.
*** These three steps are done by two different RmApplyNetwork()'s
**/

static	int	RmTidyNetworkAux1(RmProcessor Processor, ...);
static	int	RmTidyNetworkAux2(RmProcessor Processor, ...);
static	int	RmTidyHardwareFacility(RmHardwareFacility *Reset, ...);

static	void	RmTidyNetworkRead(RmNetwork Network)
{ (void) RmApplyNetwork(Network, &RmTidyNetworkAux1, Network);
  (void) RmApplyNetwork(Network, &RmTidyNetworkAux2, Network);
  (void) RmApplyHardwareFacilities(Network, &RmTidyHardwareFacility, Network);
}

/**
*** If the object is a network, simply fill in the root and recurse.
***
*** Otherwise fill in the root and obtain the Uid.
**/
static	int	RmTidyNetworkAux1(RmProcessor Processor, ...)
{ va_list	args;
  RmNetwork	Root;

  va_start(args, Processor);
  Root	= va_arg(args, RmNetwork);
  va_end(args);
  if (RmIsNetwork(Processor))
   { RmNetwork	Network = (RmNetwork) Processor;
     Network->Root	= (RmSet) Root;  
     (void) RmApplyNetwork(Network, &RmTidyNetworkAux1, Root);
   }
  else
   { Processor->Root	= Root;
     (void) RmObtainUid((RmSet) Root, (RmObject) Processor);
   }
  return(0);
}

/**
*** This routine takes care of reset facilities and configure drivers held
*** in sub-networks.
**/
static	int	RmTidyNetworkAux2(RmProcessor Processor, ...)
{ va_list	args;
  RmNetwork	Root;
  
  va_start(args, Processor);
  Root	= va_arg(args, RmNetwork);
  va_end(args);
  
  if (RmIsNetwork(Processor))
   { RmNetwork Network	= (RmNetwork) Processor;
     (void) RmApplyHardwareFacilities(Network, &RmTidyHardwareFacility, Root);
     (void) RmApplyNetwork(Network, &RmTidyNetworkAux2, Root);
   }
  return(0);
}

/**
*** This routine take all the reset facilities and configure drivers
*** held in a network, and map the Uid's currently stored in the tables
*** onto the real pointers.
**/
static int	RmTidyHardwareFacility(RmHardwareFacility *Reset, ...)
{ va_list	args;
  RmNetwork	Root;
  RmProcessor	*Table;
  int		i;
  
  va_start(args, Reset);
  Root	= va_arg(args, RmNetwork);
  va_end(args);
  
  Table = Reset->Processors;
  for (i = 0; i < Reset->NumberProcessors; i++)
    Table[i] = RmFindProcessor(Root, (int) Table[i]);
  return(0);
}

/*}}}*/
/*{{{  RmWriteNetwork() */

/**
*** Clearly RmWriteNetwork() has to be kept closely in step with
*** RmReadNetwork(). The possibility exists of having a filter function
*** to do things to the RmNetwork structure. Hence there is an auxiliary
*** routine RmWriteRmNetworkStruct() which copies the structure onto the
*** stack, applies some filtering, and does the actual write.
**/

static	int	RmWriteNetworkStruct(Stream *stream, RmNetwork Network,
			RmFilter filter);
static	int	RmWriteResets(Stream *Stream, RmNetwork Network);
static	int	RmWriteNetworkContents(RmProcessor Processor, ...);

/**
*** Step 1 : write Type_Network
***      2 : write the RmNetwork struct itself, suitably filtered
***      3 : write the reset drivers if any
***      4 : ditto for the contents
***      5 : write the contents
**/
int RmWriteNetwork(Stream *stream, RmNetwork Network, RmFilter filter)
{ int	rc;

  if ((rc = RmWriteNetworkStruct(stream, Network, filter)) eq RmE_Success)
   { 
     if (filter ne (RmFilter) NULL)
      if (filter->SendHardware)   
       if ((rc = RmWriteResets(stream, Network)) ne RmE_Success)
        return(RmErrno = rc);
     return(RmSearchNetwork(Network, &RmWriteNetworkContents, stream, filter));
   }
  elif (rc eq RmE_Skip)
   return(RmE_Success);
  else
   return(RmErrno = rc);
}

/**
*** RmWriteNetworkStruct(). This makes a copy of the RmNetwork structure
*** onto the stack, performs some filtering, and writes it out.
*** Automatic filtering includes zapping the Root pointer if this is
*** the root, for recognition purposes, and figuring out the number of
*** reset facilities and configuration drivers.
**/
static int	RmWriteNetworkStruct(Stream *stream, RmNetwork Network,
					RmFilter filter)
{ RmNetworkStruct	copy;
  int			filter_result = RmE_Success;
  word			type = Type_Network;
      
  memcpy((void *) &copy, (void *) Network, sizeof(RmNetworkStruct));
  if (Network->Root eq (RmSet) Network)
   copy.Root = (RmSet) NULL;
  
  copy.Hardware.Head = 0;
 
  if (filter ne (RmFilter) NULL)
   { if (filter->SendHardware) 
      { int resets;
	Node *Reset = Network->Hardware.Head;
	for (resets = 0; !EndOfList_(Reset); resets++)
	 Reset = Reset->Next;
	SWAP(resets)
        copy.Hardware.Head = (Node *) resets;
      }
   }
  
  if (filter ne (RmFilter) NULL)
   if (filter->Network ne NULL)
    filter_result = (*(filter->Network))(Network, &copy);

  if (filter_result ne RmE_Success)
   return(filter_result);

  SWAP(type)
  SWAP(copy.DirNode.Type)
  SWAP(copy.DirNode.Flags)
  SWAP(copy.DirNode.Key)
  SWAP(copy.DirNode.Dates.Creation)
  SWAP(copy.DirNode.Dates.Access)
  SWAP(copy.DirNode.Dates.Modified)
  SWAP(copy.DirNode.Account)
  SWAP(copy.DirNode.Nentries)
  SWAP(copy.Private)
  SWAP(copy.NoTables)
  SWAP(copy.Private2)
  SWAP(copy.Errno)

  if (Write(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);

  if (Write(stream, (BYTE *) &copy, sizeof(RmNetworkStruct), -1) ne
  		sizeof(RmNetworkStruct))
      return(RmErrno = RmE_BadFile);

  return(RmE_Success);
}

/**
*** RmWriteNetworkContents() is applied to the various entries in a network.
*** These may of course be subnets or processors.
**/
static int	RmWriteNetworkContents(RmProcessor Processor, ...)
{ va_list	args;
  Stream	*stream;
  RmFilter	filter;
    
  va_start(args, Processor);
  stream = va_arg(args, Stream *);
  filter = va_arg(args, RmFilter);
  va_end(args);
  
  if (RmIsNetwork(Processor))
   return(RmWriteNetwork(stream, (RmNetwork) Processor, filter));
  else
   return(RmWriteProcessor(stream, Processor, filter));
}

/**
*** Reset and configuration drivers. Transmitting these involves two stages.
*** First the HardwareFacility structure itself is transmitted.
*** Then a new table is allocated for all the Processors, and the Uid's
*** are put into this table. It is this table, not the table of pointers,
*** that gets transmitted.
**/
static int	RmWriteHardware(RmHardwareFacility *reset, ...)
{ RmUid		*table;
  va_list	args;
  Stream	*stream;
  int		i;
  int		rc = RmE_Success;
  word		x;

  va_start(args, reset);
  stream = va_arg(args, Stream *);
  va_end(args);
  
  if (reset->NumberProcessors eq 0) return(RmE_Success);
  table = (RmUid *) Malloc(sizeof(int) * (word) reset->NumberProcessors);
  if (table eq Null(RmUid)) return(RmErrno = RmE_NoMemory);
  for (i = 0; i < reset->NumberProcessors; i++)
  {
    table[i] = reset->Processors[i]->Uid;
    SWAP(table[i])
  }

  SWAP(reset->Type)
  SWAP(reset->NumberProcessors)
  SWAP(reset->Essential)

  x = Write(stream, (BYTE *) reset, sizeof(RmHardwareFacility), -1);
  SWAP(reset->Type)
  SWAP(reset->NumberProcessors)
  SWAP(reset->Essential)

  if (x ne sizeof(RmHardwareFacility))
   { rc = RmE_BadFile; goto done; }

  if (Write(stream, (BYTE *) table, sizeof(int) * (word) reset->NumberProcessors, -1)
  	ne (sizeof(int) * (word) reset->NumberProcessors))
   rc = RmE_BadFile;

done:
  Free(table);
  if (rc ne RmE_Success)
   RmErrno = rc;
  return(rc);
}

static int	RmWriteResets(Stream *stream, RmNetwork Network)
{ return( RmSearchHardwareFacilities(Network, &RmWriteHardware, stream));
}

/*}}}*/
/*{{{  RmReadProcessor() */

/**
*** A Processor consists of the following bits.
***
*** 1) an RmProcessorStruct
*** 2) possibly the OtherLinks
*** 3) possibly the Attributes and private attributes
***
*** The third argument to RmReadProcessor() indicates whether or not the
*** initial word giving the object type has been read yet, or not.
*** No conversion is needed for any of the connections or things like that,
*** because of the Uid mechanism.
**/

int RmReadProcessor(Stream *stream, RmProcessor *ProcessorPtr, bool recurse)
{ RmProcessor Processor = (RmProcessor)Malloc(sizeof(RmProcessorStruct));
  int rc = RmE_Success;

  if (Processor eq (RmProcessor) NULL) return(RmErrno = RmE_NoMemory);
  
  if (!recurse)
   { WORD type;
     if (FullRead(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
      { rc = RmE_BadFile; goto done; }
     SWAP(type)
     if (type ne Type_Processor)
      { rc = RmE_BadFile; goto done; }
   }
   
  if (FullRead(stream, (BYTE *) Processor, sizeof(RmProcessorStruct), -1) ne
      sizeof(RmProcessorStruct))
   { rc = RmE_BadFile; goto done; }

  SWAP(Processor->ObjNode.Type)
  SWAP(Processor->ObjNode.Flags)
  SWAP(Processor->ObjNode.Key)
  SWAP(Processor->ObjNode.Dates.Creation)
  SWAP(Processor->ObjNode.Dates.Access)
  SWAP(Processor->ObjNode.Dates.Modified)
  SWAP(Processor->ObjNode.Account)
  SWAP(Processor->ObjNode.Size)
  SWAP(Processor->Private)
  SWAP(Processor->Uid)
  SWAP(Processor->Connections)
  SWAP(Processor->AttribSize)
  SWAP(Processor->AttribFree)
  SWAP(Processor->PAttribSize)
  SWAP(Processor->PAttribFree)
  SWAP(Processor->MemorySize)
  SWAP(Processor->Type)
  SWAP(Processor->MappedTo)
  SWAP(Processor->Private2)
  SWAP(Processor->Errno)
  SWAP(Processor->SessionId)
  SWAP(Processor->ApplicationId)

  InitSemaphore(&(Processor->ObjNode.Lock), 1);   
  Processor->OtherLinks = Null(RmLink);
  InitList(&(Processor->MappedTasks));
  Processor->Root		= NULL; 
  Processor->AttribData		= NULL;
  Processor->PAttribData	= NULL;
  Processor->ObjNode.Node.Next	= NULL;
  Processor->ObjNode.Node.Prev	= NULL;
  Processor->ObjNode.Parent	= NULL;
  InitList(&(Processor->ObjNode.Contents));

	/* After the 1.2 release I decided to move the Purpose field.	*/  
	/* This hack maintains binary compatibility of resource maps.	*/
  if ((Processor->ObjNode.Size & 0x0F) ne 0)
   { switch(Processor->ObjNode.Size & 0x0F)
      { case	1 : Processor->Purpose = RmP_Helios; break;
        case	2 : Processor->Purpose = RmP_System | RmP_Helios; break;
        case	3 : Processor->Purpose = RmP_IO; break;
        case	4 : Processor->Purpose = RmP_System | RmP_Native; break;
      }
     Processor->ObjNode.Size &= ~0x0F;
   }
   
  if (Processor->Connections > 4)
   { int size = (Processor->Connections - 4) * sizeof(RmLink);
     Processor->OtherLinks = (RmLink *) Malloc(size);
     if (Processor->OtherLinks eq Null(RmLink))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Processor->OtherLinks, size, -1) ne size)
      { rc = RmE_BadFile; goto done; }
   }
#ifdef HOSTISBIGENDIAN
   { int i;
     for (i = 0; i < Processor->Connections; i++)
      { RmLink *link = RmFindLink(Processor, i);
        SWAP(link->Destination)
        SWAP(link->Target)
      }
   }
#endif

  if (Processor->AttribSize ne 0)
   { Processor->AttribData = (char *) Malloc(Processor->AttribSize);
     if (Processor->AttribData eq Null(char))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Processor->AttribData, Processor->AttribSize,
     			-1) ne Processor->AttribSize)
      { rc = RmE_BadFile; goto done; }

   }

  if (Processor->PAttribSize ne 0)
   { Processor->PAttribData = (char *) Malloc(Processor->PAttribSize);
     if (Processor->PAttribData eq Null(char))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Processor->PAttribData, Processor->PAttribSize,
     			-1) ne Processor->PAttribSize)
      { rc = RmE_BadFile; goto done; }
   }
   
done:      
  if (rc ne RmE_Success)
   { if (Processor ne (RmProcessor)NULL)
      { if (Processor->OtherLinks ne NULL)  Free(Processor->OtherLinks);
	if (Processor->AttribData ne NULL)  Free(Processor->AttribData);
	if (Processor->PAttribData ne NULL) Free(Processor->PAttribData);
        Free(Processor);
      }
     *ProcessorPtr = (RmProcessor) NULL;
     RmErrno = RmE_Success;
   }
  else
   *ProcessorPtr = Processor;
   
  return(rc);
}

/*}}}*/
/*{{{  RmWriteProcessor() */
/**
*** The inverse operation to the above, allowing for the usual filtering.
*** N.B. There is no way of filtering the OtherLinks or the Attributes.
**/
int RmWriteProcessor(Stream *stream, RmProcessor Processor, RmFilter filter)
{ RmProcessorStruct	LocalProcessor;
  int			rc;
  WORD			type;

  memcpy((void *) &LocalProcessor, (void *) Processor,
  		 sizeof(RmProcessorStruct));
  if (filter ne (RmFilter) NULL)
   if (filter->Processor ne NULL)
    { rc = (*(filter->Processor))(Processor ,&LocalProcessor);
      if (rc eq RmE_Skip)
       return(RmE_Success);
      elif (rc ne RmE_Success)
       return(RmErrno = rc);
    }

  type = Type_Processor;

  SWAP(type)
  SWAP(LocalProcessor.ObjNode.Type)
  SWAP(LocalProcessor.ObjNode.Flags)
  SWAP(LocalProcessor.ObjNode.Key)
  SWAP(LocalProcessor.ObjNode.Dates.Creation)
  SWAP(LocalProcessor.ObjNode.Dates.Access)
  SWAP(LocalProcessor.ObjNode.Dates.Modified)
  SWAP(LocalProcessor.ObjNode.Account)
  SWAP(LocalProcessor.ObjNode.Size)
  SWAP(LocalProcessor.Private)
  SWAP(LocalProcessor.Uid)
  SWAP(LocalProcessor.Connections)
  SWAP(LocalProcessor.AttribSize)
  SWAP(LocalProcessor.AttribFree)
  SWAP(LocalProcessor.PAttribSize)
  SWAP(LocalProcessor.PAttribFree)
  SWAP(LocalProcessor.MemorySize)
  SWAP(LocalProcessor.Type)
  SWAP(LocalProcessor.MappedTo)
  SWAP(LocalProcessor.Private2)
  SWAP(LocalProcessor.Errno)
  SWAP(LocalProcessor.SessionId)
  SWAP(LocalProcessor.ApplicationId)
  SWAP(LocalProcessor.DefaultLinks[0].Destination)
  SWAP(LocalProcessor.DefaultLinks[0].Target)
  SWAP(LocalProcessor.DefaultLinks[1].Destination)
  SWAP(LocalProcessor.DefaultLinks[1].Target)
  SWAP(LocalProcessor.DefaultLinks[2].Destination)
  SWAP(LocalProcessor.DefaultLinks[2].Target)
  SWAP(LocalProcessor.DefaultLinks[3].Destination)
  SWAP(LocalProcessor.DefaultLinks[3].Target)
    
  if (Write(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);

  if (Write(stream, (BYTE *) &LocalProcessor, sizeof(RmProcessorStruct), -1)
  		ne sizeof(RmProcessorStruct))
   return(RmErrno = RmE_BadFile);

  SWAP(LocalProcessor.Connections)
  SWAP(LocalProcessor.AttribSize)
  SWAP(LocalProcessor.PAttribSize)

#ifdef HOSTISBIGENDIAN
  if (LocalProcessor.Connections > 4)
   { int	 i;
     RmLink	*link;
     for (i = 4; i < LocalProcessor.Connections; i++)
     { link = RmFindLink(Processor, i);
       SWAP(link->Destination);
       SWAP(link->Target);
     }
   }
#endif
       
  if (LocalProcessor.Connections > 4)
   { int size = (LocalProcessor.Connections - 4) * sizeof(RmLink);
     if (Write(stream, (BYTE *) LocalProcessor.OtherLinks, size, -1) ne size)
      return(RmErrno = RmE_BadFile);
   }

#ifdef HOSTISBIGENDIAN
  if (LocalProcessor.Connections > 4)
   { int		 i;
     RmLink	*link;
     for (i = 4; i < LocalProcessor.Connections; i++)
     { link = RmFindLink(Processor, i);
       SWAP(link->Destination);
       SWAP(link->Target);
     }
   }
#endif

  if (LocalProcessor.AttribSize > 0)
   if (Write(stream, LocalProcessor.AttribData, LocalProcessor.AttribSize, -1)
	ne LocalProcessor.AttribSize)
    return(RmErrno = RmE_BadFile);          

  if (LocalProcessor.PAttribSize > 0)
   if (Write(stream, LocalProcessor.PAttribData, LocalProcessor.PAttribSize, -1)
	ne LocalProcessor.PAttribSize)
    return(RmErrno = RmE_BadFile);          

  return(RmE_Success);
}
/*}}}*/
/*{{{  RmReadTaskforce() */

/**
*** RmReadTaskforce() etc. are based closely on RmReadNetwork. There is
*** no need to worry about reset drivers or anything like that.
**/
static void RmTidyTaskforceRead(RmTaskforce Taskforce);

int RmReadTaskforce(Stream *stream, RmTaskforce *TaskforcePtr, bool recursive)
{ RmTaskforce	Taskforce;
  RmSet		root;
  int	   	rc;
  int	    	i;

  if (!recursive)
   { WORD type;
     if (FullRead(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
      return(RmErrno = RmE_BadFile);
     SWAP(type)
     if (type ne Type_Taskforce)
      return(RmErrno = RmE_NotTaskforce);
   }

  Taskforce = *TaskforcePtr = (RmTaskforce) Malloc(sizeof(RmTaskforceStruct));
  if (Taskforce eq (RmTaskforce) NULL) return(RmErrno = RmE_NoMemory);

  if (FullRead(stream, (BYTE *) Taskforce, sizeof(RmTaskforceStruct), -1) ne
  		sizeof(RmTaskforceStruct))
   { Free(Taskforce); return(RmErrno = RmE_BadFile); }

  SWAP(Taskforce->DirNode.Type)
  SWAP(Taskforce->DirNode.Flags)
  SWAP(Taskforce->DirNode.Key)
  SWAP(Taskforce->DirNode.Dates.Creation)
  SWAP(Taskforce->DirNode.Dates.Access)
  SWAP(Taskforce->DirNode.Dates.Modified)
  SWAP(Taskforce->DirNode.Account)
  SWAP(Taskforce->DirNode.Nentries)
  SWAP(Taskforce->Private)
  SWAP(Taskforce->NoTables)
  SWAP(Taskforce->Private2)
  SWAP(Taskforce->Errno)
  SWAP(Taskforce->ReturnCode)
  SWAP(Taskforce->State)

  InitSemaphore(&(Taskforce->DirNode.Lock), 1);
  InitList(&(Taskforce->DirNode.Entries));
  Taskforce->DirNode.Parent	= NULL;
  Taskforce->DirNode.Node.Next	= NULL;
  Taskforce->DirNode.Node.Prev	= NULL;
  root				= Taskforce->Root;
  Taskforce->Root		= NULL;
     
	/* When writing the root, the pointer is set to NULL */
	/* There is also some special work to be done to sort out the Uid */
	/* tables. */
  if (root eq (RmSet) NULL)
   { Taskforce->Root = (RmSet) Taskforce;
     if ((rc = RmReadRoot((RmSet) Taskforce)) ne RmE_Success)
      { Free(Taskforce); return(RmErrno = rc); }
   }

  for (i = 0; i < Taskforce->DirNode.Nentries; i++)
   { WORD type;
     if (FullRead(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
      { rc = RmE_BadFile; Taskforce->DirNode.Nentries = i; goto error; }
     SWAP(type)

     if (type eq Type_Taskforce)
      { RmTaskforce SubTaskforce;
        if ((rc = RmReadTaskforce(stream, &SubTaskforce, TRUE)) ne RmE_Success)
         { rc = RmE_BadFile; Taskforce->DirNode.Nentries = i; goto error; }
        AddTail(&(Taskforce->DirNode.Entries), &(SubTaskforce->DirNode.Node));
        SubTaskforce->DirNode.Parent = &(Taskforce->DirNode);
      }
     elif (type eq Type_Task)
      { RmTask		Task;
        if ((rc = RmReadTask(stream, &Task, TRUE)) ne RmE_Success)
         { Taskforce->DirNode.Nentries = i; goto error; }
        AddTail(&(Taskforce->DirNode.Entries), &(Task->ObjNode.Node));
        Task->ObjNode.Parent = &(Taskforce->DirNode);
      }
     else
      { rc = RmE_BadFile; Taskforce->DirNode.Nentries = i; goto error; }
   }

  if (Taskforce->Root eq (RmSet) Taskforce)
   RmTidyTaskforceRead(Taskforce);
   
  return(RmE_Success);
  
error:
  RmFreeTaskforce(Taskforce);
  return(RmErrno = rc);
}

static  int	RmTidyTaskforceAux1(RmTask, ...);

static	void	RmTidyTaskforceRead(RmTaskforce Taskforce)
{ (void) RmApplyTaskforce(Taskforce, &RmTidyTaskforceAux1, Taskforce);
}

static	int	RmTidyTaskforceAux1(RmTask Task, ...)
{ va_list	args;
  RmTaskforce	Root;

  va_start(args, Task);
  Root	= va_arg(args, RmTaskforce);
  va_end(args);
  if (RmIsTaskforce(Task))
   { RmTaskforce Taskforce	= (RmTaskforce) Task;
     Taskforce->Root		= (RmSet) Root;  
     (void) RmApplyTaskforce(Taskforce, &RmTidyTaskforceAux1, Root);
   }
  else
   { Task->Root	= Root;
     (void) RmObtainUid((RmSet) Root, (RmObject) Task);
   }
  return(0);
}

/*}}}*/
/*{{{  RmWriteTaskforce() */

static int	RmWriteTaskforceStruct(Stream *stream, RmTaskforce Taskforce,
			RmFilter filter);
static int	RmWriteTaskforceContents(RmTask Task, ...);

int RmWriteTaskforce(Stream *stream, RmTaskforce Taskforce, RmFilter filter)
{ int	rc;

  if ((rc = RmWriteTaskforceStruct(stream, Taskforce, filter)) eq RmE_Success)
   { 
     return(RmSearchTaskforce(Taskforce, &RmWriteTaskforceContents,
     		 stream, filter));
   }
  elif (rc eq RmE_Skip)
   return(RmE_Success);
  else
   return(RmErrno = rc);
}

static int	RmWriteTaskforceStruct(Stream *stream, RmTaskforce Taskforce,
					RmFilter filter)
{ RmTaskforceStruct	copy;
  int			filter_result = RmE_Success;
  WORD			type = Type_Taskforce;
      
  memcpy((void *) &copy, (void *) Taskforce, sizeof(RmTaskforceStruct));
  if (Taskforce->Root eq (RmSet) Taskforce)
   copy.Root = (RmSet) NULL;
  
  if (filter ne (RmFilter) NULL)
   if (filter->Taskforce ne NULL)
    filter_result = (*(filter->Taskforce))(Taskforce, &copy);

  if (filter_result ne RmE_Success)
   return(RmErrno = filter_result);

  SWAP(type)
  SWAP(copy.DirNode.Type)
  SWAP(copy.DirNode.Flags)
  SWAP(copy.DirNode.Key)
  SWAP(copy.DirNode.Dates.Creation)
  SWAP(copy.DirNode.Dates.Access)
  SWAP(copy.DirNode.Dates.Modified)
  SWAP(copy.DirNode.Account)
  SWAP(copy.DirNode.Nentries)
  SWAP(copy.Private)
  SWAP(copy.NoTables)
  SWAP(copy.Private2)
  SWAP(copy.Errno)
  SWAP(copy.ReturnCode)
  SWAP(copy.State)

  if (Write(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);

  if (Write(stream, (BYTE *) &copy, sizeof(RmTaskforceStruct), -1) ne
  		sizeof(RmTaskforceStruct))
      return(RmErrno = RmE_BadFile);

  return(RmE_Success);
}

static int	RmWriteTaskforceContents(RmTask Task, ...)
{ va_list	args;
  Stream	*stream;
  RmFilter	filter;
    
  va_start(args, Task);
  stream = va_arg(args, Stream *);
  filter = va_arg(args, RmFilter);
  va_end(args);
  
  if (RmIsTaskforce(Task))
   return(RmWriteTaskforce(stream, (RmTaskforce) Task, filter));
  else
   return(RmWriteTask(stream, Task, filter));
}

/*}}}*/
/*{{{  RmReadTask() */
int RmReadTask(Stream *stream, RmTask *TaskPtr, bool recurse)
{ RmTask Task = (RmTask)Malloc(sizeof(RmTaskStruct));
  int rc = RmE_Success;

  if (Task eq (RmTask) NULL) return(RmErrno = RmE_NoMemory);
  
  if (!recurse)
   { WORD type;
     if (FullRead(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
      { rc = RmE_BadFile; goto done; }
     SWAP(type)
     if (type ne Type_Task)
      { rc = RmE_BadFile; goto done; }
   }
   
  if (FullRead(stream, (BYTE *) Task, sizeof(RmTaskStruct), -1) ne
      sizeof(RmTaskStruct))
   { rc = RmE_BadFile; goto done; }

  SWAP(Task->ObjNode.Type)
  SWAP(Task->ObjNode.Flags)
  SWAP(Task->ObjNode.Key)
  SWAP(Task->ObjNode.Dates.Creation)
  SWAP(Task->ObjNode.Dates.Access)
  SWAP(Task->ObjNode.Dates.Modified)
  SWAP(Task->ObjNode.Account)
  SWAP(Task->ObjNode.Size)
  SWAP(Task->Private)
  SWAP(Task->Uid)
  SWAP(Task->Connections)
  SWAP(Task->AttribSize)
  SWAP(Task->AttribFree)
  SWAP(Task->PAttribSize)
  SWAP(Task->PAttribFree)
  SWAP(Task->MemorySize)
  SWAP(Task->Type)
  SWAP(Task->MaxArgIndex)
  SWAP(Task->NextArgIndex)
  SWAP(Task->MaxArgStrings)
  SWAP(Task->NextArgStrings)
  SWAP(Task->MappedTo)
  SWAP(Task->MappedNode)
  SWAP(Task->Private2)
  SWAP(Task->Errno)
  SWAP(Task->ReturnCode)

  InitSemaphore(&(Task->ObjNode.Lock), 1);   
  InitList(&(Task->ObjNode.Contents));
  Task->ObjNode.Node.Next	= NULL;
  Task->ObjNode.Node.Prev	= NULL;
  Task->ObjNode.Parent		= NULL;
  Task->Root			= NULL;
  Task->OtherChannels		= NULL;
  Task->AttribData		= NULL;
  Task->PAttribData		= NULL;
  Task->ArgIndex		= NULL;
  Task->ArgStrings		= NULL;
  Task->MappedNode.Next		= NULL;
  Task->MappedNode.Prev		= NULL;

  if (Task->Connections > 4)
   { int size = (Task->Connections - 4) * sizeof(RmChannel);
     Task->OtherChannels = (RmChannel *) Malloc(size);
     if (Task->OtherChannels eq Null(RmChannel))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Task->OtherChannels, size, -1) ne size)
      { rc = RmE_BadFile; goto done; }
   }

#ifdef HOSTISBIGENDIAN
   { int i;
     for (i = 0; i < Task->Connections; i++)
      { RmLink *link = RmFindLink((RmProcessor)Task, i);
        SWAP(link->Destination)
        SWAP(link->Target)
      }
   }
#endif

  if (Task->AttribSize ne 0)
   { Task->AttribData = (char *) Malloc(Task->AttribSize);
     if (Task->AttribData eq Null(char))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Task->AttribData, Task->AttribSize,
     			-1) ne Task->AttribSize)
      { rc = RmE_BadFile; goto done; }
   }

  if (Task->PAttribSize ne 0)
   { Task->PAttribData = (char *) Malloc(Task->PAttribSize);
     if (Task->PAttribData eq Null(char))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Task->PAttribData, Task->PAttribSize,
     			-1) ne Task->PAttribSize)
      { rc = RmE_BadFile; goto done; }
   }

  if (Task->MaxArgIndex > 0)
   { int	size = Task->MaxArgIndex * sizeof(int);
     Task->ArgIndex = (int *) Malloc(size);
     if (Task->ArgIndex eq Null(int))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Task->ArgIndex, size, -1) ne size)
      { rc = RmE_BadFile; goto done; }
   }

#ifdef HOSTISBIGENDIAN
  { int i;
    for (i = 0; i < Task->MaxArgIndex; i++)
     SWAP(Task->ArgIndex[i]);
  }
#endif
  
  if (Task->MaxArgStrings > 0)
   { int	size = Task->MaxArgStrings;
     Task->ArgStrings = (char *) Malloc(size);
     if (Task->ArgStrings eq Null(char))
      { rc = RmE_NoMemory; goto done; }
     if (FullRead(stream, (BYTE *) Task->ArgStrings, size, -1) ne size)
      { rc = RmE_BadFile; goto done; }
   }
   
done:      
  if (rc ne RmE_Success)
   { if (Task ne (RmTask)NULL)
      { if (Task->OtherChannels ne NULL)	Free(Task->OtherChannels);
	if (Task->ArgIndex ne NULL)		Free(Task->ArgIndex);
	if (Task->ArgStrings ne NULL)		Free(Task->ArgStrings);
	if (Task->AttribData ne NULL)		Free(Task->AttribData);
	if (Task->PAttribData ne NULL)		Free(Task->PAttribData);
        Free(Task);
      }
     *TaskPtr = (RmTask) NULL;
     RmErrno = rc;
   }
  else
   *TaskPtr = Task;
   
  return(rc);
}
/*}}}*/
/*{{{  RmWriteTask() */
int RmWriteTask(Stream *stream, RmTask Task, RmFilter filter)
{ RmTaskStruct		LocalTask;
  int			rc;
  WORD			type;

  memcpy((void *) &LocalTask, (void *) Task, sizeof(RmTaskStruct));
  if (filter ne (RmFilter) NULL)
   if (filter->Task ne NULL)
    { rc = (*(filter->Task))(Task, &LocalTask);
      if (rc eq RmE_Skip)
       return(RmE_Success);
      elif (rc ne RmE_Success)
       return(RmErrno = rc);
    }

  type = Type_Task;
  SWAP(type)
  SWAP(LocalTask.ObjNode.Type)
  SWAP(LocalTask.ObjNode.Flags)
  SWAP(LocalTask.ObjNode.Key)
  SWAP(LocalTask.ObjNode.Dates.Creation)
  SWAP(LocalTask.ObjNode.Dates.Access)
  SWAP(LocalTask.ObjNode.Dates.Modified)
  SWAP(LocalTask.ObjNode.Account)
  SWAP(LocalTask.ObjNode.Size)
  SWAP(LocalTask.Private)
  SWAP(LocalTask.Uid)
  SWAP(LocalTask.Connections)
  SWAP(LocalTask.AttribSize)
  SWAP(LocalTask.AttribFree)
  SWAP(LocalTask.PAttribSize)
  SWAP(LocalTask.PAttribFree)
  SWAP(LocalTask.MemorySize)
  SWAP(LocalTask.Type)
  SWAP(LocalTask.MaxArgIndex)
  SWAP(LocalTask.NextArgIndex)
  SWAP(LocalTask.MaxArgStrings)
  SWAP(LocalTask.NextArgStrings)
  SWAP(LocalTask.MappedTo)
  SWAP(LocalTask.MappedNode)
  SWAP(LocalTask.Private2)
  SWAP(LocalTask.Errno)
  SWAP(LocalTask.ReturnCode)
  SWAP(LocalTask.DefaultChannels[0].Destination)
  SWAP(LocalTask.DefaultChannels[0].Target)
  SWAP(LocalTask.DefaultChannels[1].Destination)
  SWAP(LocalTask.DefaultChannels[1].Target)
  SWAP(LocalTask.DefaultChannels[2].Destination)
  SWAP(LocalTask.DefaultChannels[2].Target)
  SWAP(LocalTask.DefaultChannels[3].Destination)
  SWAP(LocalTask.DefaultChannels[3].Target)

  if (Write(stream, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);
  if (Write(stream, (BYTE *) &LocalTask, sizeof(RmTaskStruct), -1)
  		ne sizeof(RmTaskStruct))
   return(RmErrno = RmE_BadFile);

  SWAP(LocalTask.Connections)
  SWAP(LocalTask.AttribSize)
  SWAP(LocalTask.PAttribSize)
  SWAP(LocalTask.MaxArgIndex)
  SWAP(LocalTask.NextArgIndex)
  SWAP(LocalTask.MaxArgStrings)
  SWAP(LocalTask.NextArgStrings)

#ifdef HOSTISBIGENDIAN
  if (LocalTask.Connections > 4)
   { int	 i;
     RmLink	*link;
     for (i = 4; i < LocalTask.Connections; i++)
     { link = RmFindLink((RmProcessor) Task, i);
       SWAP(link->Destination);
       SWAP(link->Target);
     }
   }
#endif

  if (LocalTask.Connections > 4)
   { int size = (LocalTask.Connections - 4) * sizeof(RmChannel);
     if (Write(stream, (BYTE *) LocalTask.OtherChannels, size, -1) ne size)
      return(RmErrno = RmE_BadFile);
   }

#ifdef HOSTISBIGENDIAN
  if (LocalTask.Connections > 4)
   { int		 i;
     RmLink	*link;
     for (i = 4; i < LocalTask.Connections; i++)
     { link = RmFindLink((RmProcessor)Task, i);
       SWAP(link->Destination);
       SWAP(link->Target);
     }
   }
#endif

  if (LocalTask.AttribSize > 0)
   if (Write(stream, LocalTask.AttribData, LocalTask.AttribSize, -1)
	ne LocalTask.AttribSize)
    return(RmErrno = RmE_BadFile);          

  if (LocalTask.PAttribSize > 0)
   if (Write(stream, LocalTask.PAttribData, LocalTask.PAttribSize, -1)
	ne LocalTask.PAttribSize)
    return(RmErrno = RmE_BadFile);          

  if (LocalTask.MaxArgIndex > 0)

   { int size = LocalTask.MaxArgIndex * sizeof(int);
#ifdef HOSTISBIGENDIAN
     { int i;
       for (i = 0; i < LocalTask.MaxArgIndex; i++)
        SWAP(Task->ArgIndex[i]);
     }
#endif
     if (Write(stream, (BYTE *) LocalTask.ArgIndex, size, -1) ne size)
      return(RmErrno = RmE_BadFile);
#ifdef HOSTISBIGENDIAN
     { int i;
       for (i = 0; i < LocalTask.MaxArgIndex; i++)
        SWAP(Task->ArgIndex[i]);
     }
#endif
   }
  if (LocalTask.MaxArgStrings > 0)
   { int size = LocalTask.MaxArgStrings;
     if (Write(stream, (BYTE *) LocalTask.ArgStrings, size, -1) ne size)
      return(RmErrno = RmE_BadFile);
   }
  return(RmE_Success);
}
/*}}}*/
/*{{{  odds and ends */
/**-----------------------------------------------------------------------------
*** More File I/O routines, this time dealing with file descriptors rather
*** than Helios streams, to satisfy the pundits.
**/

int	RmReadfd(int fd, RmNetwork *network, RmTaskforce *taskforce)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmReadStream(str, network, taskforce));
}

int	RmReadfdNetwork(int fd, RmNetwork *network)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmReadNetwork(str, network, FALSE));
}

int	RmReadfdTaskforce(int fd, RmTaskforce *taskforce)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmReadTaskforce(str, taskforce, FALSE));
}

int	RmReadfdNetworkOnly(int fd, RmNetwork *networkptr)
{ RmNetwork	Network;
  int		rc;
  word		type;
  Stream	*str = fdstream(fd);
  
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  
  if (FullRead(str, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);
  SWAP(type)

  if (type ne Type_Network)
   return(RmErrno = RmE_NotNetwork);
   
  Network = *networkptr = (RmNetwork) Malloc(sizeof(RmNetworkStruct));
  if (Network eq (RmNetwork) NULL)
   return(*networkptr  = (RmNetwork) NULL, RmErrno = RmE_NoMemory);
  if (FullRead(str, (BYTE *) Network, sizeof(RmNetworkStruct), -1) ne
  	sizeof(RmNetworkStruct))
   { Free(Network);
     *networkptr = (RmNetwork) NULL;
     return(RmErrno = RmE_BadFile);
   }
  SWAP(Network->DirNode.Type)
  SWAP(Network->DirNode.Flags)
  SWAP(Network->DirNode.Key)
  SWAP(Network->DirNode.Dates.Creation)
  SWAP(Network->DirNode.Dates.Access)
  SWAP(Network->DirNode.Dates.Modified)
  SWAP(Network->DirNode.Account)
  SWAP(Network->DirNode.Nentries)
  SWAP(Network->Private)
  SWAP(Network->NoTables)
  SWAP(Network->Hardware.Head)
  SWAP(Network->Private2)
  SWAP(Network->Errno)

  InitList(&(Network->Hardware));
  InitSemaphore(&(Network->DirNode.Lock), 1);
  InitList(&(Network->DirNode.Entries));
  Network->DirNode.Parent = Null(DirNode);

	/* When writing the root, the pointer is set to NULL */
	/* There is also some special work to be done to sort out the Uid */
	/* tables. */
  if (Network->Root eq (RmSet) NULL)
   { Network->Root = (RmSet) Network;
     if ((rc = RmReadRoot((RmSet) Network)) ne RmE_Success)
      { Free(Network); return(rc); }
   }
  
  return(RmE_Success);   
}

int	RmReadfdProcessor(int fd, RmProcessor *processor)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmReadProcessor(str, processor, FALSE));
}

int	RmReadfdTask(int fd, RmTask *task)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmReadTask(str, task, FALSE));
}

int	RmReadfdTaskforceOnly(int fd, RmTaskforce *taskforceptr)
{ RmTaskforce	Taskforce;
  int		rc;
  word		type;
  Stream	*str = fdstream(fd);
  
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  
  if (FullRead(str, (BYTE *) &type, sizeof(WORD), -1) ne sizeof(WORD))
   return(RmErrno = RmE_BadFile);
  SWAP(type)
  if (type ne Type_Taskforce)
   return(RmErrno = RmE_NotNetwork);
   
  Taskforce= *taskforceptr = (RmTaskforce) Malloc(sizeof(RmTaskforceStruct));
  if (Taskforce eq (RmTaskforce) NULL)
   return(*taskforceptr  = (RmTaskforce) NULL, RmErrno = RmE_NoMemory);
  if (FullRead(str, (BYTE *) Taskforce, sizeof(RmTaskforceStruct), -1) ne
  	sizeof(RmTaskforceStruct))
   { Free(Taskforce);
     *taskforceptr = (RmTaskforce) NULL;
     return(RmErrno = RmE_BadFile);
   }
  SWAP(Taskforce->DirNode.Type)
  SWAP(Taskforce->DirNode.Flags)
  SWAP(Taskforce->DirNode.Key)
  SWAP(Taskforce->DirNode.Dates.Creation)
  SWAP(Taskforce->DirNode.Dates.Access)
  SWAP(Taskforce->DirNode.Dates.Modified)
  SWAP(Taskforce->DirNode.Account)
  SWAP(Taskforce->DirNode.Nentries)
  SWAP(Taskforce->Private)
  SWAP(Taskforce->NoTables)
  SWAP(Taskforce->Private2)
  SWAP(Taskforce->Errno)
  SWAP(Taskforce->ReturnCode)
  SWAP(Taskforce->State)

  InitSemaphore(&(Taskforce->DirNode.Lock), 1);
  InitList(&(Taskforce->DirNode.Entries));
  Taskforce->DirNode.Parent = Null(DirNode);

	/* When writing the root, the pointer is set to NULL */
	/* There is also some special work to be done to sort out the Uid */
	/* tables. */
  if (Taskforce->Root eq (RmSet) NULL)
   { Taskforce->Root = (RmSet) Taskforce;
     if ((rc = RmReadRoot((RmSet) Taskforce)) ne RmE_Success)
      { Free(Taskforce); return(rc); }
   }
  
  return(RmE_Success);   
}

int	RmWritefd(int fd, RmNetwork network, RmTaskforce taskforce)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteStream(str, network, taskforce, (RmFilter) NULL));
}

int	RmWritefdNetwork(int fd, RmNetwork network)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteNetwork(str, network, (RmFilter) NULL));
}

int	RmWritefdTaskforce(int fd, RmTaskforce taskforce)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteTaskforce(str, taskforce, (RmFilter) NULL));
}

int	RmWritefdNetworkOnly(int fd, RmNetwork network)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteNetworkStruct(str, network, (RmFilter) NULL));
}


int	RmWritefdTaskforceOnly(int fd, RmTaskforce taskforce)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteTaskforceStruct(str, taskforce, (RmFilter) NULL));
}

int	RmWritefdProcessor(int fd, RmProcessor processor)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteProcessor(str, processor, (RmFilter) NULL));
}

int	RmWritefdTask(int fd, RmTask task)
{ Stream	*str = fdstream(fd);
  if (str eq Null(Stream))
   return(RmErrno = RmE_BadArgument);
  else
   return(RmWriteTask(str, task, (RmFilter) NULL));
}
/*}}}*/

/*}}}*/
