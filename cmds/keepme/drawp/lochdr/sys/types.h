/* Charlie's local definition with time_t not defined, since it is defined */
/*    in the ANSI file.                                                    */
/*
 * $Header: types.h,v 1.1 90/01/13 20:12:41 charles Locked $
 * $Source: /server/usr/users/charles/world/drawp/RCS/lochdr/sys/types.h,v $
 *
 * Copyright (c) 1988 Acorn Computers Ltd., Cambridge, England
 *
 * $Desc$
 * $Log:	types.h,v $
 * Revision 1.1  90/01/13  20:12:41  charles
 * Initial revision
 * 
 * Revision 1.9  89/12/15  16:34:46  pete
 * File unchanged
 * 
 * Revision 1.8  89/11/28  10:05:46  pete
 * Commented out uint defn. 'cos it conflicts with my one (Pete)
 * 
 * Revision 1.7  89/11/03  15:00:21  charles
 * File unchanged
 * 
 * Revision 1.6  89/08/15  13:43:14  charles
 * File unchanged
 * 
 * Revision 1.5  89/08/11  17:47:20  charles
 * File unchanged
 * 
 * Revision 1.4  89/08/10  19:53:33  charles
 * File unchanged
 * 
 * Revision 1.3  89/08/10  19:41:50  charles
 * File unchanged
 * 
 * Revision 1.2  89/08/10  19:01:02  charles
 * File unchanged
 * 
 * Revision 1.1  89/08/10  16:24:07  charles
 * Initial revision
 * 
 * Revision 1.3  89/08/10  15:46:52  charles
 * "just_to_release_lock"
 * 
 * Revision 1.2  89/07/10  18:17:53  charles
 * Changed so can co-exist with ANSI files
 * 
 * Revision 1.1  89/07/10  13:56:25  charles
 * Initial revision
 * 
 * Revision 1.3  88/06/17  20:22:16  beta
 * Acorn Unix initial beta version
 * 
 */
/* @(#)types.h	1.4 87/09/03 3.2/4.3NFSSRC */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)types.h	7.1 (Berkeley) 6/4/86
 */

#ifndef _TYPES_
#define	_TYPES_
/*
 * Basic system types and major/minor device constructing/busting macros.
 */

/* major part of a device */
#define	major(x)	((int)(((unsigned)(x)>>8)&0377))

/* minor part of a device */
#define	minor(x)	((int)((x)&0377))

/* make a device number */
#define	makedev(x,y)	((dev_t)(((x)<<8) | (y)))

typedef	unsigned char	u_char, u_byte;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

#if 0
typedef	unsigned short	ushort;		/* sys III compat */
#endif

#ifdef vax
typedef	struct	_physadr { int r[1]; } *physadr;
typedef	struct	label_t	{
	int	val[14];
} label_t;
#endif /* NORCROFT ANSI 'C' objects to identifier 'vax' at this point */

#ifdef arm
typedef	struct	_physadr { int r[1]; } *physadr;
typedef struct label_t {
	int	val[16];		/* bigger than need be, so what */
} label_t;
#endif /* NORCROFT ANSI 'C' objects to identifier 'arm' at this point */

typedef	struct	_quad { long val[2]; } quad;
typedef	long	daddr_t;
typedef	char *	caddr_t;
typedef	u_long	ino_t;
typedef	long	swblk_t;
/*typedef	long	size_t; Declared as 'unsigned int' in ansi headers */
/*typedef	long	time_t; Declared as 'unsigned int' in ansi headers */
typedef	short	dev_t;
typedef	long	off_t;
typedef	u_short	uid_t;
typedef	u_short	gid_t;
typedef int	key_t;		/* For System V IPC calls */

#define	NBBY	8		/* number of bits in a byte */
/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields (the filesystem macros use chars).
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	256
#endif

typedef long	fd_mask;
#define NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))

#endif
