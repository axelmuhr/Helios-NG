/* $Header: microlink.c,v 1.4 91/01/31 13:51:21 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/microlink.c,v $ */

/*------------------------------------------------------------------------*/
/*                                          microlink/source/microlink.c  */
/*------------------------------------------------------------------------*/

/* This is the source code for the helios microlink server which starts   */
/*    up a server which manages objects for each of the aspects of        */
/*    communication on the mirco-controller.                              */

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include "microlink/private/microlink.h"
#include "microlink/general.h"
#include <unistd.h>

/*------------------------------------------------------------------------*/
/*                                       Diagnostics semaphore interlock  */
/*------------------------------------------------------------------------*/

/* Interlocks calls to printf and the like for diagnostics ...            */

Semaphore diag;

/*------------------------------------------------------------------------*/
/*                                              Hermes Request Semaphore  */
/*------------------------------------------------------------------------*/

/* The following semaphore is used to interlock the transmission of       */
/*   messages to Hermes and waiting for the reply: Thus a message will    */
/*   never be transmitted until a reply from the previos one is obtained  */
/* This function is used in MlkDoHermesRequest(...)                       */
/* It is initialised on entry to main(...)                                */

Semaphore MlkLinkRequestInterlock;
int dgDiag = 0;

/*------------------------------------------------------------------------*/
/*                           The root-directory for the microlink server  */
/*------------------------------------------------------------------------*/

/* A server normally manifests itself as an entry in the directory        */
/*    corresponding to the processor on which it is running. This entry   */
/*    is the 'server root directory' and is itself a directory and in     */
/*    this case contains an entry for each type of communication which    */
/*    typically occurs over the microlink channel.                        */
/* The whole naming structure exists in memory, and the following         */
/*    static definition defines the object which forms the microlink      */
/*    server entry in the processor directory.                            */

static DirNode microlinkDir;

/*------------------------------------------------------------------------*/
/*                               Name for the microlink server directory  */
/*------------------------------------------------------------------------*/

/* It is the following name which is assigned to the directory created    */
/*   corresponding to the microlink server.                               */

#define MicrolinkName "microlink"

/*------------------------------------------------------------------------*/
/*                       Look-Ahead Declarations of the service routines  */
/*------------------------------------------------------------------------*/

/* These are forward definitions for the functions that service the       */
/*     individual requests.                                               */

static void mlkDoEscape  ( ServInfo * );   /* Code unknown by dispatcher  */
static void mlkDoOpen    ( ServInfo * );   /* FG_Open                     */
static void mlkDoCreate  ( ServInfo * );   /* FG_Create                   */
static void mlkDoLocate  ( ServInfo * );   /* FG_Locate                   */
static void mlkDoObjInfo ( ServInfo * );   /* FG_ObjectInfo               */
static void mlkDoSrvInfo ( ServInfo * );   /* FG_ServerInfo               */
static void mlkDoDelete  ( ServInfo * );   /* FG_Delete                   */
static void mlkDoRename  ( ServInfo * );   /* FG_Rename                   */
static void mlkDoLink    ( ServInfo * );   /* FG_Link                     */
static void mlkDoProtect ( ServInfo * );   /* FG_Protect                  */
static void mlkDoSetDate ( ServInfo * );   /* FG_SetDate                  */
static void mlkDoRefine  ( ServInfo * );   /* FG_Refine                   */
static void mlkDoClose   ( ServInfo * );   /* FG_CloseObj                 */
# if 0                                     /* Wait for Helios 1.2 ...     */
static void mlkDoRevoke  ( ServInfo * );   /* FG_Revoke                   */
# endif                                    /* ... End Of Condition        */

/*------------------------------------------------------------------------*/
/*                                                Dispatcher information  */
/*------------------------------------------------------------------------*/

/* The server-library provides a function which sits on the message port  */
/*   corresponding to the microlink-root-directory, picks up server       */
/*   request messages and dispatches then to routines which service the   */
/*   given entry. The following structure is the control structure for    */
/*   that routine. It describes which message port to sit on to await     */
/*   requests, and it describes which routines to dispatch to and         */
/*   so-forth.                                                            */

static DispatchInfo microlinkInfo =
{  &microlinkDir,                 /* Directory structure for this server  */
   NullPort,                      /* Req port for server (filled later)   */
   SS_Device,                     /* Sub-system code: not sure about this */
   NULL,                          /* Name of parent dir (filled later)    */
   { mlkDoEscape,ReqStackSize },  /* Dispatch entry for unknown codes     */
   {                              /* Dispatch table for known codes :     */
      { mlkDoOpen,   ReqStackSize }, /* Each pair consists of the name of */
      { mlkDoCreate, ReqStackSize }, /*   the function to call (in a      */
      { mlkDoLocate, ReqStackSize }, /*   forked off process), and the    */
      { mlkDoObjInfo,ReqStackSize }, /*   amount of space to allocate on  */
      { mlkDoSrvInfo,ReqStackSize }, /*   the stack for this process.     */
      { mlkDoDelete, ReqStackSize }, /* ... continued ...                 */
      { mlkDoRename, ReqStackSize }, /* ... continued ...                 */
      { mlkDoLink,   ReqStackSize }, /* ... continued ...                 */
      { mlkDoProtect,ReqStackSize }, /* ... continued ...                 */
      { mlkDoSetDate,ReqStackSize }, /* ... continued ...                 */
      { mlkDoRefine, ReqStackSize }, /* ... continued ...                 */
      { mlkDoClose,  ReqStackSize }  /* ... continued ...                 */
#     if 0                           /* Not until we get Helios 1.2  ...  */
 ,    { mlkDoRevoke, ReqStackSize }, /* ... continued ...                 */
      { InvalidFn,   ReqStackSize }, /* ... continued ...                 */
      { InvalidFn,   ReqStackSize }  /* ... continued                     */
#     endif

   }
};

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main(int argc,char **argv)
/* When this program is called it installs itself as a server, placing    */
/*   an entry for itself in the directory corresponding to the processor  */
/*   on which it is running, and placing entries in this directory for    */
/*   the individual services it provides. In doing so it creates a        */
/*   message port associated with it's root directory entry, and it       */
/*   subsequently calls the dispatcher in the server-library which awaits */
/*   I/O requests on this port and passses them on to appropriate         */
/*   routines in this server.                                             */
{  char    machineName[MaxMachineNameLength+1];
   Object *machineObject;
   Port    microlinkRequests;

   /* Use fork(...) to push this task into the background */
   if(fork()) return 0;

   fflush(stdout);
   if(argc>1 && !strcmp(argv[1],"-d")) dgDiag = 1; else dgDiag = 0;

   InitSemaphore(&diag,1);
 
   /* Initialise the sempahore whihc ensures that no new messages are     */
   /*    sent to hermes before the old one is received:                   */
   InitSemaphore(&MlkLinkRequestInterlock,1);
  
   /* First find the name of this machine. Look-up this name in the       */
   /*    processor's name table and return the entry for it as an object. */
   /* Set the name of the parent of the microlink server root directory:  */
   /*    since the microlink server root directory is inserted into the   */
   /*    directory for the processor, the parent name is the name of the  */
   /*    processor directory.                                             */
   MachineName(machineName);
   machineObject            = Locate(NULL,machineName);
   microlinkInfo.ParentName = machineName;

   /* Further intialize the microlink directory description. Give it a    */
   /*    name and initialize all fields and so-forth.                     */
   InitNode
   (  (ObjNode*)&microlinkDir, MicrolinkName, 
      Type_Directory, 0, DefDirMatrix
   );
   InitList(&microlinkDir.Entries);
   microlinkDir.Nentries = 0;
    
   /* Allocate a new request port on which to receive requests aimed at   */
   /*    this server, and set the entry in the dispatcher information     */
   /*    which identifies this port.                                      */
   microlinkRequests     = NewPort();
   microlinkInfo.ReqPort = microlinkRequests;
    
   /* Create the entry for this server in the name-table in the           */
   /*    directory for this processor, and in doing so associate it with  */
   /*    the request-port allocated so that requests with the name of     */
   /*    the microlink server in their paths get routed through the       */
   /*    message-port:                                                    */
   {  Object    *entryForMicrolink;
      NameInfo   nameInfo;
      nameInfo.Port     = microlinkRequests;
      nameInfo.Flags    = Flags_StripName;
      nameInfo.Matrix   = DefDirMatrix;
      nameInfo.LoadData = NULL;
      entryForMicrolink = Create ( machineObject, microlinkDir.Name,
                                   Type_Name,     sizeof(NameInfo),
                                   (byte*)&nameInfo
                                 );
      if(entryForMicrolink==NULL)
      {  printf("microlink : Failed to self-install in name table\n");
         Exit(1); 
         for(;;) printf("microlink : What the hell is going on ?\n");
      }
   }
   
   /* Put in the '..' entry in the microlink directory ...               */
   {  LinkNode *parent;
      parent 
         = (LinkNode*)Malloc(sizeof(LinkNode)+(word)strlen(machineName));
      InitNode(&parent->ObjNode,"..",Type_Link,0,DefDirMatrix);
      parent->Cap = machineObject->Access; strcpy(parent->Link,machineName);
      microlinkDir.Parent = (DirNode*)parent;
   }
   
   Close(machineObject); /* Locate() and Close() form pairs              */
   
   /* Now the microlink directory entry has been made in the name table  */
   /*    in the directory for the processor running this task. It has    */
   /*    a message port associated with it, and currently has no         */
   /*    entries in it. The next task is to set-up the entries in the    */
   /*    microlink directory for each individual service provided by     */
   /*    this server, and then to set the dispatcher waiting on the      */
   /*    message port to await requests.                                 */

   /* Each of the 'mlkAdd...Entry' functions are described elsewhere.    */
   /* They install themselves as a name into the microlink root          */
   /*    directory establishing themselves as services which             */
   /*    communicate over the microlink.                                 */
   mlkAddDigitiserEntry  (&microlinkDir);
   mlkAddHermvEntry      (&microlinkDir);
   mlkAddGeneralEntry    (&microlinkDir);
   mlkAddRawDigEntry     (&microlinkDir);
   /* ... add more entries in here ... */
    
   Dispatch(&microlinkInfo);
}

/*------------------------------------------------------------------------*/
/*                                                        mlkDoOpen(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoOpen(ServInfo *si)
/* This function services the 'open' requests to entries in the microlink */
/*   directory. They do this by determining which service the node        */
/*   corresponds to and performing the appropriate service functions for  */
/*   that node.                                                           */
{  MicrolinkNode *nde;
   MsgBuf       *rply;
   Port          reqp;

   /* Get the target object refrred to, trap if not found:                */
   nde = (MicrolinkNode*) GetTarget(si);
   if(nde==NULL) { ErrorMsg(si->m,EC_Error|EG_Unknown|EO_Object); return; }
   
   /* Check that we have permission to open it: */
   {  IOCMsg2 *cb;
      cb = (IOCMsg2*)si->m->Control;
      if ( !CheckMask
            (  cb->Common.Access.Access,(AccMask)(cb->Arg.Mode&Flags_Mode)  )
         )
      {  ErrorMsg(si->m,EC_Error|EG_Protected|EO_File);
         return;
      }
   }

   /* Allocate message buffer in which to send reply: */
   rply = New(MsgBuf);
   if(rply==NULL) 
   {  ErrorMsg(si->m,EC_Error|EG_NoMemory); return;  }
   
   /* Reply to client, providing a message port on which to send requests */
   FormOpenReply
   (  rply,si->m,&nde->obj,Flags_Server|Flags_Closeable,si->Pathname  );
   reqp = NewPort(); rply->mcb.MsgHdr.Reply = reqp;
   
   /* Trap if the request is to the directory itself: */
   if(nde->obj.Type==Type_Directory)
   {  PutMsg(&rply->mcb);           /* Send reply message                 */
      DirServer(si,si->m,reqp);     /* Delegate to servlib function       */
      FreePort(reqp);               /* Free message port                  */
      Free(rply);                   /* Free reply message buffer          */
      return;                       /* Return: finish task                */
   }
   
   if(nde->obj.Type==Type_File)
   {  switch(nde->objTyp)
      {  case MlkDigitiser: mlkServeDigitiser(si,nde,rply);       break;
         case MlkHermv:     mlkServeHermv    (si,nde,rply);       break;
         case MlkGeneral:   mlkServeHermv    (si,nde,rply);       break;
         case MlkRawDig:    mlkServeRawDig   (si,nde,rply);       break;
         default: ErrorMsg(si->m,EC_Error|EG_Invalid|EO_Object);  break;
      }
      FreePort(reqp);               /* Free port-assignment               */
      Free(rply);                   /* Free reply message buffer          */
   }
   
   ErrorMsg(si->m,EC_Error|EG_Invalid|EO_Object); return;
}

/*------------------------------------------------------------------------*/
/*                                                      mlkDoCreate(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoCreate( ServInfo *si )
/* It is illegal to try to create any objects in the microlink server     */
{  ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                                      mlkDoLocate(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoLocate( ServInfo *si )
/* This function simply deferres to the server-library implementation */
{  DoLocate(si);  }

/*------------------------------------------------------------------------*/
/*                                                     mlkDoObjInfo(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoObjInfo( ServInfo *si )
/* Defer to servlib: */
{  DoObjInfo(si);  }

/*------------------------------------------------------------------------*/
/*                                                     mlkDoSrvInfo(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoSrvInfo( ServInfo *si )
/* This function does nothing, but returns with no error code */
{  NullFn(si);  }

/*------------------------------------------------------------------------*/
/*                                                      mlkDoDelete(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoDelete( ServInfo *si )
/* Deleting microlink objects is not permitted */
{  ObjNode *nde;

   nde = GetTarget(si);
   if(nde->Type==Type_Directory)
   {  ErrorMsg(si->m,(word)Err_Null);
      raise(SIGINT); /* Self-kill */
   }
   ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                                      mlkDoRename(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoRename( ServInfo *si )
/* Renaming microlink objects is not permitted */
{  ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                                        mlkDoLink(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoLink( ServInfo *si )
/* It is not permitted to create links in the microlink directory */
{  ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                                     mlkDoProtect(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoProtect( ServInfo *si )
/* Defer to servlib: */
{  DoProtect(si);  }

/*------------------------------------------------------------------------*/
/*                                                     mlkDoSetDate(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoSetDate( ServInfo *si )
/* Defer to servlib: */
{  DoSetDate(si);  }


/*------------------------------------------------------------------------*/
/*                                                      mlkDoRefine(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoRefine( ServInfo *si )
/* Defer to servlib: */
{  DoRefine(si);  }

/*------------------------------------------------------------------------*/
/*                                                       mlkDoClose(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoClose( ServInfo *si )
/* Who knows what this function is supposed to do ?                       */
/* --- Close requests should be sent down the reply port returned to the  */
/*       client which sent the Open request.                              */
/* Consequently, this function returns an error.                          */
{  ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                                      mlkDoRevoke(...)  */
/*------------------------------------------------------------------------*/

# if 0                                     /* Wait for Helios 1.2 ...     */
static void mlkDoRevoke( ServInfo *si )
/* Defer to servlib: */
{  DoRevoke(si);  }
# endif                                    /* ... End Of Condition        */

/*------------------------------------------------------------------------*/
/*                                                      mlkDoEscape(...)  */
/*------------------------------------------------------------------------*/

static void mlkDoEscape( ServInfo *si )
/* This function for unknown codes: */
{  ErrorMsg(si->m,EC_Error|EG_WrongFn|EO_Object);
   return;
}

/*------------------------------------------------------------------------*/
/*                                            mlkPerformLinkRequest(...)  */
/*------------------------------------------------------------------------*/

static void MlkForkedLinkRequest(MlkLinkRequestControl *ctx);

void MlkPerformLinkRequest(MlkLinkRequestControl *ctl)
/* This funciton is the function which performs interlocked request/reply */
/*   exchanges over the microlink: The whole exchange is governed by the  */
/*   control structure <ctl>. Before returning this routine will have     */
/*   initialised the semaphore in the structure <ctl> so that in order to */
/*   wait for the request to be complete (this function does NOT block)   */
/*   the client merely has to wait on the semaphore. As a result, the     */
/*   control structure will contain some status about the message and     */
/*   possibly a reply flag.                                               */
{  InitSemaphore(&ctl->sem,0);
   Fork(ReqStackSize,MlkForkedLinkRequest,4,ctl);
   return;
}

/*------------------------------------------------------------------------*/
/*                                             MlkForkedLinkRequest(...)  */
/*------------------------------------------------------------------------*/

static void MlkForkedLinkRequest(MlkLinkRequestControl *ctl)
/* This function is forked off by the above and performs a request/reply  */
/*    sequence (interlocked) over the microlink, then signals a semaphore */
/*    in the control structure passed as a parameter to indicate that the */
/*    request has been completed.                                         */
{  word res,hdl;

   Wait(&MlkLinkRequestInterlock);

   if(ctl->rxBuf)
   {  hdl = ML_SetUpRx(ctl->rxBuf,ctl->rxBuf[0]);
      if(hdl<0) 
      {  ctl->status = General_RxFailed;
         ctl->code   = hdl;
         Signal(&ctl->sem);
         Signal(&MlkLinkRequestInterlock);
         return;
      }
   }

   if(ctl->txBuf)
   {  res = ML_Transmit(ctl->txBuf);
      if(res<0)
      {  ctl->status = General_TxFailed;
         ctl->code   = res;
         if(ctl->rxBuf) ML_WaitForRx(hdl,0); /* Discard rx buffer */
         Signal(&ctl->sem);
         Signal(&MlkLinkRequestInterlock);
         return;
      }
   }

   if(ctl->rxBuf)
   {  res = ML_WaitForRx(hdl,(word)(OneSec/10));
      if(res<0)
      {  ctl->status = General_RxFailed;
         ctl->code   = res;
         Signal(&ctl->sem);
         Signal(&MlkLinkRequestInterlock);
         return;
      }
   }

   ctl->status = General_Ok;
   Signal(&ctl->sem);
   Signal(&MlkLinkRequestInterlock);
}

