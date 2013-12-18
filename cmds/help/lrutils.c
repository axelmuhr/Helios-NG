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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)lrutils.c	5.1 (Berkeley) 1/23/91";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include "lrucache.h"

/*
 *  LRUGETPG -- Get a free page from the LRU cache.
 *
 *	This routine grows the cache if necessary, finds an unused page if
 *	it can, and handles flushing dirty buffers to disk.
 *
 *	One of the parameters to this routine (f) is the routine that called
 *	us.  If we have to grow the cache, we call this routine recursively
 *	in order to fill the buffer.  The reason for this is that we have
 *	two interfaces that call lrugetpg().  Lruget() fills a page from disk,
 *	and lrugetnew() just allocates a new (empty) page.
 *
 *	Parameters:
 *		l -- LRU cache to use.
 *		pgno -- page number for which we want a buffer
 *		nread -- pointer to an int to get number of bytes read
 *		f -- who called us
 *
 *	Returns:
 *		(u_char *) pointer to buffer to use, or NULL on failure.
 *
 *	Warnings:
 *		The buffer returned is locked down until the user does an
 *		explicit lrurelease() on it.
 */

u_char *
lrugetpg(l, pgno, nread, f)
	LRUCACHE *l;
	int pgno;
	int *nread;
	u_char *(*f)();
{
	CACHE_ENT *ce;
	LRU_ENT *lruent;
	u_char *buffer;

	/* if we're allowed to grow the cache, do so */
	if (l->lru_cursz < l->lru_csize) {

		/* get a buffer */
		if ((buffer = (u_char *) malloc((unsigned) l->lru_psize))
		    == (u_char *) NULL)
			return ((u_char *) NULL);

		/* get and LRU list entry */
		if ((lruent = (LRU_ENT *) malloc((unsigned) sizeof(LRU_ENT)))
		    == (LRU_ENT *) NULL)
			return ((u_char *) NULL);
		lruent->l_buffer = buffer;
		lruent->l_pgno = pgno;
		lruent->l_flags = 0;

		/* manage spaghetti */
		lruent->l_prev = (LRU_ENT *) NULL;
		lruent->l_next = l->lru_head;
		if (l->lru_head != (LRU_ENT *) NULL)
			l->lru_head->l_prev = lruent;
		l->lru_head = lruent;
		if (l->lru_tail == (LRU_ENT *) NULL)
			l->lru_tail = lruent;

		/* add it to the hash table */
		ce = lruhashput(l, pgno, lruent);
		l->lru_cursz++;
	} else {
		lruent = l->lru_tail;

		/* find the oldest unused buffer */
		while (lruent != (LRU_ENT *) NULL
		       && !(lruent->l_flags & F_FREE))
			lruent = lruent->l_prev;

		/* if no free buffer was found, we have to grow the cache */
		if (lruent == (LRU_ENT *) NULL) {
			if (lrugrow(l) < 0)
				return ((u_char *) NULL);
			return ((*f)((LRU) l, pgno, nread));
		}

		/* okay, found a free buffer -- update hash table and list */
		ce = lruhashget(l, lruent->l_pgno);
		if (lruhashdel(l, lruent->l_pgno) < 0)
			return ((u_char *) NULL);

		/* flush the old page to disk, if necessary */
		if (lruent->l_flags & F_DIRTY)
			if (lruflush(l, lruent) < 0)
				return ((u_char *) NULL);

		/* update stats, hash table, and list */
		lruent->l_pgno = pgno;
		lruhead(l, lruent);
		ce = lruhashput(l, pgno, lruent);
		buffer = lruent->l_buffer;
	}
#ifdef lint
	ce = ce;
#endif /* lint */

	/* lock this page down */
	lruent->l_flags &= ~F_FREE;

	return (buffer);
}

/*
 *  LRUHEAD -- Move an LRU list entry to the head of the list.
 *
 *	The head of the list is where the most recently used item goes.
 *
 *	Parameters:
 *		l -- LRU cache
 *		lruent -- entry to move to the head of the list
 *
 *	Returns:
 *		None
 *
 *	Side Effects:
 *		Updates the cache's head and tail pointers as required.
 */

void
lruhead(l, lruent)
	LRUCACHE *l;
	LRU_ENT *lruent;
{
	LRU_ENT *next;
	LRU_ENT *prev;

	if (l->lru_head == lruent)
		return;

	next = lruent->l_next;
	prev = lruent->l_prev;
	lruent->l_prev = (LRU_ENT *) NULL;
	lruent->l_next = l->lru_head;
	l->lru_head->l_prev = lruent;
	l->lru_head = lruent;

	prev->l_next = next;
	if (next != (LRU_ENT *) NULL)
		next->l_prev = prev;

	if (l->lru_tail == lruent)
		l->lru_tail = prev;
}

/*
 *  LRUGROW -- Grow the LRU cache
 *
 *	This routine is called only if we can't satisfy a user's get() request
 *	using an existing buffer.  We need to rebuild the hash table so that
 *	subsequent lookups work correctly, since the cache size is used to
 *	compute hash keys.
 *
 *	Parameters:
 *		l -- LRU cache to grow
 *
 *	Returns:
 *		Zero on success, -1 on failure
 */

int
lrugrow(l)
	LRUCACHE *l;
{
	int oldsz, newsz;
	CACHE_ENT **New;
	CACHE_ENT *ce, *nce;
	int h;
	int i;

	oldsz = l->lru_csize;
	newsz = l->lru_csize + 1;

	/* get a new hash table */
	if ((New = (CACHE_ENT **) malloc((unsigned)newsz * sizeof(CACHE_ENT *)))
	    == (CACHE_ENT **) NULL)
		return (-1);

	/* build the new hash table */
	bzero((char *) New, (size_t) (newsz * sizeof(CACHE_ENT *)));
	for (i = oldsz; --i >= 0; ) {
		for (ce = l->lru_cache[i]; ce != (CACHE_ENT *) NULL; ) {
			nce = ce->c_chain;
			h = ce->c_pgno % newsz;
			ce->c_chain = New[h];
			New[h] = ce;
			ce = nce;
		}
	}

	/* get rid of the old hash table, and update the cache */
	free ((char *) (l->lru_cache));
	l->lru_cache = New;
	l->lru_csize = newsz;

	return (0);
}
