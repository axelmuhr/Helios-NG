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
 *  @(#)btree.h	5.1 (Berkeley) 1/23/91
 */

typedef u_char	*BTREE;		/* should really be (void *) */ 

/* #define	DEBUG */

#define RET_ERROR	-1
#define RET_SUCCESS	 0
#define RET_SPECIAL	 1

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif /* ndef TRUE */

#ifndef NULL
#define NULL	0
#endif /* ndef NULL */

/* libc */
#ifndef __HELIOS
#if !defined(__STDC__) && !defined(RS6000)
extern char *malloc();
#endif

/* these are defined in lrucache.c */
extern u_char	*lruinit();
extern u_char	*lruget();
extern u_char	*lrugetnew();
extern int	lrusync();
extern int	lruwrite();
extern int	lrurelease();
extern void	lrufree();

/* these are defined here */
extern BTREE	bt_open();
extern int	bt_close();
extern int	bt_delete();
extern int	bt_get();
extern int	bt_put();
extern int	bt_seq();
extern int	bt_sync();
#else
#include <stdlib.h>
#include <string.h>
BTREE bt_open(char *f, int flags, int mode, BTREEINFO *b);
int bt_get(BTREE tree, DBT *key, DBT *data, int flag);
int bt_put(BTREE tree, DBT *key, DBT *data, int flag);
int bt_delete(BTREE tree, DBT *key, int flags);
int bt_sync(BTREE tree);
int bt_seq(BTREE tree, DBT *key, DBT *data, int flags);
int bt_close(BTREE tree);
int lruwrite(u_char * lru, int pgno);
int lrusync(u_char * lru);
int lrurelease(u_char * lru, int pgno);
void lrufree(u_char * lru);
#endif

/*
 *  Private types.  What you choose for these depends on how big you
 *  want to let files get, and how big you want to let pages get.
 */

#ifndef SUN4
typedef u_long	index_t;	/* so # bytes on a page fits in a long */
#endif

typedef u_long	pgno_t;		/* so # of pages in a btree fits in a long */

/*
 *  When we do searches, we push the parent page numbers onto a stack
 *  as we descend the tree.  This is so that for insertions, we can
 *  find our way back up to do internal page insertions and splits.
 */

typedef struct BTSTACK {
	pgno_t		bts_pgno;
	struct BTSTACK	*bts_next;
} BTSTACK;

/*
 *  Every btree page has a header that looks like this.  Flags are given
 *  in the #define's for the F_ flags (see below).
 */

typedef struct BTHEADER {
	pgno_t h_pgno;		/* page number of this page */
	pgno_t h_prevpg;	/* left sibling */
	pgno_t h_nextpg;	/* right sibling */

#define F_LEAF		0x01	/* leaf page, contains user data */
#define F_CONT		0x02	/* continuation page (large items) */
#define F_DIRTY		0x04	/* need to write to disk */
#define F_PRESERVE	0x08	/* never delete this chain of pages */

	u_long h_flags;		/* page state */
	index_t h_lower;	/* lower bound of free space on page */
	index_t h_upper;	/* upper bound of free space on page */
	index_t h_linp[1];	/* VARIABLE LENGTH DATA AT END OF STRUCT */
} BTHEADER;

/*
 *  HTBUCKETs are hash table buckets for looking up pages of in-memory
 *  btrees by page number.  We use this indirection, rather than direct
 *  pointers, so that the code for manipulating in-memory trees is the
 *  same as that for manipulating on-disk trees.
 */

typedef struct HTBUCKET {
	pgno_t		ht_pgno;
	BTHEADER	*ht_page;
	struct HTBUCKET	*ht_next;
} HTBUCKET;

typedef HTBUCKET	**HTABLE;

/* minimum size we'll let a page be */
#define MINPSIZE	512

/* default cache size, in bytes */
#define DEFCACHE	(20 * 1024)

/* hash table size for in-memory trees */
#define	HTSIZE		128

/* generate a hash key from a page number */
#define HASHKEY(pgno)	((pgno - 1) % HTSIZE)

/*
 *  Disk btrees have a file descriptor, and may also have an lru buffer
 *  cache, if the user asked for one.
 */

typedef struct BTDISK {
	int	d_fd;
	u_char	*d_cache;
} BTDISK;

/*
 *  Cursors keep track of the current location in a sequential scan of
 *  the database.  Since btrees impose a total ordering on keys, we can
 *  walk forward or backward through the database from any point.  Cursors
 *  survive updates to the tree, and can be used to delete a particular
 *  record.
 */

typedef struct CURSOR {
	pgno_t		c_pgno;		/* pgno of current item in scan */
	index_t		c_index;	/* index of current item in scan */
	u_char		*c_key;		/* current key, used for updates */

#define CRSR_BEFORE	0x01

	u_char		c_flags;	/* to handle updates properly */
} CURSOR;

/*
 *  The private btree data structure.  The user passes a pointer to one of
 *  these when we are to manipulate a tree, but the BTREE type is opaque
 *  to him.
 */

typedef struct BTREEDATA_P {
	char		*bt_fname;		/* NULL for in-memory trees */
	union {
		BTDISK	bt_d;			/* for on-disk btrees */
		HTABLE	bt_ht;			/* hash table for mem trees */
	} bt_s;
	size_t		bt_psize;		/* page size for btree pages */
	int		(*bt_compare)();	/* key comparison function */
	pgno_t		bt_npages;		/* number of pages in tree */
	BTHEADER	*bt_curpage;		/* current page contents */
	pgno_t		bt_free;		/* free pg list for big data */
	CURSOR		bt_cursor;		/* cursor for scans */
	BTSTACK		*bt_stack;		/* parent stack for inserts */
	u_long		bt_lorder;		/* byte order (endian.h) */

#define BTF_METAOK	0x01	/* meta-data written to start of file */
#define BTF_SEQINIT	0x02	/* we have called bt_seq */
#define BTF_ISWRITE	0x04	/* tree was opened for write */
#define BTF_NODUPS	0x08	/* tree created for unique keys */

	u_long		bt_flags;		/* btree state */
} BTREEDATA_P;

typedef BTREEDATA_P	*BTREE_P;

/*
 *  The first thing in a btree file is a BTMETA structure.  The rest of
 *  the first page is empty, so that all disk operations are page-aligned.
 */

typedef struct BTMETA {
	u_long	m_magic;
	u_long	m_version;
	size_t	m_psize;
	pgno_t	m_free;
	u_long	m_flags;
	u_long	m_lorder;
} BTMETA;

#define P_NONE		0		/* invalid page number in tree */
#define P_ROOT		1		/* page number of root pg in btree */

#define NORELEASE	0		/* don't release a page during write */
#define RELEASE		1		/* release a page during write */

#define INSERT		0		/* doing an insert operation */
#define DELETE		1		/* doing a delete operation */

/* get the next free index on a btree page */
#define NEXTINDEX(p)	(index_t) ((((int)(p)->h_lower) - ((int)((((u_char *)(&((p)->h_linp[0])))) - ((u_char *) (p)))))/(sizeof(index_t)))

/* is a BTITEM actually on the btree page? */
#define VALIDITEM(t, i)	((i)->bti_index < (index_t) NEXTINDEX((t)->bt_curpage))

/* guarantee longword alignment so structure refs work */
#define LONGALIGN(p) (((long)(p) + 3) & ~ 0x03)

/* get a particular datum (or idatum) off a page */
#define GETDATUM(h,i)	 (((u_char *) h) + h->h_linp[i])

/* is a {key,datum} too big to put on a single page? */
#define TOOBIG(t, sz)	(sz >= t->bt_psize / 5)

/* is this a disk tree or a memory tree? */
#define ISDISK(t)	(t->bt_fname != (char *) NULL)

/* does the disk tree use a cache? */
#define ISCACHE(t)	(t->bt_s.bt_d.d_cache != (u_char *) NULL)

/*
 *  DATUMs are for user data -- one appears on leaf pages for every
 *  tree entry.  The d_bytes[] array contains the key first, then the data.
 *
 *  If either the key or the datum is too big to store on a single page,
 *  a bit is set in the flags entry, and the d_bytes[] array contains a
 *  pgno pointing to the page at which the data is actually stored.
 *
 *  Note on alignment:  every DATUM is guaranteed to be longword aligned
 *  on the disk page.  In order to force longword alignment of user key
 *  and data values, we must guarantee that the d_bytes[] array starts
 *  on a longword boundary.  This is the reason that d_flags is a u_long,
 *  rather than a u_char (it really only needs to be two bits big).  This
 *  is necessary because we call the user's comparison function with a
 *  pointer to the start of the d_bytes array.  We don't need to force
 *  longword alignment of the data following the key, since that is copied
 *  to a longword-aligned buffer before being returned to the user.
 */

typedef struct DATUM {
	size_t d_ksize;		/* size of key */
	size_t d_dsize;		/* size of data */

#define D_BIGDATA	0x01	/* indirect datum ptr flag */
#define D_BIGKEY	0x02	/* indirect key ptr flag */

	u_long d_flags;		/* flags (indirect bit) */
	u_char d_bytes[1];	/* VARIABLE LENGTH DATA AT END OF STRUCT */
} DATUM;

/* BTITEMs are used to return (page, index, datum) tuples from searches */
typedef struct BTITEM {
	pgno_t bti_pgno;
	index_t bti_index;
	DATUM *bti_datum;
} BTITEM;

/*
 *  IDATUMs are for data stored on internal pages.  This is the (key, pgno)
 *  pair, such that key 'key' is the first entry on page 'pgno'.  If our
 *  internal page contains keys (a) and (b) next to each other, then all
 *  items >= to (a) and < (b) go on the same page as (a).  There are some
 *  gotchas with duplicate keys, however.  See the split code for details.
 *
 *  If a key is too big to fit on a single page, then the i_bytes[] array
 *  contains a pgno pointing to the start of a chain that actually stores
 *  the bytes.  Since items on internal pages are never deleted from the
 *  tree, these indirect chains are marked as special, so that they won't
 *  be deleted if the corresponding leaf item is deleted.
 *
 *  As for DATUMs, IDATUMs have a u_long flag entry (rather than u_char)
 *  in order to guarantee that user keys are longword aligned on the disk
 *  page.
 */

typedef struct IDATUM {
	size_t i_size;
	pgno_t i_pgno;
	u_long i_flags;		/* see DATUM.d_flags, above */
	u_char i_bytes[1];	/* VARIABLE LENGTH DATA AT END OF STRUCT */
} IDATUM;

/* all private interfaces have a leading _ in their names */
#ifdef __HELIOS

/* XXX remember this when we move to the new UFS */

#define fsync(x)	0

int _bt_getbig(BTREE_P t, pgno_t pgno, u_char **p, int *sz);
int _bt_delindir(BTREE_P t, pgno_t chain);
int _bt_indirect(BTREE_P t, DBT *data, pgno_t *pgno);
int _bt_markchain(BTREE_P t, pgno_t chain);
int _bt_crsrdel(BTREE_P t);
int _bt_delone(BTREE_P t, index_t index);
int _bt_insert(BTREE_P t, BTITEM *item, DBT *key, DBT *data, int flag);
int _bt_inserti(BTREE_P t, IDATUM *id, pgno_t pgno);
int _bt_insertat(BTREE_P t, u_char *p, index_t index);
BTITEM *_bt_first(BTREE_P t, DBT *key);
BTITEM *_bt_search(BTREE_P t, DBT *key);
BTITEM *_bt_searchr(BTREE_P t, DBT *key);
index_t _bt_binsrch(BTREE_P t, u_char *key);
int _bt_seqinit(BTREE_P t, DBT *key, int flags);
int _bt_seqadvance(BTREE_P t, int flags);
int _bt_split(BTREE_P t);
int _bt_reposition(BTREE_P t, IDATUM *New, pgno_t parent, pgno_t prev);
int _bt_isonpage(BTREE_P t, IDATUM *New, pgno_t prev);
int _bt_splitroot(BTREE_P t);
int _bt_dopsplit(BTREE_P t, BTHEADER *left, BTHEADER *right);
int _bt_getpage(BTREE_P t, pgno_t pgno);
int _bt_getmpage(register BTREE_P t, pgno_t pgno);
int _bt_getdpage(register BTREE_P t, pgno_t pgno);
int _bt_pgout(BTHEADER *h, u_char *_lorder);
int _bt_pgin(BTHEADER *h, u_char *_lorder);
BTHEADER *_bt_allocpg(BTREE_P t);
int _bt_write(BTREE_P t, BTHEADER *h, int relflag);
int _bt_release(BTREE_P t, BTHEADER *h);
int _bt_wrtmeta(BTREE_P t);
int _bt_fixscan(BTREE_P t, index_t index, DATUM *newd, int op);
int _bt_crsrkey(BTREE_P t);
int _bt_buildret(BTREE_P t, DATUM *d, DBT *data, DBT *key);
int _bt_cmp(BTREE_P t, u_char *p, index_t n);
int _bt_push(BTREE_P t, pgno_t pgno);
pgno_t _bt_pop(BTREE_P t);
u_char *lruinit(int fd, int cachesz, int pagesz, char *opaque, int (*inproc)(), int (*outproc)());
u_char *lruget(u_char * lru, int pgno, int *nread);
u_char *lrugetnew(u_char * lru, int pgno, int *nread);
#else
extern BTITEM	*_bt_search();
extern BTITEM	*_bt_searchr();
extern BTHEADER	*_bt_allocpg();
extern index_t	_bt_binsrch();
extern int	_bt_isonpage();
extern BTITEM	*_bt_first();
extern int	_bt_release();
extern int	_bt_wrtmeta();
extern int	_bt_delindir();
extern int	_bt_pgout();
extern int	_bt_pgin();
extern int	_bt_fixscan();
extern int	_bt_indirect();
extern int	_bt_crsrdel();
extern int	_bt_push();
extern pgno_t	_bt_pop();
extern int	strcmp();
#endif
