/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * @(#)lrucache.h	5.1 (Berkeley) 1/23/91
 */

/*
 *  LRU list entries.  The head of the list is the most-recently requested
 *  block; the tail is the least-recently requested one.
 */

typedef struct LRU_ENT {
	u_char	*l_buffer;		/* buffer we return to user */
	int	l_pgno;			/* logical page number */
	int	l_flags;		/* FREE and DIRTY bits */
	struct LRU_ENT	*l_prev;	/* predecessor in LRU list */
	struct LRU_ENT	*l_next;	/* successor in LRU list */
} LRU_ENT;

/*
 *  Cache entries.  We use a hash table to avoid a linear walk of the LRU
 *  list when we need to look up blocks by number.  The hash table is
 *  chained.
 */

typedef struct CACHE_ENT {
	int			c_pgno;
	LRU_ENT			*c_lruent;
	struct CACHE_ENT	*c_chain;
} CACHE_ENT;

/*
 *  The LRU cache structure.  The cache size (lru_csize) is the largest size
 *  the user wants us to grow to; current size (lru_cursz) is always less than
 *  or equal to lru_csize.  Note that we will grow the cache (lru_csize) if
 *  it's the only way that we can satisfy a user's block request.
 */

typedef struct LRUCACHE {
	int		lru_fd;
	int		lru_csize;
	int		lru_psize;
	int		lru_cursz;
	u_char		*lru_opaque;		/* passed to inproc, outproc */
	int		(*lru_inproc)();
	int		(*lru_outproc)();
	LRU_ENT		*lru_head;
	LRU_ENT		*lru_tail;
	CACHE_ENT	**lru_cache;
} LRUCACHE;

#ifndef NULL
#define NULL	0
#endif /* ndef NULL */

/* this is the opaque type we return for LRU caches */
typedef	u_char	*LRU;

/* bits for l_flags in LRU_ENT structure */
#define	F_DIRTY		(1 << 0)
#define F_FREE		(1 << 1)

/* lru module routines */
#ifndef __HELIOS
extern CACHE_ENT	*lruhashget();
extern CACHE_ENT	*lruhashput();
extern int 		lruhashdel();
extern void		lruhead();
extern int 		lrugrow();
extern LRU		lruinit();
extern int		lruwrite();
extern int		lrusync();
extern u_char		*lruget();
extern u_char		*lrugetnew();
extern u_char		*lrugetpg();
extern int		lrurelease();
extern void		lrufree();
#else
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

LRU lruinit(int fd, int cachesz, int pagesz, char *opaque, int (*inproc)(), int (*outproc)());
u_char *lruget(LRU lru, int pgno, int *nread);
u_char *lrugetnew(LRU lru, int pgno, int *nread);
int lruflush(LRUCACHE *l, LRU_ENT *lruent);
int lruwrite(LRU lru, int pgno);
int lrusync(LRU lru);
int lrurelease(LRU lru, int pgno);
void lrufree(LRU lru);
CACHE_ENT *lruhashget(LRUCACHE *l, int pgno);
CACHE_ENT *lruhashput(LRUCACHE *l, int pgno, LRU_ENT *lruent);
int lruhashdel(LRUCACHE *l, int pgno);
u_char *lrugetpg(LRUCACHE *l, int pgno, int *nread, u_char *(*f)());
void lruhead(LRUCACHE *l, LRU_ENT *lruent);
int lrugrow(LRUCACHE *l);
#endif
