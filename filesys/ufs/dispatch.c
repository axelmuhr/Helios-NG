/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 6 August 1992                                                */
/* File: dispatch.c                                                   */
/*                                                                    */
/*
 * This file contains the main dispatcher routine, as well as its
 * support routines.  These support routines provide the interface
 * from Helios into the UNIX kernel.
 */
/*
 * $Id: dispatch.c,v 1.4 1993/04/08 14:50:19 nickc Exp $ 
 * $Log: dispatch.c,v $
 * Revision 1.4  1993/04/08  14:50:19  nickc
 * (Alex S) added support for lstat()
 *
 * Revision 1.3  1992/11/05  15:11:48  al
 * Added support for symbolic links external to UFS.
 *
 * Revision 1.2  1992/10/13  11:47:03  al
 * Syntax fixed to compile for C40
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#include "param.h"
#include "malloc.h"
#include "buf.h"
#include "filedesc.h"
#include "file.h"
#include "user.h"
#include "stat.h"
#include "proc.h"
#include "mount.h"
#include "vnode.h"
#include "fcntl.h"

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <module.h>
#include <device.h>

#define UFSOPENFILE
#include "dispatch.h"
#include "private.h"

/*
 * Variables local to the dispatcher.
 */
#define STACKSIZE	10000		/* whatalotigot */
#define PID_MIN		42		/* Life the universe and everything */
static List ClientsRoot;		/* Root of clients */
static Semaphore ClientsLock;		/* Semaphore for access to list */
static Semaphore ClientsActive;		/* Number of active clients */
static int pid_counter = PID_MIN;	/* The answer !!! */

/*
 * HELIOS filesystem resource variables.
 */
word SyncTime;		/* Maximum idle time before sync */
extern uid_t rootuid;

/*
 * UNIX resource variables.
 */
extern struct plimit limit0;

/*
 * Prototypes for UNIX system call routines.
 */
extern sync();

/*
 *  Returns whether the client given is the root or startup user.
 */
int rootuser(ClientState *client)
{	uid_t uid = client->ugid & 0xFFFF;

	return ((uid == 0) || (uid == rootuid));
}

/*
 * Create a client structure and add it to the list.
 */
int createclient(ClientState **client)
{
	struct proc *proc;
	
	*client = (ClientState *)malloc(sizeof(ClientState),M_CLIENT,M_NOWAIT);
	if (*client == NULL) {
		panic("No more memory to create clients");
		return(ENOMEM);
	}
	bzero(*client,sizeof(ClientState));	/* Sets ofname + force_close */

	/* Initial Values etc. */
	proc = &(*client)->p;
		
	/* Initialise proc */
	proc->p_cred = (struct pcred *)malloc(sizeof(struct pcred),M_CRED,M_NOWAIT);
	bcopy(proc0.p_cred, proc->p_cred, sizeof(struct pcred)); /* Dup proc0 */
	proc->p_ucred = crget();		/* Zero Credentials */
	proc->p_fd = fdcopy(&proc0);		/* Copy proc0's fd */
	proc->p_pid = pid_counter++;
	if (pid_counter < PID_MIN) pid_counter = PID_MIN;

	/* Initial limits for all the same */
	proc->p_limit = proc0.p_limit;		/* Copy proc0's limit */
	proc->p_limit->p_refcnt++;

	/* Set the pointers to the root file system and vnode. */
	proc->p_fd->fd_cdir = rootdir;
	vref(proc->p_fd->fd_cdir);
	proc->p_fd->fd_rdir = NULL;

	/* Helios specific stuff */
	proc->p_symlink = NULL;
#ifdef MONITOR
printf("Client 0x%x   PID %d Created\n",*client,(*client)->p.p_pid);
#endif
	return(0);
}

/*
 * Remove a client from the list.
 */
void removeclient(ClientState *client) 
{	struct proc *p = &(client->p);

#ifdef MONITOR
printf("Client 0x%x   PID %d Terminated  uid=%d\n",
	client,p->p_pid,client->ugid & 0xFFFF);
#endif
	/* Remove client from list */
	Wait(&ClientsActive);
	Wait(&ClientsLock);	/* Locking queue lock all clients */
	Remove(&client->node);
	Signal(&ClientsLock);

	/* Free references and memory */
	fdfree(p);
	p->p_limit->p_refcnt--;		/* XXX AMS Limits all the same so far */
	if (--p->p_cred->p_refcnt == 0) {
		crfree(p->p_ucred);
		free(p->p_cred,M_CRED);
	}

	/* This check can be removed later.  It may not be necessary */
	if (p->p_symlink != NULL) {
		printf("removeclient; Warning, Link name '%s' was left over\n",
			p->p_symlink);
		free(p->p_symlink,M_NAMEI);
	}

	free(client,M_CLIENT);
}

/*
 * Force the close of a single client (given PID).
 */
static word is_pid_close(ClientState *client, pid_t pid)
{
	if (client->p.p_pid == pid) {
		client->force_close = TRUE;
		return TRUE;
	} else {
		return FALSE;
	}
}
int force_close_client(pid_t pid)
{	ClientState *client;
	
	Wait(&ClientsLock);	/* Locking queue lock all clients */
	client = (ClientState *) SearchList(&ClientsRoot,
						(WordFnPtr)is_pid_close,pid);
	Signal(&ClientsLock);
	if (client == NULL) 
		return(ENOENT);	/* Failed */
	
	return(0);
}

/*
 * Force the closure of all open files and active clients
 */
static void force_close(ClientState *client)
{
	client->force_close = TRUE;
}
static void kill_all_clients(void)
{
	Wait(&ClientsLock);	/* Locking queue lock all clients */
	WalkList(&ClientsRoot,(WordFnPtr)force_close);
	Signal(&ClientsLock);
}

/*
 * Look for a client which matches the user in the list, and return duplicate
 * if found.
 */
static word is_user(ClientState *client, uid_t uid)
{
	if (client->p.p_cred->p_ruid == uid) {
		return TRUE;
	} else {
		return FALSE;
	}
}
ClientState *finddupclient(uid_t uid, gid_t gid)
{
	ClientState *client, *newclient;
	struct ucred *cr;
	int i;

	/* This maintains downward capability below 1.3 */
	if (uid == 0xFFFF)	return (NULL);

	/* Lock the list while we search for the client. */
	Wait(&ClientsLock);
	if ((client = (ClientState *)
	    SearchList(&ClientsRoot,(WordFnPtr)is_user,uid)) == NULL) {
		Signal(&ClientsLock);
		return(NULL);
	}

	/* If one is found, duplicate it and set the reference pointers */
	if ((newclient = (ClientState *)
			malloc(sizeof(ClientState),M_CLIENT,M_NOWAIT)) == NULL) {
		Signal(&ClientsLock);
		printf("finddupclient; No memory to duplicate client\n");
		return(NULL);	/* No memory to duplicate */
	}

	/* Copy the references */
	bzero(newclient,sizeof(ClientState));	/* Sets ofname + force_close */
	newclient->p.p_cred = client->p.p_cred;
	newclient->p.p_cred->p_refcnt++;
	newclient->p.p_fd = client->p.p_fd;
	newclient->p.p_fd->fd_refcnt++;
	newclient->p.p_limit = client->p.p_limit;
	newclient->p.p_limit->p_refcnt++;

	/* Add to the list */
	AddHead(&ClientsRoot,&newclient->node);

	/* Free the list */
	Signal(&ClientsLock);
	Signal(&ClientsActive);
#ifdef DEBUG
printf ("client found and duplicated 0x%x\n",newclient);
#endif
	/* Initialise other things */
	newclient->p.p_pid = pid_counter++;
	if (pid_counter < PID_MIN) pid_counter = PID_MIN;

	/* Setup the group structure */
	cr = newclient->p.p_ucred;
	for (i=0; (i < cr->cr_ngroups) && (cr->cr_groups[i] != gid); i++);
	if ((i == cr->cr_ngroups) && (cr->cr_ngroups < NGROUPS)) {
		/* Not found, so add group to credentials */
		cr->cr_groups[cr->cr_ngroups++] = gid;
	}

	return(newclient);
}

/*
 * Routines to return a null terminated list of pointers to malloced
 * UfsOpenFile structures.  This gives the full list of open files.
 */
struct add_open_args{
	struct UfsOpenFile **list;
	int *index;
	int max;
};
static void add_open_list(ClientState *client, struct add_open_args *args)
{	struct UfsOpenFile **list = args->list;
	int *index = args->index;
	int max = args->max;
	struct UfsOpenFile *item;
	int len;

	/* An open file or no room ? */
	if ((client->ofname == NULL) || (*index >= max))
		return;

	/* Get some memory for them all */
	len = UfsOpenFileSize + strlen(client->ofname);
	len = ((len >> 2) + 1) << 2;	/* Word align (sort of) */
	if ((item = (struct UfsOpenFile *)malloc(len, M_TEMP, M_NOWAIT))
		== NULL) {
		/* Not enough memory */
		return;
	}

	/* Setup the item */
	item->len = len;
	item->uid = client->ugid & 0xFFFF;
	item->pid = client->p.p_pid;
	strcpy(item->name,client->ofname);

	/* Add the item to the list */
	list[(*index)++] = item;		/* Add to array */
	return;
}
struct UfsOpenFile **get_open_files(int *max)
{	int index;
	struct UfsOpenFile **ret;
	struct add_open_args args;

	/* Initialise */
	*max = TestSemaphore(&ClientsActive)+1;
	index = 0;
	ret = (struct UfsOpenFile **)
		malloc(sizeof(struct UfsOpenFile *) * (*max),
						M_TEMP, M_WAITOK);
	if (ret == NULL)
		return(NULL);

	args.list = ret;
	args.index = &index;
	args.max = *max - 1;

	/* Build the list */
	Wait(&ClientsLock);
	WalkList(&ClientsRoot,(WordFnPtr)add_open_list,&args);
	Signal(&ClientsLock);

	/* Anything there? */
	if (index == 0) {
		free(ret, M_TEMP);
		return(NULL);
	}

	ret[index] = NULL;	/* Null terminated */
	return(ret);
}

/*
 * This is a simple debug routine to display any clients active,
 * their calls and any open files.
 */
static void show_client_active(ClientState *client)
{	char *ofname = client->ofname;
	char *fn = (char *)procname(client->SysCall);
	int pid = client->p.p_pid;

	if (fn == NULL) fn = "<Unknown>";

	if (ofname == NULL)
		printf("Client pid %d, fn %s active\n",pid,fn);
	else	printf("Client pid %d, fn %s, file %s active\n",pid,fn,ofname);

	return;
}

/* 
 * Create a new MCB structure 
 */
MyMCB *NewMyMCB(void)
{
	MyMCB *m;
	
	while ((m = (MyMCB *)malloc(sizeof(MyMCB),M_MCB,M_WAITOK)) == NULL) 
		Delay(OneSec);
	m->mcb.Control = (word *)&m->control;
	m->mcb.Data = (byte *)&m->data;
	return(m);
}

/*
 * Routine to perform a system sync on the disks when the disk is quiet.
 */
/* 
 * Handle Sync Request
 */
int hd_disk_operations(void);
void system_sync(ClientState *client)
{
	struct fd_args fd_args;
	int error,retval=0;

	/* Sync the disk */
	error = syscall(client,sync,(struct syscall_args *)&fd_args,&retval);

	/* Remove the client */
	removeclient(client);
}

/*
 * The MAIN dispatcher.
 */
/* Dispatch until doloop is false. */
void dispatcher(Port reqport)
{
  MyMCB *mymcb = NULL;
  MCB *mcb;
  IOCMsg2 *req;
  ClientState *newclient = NULL;
  ClientState *c;
  Capability *cap;
  void (*fn)();
  int doloop = TRUE;
  int idle_terminate = FALSE;
  int die = FALSE;
  int e;
  word ugid;
  uid_t uid;
  gid_t gid;
  struct ucred *cr;

  /* Initialise HELIOS Filesystem Variables */
  SyncTime = IOCTimeout;

  /* Setup the list of clients and the semaphore */
  InitList(&ClientsRoot);
  InitSemaphore(&ClientsLock,1);
  InitSemaphore(&ClientsActive,0);

  while (doloop) {
  	/* Prepare to receive */
  	if (mymcb == NULL) {
  		mymcb = NewMyMCB();

  		/* Quick Access Variables */
  		mcb = &(mymcb->mcb);
  		req = (IOCMsg2 *)mcb->Control;
  		cap = &(req->Common.Access);
  	}
  	mcb->MsgHdr.Dest = reqport;
  	mcb->Timeout = SyncTime;

  	/* Create a new client if required */ 
  	if (newclient == NULL) 
  		while (createclient(&newclient)) Delay(OneSec);
#ifdef MONITOR
printf("dispatch: waiting for msg\n");
#endif
  	/* Receive the message */
  	e = GetMsg(mcb);
  	if ((e & EG_Mask) == EG_Invalid) {
  		printf("Invalid message received\n");
		continue;
  	}
#ifdef DEBUG
printf("dispatch: e=0x%x\n",e);
#endif
  	/* Process the message */       
  	if (e > 0) {
  		/* Extract the user and group ids */
  		ugid = ((word *)cap)[1];
  		uid = ugid & 0xFFFF;
  		gid = (ugid >> 16) & 0xFFFF;
  		if ((c = finddupclient(uid,gid)) == NULL) {
  			c = newclient;
  			newclient = NULL;

  			/* Add to the HELIOS client list */
  			Wait(&ClientsLock);
  			AddHead(&ClientsRoot,&c->node);
  			Signal(&ClientsLock);
  			Signal(&ClientsActive);

  			if (uid != 0xFFFF) { /* Downward compatability */
#ifdef DEBUG
printf("newclient 0x%x used uid=%d\n",c,uid);
#endif
				/* Set up user id */
				c->p.p_cred->p_ruid = uid;
				c->p.p_cred->p_svuid = uid;
				c->p.p_cred->p_rgid = gid;
				c->p.p_cred->p_svgid = gid;

				/* And in the credentials. */
				cr = c->p.p_ucred;
				cr->cr_uid = uid;
				cr->cr_gid = gid;
				cr->cr_ngroups = 1;
			}
		}
		if (uid == 0xFFFF)
			c->ugid = 0;	/* Downward compatibility */
		else c->ugid = (gid << 16) | uid;

#ifdef MONOTOR
printf("dispatch  uid=%d   gid=%d     Msg %x\n",uid,gid,e); 
#endif
		switch (e & FG_Mask)
		{
		case FG_Open:		fn = do_open;	   break;
		case FG_Create: 	fn = do_create;    break;
		case FG_Locate: 	fn = do_locate;    break;
		case FG_ObjectInfo:	fn = do_objinfo;   break; 
		case FG_ServerInfo:	fn = do_servinfo;  break; 
		case FG_Delete: 	fn = do_delete;    break; 
		case FG_Rename: 	fn = do_rename;    break; 
		case FG_Link:		fn = do_link;	   break; 
		case FG_Protect:	fn = do_protect;   break; 
		case FG_SetDate:	fn = do_setdate;   break; 
		case FG_Refine: 	fn = do_refine;    break; 
		case FG_Revoke: 	fn = do_revoke;    break; 

		/* The private protocols */
		case FG_UfsChown:	fn = do_chown;	   break;
		case FG_UfsMknod:	fn = do_mknod;	   break;
		case FG_UfsMount:	fn = do_mount;	   break;
		case FG_UfsGetfsstat:	fn = do_getfsstat; break;
		case FG_UfsSync:	fn = do_sync;	   break;
		case FG_UfsUnmount:	fn = do_unmount;   break;
		case FG_UfsStat:	fn = do_stat;	   break;
		case FG_UfsLstat:	fn = do_lstat;	   break;
		case FG_UfsLink:	fn = do_hardlink;  break;
		case FG_UfsGetopen:	fn = do_getopen;   break;
		case FG_UfsCloseopen:	fn = do_closeopen; break;
		case FG_UfsGetmem:	fn = do_getmem;    break;

		case FG_Terminate:      
			/* Only root or starting user may shutdown */
			if (rootuser(c)) {
				/* This is a good time to sync. */
				fn = do_sync;

				/* How do we want to terminate */
				switch (e & 0xF) {
				case UfsTermHard:
					kill_all_clients();
					/* and fall through ... */
				case UfsTermNow:
					/* Last indirect request ! */
					doloop = FALSE;
					/* and fall through ... */
				case UfsTermSoft:
					idle_terminate = TRUE;
					break;
				}

				break;
			} else {
				/* Attempted security breach */
				printf("%s %s %d\n",
					"Dispatcher received invalid",
					"shutdown request from user id",
					uid);
				InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
					EC_Error|SS_HardDisk|EG_WrongFn);
				PutMsg(mcb);
				continue;
			}
		case FG_CloseObj:
		default:
			printf("%s %x from user %d\n",
				"Dispatcher received unknown request",
				e, uid);
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
				EC_Error|SS_HardDisk|EG_WrongFn);
			PutMsg(mcb);
			continue;
		}

		/* Fork off the handler */
		if (!Fork(STACKSIZE,fn,sizeof(mymcb)+sizeof(c),mymcb,c)) {
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
				EC_Error|SS_HardDisk|EG_NoMemory);
			PutMsg(mcb);
			continue;
		}
		mymcb = NULL;
	} else if ((e & EG_Mask) == EG_Timeout) {
		/* On a timeout, sync the disks */
		Wait(&ClientsLock);
		AddHead(&ClientsRoot,&newclient->node);
		Signal(&ClientsLock);
		Signal(&ClientsActive);

		if (Fork(STACKSIZE,system_sync,
		    sizeof(newclient),newclient)) {
			newclient = NULL;
		} else {
			printf("Could not perform a system sync\n");
		}

		/* Are we waiting to terminate, and this the only one ? */
		if (idle_terminate && (TestSemaphore(&ClientsActive) <= 1))
			doloop = FALSE;
	} else {
		printf("dispatcher error 0x%x in receiving message\n",e);
	}
  }

  /* Loop complete, so create sync client */
  if (newclient == NULL) 
  	while (createclient(&newclient)) Delay(OneSec);

  /* Prepare to receive anything else ? */
  if (mymcb == NULL) {
  	mymcb = NewMyMCB();

  	/* Quick Access Variables */
  	mcb = &(mymcb->mcb);
  	req = (IOCMsg2 *)mcb->Control;
  	cap = &(req->Common.Access);
  }

  /*
   * Wait until all clients have completed their work, allowing only 
   * a hard terminate message.
   */
  doloop = 0;
  while (TestSemaphore(&ClientsActive)) {
	InitMCB(mcb,0,reqport,NullPort,0);
	mcb->Timeout = OneSec;

	/* Get the message */
  	e = GetMsg(mcb);
  	if ((e & EG_Mask) == EG_Invalid) {
  		printf("Invalid message received\n");
		continue;
  	}

	/* Is this a terminate */
	if (e > 0) {
  		ugid = ((word *)cap)[1];
  		uid = ugid & 0xFFFF;
		if (uid == 0xFFFF) uid = 0;
		if (((uid==0) || (uid==rootuid)) && 
		    ((e & FG_Mask) == FG_Terminate)) {
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,Err_Null);
			if ((e & 0xF) == UfsTermHard)
				kill_all_clients();
			else if ((e & 0xF) == UfsTermDie) {
				kill_all_clients();
				die = TRUE;
				break;
			}
		} else {
			/* Invalid message */
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,
				EC_Error|SS_HardDisk|EG_WrongFn);
		}
		PutMsg(mcb);
	} else if ((e & EG_Mask) == EG_Timeout) {
		if (!(doloop++ % 10))
		   printf("Dispatcher terminating; %s\n",
				"waiting for clients to exit");
	}
  }

  /* Free outstanding memory */
  free(mymcb,M_MCB);

  /* Sync the system */
  Wait(&ClientsLock);
  AddHead(&ClientsRoot,&newclient->node);
  Signal(&ClientsLock);
  Signal(&ClientsActive);
  if (!Fork(STACKSIZE,system_sync,sizeof(newclient),newclient)) {
	printf("ERROR: Terminate could not perform a system sync\n");
  }

  /* Wait until all system sync is complete */
  doloop = 0;
  while (TestSemaphore(&ClientsActive)) {
	if (!(doloop++ % 10))
	   printf("Dispatcher; waiting for system sync to complete\n");
	Delay(OneSec);

	/* If no disk operations and someone still around, after 10s error! */
	if (die && (!hd_disk_operations()) && (doloop > 10))	break;

	/* If disk operations after 2 mins, and die just exit */
	if (die && (hd_disk_operations()) && (doloop > 120))	break;
  }

  /* Any panic ? */
  if ((e = TestSemaphore(&ClientsActive)) > 0) {
	printf("dispatcher panic, terminating when %d calls still active\n");
	WalkList(&ClientsRoot,(WordFnPtr)show_client_active,NULL);
  }
}


