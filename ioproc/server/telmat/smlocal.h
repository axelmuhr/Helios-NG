/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  smlocal.h                                                           --
--                                                                      --
--  Author:  BLV 12.5.89 ( based on sunlocal.h)				--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1987, Perihelion Software Ltd.        */

#if (!TR5)	/* ne pas definir : defini dans select.h */

#define FD_SETSIZE 64
#define NBBY 8
typedef long fd_mask;
#define NFDBITS (sizeof(fd_mask) * NBBY)
#define howmany(x, y) (((x)+((y)-1))/(y))

typedef struct fd_set {
	fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((char *) p, 0, sizeof(*(p)))

struct timeval {
        long tv_sec;
        long tv_usec;
};

#endif

#define getdtablesize() (OPEN_MAX)

extern char *sys_errlist[];
extern int  sys_nerr;

#if (!TR5)
typedef long clock_t;
#endif

#undef CLK_TCK
#define CLK_TCK 1

#define clock() time(NULL)	/* A definir clock renvoi 0 */

#ifndef Files_Module
extern struct stat searchbuffer;
#endif

#if (!TR5)
#ifdef Local_Module
/* #define rmdir unlink --  not use with telmat 3.2 unix version */
#endif
#endif

extern void socket_write();
extern void socket_read();

#ifdef Local_Module
static  void pipe_broken();
#endif














