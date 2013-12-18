/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 14 August 1992                                               */
/* File: privfns.c                                                    */
/*                                                                    */
/* 
 * This file contains the private functions to call the Helios 
 * BSD 4.4 file server in order to provide the actual UNIX functions.
 */
/* $Id: privfns.c,v 1.5 1993/04/08 14:49:49 nickc Exp $ */
/* $Log: privfns.c,v $
 * Revision 1.5  1993/04/08  14:49:49  nickc
 * (Alex S) added support for lstat()
 *
 * Revision 1.4  1993/04/08  11:53:27  nickc
 * fixed compile time error
 *
 * Revision 1.3  1993/03/29  10:01:01  al
 * Add flock support.
 *
 * Revision 1.2  1992/10/13  11:51:10  al
 * Fixed syntax to compile for C40
 *
 * Revision 1.1  1992/09/16  10:01:43  al
 * Initial revision
 * */

#define NFS
#include "param.h"
#include "mount.h"
#include "socket.h"
#define KERNEL
#include "ioctl.h"
#undef KERNEL
#include "unistd.h"

#include <stdlib.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#undef ERANGE
#include <sys/errno.h>

#define UFSOPENFILE
#include "../private.h"

#define MAXPATH	200

/*
 * Extract the posix error code.
 */
extern int errno;
static int posix_error(word e)
{
	if( e >= 0 ) {
		return (int)e;
	}
	if ( (e&SS_Mask) == SS_Loader || (e&SS_Mask) == SS_TFM ) {
		return ENOEXEC;
	}

	switch( e&EG_Mask )
	{
	case EG_Errno:		return (int)(e & EO_Mask);
	case EG_NoMemory:	return ENOMEM;
	case EG_Create:		return EEXIST;
	case EG_Delete:		return EIO;
	case EG_Protected: 	return EACCES;
	case EG_Timeout:	return EAGAIN;
	case EG_Unknown:	return ENOENT;
	case EG_FnCode:		return EIO;
	case EG_Name:		return ENOENT;
	case EG_Invalid:	return EINVAL;
	case EG_InUse:		return EBUSY;
	case EG_Congested:	return EIO;
	case EG_WrongFn:	return EIO;
	case EG_Exception:
	{
		if( ( e & EE_Mask ) == EE_Abort ) return EINTR;
		elif( (e & EE_Signal) == EE_Signal )
		{
			/* by returning EE_Signal, servers can provoke a signal */
			/* routine						*/
			return EINTR;
		}
		return EIO;
	}
	case EG_Broken:		
		if( (e & EO_Mask) == EO_Pipe ) 
		{
			return EPIPE;
		}
		return EIO;
	case EG_WrongSize:	return EIO;
	case EG_Parameter:	return EINVAL;
	default:		return EIO;
	}
}

/*
 * Given two file names, this routine returns an object whose
 * directory is common to both, and adjusts the pointers in the file
 * names to point to the unique bit of the filenames.
 * This routine was swiped and adjusted from mv (thanks Paul).
 */
Object *Find_Common(char **src, char **dst)
{
	char	*oldname, *newname;
	char	*startold, relpath[MAXPATH];

	startold = oldname = *src;	/* get ptrs to actual str ptrs */
	newname = *dst;

	/* find common relative path */
	/* first find point where names differ */
	while (*++oldname == *++newname && *oldname != '\0') ;/*null stat*/

	/* fix problems where a pathname includes another */
	/* i.e. /test is incl. in /test2 partway */
	if (*oldname == '/' && *newname != '\0') {
		/* dir completely incorporated into other */
		oldname--;	/* get past /test/ -> /test2 */
		newname--;
	}
	while (*oldname != '/')	{
		/* dir partly incorporated into other */
		--newname;	/* get past /abc123 -> /abc987 */
		--oldname;
	}

	memcpy(relpath, startold, (size_t) (oldname - startold));
	relpath[oldname-startold] = '\0';

	oldname++; /* inc past leading '/' */

	 /* if not zero length i.e. not in same dir as oldname */
	if (*newname != '\0')
		newname++; /* inc past '/' */

	*src = oldname;	/* pass new path positions back */
	*dst = newname;

	/* return obj for common parent to both paths */
	return(Locate(CurrentDir, relpath));
}

/*
 * Change the uid/gid of the given file.
 */
int chown(char *path, int uid, int gid)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
  
	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;

	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsChown);
	MarshalCommon(&m, cdobj(), path);
	MarshalWord(&m, uid);
	MarshalWord(&m, gid);

 
	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * Create the special file
 */
int mknod(char *path, int fmode, int dev)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
  
	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;

	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsMknod);
	MarshalCommon(&m, cdobj(), path);
	MarshalWord(&m, fmode);
	MarshalWord(&m, dev);

 
	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * Call the mount function to mount a remote disk.
 */
int mount(int type, const char *dir, int flags, void *data)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	Object *dirobj, *specobj, *comobj;
	struct ufs_args *ufs_args = (struct ufs_args *)data;
	struct nfs_args *nfs_args = (struct nfs_args *)data;
	char *dirch, *specch;
	  
	/* Look for target object */
	if ((dirobj = Locate(cdobj(),(char *)dir)) == NULL) {
		errno = ENOENT;
		return(-1);
	}

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsMount);

	
	if (type == MOUNT_UFS) {
		/* Build the special dev name into the message */
		if ((specobj = Locate(cdobj(),ufs_args->fspec)) == NULL) {
			errno = ENODEV;
			FreePort(reply);
			return(-1);
		}
		dirch = dirobj->Name;
		specch = specobj->Name;
		if ((comobj = Find_Common(&dirch,&specch)) == NULL) {
			errno = ENODEV;
			FreePort(reply);
			return(-1);
		}

		/* Place target and special into common */
		MarshalCommon(&m,comobj,dirch);
		MarshalWord(&m,type);
		MarshalWord(&m,flags);

		MarshalString(&m,specch);
		MarshalOffset(&m);
		MarshalData(&m,sizeof(struct ufs_args),(byte *)ufs_args);
	} else if (type == MOUNT_NFS) {
		/* Break up NFS_ARGS and place into message header */
		MarshalCommon(&m,cdobj(),(char *)dir);
		MarshalWord(&m,type);
		MarshalWord(&m,flags);

		MarshalOffset(&m);
		MarshalData(&m,sizeof(struct nfs_args),(byte *)nfs_args);
		MarshalOffset(&m);
		MarshalData(&m,sizeof(struct sockaddr),(byte *)nfs_args->addr);
		MarshalOffset(&m);
		MarshalData(&m,sizeof(nfsv2fh_t),(byte *)nfs_args->fh);
		MarshalString(&m,nfs_args->hostname);
	} else {
		errno = ENODEV;
		FreePort(reply);
		return(-1);
	}

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * Call the getfsstat to get all filesystem statistics.
 * Note the appearance of dir at the end.  This is non standard since
 * more than one fileserver may be running and this is a path to the file
 * server.
 */
int getfsstat(struct statfs *buf, long bufsize, int flags, char *dir)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsGetfsstat);

	/* Setup the arguments */
	MarshalCommon(&m,cdobj(),dir);
	MarshalWord(&m,bufsize);
	MarshalWord(&m,flags);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	m.Data    = (byte *)buf; 	   
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}
	return(Control_V[0]);
}

/*
 * stat system call.
 */
int stat(char *fname, struct stat *ub)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsStat);

	/* Setup the arguments */
	MarshalCommon(&m,cdobj(),fname);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	m.Data    = (byte *)ub;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}
	return(0);
}

/*
 * lstat system call.
 */
int lstat(char *fname, struct stat *ub)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */

 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsLstat);

	/* Setup the arguments */
	MarshalCommon(&m,cdobj(),fname);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	m.Data    = (byte *)ub;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}
	return(0);
}

/*
 * Execute the sync function to sync all the filesystems.
 */
int sync(char *dir)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsSync);

	/* Setup the arguments */
	MarshalCommon(&m,cdobj(),dir);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * Unmount system call.
 */
int unmount(const char *pathp, int flags)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V;
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsUnmount);

	/* Setup the arguments */
	MarshalCommon(&m,cdobj(),(char *)pathp);
	MarshalWord(&m,flags);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * Call the link function to creat a hard link to a file/dir.
 */
int link(char *target, char *linkname)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;
	Object *targobj, *linkobj, *comobj;
	char *targch, *linkch;
	char linkbuf[MAXPATH], tmp[MAXPATH];
	  
	/* Look for target object */
	if ((targobj = Locate(cdobj(),(char *)target)) == NULL) {
		errno = ENOENT;
		return(-1);
	}

	/* Build link name */
	if (*linkname == '/') {
		/* Absolute path given */
		strcpy(linkbuf,linkname);
		linkch = (char *)strrchr(linkbuf,'/');  /* We know it is there */
		*linkch = '\0';		/* Seperate the two */
		strcpy(tmp,++linkch);

		/* Find destination */
		if ((linkobj = Locate(cdobj(),linkbuf)) == NULL) {
			errno = ENOENT;
			return(-1);
		}

		/* Build destination */
		strcpy(linkbuf,linkobj->Name);
		strcat(linkbuf,tmp);
	} else {
		/* This is relative from the current directory */
		strcpy(linkbuf,((Object *)cdobj())->Name);
		strcat(linkbuf,"/");
		strcat(linkbuf,linkname);
	}

	/* Extract the common sections */
	targch = targobj->Name;
	linkch = linkbuf;
	if ((comobj = Find_Common(&targch,&linkch)) == NULL) {
		errno = ENODEV;
		return(-1);
	}

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsLink);

	
	/* Place target and link into common */
	MarshalCommon(&m,comobj,targch);
	MarshalString(&m,linkch);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(-1);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	FreePort(reply);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
}

/*
 * This is the ioctl function to the ufs file server.
 */
int ioctl(int fdes, unsigned long cmd, caddr_t cmarg)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	Stream *fdst = (Stream *)fdstream(fdes);
	int size = IOCPARM_LEN(cmd);
	int input = cmd & IOC_IN;

	/* Ensure no overflow */
	if (size > IOCDataMax) {
		errno = ENOTTY;
		return(-1);
	}

	/* Setup and send the message */
	m.Control = Control_V;
	m.Data = (byte *)cmarg;
	if (cmd & IOC_OUT)
		bzero(cmarg,size);
	m.Timeout = IOCTimeout;
	InitMCB(&m,MsgHdr_Flags_preserve,fdst->Server,fdst->Reply,
		(input ? FG_SetInfo : FG_GetInfo) | FF_Ioctl);
	MarshalWord(&m,cmd);	/* Command */
	MarshalWord(&m,0);	/* Data offset */
	if (input) m.MsgHdr.DataSize = size;
	PutMsg(&m);

	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, fdst->Reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	if (e < 0) {
	 	errno = posix_error(e);
		return(-1);
	}

	return(0);
	
}

/*
 * This is the flock call to the UFS file server.
 */
int flock(int fd, int cmd)
{
	Stream *stream = (Stream *)fdstream(fd);
	word e;
	MCB m;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];

	/* Initialise the MCB */
	m.Control = Control_V;
	m.Data = Data_V;
	m.Timeout = -1;
	
	Wait(&stream->Mutex);

	/* Setup the message */
	InitMCB(&m,MsgHdr_Flags_preserve,
		stream->Server,stream->Reply,FC_GSP+FG_UfsFlock);
	MarshalWord(&m,cmd);
	e = PutMsg(&m);

	/* Get the reply (if no error) */
	if (e == Err_Null) {
		InitMCB(&m,MsgHdr_Flags_preserve,stream->Reply,NullPort,0);
		e = GetMsg(&m);
	}

	Signal(&stream->Mutex);

	if (e != Err_Null) {
		errno = posix_error(e);
		return(-1);
	}
	return(0);
}


/*
 * Routine which will return a malloced list of pointers to malloced
 * UfsOpenFile structures.  This will return a null terminated list
 * of open files.  The value returned is the maximum number of value the
 * array returned will cater for.  Path indicates the file server.
 */
struct UfsOpenFile **getopenfiles(char *path, int *max)
{	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	byte *data;
	Port reply;
	int index;
	struct UfsOpenFile **ret;
	struct UfsOpenFile *item;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsGetopen);

	
	/* Place path into common */
	MarshalCommon(&m,cdobj(),path);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(NULL);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	if (e < 0) {
		FreePort(reply);
	 	errno = posix_error(e);
		return(NULL);
	}

	/* Any data ? */
	*max = Control_V[0];
	if (*max == 0) {
		FreePort(reply);
		errno = 0;
		return(NULL);
	}

	/* Create the maximum array */
	ret = (struct UfsOpenFile **)
			Malloc(sizeof(struct UfsOpenFile *) * (*max));
	index = 0;

	/* Copy the malloced data across into the array */
loop:
	data = Data_V;
	for (item = (struct UfsOpenFile *)data;
	     item->len != 0;
	     data += item->len, item = (struct UfsOpenFile *)data, index++) {
		/* Copy across the data */
		ret[index] = (struct UfsOpenFile *)Malloc(item->len);
		memcpy(ret[index],data,item->len);
	}

	if ((m.MsgHdr.FnRc & ReadRc_EOD) == 0) {
		/* There is more on the way */
		InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
		m.Timeout = IOCTimeout;
		e = GetMsg(&m);
		if (e >= 0) goto loop;
	}
	ret[index] = NULL;
	
	FreePort(reply);
	return(ret);

}

/*
 * This routine will force the closure of a file, given the pid.
 * Note:  Only the root user or the startup client may close the file.
 */
int forceopenclosed(char *path, int pid)
{
	MCB m;
	word e;
	word Control_V[IOCMsgMax];
	byte Data_V[IOCDataMax];
	Port reply;

	/* Prepare MCB for marshalling */
 	reply = NewPort ();					
	m.Control = Control_V;
	m.Data    = Data_V; 	   
	m.Timeout = IOCTimeout;
	InitMCB(&m, MsgHdr_Flags_preserve, MyTask->IOCPort, reply, 
		FC_GSP | SS_HardDisk | FG_UfsCloseopen);

	
	/* Place path and pid into common */
	MarshalCommon(&m,cdobj(),path);
	MarshalWord(&m,pid);

	/* Send the message to the server*/
	e = PutMsg(&m);
	if (e != Err_Null) {
		FreePort(reply);
		errno = posix_error(e);
		return(NULL);
	}
 	
	/* Expect result response from the ufs file-server. */
	InitMCB(&m, MsgHdr_Flags_preserve, reply, NullPort, 0);
	m.Timeout = IOCTimeout;
	e = GetMsg(&m);
	if (e < 0) {
		FreePort(reply);
	 	errno = posix_error(e);
		return(-1);
	}
	return(0); 
}
