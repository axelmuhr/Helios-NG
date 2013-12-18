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
 * $Id: private.h,v 1.3 1993/04/08 14:50:39 nickc Exp $ 
 * $Log: private.h,v $
 * Revision 1.3  1993/04/08  14:50:39  nickc
 * (Alex S) added support for lstat()
 *
 * Revision 1.2  1992/11/24  11:24:47  al
 * Added FG_UfsFlock for flock support
 *
 * Revision 1.1  1992/09/16  09:29:06  al
 * Initial revision
 *
 */

/* Direct to UNIX System Calls */
#define	FG_UfsLink	    0x00009010	/* Create hard link */
#define	FG_UfsChown	    0x00009020	/* Change ownership */
#define	FG_UfsMknod	    0x00009030	/* Make a device node */
#define	FG_UfsMount	    0x00009040	/* Mount a filesystem */
#define	FG_UfsUnmount	    0x00009050	/* Unmount a filesystem */
#define	FG_UfsSync	    0x00009060	/* Sync all mounted filesystems */
#define	FG_UfsGetfsstat     0x00009070	/* Get stat for all mounted f/s */
#define	FG_UfsStat          0x00009080	/* Get stat for specific file */
#define FG_UfsFlock         0x00009090	/* flock on a specific file */
#define	FG_UfsLstat         0x000090A0	/* Get lstat for specific file */

/* Internal/Specific Calls to the file system */
#define	FG_UfsGetopen	    0x00009110	/* Get a list of all open files */
#define	FG_UfsCloseopen	    0x00009120	/* Force a list of open files closed */
#define	FG_UfsGetmem	    0x00009130	/* Get memory usage */
#define	FG_UfsGetconfig	    0x00009140	/* Get file system configuration info */
#define	FG_UfsSetconfig	    0x00009150	/* Set file system configuration info */

/* Degrees of termination */
#define UfsTermDie	3	/* Just quit as quit as you can */
#define UfsTermHard	2	/* Force open files closed and terminate */
#define UfsTermNow	1	/* Terminate when active clients complete */
#define UfsTermSoft	0	/* Terminate when inactive timeout occurs */

#ifdef UFSOPENFILE
/* Open file type structure */
struct UfsOpenFile {
	int	len;		/* Size of this record */
	uid_t	uid;		/* User who has this file open */
	pid_t	pid;		/* Unique identifier */
	char name[MAXPATHLEN];	/* The full name of the open file */
};
#define UfsOpenFileSize (sizeof(int)+sizeof(uid_t)+sizeof(pid_t)+1)

/* This is defined in dispatch but is actually private */
struct UfsOpenFile **get_open_files(int *max);

#endif /* UFSOPENFILE */

/* The private message structure */
typedef struct IOCChown {
	IOCCommon	Common;		/* Common part of message. */
	int		uid;		/* User ID */
	int		gid;		/* Group ID */
} IOCChown;

typedef struct IOCUfs {
	IOCCommon	Common;		/* Common part of message. */
	int		Type;		/* Type of f/s */
	int		Flags;		/* Flags */
	String		DevName;	/* Device Name */
	Offset		Data;		/* ufs_args data */
} IOCUfs;

typedef struct IOCNfs {
	IOCCommon	Common;		/* Common part of message. */
	int		Type;		/* Type of f/s */
	int		Flags;		/* Flags */
	Offset		Data;		/* nfs_args data */
	Offset		Addr;		/* sockaddr data */
	Offset		Fh;		/* Filehandle */
	Offset		Hostname;	/* Hostname */
} IOCNfs;

typedef struct IOCGetfsstat {
	IOCCommon	Common;		/* Common part of message */
	long		Bufsize;	/* Size of buffer */
	int		Flags;		/* Any Flags ? */
} IOCGetfsstat;

typedef struct IOCGetmemReply {
	long	cache_hits;
	long	cache_misses;
	long	hd_reads;
	long	hd_writes;
	word	hd_numbufs;
	word	clients_active;
} IOCGetmemReply;

