/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 14 August 1992                                               */
/* File: private.h                                                    */
/*                                                                    */
/*
 * This file contains the codes and function prototypes of the private
 * operations to the file server.
 */
/*
 * $Id: private.c,v 1.2 1993/04/08 14:49:37 nickc Exp $ 
 * $Log: private.c,v $
 * Revision 1.2  1993/04/08  14:49:37  nickc
 * (Alex S) added support for lstat()
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
#include "stat.h"
#include "proc.h"
#define NFS
#include "mount.h"
#undef NFS
#include "vnode.h"
#include "fcntl.h"

#include <helios.h>
#include <syslib.h>
#include <stdarg.h>
#include <servlib.h>
#include <codes.h>
#include <gsp.h>
#include <device.h>
#include <dirent.h>

#define UFSOPENFILE
#include "dispatch.h"
#include "private.h"

/*
 * Prototypes for HELIOS->UNIX system call routines (direct into kernel).
 */

/*
 * Prototypes for UNIX system call routines.
 */
extern chown(), mknod(), mount(), unmount(), getfsstat();
extern sync(), link(), stat(), lstat();

/*
 * Change the ownership of a file.
 */
void do_chown(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCChown *req = (IOCChown *)m->Control;
	int uid = req->uid;
	int gid = req->gid;
	char fullname[MAXPATHLEN];
	struct open_args open_args;
	int error, retval;

	/* Get the target file to chown */
	makepath(mymcb,fullname);

	/* Setup the parameters */
	open_args.fname = fullname;
	open_args.mode = uid;
	open_args.crtmode = gid;

	/* Call the routine */
	error = syscall(client,chown,
		(struct syscall_args *)&open_args,&retval);
#ifdef DEBUG
	IOdebug("chown on %s uid=%d  gid=%d   error=%d",fullname,uid,gid,error);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else
		ErrorMsg(m,Err_Null);

	do_complete(client,mymcb);
}

/*
 * Build a special file.
 */
void do_mknod(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCChown *req = (IOCChown *)m->Control;
	int fmode = req->uid;
	int dev = req->gid;
	char fullname[MAXPATHLEN];
	struct open_args open_args;
	int error, retval;

	/* Get the target file to chown */
	makepath(mymcb,fullname);

	/* Setup the parameters */
	open_args.fname = fullname;
	open_args.mode = fmode;	
	open_args.crtmode = dev;

	/* Call the routine */
	error = syscall(client,mknod,
		(struct syscall_args *)&open_args,&retval);
#ifdef DEBUG
	IOdebug("mknod on %s fmode=%d  dev=%d   error=%d",
			fullname,fmode,dev,error);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else
		ErrorMsg(m,Err_Null);

	do_complete(client,mymcb);
}

/*
 * Mount a file system.
 */
void do_mount(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCUfs *req = (IOCUfs *)(m->Control);
	IOCNfs *req2 = (IOCNfs *)(m->Control);
	struct mount_args mount_args;
	struct ufs_args *ufs_args;
	struct nfs_args *nfs_args;
	char dirname[MAXPATHLEN], specname[MAXPATHLEN];
	int error,retval;
	
	/* Get the directory file name */
	makepath(mymcb,dirname);

	/* Setup the arguments */
	mount_args.type = req->Type;
	mount_args.dir = dirname;	
	mount_args.flags = req->Flags;

	if (mount_args.type == MOUNT_UFS) {
		/* Adjust the arguments */
		mount_args.data = (caddr_t)(m->Data + req->Data);
		ufs_args = (struct ufs_args *)mount_args.data;

		/* Get the device special name */
		if (req->Common.Next >= req->Common.Name)
			req->Common.Next = req->DevName + 
				   (req->Common.Next - req->Common.Name);	
		req->Common.Name = req->DevName;
		makepath(mymcb,specname);	
		ufs_args->fspec = specname;
	} else {
		mount_args.data = (caddr_t)(m->Data + req2->Data);
		nfs_args = (struct nfs_args *)mount_args.data;
		nfs_args->addr = (struct sockaddr *)(m->Data + req2->Addr);
		nfs_args->fh = (nfsv2fh_t *)(m->Data + req2->Fh);
		nfs_args->hostname = (char *)(m->Data + req2->Hostname);
	}	

	/* Call the routine */
	error = syscall(client,mount,
		(struct syscall_args *)&mount_args,&retval);
#ifdef DEBUG
	printf("mount %s on %s  type=%s  error=%d\n",
			dirname,
			(mount_args.type == MOUNT_UFS)?
				ufs_args->fspec : nfs_args->hostname,
			(mount_args.type == MOUNT_UFS)?"UFS":"NFS",
			error);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else
		ErrorMsg(m,Err_Null);

	do_complete(client,mymcb);
}

/*
 * Get all the file system statistics.
 */
void do_getfsstat(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCGetfsstat *req = (IOCGetfsstat *)(m->Control);
	struct getfsstat_args getfsstat_args;
	int error,retval;
	byte *buf, *olddata;
	int i;

	/* Keep for later reset */
	olddata = m->Data;

	/* Setup the arguments */
	getfsstat_args.bufsize = req->Bufsize;
	getfsstat_args.flags = req->Flags;
	if (req->Bufsize > IOCDataMax) {
		if ((buf = (byte *)malloc(req->Bufsize,M_TEMP,M_NOWAIT)) == NULL) {
			ErrorMsg(m,EC_Error|EG_Errno|ENOMEM);
			do_complete(client,mymcb);
			return;
		}
		getfsstat_args.buf = (struct statfs *)buf;
		m->Data = buf;
	} else {
		buf = NULL;
		getfsstat_args.buf = (struct statfs *)m->Data;
	}		

	/* Call the routine */
	error = syscall(client,getfsstat,
		(struct syscall_args *)&getfsstat_args,&retval);
#ifdef DEBUG
	printf("getfsstat error=%d  retval=%d\n",error,retval);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else {
		/* Send back the size read */
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);
		MarshalWord(m,retval);
		MarshalData(m,retval*sizeof(struct statfs),
				(byte *)getfsstat_args.buf); /* Itself ? */
		PutMsg(m);
#ifdef DEBUG
{
int i;
for (i=0; i<retval; i++)
	printf("do_getfsstats: %s mounted on %s\n",
		getfsstat_args.buf[i].f_mntfromname,
		getfsstat_args.buf[i].f_mntonname);
}
#endif
	}
	if (buf) free(buf,M_TEMP);
	m->Data = olddata;		/* And restore if necessary */
	do_complete(client,mymcb);
}

/*
 * Get the stat for a specific file.
 */
void do_stat(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	struct stat_args stat_args;
	int error,retval;
	char filename[MAXPATHLEN];

	/* Get the directory file name */
	makepath(mymcb,filename);

	/* Setup the arguments */
	stat_args.fname = filename;
	stat_args.ub = (struct stat *)m->Data; /* Must be big enough */

	/* Call the routine */
	error = syscall(client,stat,
		(struct syscall_args *)&stat_args,&retval);
#ifdef DEBUG
	printf("do_stat error=%d  retval=%d\n",error,retval);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else {
		/* Send back the size read */
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);
		MarshalData(m,sizeof(struct stat),
				(byte *)stat_args.ub);	/* Put in itself */
		PutMsg(m);
	}

	do_complete(client,mymcb);
}

/*
 * Get the lstat for a specific file.
 */
void do_lstat(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	struct stat_args stat_args;
	int error,retval;
	char filename[MAXPATHLEN];

	/* Get the directory file name */
	makepath(mymcb,filename);

	/* Setup the arguments */
	stat_args.fname = filename;
	stat_args.ub = (struct stat *)m->Data; /* Must be big enough */

	/* Call the routine */
	error = syscall(client,lstat,
		(struct syscall_args *)&stat_args,&retval);
#ifdef DEBUG
	printf("do_lstat error=%d  retval=%d\n",error,retval);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else {
		/* Send back the size read */
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);
		MarshalData(m,sizeof(struct stat),
				(byte *)stat_args.ub);	/* Put in itself */
		PutMsg(m);
	}

	do_complete(client,mymcb);
}

/* 
 * Handle Sync Request
 */
int hd_disk_operations(void);
void do_sync(MyMCB *mymcb, ClientState *client)
{
	struct fd_args fd_args;
	int error,retval=0;

#ifdef SHOWCALLS
	printf("sync; %s + sync memory=0x%x",fullname,req->flag);
#endif

	fd_args.fd = TRUE;

	/* Sync the disk */
	error = syscall(client,sync,(struct syscall_args *)&fd_args,&retval);

	/* Wait until all disk operations are complete before replying */
	while (hd_disk_operations())
		Delay(100000);	/* 1/10 sec */

	if (error) ErrorMsg(&mymcb->mcb,EC_Error|EG_Errno|error);
	else ErrorMsg(&mymcb->mcb,Err_Null);

	do_complete(client,mymcb);
}

/*
 * Unmount a mounted directory.
 */
void do_unmount(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCChown *req = (IOCChown *)m->Control;
	int flags = req->uid;
	char fullname[MAXPATHLEN];
	struct mode_args mode_args;
	int error, retval;

	/* Get the target to unmount */
	makepath(mymcb,fullname);

	/* Setup the parameters */
	mode_args.name = fullname;
	mode_args.mode = flags;

	/* Call the routine */
	error = syscall(client,unmount,
		(struct syscall_args *)&mode_args,&retval);
#ifdef DEBUG
	printf("unmount on %s flags=%x  err=%d  retval=%d\n",
		fullname,flags,error,retval);
#endif

	/* The result */
	if (error)
		ErrorMsg(m,EC_Error|EG_Errno|error);
	else
		ErrorMsg(m,Err_Null);

	do_complete(client,mymcb);
}

/*
 * Create a hard link to a target.
 */
void do_hardlink(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	struct rename_args rename_args;
	char targname[MAXPATHLEN], linkname[MAXPATHLEN];
	int error,retval;
	
	/* Get the target file name */
	makepath(mymcb,targname);	

	/* Get the link file name */
	if (req->Common.Next >= req->Common.Name)
		req->Common.Next = req->Arg.ToName + 
				   (req->Common.Next - req->Common.Name);
	req->Common.Name = req->Arg.ToName;
	makepath(mymcb,linkname);	

#ifdef SHOWCALLS
	IOdebug("do_link ('%s'->'%s' called",targname,linkname);
#endif

	rename_args.from = targname;
	rename_args.to = linkname;
	error = syscall(client,link,(struct syscall_args *)&rename_args,
			&retval);
#ifdef DIAGNOSTIC
	printf("LINK (hard): (%s)->(%s): error=%d    retval=0x%x\n",
		targname,linkname,error,retval);
#endif
	if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
	else ErrorMsg(m,Err_Null);
		
	do_complete(client,mymcb);
}


/*
 * Return a list of open files to the user.
 */
void do_getopen(MyMCB *mymcb, ClientState *client)
{	MCB *m = &mymcb->mcb;
	byte *data;
	struct UfsOpenFile **list;
	struct UfsOpenFile *item;
	int index = 0;
	int seq = 0;
	int size;
	int numentries;

	if ((list = get_open_files(&numentries)) != NULL) {
loop:
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,seq);
		MarshalWord(m,numentries);
		size = UfsOpenFileSize;	/* Must be room for 1 more */
		data = m->Data;
		while (((item = list[index]) != NULL) && (size < IOCDataMax)) {
			/* Move data into msg */
			memcpy(data,item,item->len);
			data += item->len;
			size += item->len;

			free(item, M_TEMP);
			index++;
		}

		/* Create last null entry */
		bzero(data,UfsOpenFileSize);
		m->MsgHdr.DataSize = size;

		/* Really the last, or is there still more */
		if (list[index]) {
			/* There is more data coming */
			m->MsgHdr.Flags |= MsgHdr_Flags_preserve;
			PutMsg(m);
			seq++;
			goto loop;
		} else {
			/* This is really the last */
			m->MsgHdr.FnRc |= ReadRc_EOD;
			PutMsg(m);
		}

		/* Return the list */
		free(list, M_TEMP);
	} else {
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,0);
		MarshalWord(m,0);
		PutMsg(m);
	}

	do_complete(client,mymcb);
}

/*
 * Force the closure of an already open file.
 */
void do_closeopen(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)m->Control;
	pid_t pid = (pid_t)req->Arg.Mode;
	int error, retval;


	/* Check if the user is allowed to force the closure */
	if (rootuser(client)) {
		/* Call direct.  Note! Does not access UNIX kernel. */
		error = force_close_client(pid);

#ifdef DEBUG
	IOdebug("Force closure on pid=%d   error=%d",pid,error);
#endif

		/* The result */
		if (error)
			ErrorMsg(m,EC_Error|EG_Errno|error);
		else
			ErrorMsg(m,Err_Null);
	} else ErrorMsg(m,EC_Error|SS_HardDisk|EG_WrongFn);


	do_complete(client,mymcb);
}

/*
 * Return the memory statistics.
 */
void do_getmem(MyMCB *mymcb, ClientState *client)
{	MCB *m = &mymcb->mcb;
	byte *temp;
	
	/* Just send the man the stats */
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,0);
	temp = m->Data;
	m->Data = (byte *)&kmemstats;
	m->MsgHdr.DataSize = sizeof(struct kmemstats)*M_LAST;
	PutMsg(m);
	m->Data = temp;

	do_complete(client,mymcb);
}


