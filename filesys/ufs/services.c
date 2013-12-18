/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 7 August 1992                                                */
/* File: services.c                                                   */
/*                                                                    */
/*
 * This file contains the service routines for the dispatcher.
 * It also contains the service support routines.
 * Other service routines for open files and directories may be found
 * in fileserv.c and dirserv.c respectively.
 */
/*
 * $Id: services.c,v 1.3 1992/11/05 15:11:48 al Exp al $ 
 * $Log: services.c,v $
 * Revision 1.3  1992/11/05  15:11:48  al
 * Added support for symbolic links external to UFS.
 *
 * Revision 1.2  1992/10/05  16:27:12  al
 * Removed spurios debugs
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
#include "quota.h"
#include "inode.h"
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
#include <dirent.h>

#include "dispatch.h"

/*
 * Prototypes for HELIOS->UNIX system call routines (direct into kernel).
 */
extern sysdelete(), syschmod();

/*
 * Prototypes for UNIX system call routines.
 */
extern stat(), statfs(), open(), mkdir(), rename(), close(), utimes(), sync();
extern lstat(), symlink(), readlink();

/*
 * Utilities.
 */
void pathcat(string s1, string s2)
{	string tmp=s1;

	while( *s1 ) s1++;
	if( *(s1-1) != c_dirchar && *s2 != c_dirchar ) *s1++ = c_dirchar;
	while( (*s1++ = *s2++) != 0 );
	s1--;	/* Back to 0 */
	while ((*(--s1) == '/') && (s1 > tmp)) *s1 = 0;	 /* Trap / terminate */
}


/*
 * The UNIX <-> HELIOS access mode mask matrix routines.
 */
static AccMask mode2mask(word type, unsigned int mode)
{
	AccMask mask = 0;
	switch( type )
	{
	case Type_Directory:
	case Type_Link:
		if( mode & 1 ) mask |= AccMask_W;
		if( mode & 2 ) mask |= AccMask_W;		
		if( mode & 4 ) mask |= AccMask_R;
		break;
		
	case Type_File:
	default:
		if( mode & 1 ) mask |= AccMask_E;
		if( mode & 2 ) mask |= AccMask_W;
		if( mode & 4 ) mask |= AccMask_R;
		break;
	}
	return mask;
}

Matrix mode2matrix(word type, unsigned int mode)
{
	Matrix matrix = 0;
	matrix |= mode2mask(type, mode       & 07) << 24;
	matrix |= mode2mask(type,(mode >> 3) & 07) << 16;
	matrix |= mode2mask(type,(mode >> 3) & 07) <<  8;
	matrix |= mode2mask(type,(mode >> 6) & 07) <<  0;

	matrix |= AccMask_A;
	matrix |= AccMask_D*0x00010101;

	switch( type )
	{
	case Type_Directory:
	case Type_Link:
		matrix |= AccMask_V;
		matrix |= (AccMask_X|AccMask_Y)*0x00010100;
		matrix |= AccMask_Z*0x01000000;
		break;
	}

	return matrix;
}

static u_int mask2mode(enum vtype type, AccMask mask)
{
	u_int mode = 0;
	
	switch (type)
	{
	case VDIR:
	case VLNK:
		if (mask & AccMask_W) mode |= 2;
		if (mask & AccMask_R) mode |= 5;	/* 1 | 4 */
		break;
	case VREG:
	default:
		if (mask & AccMask_E) mode |= 1;
		if (mask & AccMask_W) mode |= 2;
		if (mask & AccMask_R) mode |= 4;
		break;
	}
	return(mode);
}

u_int matrix2mode(enum vtype type, Matrix matrix)
{
	unsigned int mode = 0;

	mode |= mask2mode(type,(matrix >> 24)&0377);
/*	mode |= mask2mode(type,(matrix >> 16)&0377) << 3; */
	mode |= mask2mode(type,(matrix >> 8)&0377) << 3;
	mode |= mask2mode(type,(matrix >> 0)&0377) << 6;
	
	return (mode);
}

/*
 * Create the actual path name on the disk.
 */
void makepath(MyMCB *mymcb, char *buf)
{
	MCB *mcb = &mymcb->mcb;
	IOCCommon *req = (IOCCommon *)mcb->Control;
	int context = req->Context;
	int name = req->Name;
	int next = req->Next;
	char *cpath = mcb->Data+context;
	char *tpath = mcb->Data+name;
	char *p;
	int len;
		
#ifdef MONITOR
printf("makepath context=%d",context);
printf("makepath name=%d",name);
printf("makepath next=%d",next);
printf("makepath cpath='%s'",cpath);
printf("makepath tpath='%s'",tpath);
#endif

	/* If next is at the end of context string, step on to name */
	if ((next < name) && (mcb->Data[next] == 0)) next = name;
	
	/* If there is no context string, or we have already used some 
	 * of the the name, search from the root.  Otherwise get hold
	 * of the context object.
	 */
	if ((context == -1) || ((name > 0) && (next >= name))) {
		next -= name;
		cpath = "/";
	} else {
		cpath += next;
		next = 0;
	}
#ifdef DEBUG
printf("dispatcher: makepath cpath='%s' tpath='%s'",cpath,tpath);
#endif	
	if ((name == -1) || (tpath[next] == 0)) tpath = "";
	else tpath += next;
	
	/* Make the path name */
	strcpy(buf,"/");
	while (*cpath == c_dirchar) cpath++;
	strcat(buf,cpath);
	p = buf+strlen(buf);

	if (p == (buf+1)) p = buf;
	
	for (;;) {
		while (*tpath == c_dirchar) tpath++;
		len = splitname(p+1,c_dirchar,tpath);
		if (len == 0) break;
		
		if ((p[1] == '.') && (p[2] == 0)) {
			tpath += len;
			continue;
		}

		if ((p[1] == '.') && (p[2] == '.') && (p[3] == 0)) {
			while ((p != buf) && (*p != c_dirchar)) p--;
			*p = 0;
		} else {
			*p = c_dirchar;
			p+= len;
		}
		
		tpath += len;
	}
	
	if (*buf == '\0') strcpy(buf,"/");

#ifdef MONITOR
printf("dispatcher: makepath cpath='%s' tpath='%s' buf='%s'",cpath,tpath,buf);
#endif
}

/*
 * Create an error message, and return to sender.
 */
void ErrorMsg(MCB *mcb, word err)
{
	err |= SS_HardDisk;
	
	if( mcb->MsgHdr.Reply == NullPort ) return;

#ifdef DEBUG
printf("ErrorMsg: %x %x %x",mcb,mcb->MsgHdr.Reply,err);
#endif
	*((int *)mcb) = 0;	/* no shorts at present */
	mcb->MsgHdr.Dest = mcb->MsgHdr.Reply;
	mcb->MsgHdr.Reply = NullPort;
	mcb->MsgHdr.FnRc = err;
	PutMsg(mcb);
}

/* 
 * Formulate an open reply, returns TRUE on error, FALSE if not.
 */
int form_open_reply(MyMCB *mymcb, ClientState *client, int handle, 
			char *name, word *type, word flags)
{
	MCB *mcb = &mymcb->mcb;
	char pathname[MAXPATHLEN];
	Capability cap;
	Matrix matrix;
	AccMask mask = 0;
	struct file *fp = client->p.p_fd->fd_ofiles[handle];
	struct vnode *vp = (struct vnode *)fp->f_data;
	struct inode *ip = VTOI(vp);
		
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	
	/* @@@ get some flags from attr, also set up capability */
	switch( vp->v_type )
	{
	case VREG:
	case VBLK:
	case VCHR:	*type = Type_File;	break;
	case VDIR:	*type = Type_Directory;	break;
	case VLNK:	*type = Type_Link;	break;
	case VFIFO:	*type = Type_Fifo;	break;
	case VSOCK:	*type = Type_Socket;	break;
	default:
	case VBAD:
	case VNON:	return(TRUE);
	}

	/* we could use the inode number to make a capability here ?? */
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = client->ugid;

	matrix = mode2matrix(*type, ip->i_mode);
	
	if (client->p.p_cred->p_ruid == ip->i_uid) 
		mask |= matrix & AccMatrix_V;
	if (client->p.p_cred->p_rgid == ip->i_gid)
		mask |= (matrix & AccMatrix_X ) >> 8;
	
	/* we always get Z (= public) access */
	mask |= (matrix & AccMatrix_Z)>>24;

	cap.Access = mask;
	
	MarshalWord(mcb,*type);
	MarshalWord(mcb,flags);
	MarshalCap(mcb,&cap);
	strcpy(pathname,canonname);
	pathcat(pathname,name);
	MarshalString(mcb,pathname);
#ifdef DEBUG
printf("form_open_reply: '%s'",pathname);
#endif
	return(FALSE);
}

/* 
 * Formulate a stat reply, returns TRUE on error, FALSE if not.
 */
int form_stat_reply(MyMCB *mymcb, ClientState *client, char *name,
			struct stat *retstat, word flags)
{
	MCB *mcb = &mymcb->mcb;
	char pathname[MAXPATHLEN];
	Capability cap;
	Matrix matrix;
	AccMask mask = 0;
	word type;
			
	InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);
	
	/* @@@ get some flags from attr, also set up capability */
#ifdef MONITOR
printf("form_stat_reply: st_mode=%d,0x%x",retstat->st_mode,retstat->st_mode);
#endif
	switch (retstat->st_mode & S_IFMT) {
	case S_IFREG:
	case S_IFBLK:
	case S_IFCHR:	type = Type_File;	break;
	case S_IFDIR:	type = Type_Directory;	break;
	case S_IFLNK:	type = Type_Link;	break;
	case S_IFIFO:	type = Type_Fifo;	break;
	case S_IFSOCK:	type = Type_Socket;	break;
	default:	return(TRUE);
	}

#ifdef DEBUG
printf("form_stat_reply: %d (F=%d,D=%d,L=%d,O=%d,S=%d)",type,Type_File,Type_Directory,Type_Link,Type_Fifo,Type_Socket);
#endif
	/* we could use the inode number to make a capability here ?? */
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = client->ugid;

	matrix = mode2matrix(type, retstat->st_mode & ~S_IFMT);

	if (client->p.p_cred->p_ruid == retstat->st_uid) 
		mask |= matrix & AccMatrix_V;
	if (client->p.p_cred->p_rgid == retstat->st_gid) 
		mask |= (matrix & AccMatrix_X ) >> 8;
	
	/* we always get Z (= public) access */
	mask |= (matrix & AccMatrix_Z)>>24;
#ifdef DEBUG
printf("form_stat_reply: mode=0x%x   matrix=0x%x   mask=0x%x",retstat->st_mode,matrix,mask);
#endif
	cap.Access = mask;
	
	MarshalWord(mcb,type);
	MarshalWord(mcb,flags);
	MarshalCap(mcb,&cap);
	strcpy(pathname,canonname);
	pathcat(pathname,name);
 	MarshalString(mcb,pathname);
#ifdef MONOTOR
printf("form_stat_reply: Fullpathname = '%s'",pathname);
#endif
	return(FALSE);
}

/*
 * Handle the forward request for a link.
 */
void handle_symlink(MyMCB *mymcb, ClientState *client, char *fullname)
{
	MCB *mcb = &mymcb->mcb;
	MyMCB *my = NewMyMCB();
	IOCCommon *req = (IOCCommon *)mcb->Control;
	word next = req->Next;
	byte *data = mcb->Data;
	char *linkname, *uen;
	Capability cap;
	int i;

#ifdef DEBUG
	printf("handle_symlink called with '%s'\n",fullname);
#endif
	if (my == NULL)	{
		ErrorMsg(mcb,EC_Error|EG_NoMemory|EO_Message);
		return;
	}

	/* The target name should be in proc->p_symlink */
	if ((linkname = client->p.p_symlink) == NULL) {
		printf("handle_symlink; Failed to get symbolic link name for '%s'\n",
			fullname);
		ErrorMsg(mcb,EC_Error|EG_Errno|EXDEV);
		return;
	}
	client->p.p_symlink = NULL;

	/* Check if this refers to the outside world */
	uen = UFS_EXTERNAL_NAME;
	for (i=0; uen[i] == linkname[i]; i++);
	if (uen[i] != '\0') {
		printf("handle_symlink; Internal error with namei for '%s' link '%s'\n",
			fullname,linkname);
		free(linkname,M_NAMEI);
		ErrorMsg(mcb,EC_Error|EG_Errno|EXDEV);
		return;
	}

	/* Capability and Name */
	((word *)&cap)[0] = -1;
	((word *)&cap)[1] = client->ugid;

	/* Marshal in the common data */
	InitMCB(&my->mcb,mcb->MsgHdr.Flags,NullPort,
			mcb->MsgHdr.Reply,mcb->MsgHdr.FnRc);

#ifdef DEBUG
	printf("handle_symlink forwarding to '%s'  Fn=0x%x\n",
		&linkname[i],mcb->MsgHdr.FnRc);
#endif

	MarshalString(&my->mcb,&linkname[i]);
	MarshalWord(&my->mcb,-1);
	MarshalWord(&my->mcb,1);
	MarshalCap(&my->mcb,&cap);
  	my->mcb.Timeout = IOCTimeout;

	/* Copy across any more parameters in the control vector */
	while (mcb->MsgHdr.ContSize > my->mcb.MsgHdr.ContSize) {
		i = my->mcb.MsgHdr.ContSize;
		MarshalWord(&my->mcb,mcb->Control[i]);
	}

	/* and in the data vector */
	while (data[next++] != '\0');
	if (next < mcb->MsgHdr.DataSize)
		MarshalData(&my->mcb,mcb->MsgHdr.DataSize-next,&data[next]);

	/* Send and return */
	SendIOC(&my->mcb);
	free(my,M_MCB);
	free(linkname,M_NAMEI);
	return;
}

/*
 * The helios locate routine to determine if a file (object) exists.
 */
void do_locate(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	char fullname[MAXPATHLEN];
	struct stat_args stat_args;	
	struct stat resstat;
	int error, retval;
		
#ifdef SHOWCALLS
printf("dispatcher: do_locate called\n");
#endif
	/* Get the full file name */
	makepath(mymcb,fullname);	

	stat_args.fname = fullname;
	stat_args.ub = &resstat;
	error = syscall(client,stat,
		(struct syscall_args *)&stat_args,&retval);

	if (error == EXDEV) {
		handle_symlink(mymcb,client,fullname);
	} else {
		m->MsgHdr.FnRc = SS_HardDisk;

		/* Send the reply. */
		if (error || form_stat_reply(mymcb,client,fullname,&resstat,0)) {
			ErrorMsg(m,EC_Error|EG_Unknown|EO_Object);
		} else {
			PutMsg(m);
		}
	}

	do_complete(client,mymcb);

#ifdef SHOWCALLS
	printf("dispatcher: do_locate complete\n");
#endif
}

/*
 * Open a file/directory for reading/writing and call the appropriate
 * server to handle future requests on the open file/directory.
 */
void do_open(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)m->Control;
	int mode = req->Arg.Mode;
	char fullname[MAXPATHLEN];
	Port reqport;
	struct open_args open_args;	
	int error, retval;
	word type;
		
	/* Get the full file name */
	makepath(mymcb,fullname);	
	
	/* Open the file */
	open_args.fname = fullname;
	open_args.crtmode = 0666;
#ifdef SHOWCALLS
printf("do_open: opening file '%s'\n",fullname);
#endif
	
	/* Translate Helios/Unix Modes */
	if ((mode & O_ReadWrite) == O_ReadWrite) 
		open_args.mode = O_RDWR;
	else {
		if ((mode & O_ReadOnly) == O_ReadOnly) open_args.mode = O_RDONLY;
		else if ((mode & O_WriteOnly) == O_WriteOnly) open_args.mode = O_WRONLY;
		else open_args.mode = 0;
	}
	if ((mode & O_Create) == O_Create)	open_args.mode |= O_CREAT;
	if ((mode & O_Exclusive) == O_Exclusive) open_args.mode |= O_EXCL;
	if ((mode & O_Truncate) == O_Truncate)	open_args.mode |= O_TRUNC;
	if ((mode & O_NonBlock) == O_NonBlock)	open_args.mode |= O_NONBLOCK;
	if ((mode & O_Append) == O_Append)	open_args.mode |= O_APPEND;
		
	/* Do the "open" */
 	error = syscall(client,open,(struct syscall_args *)&open_args,&retval);
#ifdef DEBUG
printf("OPEN (%s): Hmode=0x%x Umode=0x%x  error=%d  fd =%d\n",fullname,mode,open_args.mode,error,retval);
#endif

	/* Send the reply. */
	if (error == EXDEV) {
		handle_symlink(mymcb,client,fullname);
	} else if (error) {
		ErrorMsg(m,EC_Error|EG_Errno|error);
	} else if (form_open_reply(mymcb,client,retval,fullname,&type,
					Flags_Closeable|Flags_NoIData)) {
		ErrorMsg(m,EC_Error|EG_Invalid);
	} else {
		/* Get a return reply port and reply */
		reqport = NewPort();
		m->MsgHdr.Reply = reqport;
		PutMsg(m);

		/* Set debugging and other info */
		client->ofname = fullname;
		client->force_close = FALSE;
		
		/* Now operate */
		if (type == Type_Directory) 
			dir_server(mymcb,client,retval,fullname);
		else
			file_server(mymcb,client,retval);

		/* This file/directory is no longer open */
		client->ofname = NULL;

		/* Free return reply port */
		FreePort(reqport);
	}

	/* Release Resources */
	do_complete(client,mymcb);

#ifdef SHOWCALLS
printf("OPEN (%s): completed\n",fullname);
#endif
}

/*
 * Return the information of a given object.
 */
void do_objinfo(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	char fullname[MAXPATHLEN];
	struct stat_args stat_args;	
	struct stat resstat;
	int error, retval;
	word type, flags;
	Matrix matrix;
	char *uen;
	int uid, i;
		
	/* Get the full file name */
	makepath(mymcb,fullname);	
	
#ifdef SHOWCALLS
printf("dispatcher: do_objinfo called: '%s'",fullname);
#endif
	stat_args.fname = fullname;
	stat_args.ub = &resstat;
	error = syscall(client,lstat,
		(struct syscall_args *)&stat_args,&retval);

	/* Send the reply. */
	if (error) {
		ErrorMsg(m,EC_Error|EG_Unknown|EO_Object);
		goto done;
	}

	/* O.K. so far */
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);

	/* Get Type and matrix */
	switch (resstat.st_mode & S_IFMT) {
	case S_IFREG:
	case S_IFBLK:
	case S_IFCHR:	type = Type_File;	break;
	case S_IFDIR:	type = Type_Directory;	break;
	case S_IFLNK:	type = Type_Link;	break;
	case S_IFIFO:	type = Type_Fifo;	break;
	case S_IFSOCK:	type = Type_Socket;	break;
	default:	type = Type_File;	break;
	}
	matrix = mode2matrix(type,resstat.st_mode & ~S_IFMT);

	/* Setup reply */
#ifdef DEBUG
printf("objinfo reply: mode=0x%x   matrix=0x%x   type=0x%x   name='%s'",
	resstat.st_mode,matrix,type,objname(fullname));
#endif
	/* Marshal DirEntry */
	MarshalData(m,4,(byte *)&type);
	MarshalData(m,4,(byte *)&flags);
	MarshalData(m,4,(byte *)&matrix);
	MarshalData(m,32,objname(fullname));

	if (type == Type_Link) {
		struct readlink_args readlink_args;
		char linkname[MAXPATHLEN];
		Capability cap;

		/* Get the target name of the symbolic link */
		readlink_args.name = fullname;
		readlink_args.buf = linkname;
		readlink_args.count = MAXPATHLEN-1;
		error = syscall(client,readlink,
			(struct syscall_args *)&readlink_args,&retval);

		if (error) {
			ErrorMsg(m,EC_Error|EG_Errno|error);
			goto done;
		}
		linkname[retval] = '\0';

		/* Check if this refers to the outside world */
		uen = UFS_EXTERNAL_NAME;
		for (i=0; uen[i] == linkname[i]; i++);
		if (uen[i] == '\0') {
			/* External name, the rest is the full name */
			strcpy(fullname,&linkname[i]);
		} else {
			/* Internal name, Create canonical path */
			strcpy(fullname,canonname);
			pathcat(fullname,linkname);
		}

		/* Capability and Name */
		((word *)&cap)[0] = -1;
		((word *)&cap)[1] = client->ugid;
		MarshalData(m,sizeof(Capability),(byte *)&cap);
		MarshalData(m,strlen(fullname)+1,fullname);
	} else {
#ifdef DEBUG
printf("objinfo reply: uid=%d   size=%d   ctime=%d   atime=%d   mtime=%d",resstat.st_uid,resstat.st_ctime,resstat.st_atime,resstat.st_mtime);
#endif
		/* Account Size and Dates */
		uid = resstat.st_uid;		/* st_uid is too short */
		MarshalData(m,4,(byte *)&uid);
		MarshalData(m,4,(byte *)&resstat.st_size);
		MarshalData(m,4,(byte *)&resstat.st_ctime);
		MarshalData(m,4,(byte *)&resstat.st_atime);
		MarshalData(m,4,(byte *)&resstat.st_mtime);
	}
		
	PutMsg(m);
done:
	do_complete(client,mymcb);
}

/*
 * Create either an empty file, or a directory.
 */
void do_create(MyMCB *mymcb, ClientState *client)
{
	MCB *mcb = &mymcb->mcb;
	IOCCreate *req = (IOCCreate *)mcb->Control;
	char fullname[MAXPATHLEN];
	int error, retval;

	/* Get the full file name */
	makepath(mymcb,fullname);	

#ifdef SHOWCALLS
printf("dispatcher: do_create called; '%s',fullname");
#endif
	
	if (req->Type == Type_File) {
		/* Create a file by open and close. */
		struct open_args open_args;	
		struct fd_args fd_args;
		word type;
		
		open_args.fname = fullname;
		open_args.crtmode = 0666;
		open_args.mode |= O_CREAT | O_EXCL | O_WRONLY;
	
		/* Do the "open" */
 		if (error = syscall(client,open,(struct syscall_args *)&open_args,&retval))
			ErrorMsg(mcb,EC_Error|EG_Errno|error); /* Open Failed */
 		else {  /* Open O.K., File must be closed */
 			if (form_open_reply(mymcb,client,retval,fullname,&type,0))
	 			ErrorMsg(mcb,mcb->MsgHdr.FnRc);
 			else PutMsg(mcb);
	
			/* Close the file */
			fd_args.fd = retval;
			error = syscall(client,close,
					(struct syscall_args *)&fd_args,&retval);
 		}
	} else if (req->Type == Type_Directory) {
		/* Make a directory */
		Capability cap;
		struct mode_args mode_args;
		Matrix matrix;
		word mode = 0755;	/* XXX AMS - ... for the moment */
		AccMask mask = 0;
			
		mode_args.name = fullname;
		mode_args.mode = mode;
		if (error = syscall(client,mkdir,(struct syscall_args *)&mode_args,&retval))
			ErrorMsg(mcb,EC_Error|EG_Errno|error); /* mkdir Failed */
 		else {
			InitMCB(mcb,0,mcb->MsgHdr.Reply,NullPort,0);

			((word *)&cap)[0] = -1;
			((word *)&cap)[1] = client->ugid;

			matrix = mode2matrix(Type_Directory, mode);
	
			mask |= matrix & AccMatrix_V;
			mask |= (matrix & AccMatrix_X) >> 8;
			mask |= (matrix & AccMatrix_Z)>>24;

			cap.Access = mask;
	
			MarshalWord(mcb,Type_Directory);
			MarshalWord(mcb,0);
			MarshalCap(mcb,&cap);
			MarshalString(mcb,fullname);
			
			PutMsg(mcb);	
 		}
	} else {
		ErrorMsg(mcb,EC_Error|EG_Create|EO_Object);
		printf("Create type 0x%x (%s) not supported",
				req->Type,fullname);
	}

	do_complete(client,mymcb);
}

/*
 * This gets the server information using UNIX statfs.
 */
void do_servinfo(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	char fullname[MAXPATHLEN];
	struct statfs_args statfs_args;
	struct statfs buf;
	int error, retval;
	FSInfo fsi;
	
	/* Get the full file name */
	makepath(mymcb,fullname);	

	statfs_args.path = fullname;
	statfs_args.buf = &buf;
	error = syscall(client,statfs,(struct syscall_args *)&statfs_args,&retval);
	if (error == 0) {
		fsi.Flags = Flags_Server;
		fsi.Size = buf.f_fsize * buf.f_blocks;
		fsi.Avail = buf.f_fsize * buf.f_bavail;
		fsi.Used = buf.f_fsize * (buf.f_blocks - buf.f_bfree);

		InitMCB(m,0,m->MsgHdr.Reply,NullPort,0);
		MarshalData(m,sizeof(fsi),(byte *)&fsi);
		PutMsg(m);
		
#ifdef DEBUG
printf("STATFS: '%s'\nf_type=%d\tf_flags=0x%x\tDisk Blk Size=%d",fullname,buf.f_type,buf.f_flags,buf.f_fsize);
printf("Transfer Blk Size=%d\tTotal Blks=%d\tFree Blks=%d",buf.f_bsize,buf.f_blocks,buf.f_bfree);
printf("Available Blks=%d\tTotal File Nodes=%d\tFree File Nodes=%d",buf.f_bavail,buf.f_files,buf.f_ffree);
printf("F/S Id=%d\tDirectory Mounted On='%s'\tMounted F/S='%s'",buf.f_fsid,buf.f_mntonname,buf.f_mntfromname);
#endif
	} else ErrorMsg(m,EC_Error|EG_Errno|error);

	do_complete(client,mymcb);
}

/*
 * Delete either a file or a directory.  This works by calling sysdelete
 * which is a function AMS made to delete a file/dir.  sysdelete is basically
 * a merge of unlink and rmdir.
 */
void do_delete(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	struct name_args name_args;
	int error,retval=0;
	char fullname[MAXPATHLEN];
	
	/* Get the full file name */
	makepath(mymcb,fullname);	

#ifdef SHOWCALLS
printf("dispatcher: do_delete called: '%s'",fullname);
#endif
	/* The call */
	name_args.name = &fullname[0];
	error = syscall(client,sysdelete,(struct syscall_args *)&name_args,&retval);
	if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
	else ErrorMsg(m,Err_Null);
		
	do_complete(client,mymcb);
}

/*
 * This calls the system rename function to rename a file or directory.
 */
void do_rename(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	struct rename_args rename_args;
	char fromname[MAXPATHLEN], toname[MAXPATHLEN];
	int error,retval;
	
	/* Get the "from" file name */
	makepath(mymcb,fromname);	

	/* Get the "to" file name */
	if (req->Common.Next >= req->Common.Name)
		req->Common.Next = req->Arg.ToName + 
				   (req->Common.Next - req->Common.Name);	
	req->Common.Name = req->Arg.ToName;
	makepath(mymcb,toname);	

#ifdef SHOWCALLS
	printf("dispatcher: do_rename ('%s'->'%s' called",fromname,toname);
#endif

	rename_args.from = fromname;
	rename_args.to = toname;
	error = syscall(client,rename,(struct syscall_args *)&rename_args,&retval);
#ifdef DIAGNOSTIC
	printf("RENAME: (%s)->(%s): error=%d    retval=0x%x\n",fromname,toname,error,retval);
#endif
	if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
	else ErrorMsg(m,Err_Null);
		
	do_complete(client,mymcb);
}

/*
 * This changes the modification/access date/time of a file/directory.
 */
void do_setdate(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg4 *req = (IOCMsg4 *)(m->Control);
	struct utimes_args utimes_args;
	struct timeval time[2];
	char fullname[MAXPATHLEN];
	int error,retval;
	
	/* Get the full file name */
	makepath(mymcb,fullname);	

	/* Set the modification and access times */
	time[0].tv_sec = req->Dates.Access;
	time[0].tv_usec = 0;
	time[1].tv_sec = req->Dates.Modified;
	time[1].tv_usec = 0;

	utimes_args.name = fullname;
	utimes_args.tptr = &time[0];

	error = syscall(client,utimes,(struct syscall_args *)&utimes_args,&retval);
#ifdef DEBUG
	printf("UTIMES: (%s): error=%d    retval=0x%x\n",fullname,error,retval);
#endif
	if (error == EXDEV) handle_symlink(mymcb,client,fullname);
	else if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
	else ErrorMsg(m,Err_Null);
	
	do_complete(client,mymcb);
}

/*
 * Create a symbolic link.
 */
void do_link(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg3 *req = (IOCMsg3 *)(m->Control);
	struct rename_args rename_args;
	char fullname[MAXPATHLEN], linkname[MAXPATHLEN];
	int error,retval,i;
	int max = strlen(canonname);
	
	/* Get the full file name */
	makepath(mymcb,fullname);	

	/* Get the linkee file name */
	strcpy(linkname,m->Data+req->Name);
#ifdef DEBUG
printf("do_link: name is '%s'",linkname);
#endif

	/* Check validity */
	for (i=0; (i < max) && (canonname[i] == linkname[i]); i++);
	if (i == max) {
 		rename_args.from = &linkname[max];
		rename_args.to = fullname;
#ifdef DEBUG
printf("do_link: creating link '%s' to '%s'",rename_args.to,rename_args.from);
#endif
		error = syscall(client,symlink,
			(struct syscall_args *)&rename_args,&retval);
		if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
		else ErrorMsg(m,Err_Null);
	} else {
		/* This link is outside of the system */
		strcpy(linkname,UFS_EXTERNAL_NAME);
		strcat(linkname,m->Data+req->Name);

		/* Now the call */
		rename_args.from = linkname;
		rename_args.to = fullname;
#ifdef DEBUG
printf("do_link: creating link '%s' to '%s'",rename_args.to,rename_args.from);
#endif
		error = syscall(client,symlink,
			(struct syscall_args *)&rename_args,&retval);
		if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
		else ErrorMsg(m,Err_Null);
	}
#ifdef DEBUG
	printf("do_link called with full='%s'  link='%s'",
		fullname,linkname);
#endif

	do_complete(client,mymcb);
}

/*
 * Do the protect / chmod function.
 */
void do_protect(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	struct matrix_args matrix_args;
	char fullname[MAXPATHLEN];
	int error,retval;
	
	/* Get the full file name */
	makepath(mymcb,fullname);	

	/* Setup the matrix arguments and make the call */
	matrix_args.name = fullname;
	matrix_args.matrix = req->Arg.Matrix;
	error = syscall(client,syschmod,
				(struct syscall_args *)&matrix_args,&retval);
#ifdef DEBUG
	printf("CHMOD: (%s): error=%d    retval=0x%x\n",fullname,error,retval);
#endif
	if (error) ErrorMsg(m,EC_Error|EG_Errno|error);
	else ErrorMsg(m,Err_Null);
	
	do_complete(client,mymcb);
}

/*
 * Refine the current capability.
 */
void do_refine(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	AccMask mask = req->Arg.AccMask;
	Capability cap = req->Common.Access;
	char fullname[MAXPATHLEN];
	struct stat_args stat_args;	
	struct stat resstat;
	int error, retval;
	word type;
		
	/* Get the full file name */
	makepath(mymcb,fullname);	
	
#ifdef SHOWCALLS
printf("dispatcher: do_refine called: '%s'",fullname);
#endif
	stat_args.fname = fullname;
	stat_args.ub = &resstat;
	error = syscall(client,stat,
		(struct syscall_args *)&stat_args,&retval);

	/* Send the reply. */
	if (error == EXDEV) {
		handle_symlink(mymcb,client,fullname);
		goto done;
	}
	if (error) {
		ErrorMsg(m,EC_Error|EG_Unknown|EO_Object);
		goto done;
	}

	/* O.K. so far */
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);

	/* Get Type and matrix */
	switch (resstat.st_mode & S_IFMT) {
	case S_IFREG:
	case S_IFBLK:
	case S_IFCHR:	type = Type_File;	break;
	case S_IFDIR:	type = Type_Directory;	break;
	case S_IFLNK:	type = Type_Link;	break;
	case S_IFIFO:	type = Type_Fifo;	break;
	case S_IFSOCK:	type = Type_Socket;	break;
	default:	type = Type_File;	break;
	}

	/* Form the new mask */
	mask &= mode2mask(type,resstat.st_mode & ~S_IFMT);
	cap.Access = mask;

	/* Marshal and Send Reply */
	MarshalData(m,sizeof(cap),(byte *)&cap);
	PutMsg(m);
done:
	do_complete(client,mymcb);
}

/*
 * Revoke the current capability.
 * XXX AMS  This does nothing but return the capability given anyway.
 */
void do_revoke(MyMCB *mymcb, ClientState *client)
{
	MCB *m = &mymcb->mcb;
	IOCMsg2 *req = (IOCMsg2 *)(m->Control);
	Capability cap = req->Common.Access;
	char fullname[MAXPATHLEN];
	struct stat_args stat_args;	
	struct stat resstat;
	int error, retval;

	/* Get the full file name */
	makepath(mymcb,fullname);	
	
#ifdef SHOWCALLS
printf("dispatcher: do_revoke called: '%s'",fullname);
#endif
	stat_args.fname = fullname;
	stat_args.ub = &resstat;
	error = syscall(client,stat,
		(struct syscall_args *)&stat_args,&retval);

	/* Send the reply. */
	if (error == EXDEV) {
		handle_symlink(mymcb,client,fullname);
	} else if (error) {
		ErrorMsg(m,EC_Error|EG_Unknown|EO_Object);
	} else {
		/* Marshal and Send Reply */
		InitMCB(m,0,m->MsgHdr.Reply,NullPort,SS_HardDisk);
		MarshalData(m,sizeof(cap),(byte *)&cap);
		PutMsg(m);
	}

	do_complete(client,mymcb);
}

