/* (C)1992 Perihelion Software Limited                                */
/* Author: Alex Schuilenburg                                          */
/* Date: 7 August 1992                                                */
/* File: dispatch.h                                                   */
/*                                                                    */
/*
 * This file contains the main dispatcher routine, as well as its
 * support routines.  These support routines provide the interface
 * from Helios into the UNIX kernel.
 */
/*
 * $Id: dispatch.h,v 1.4 1993/04/08 14:51:02 nickc Exp $ 
 * $Log: dispatch.h,v $
 * Revision 1.4  1993/04/08  14:51:02  nickc
 * (Alex S) added support for lstat()
 *
 * Revision 1.3  1992/11/24  11:22:43  al
 * Added flock_args for flock support.
 *
 * Revision 1.2  1992/11/05  14:15:11  al
 * Added NewMyMCB prototype (for services to use).
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

#ifndef _DISPATCH_H_
#define _DISPATCH_H_

/* Type Definitions*/
typedef struct ClientState
{
	Node		node;
	void		(*SysCall)();
	word		ugid;		/* User and group ID */

	/* Unix Stuff */
	struct proc	p;
	
	/* Debug Info */
	char *ofname;			/* Open File Name */
	int force_close;		/* Force a close on the open file */
} ClientState;

typedef struct MyMCB {
	MCB		mcb;			/* message control block*/
	word		control[IOCMsgMax];	/* control vector	*/
	byte		data[IOCDataMax]; 	/* data vector		*/
} MyMCB;

/*
 *
 */
extern char *progname;		/* This programe's name */
extern char *canonname;		/* Canonical path name of F/S under HELIOS */

/*
 * Structures used by system calls.
 */
struct syscall_args {
	char 	*dummy;
};

struct stat_args {	/* stat, lstat */
	char 	*fname;
	struct stat *ub;
};

struct statfs_args {	/* statfs */
	char 	*path;
	struct statfs *buf;
};

struct fstat_args {	/* fstat */
	int 	fd;
	struct stat *buf;
};

struct open_args {	/* open, mknod, chown */
	char 	*fname;
	int 	mode;
	int 	crtmode;	
};

struct fd_args {	/* fchdir, close, fsync, umask */
	int 	fd;
};

struct lseek_args {	/* lseek */
	int 	fd;
	off_t 	off;
	int 	sbase;
};

struct getdirent_args {	/* getdirentries */
	int 	fd;
	char 	*buf;
	unsigned count;
	long 	*basep;
};

struct mode_args {	/* mkfifo, saccess, chflags, chmod, mkdir, unmount */
	char 	*name;
	int 	mode;
};

struct rename_args {	/* link, symlink, rename */
	char 	*from;
	char 	*to;
};

struct readlink_args {	/* readlink */
	char 	*name;
	char 	*buf;
	int	count;
};

struct name_args {	/* chdir, chroot, unlink, rmdir, revoke */
	char 	*name;
};

struct mount_args {	/* mount */
	int	type;
	char	*dir;
	int	flags;
	caddr_t	data;
};

struct getfsstat_args {	/* getfsstat */
	struct statfs *buf;
	long	bufsize;
	int	flags;
};

struct rdwr_args {	/* read, write */
		int	fd;
		char	*buf;
		unsigned count;
};

struct ioctl_args {	/* ioctl */
	int	fdes;
	int	cmd;
	caddr_t	cmarg;
};

struct utimes_args {	/* utimes */
	char 	*name;
	struct timeval *tptr;
};

struct ftrunc_args {	/* ftruncate */
	int 	fd;
	off_t 	length;
};

struct vntype_args {	/* sysvntype for namei */
	char 	*name;
	enum vtype *vtype;
};

struct matrix_args {	/* syschmod for chmod */
	char 	*name;
	Matrix	matrix;
};

struct flock_args {	/* flock */
	int	fdes;
	int	cmd;
};

struct ufsrw
{
	ClientState *client;
	word handle;			/* File Handle */
	struct file *fp;		/* Quick Reference to File Handle */
	struct rdwr_args rdwr_args;	/* Arguments for R/W */
	char buf[MAXBSIZE];		/* Buffer for data */
};

/*
 * Dispatcher Routines
 */
int rootuser(ClientState *client);
void removeclient(ClientState *client);
int force_close_client(pid_t pid);	/* Given pid */

#define FreeMyMCB(mcb)		free(mcb,M_MCB);
#define do_complete(c,m)	removeclient(c);  free(m,M_MCB);

/*
 * HELIOS system call routines.
 */
int syscall(ClientState *cs, int (*fn)(), struct syscall_args *uap, int *retval);

/*
 * Utilities.
 */
void makepath(MyMCB *mymcb, char *buf);
void pathcat(string s1, string s2);
Matrix mode2matrix(word type, unsigned int mode);
MyMCB *NewMyMCB(void);

/*
 * Service Routines
 */
void do_open(MyMCB *mcb, ClientState *client);
void do_create(MyMCB *mcb, ClientState *client);
void do_locate(MyMCB *mcb, ClientState *client);
void do_objinfo(MyMCB *mcb, ClientState *client);
void do_create(MyMCB *mcb, ClientState *client);
void do_servinfo(MyMCB *mcb, ClientState *client);
void do_delete(MyMCB *mcb, ClientState *client);
void do_rename(MyMCB *mcb, ClientState *client);
void do_setdate(MyMCB *mymcb, ClientState *client);
void do_link(MyMCB *mymcb, ClientState *client);
void do_protect(MyMCB *mymcb, ClientState *client);
void do_refine(MyMCB *mymcb, ClientState *client);
void do_revoke(MyMCB *mymcb, ClientState *client);
void do_terminate(MyMCB *mymcb, ClientState *client);

/* The private prototypes. */
void do_chown(MyMCB *mcb, ClientState *client);
void do_mknod(MyMCB *mcb, ClientState *client);
void do_mount(MyMCB *mcb, ClientState *client);
void do_getfsstat(MyMCB *mcb, ClientState *client);
void do_stat(MyMCB *mcb, ClientState *client);
void do_lstat(MyMCB *mcb, ClientState *client);
void do_sync(MyMCB *mcb, ClientState *client);
void do_unmount(MyMCB *mcb, ClientState *client);
void do_hardlink(MyMCB *mcb, ClientState *client);
void do_getopen(MyMCB *mymcb, ClientState *client);
void do_closeopen(MyMCB *mymcb, ClientState *client);
void do_getmem(MyMCB *mymcb, ClientState *client);

void file_server(MyMCB *mymcb, ClientState *client, word handle);
void dir_server(MyMCB *mymcb, ClientState *client, word handle, char *name);

#endif /* _DISPATCH_H_ */
