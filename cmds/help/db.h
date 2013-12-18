/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 *
 *	@(#)db.h	5.2 (Berkeley) 2/12/91
 */

#ifndef _DB_H_
#define	_DB_H_

#ifdef __HELIOS
#include <stddef.h>
#endif

#ifdef RS6000
#include <sys/types.h>
#endif

/* flags for DB.put() call */
#define	R_IBEFORE	1		/* RECNO */
#define	R_IAFTER	2		/* RECNO */
#define	R_NOOVERWRITE	3		/* BTREE, HASH, RECNO */
#define	R_PUT		4		/* BTREE, HASH, RECNO */

/* flags for DB.seq() call */
#define	R_CURSOR	1		/* BTREE, RECNO */
#define	R_FIRST		2		/* BTREE, HASH, RECNO */
#define	R_LAST		3		/* BTREE, RECNO */
#define	R_NEXT		4		/* BTREE, HASH, RECNO */
#define	R_PREV		5		/* BTREE, RECNO */

/* key/data structure -- a data-base thang */
typedef struct {
	u_char *data;
	int size;
} DBT;

/* access method description structure */
typedef struct {
	char *internal;		/* access method private; really void * */
	int (*close)();
	int (*Delete)();
	int (*get)();
	int (*put)();
	int (*seq)();
	int (*sync)();
} DB;

#define	BTREEMAGIC	0x053162
#define	BTREEVERSION	2

/* structure used to pass parameters to the btree routines */
typedef struct {
#define	R_DUP		0x01	/* duplicate keys */
	u_long flags;
	int cachesize;		/* bytes to cache */
	int psize;		/* page size */
	int (*compare)();	/* compare function */
	int lorder;		/* byte order */
} BTREEINFO;

#define	HASHMAGIC	0x061561
#define	HASHVERSION	1

/* structure used to pass parameters to the hashing routines */
typedef struct {
	int bsize;		/* bucket size */
	int ffactor;		/* fill factor */
	int nelem;		/* number of elements */
	int ncached;		/* bytes to cache */
	int (*hash)();		/* hash function */
	int lorder;		/* byte order */
} HASHINFO;

/* structure used to pass parameters to the record routines */
typedef struct {
#define	R_FIXEDLEN	0x01	/* fixed-length records */
	u_long flags;
	int cachesize;		/* bytes to cache */
	size_t reclen;		/* record length (fixed-length records) */
	u_char bval;		/* delimiting byte (variable-length records */
} RECNOINFO;

/* key structure for the record routines */
typedef struct {
	u_long number;
	u_long offset;
	u_long length;
#define	R_LENGTH	0x01	/* length is valid */
#define	R_NUMBER	0x02	/* record number is valid */
#define	R_OFFSET	0x04	/* offset is valid */
	u_char valid;
} RECNOKEY;

/* Little endian <--> big endian long swap macros. */
#define BLSWAP(a) { \
	u_long _tmp = (u_long)a; \
	((u_char *)&a)[0] = ((u_char *)&_tmp)[3]; \
	((u_char *)&a)[1] = ((u_char *)&_tmp)[2]; \
	((u_char *)&a)[2] = ((u_char *)&_tmp)[1]; \
	((u_char *)&a)[3] = ((u_char *)&_tmp)[0]; \
}
#define	BLSWAP_COPY(a,b) { \
	((u_char *)&(b))[0] = ((u_char *)&(a))[3]; \
	((u_char *)&(b))[1] = ((u_char *)&(a))[2]; \
	((u_char *)&(b))[2] = ((u_char *)&(a))[1]; \
	((u_char *)&(b))[3] = ((u_char *)&(a))[0]; \
}


/* Little endian <--> big endian short swap macros. */
#define BSSWAP(a) { \
	u_short _tmp = (u_short)a; \
	((u_char *)&a)[0] = ((u_char *)&_tmp)[1]; \
	((u_char *)&a)[1] = ((u_char *)&_tmp)[0]; \
}
#define BSSWAP_COPY(a,b) { \
	((u_char *)&(b))[0] = ((u_char *)&(a))[1]; \
	((u_char *)&(b))[1] = ((u_char *)&(a))[0]; \
}
#ifndef __HELIOS
#if defined(__STDC__)
#define __P(protos) protos
#else
#define __P(protos) ()
#endif

DB *btree_open __P((const char *file, int flags, int mode, const BTREEINFO *Private));
DB *hash_open __P((const char *file, int flags, int mode, const HASHINFO *Private));
DB *recno_open __P((const char *file, int flags, int mode, const RECNOINFO *Private));

#else

DB *btree_open(const char *file, int flags, int mode, const BTREEINFO *Private);
DB *hash_open(const char *file, int flags, int mode, const HASHINFO *Private);
DB *recno_open(const char *file, int flags, int mode, const RECNOINFO *Private);

#endif

/*
 * XXX If you're not running on a BSD system, you probably don't have
 * endian.h.  A copy of endian.h is provided here, but it may not include
 * your machine type.  You should edit it and add your machine type or this
 * code won't compile.
 */
#include "endian.h"

#ifndef EFTYPE
#define EFTYPE EINVAL
#endif /* ndef EFTYPE */

#endif /* !_DB_H_ */
