/* fcntl.h: Posix library file control header			*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: fcntl.h,v 1.2 90/10/09 11:30:23 nick Exp $ */

#ifndef _fcntl_h
#define _fcntl_h

#ifndef _types_h
#include <sys/types.h>
#endif

/* Command values for fcntl()					*/

#define F_DUPFD		1	/* duplicate file descriptor	*/
#define F_GETFD		2	/* get file descriptor flags	*/
#define F_GETLK		3	/* get record locking info	*/
#define F_SETFD		4	/* set file descriptor flags	*/
#define F_GETFL		5	/* get file status flags	*/
#define F_SETFL		6	/* set file status flags	*/
#define F_SETLK		7	/* set record locking info	*/
#define F_SETLKW	8	/* set record locking info & wait*/


/* file access modes						*/

#define	O_RDONLY	0x0001
#define	O_WRONLY	0x0002
#define	O_RDWR		0x0003
#define	O_CREAT		0x0100
#define	O_EXCL		0x0200
#define	O_TRUNC		0x0400
#define O_NONBLOCK	0x0800
#define	O_APPEND	0x1000
#define O_NOCTTY	0x4000

#define O_ACCMODE	0x0003

/* file descriptor flags						*/

#define FD_CLOEXEC	0x00008000	/* close file on exec();	*/

#ifndef _POSIX_SOURCE

#define FD_ALLOC	0x00010000	/* fd allocated			*/
#define FD_INUSE	0x00020000	/* fd in use			*/

/* These masks define the set of bits which may be got/set by fcntl	*/

#define FD_GFDMASK	(FD_CLOEXEC)
#define FD_SFDMASK	(FD_CLOEXEC)
#define FD_GFLMASK	(O_APPEND|O_NONBLOCK|O_ACCMODE)
#define FD_SFLMASK	(O_APPEND|O_NONBLOCK)

#ifdef _BSD

/* extra fcntl() commands, return errors */

#define F_GETOWN	9
#define F_SETOWN	10

#endif

#endif

extern int 	open(char *name, int oflag, ... );
extern int 	creat(char *path, mode_t mode);
extern int 	fcntl(int fd, int cmd, ... );

#endif

/* end of fcntl.h */
