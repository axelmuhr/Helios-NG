/* sys/types.h: Posix library types					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: types.h,v 1.5 1993/08/18 16:17:33 nickc Exp $ */

#ifndef _types_h
#define _types_h

#ifdef __cplusplus
#define signed
#endif

#ifndef NULL
#define NULL (0)
#endif

#ifndef __time_h
typedef	unsigned int clock_t; 
typedef	unsigned int time_t;
#endif

typedef	unsigned int dev_t;
typedef	unsigned int gid_t;
typedef	unsigned int ino_t;
typedef	unsigned int mode_t;
typedef	unsigned int nlink_t;
typedef	unsigned int off_t;
typedef   signed int pid_t;
typedef	unsigned int uid_t;

#ifndef _POSIX_SOURCE

typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef char *         caddr_t;

struct iovec {
	caddr_t		iov_base;
	int		iov_len;
};

/* File descriptor set for select, plus macros to manipulate it */

typedef struct fd_set { int fds_bits[2]; } fd_set;

#define bitsperint (sizeof(int)*8)

#define FD_SETSIZE	64

#define FD_SET(fd,fdset) ((fdset)->fds_bits[(fd)/bitsperint]|=(1<<((fd)%bitsperint)))
#define FD_CLR(fd,fdset) ((fdset)->fds_bits[(fd)/bitsperint]&=~(1<<((fd)%bitsperint)))
#define FD_ISSET(fd,fdset) ((fdset)->fds_bits[(fd)/bitsperint]&(1<<((fd)%bitsperint)))
#define FD_ZERO(fdset) ((fdset)->fds_bits[0]=0,(fdset)->fds_bits[1]=0)

#ifdef _BSD
/* BSD queuing routines */
struct qelem
{
	struct qelem	*q_forw;
	struct qelem 	*q_back;
	char 		 q_data[1];	/* or more */
};
extern void insque(struct qelem *elem, struct qelem *pred);
extern void remque(struct qelem *elem);
#endif

#endif

#endif

/* end of sys/types.h */
