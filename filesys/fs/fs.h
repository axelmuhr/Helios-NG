/* $Header: /usr/perihelion/Helios/filesys/fs/RCS/fs.h,v 1.1 90/10/05 16:28:32 nick Exp $ */

/* $Log:	fs.h,v $
 * Revision 1.1  90/10/05  16:28:32  nick
 * Initial revision
 * 
 * Revision 1.1  90/01/02  19:03:43  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                   H E L I O S   F I L E S E R V E R                  **
**                   ---------------------------------                  **
**                                                                      **
**                   Copyright (C) 1988, Parsytec GmbH                  **
**                          All Rights Reserved.                        **
**                                                                      **
** fs.h	  								**
**                                                                      **
**	Definitions of the FileSystem parameters.			**
**									**
**************************************************************************
** HISTORY  :								**
**-----------								**
** Author   :  30/06/88  A.Ishan 					**
** Modified :  19/01/89  H.J.Ermen  - "Superblock" adaption		**
**                                  - Prototypes can be found in the    **
**				      general header file "nfs.h"	**
**	       20/03/89  H.J.Ermen  - Full parametrisation		**
*************************************************************************/


#ifndef	__fs_h
#define __fs_h

#ifndef daddr_t
#  define daddr_t word		/* disc address type */
#endif

#ifndef dev_t
#  define dev_t word		/* device type */
#endif

struct fs {
	daddr_t		fs_sblknr;	/* addr of summary-block in filesys */
	daddr_t		fs_iblknr;	/* addr of info-block in filesys */
	daddr_t		fs_rblknr;	/* addr of root-dir block */
	word		fs_cgoffset;	/* cylinder group offset in cylinder */
	word		fs_size;	/* number of blocks in fs */
	word		fs_dsize;	/* number of data blocks in fs */
	word		fs_ncg;		/* number of cylinder groups */
	word		fs_bsize;	/* size of basic blocks in fs */
	word		fs_maxdpb;	/* # of dir-entries per block */
	word		fs_frag;	/* number of frags in a block in fs */
	word		fs_fsize;	/* size of frag blocks in fs */
	word		fs_minfree;	/* minimum percentage of free blocks */
	word		fs_syncop;	/* TRUE: only bwrites are used */
	word		fs_maxcontig;	/* max number of contiguous blks */
	word		fs_ndaddr;	/* # of direct blocks */
	word		fs_niaddr;	/* # of indirect blocks */
	word		fs_magic;	/* magic number */
	word		fs_szcg;	/* size of cg-structure */
	word		fs_szfs;        /* size of fs-structure */
	word		fs_maxnii;	/* max # of incore inodes */
	word		fs_cgsize;	/* cylinder group size */
	word		fs_ncgcgoff;	/* product: ncg * cgoffset */
	word		fs_maxpsz;	/* # of different packets */
	word		fs_psmal;	/* size of a small packet */
	word		fs_pmedi;	/* size of a medium packet */
	word		fs_phuge;	/* size of a huge packet */
	word		fs_pscnt;	/* # of small packets */
	word		fs_pmcnt;	/* # of medium packets */
	word		fs_phcnt;	/* # of huge packets */
	word		fs_possindb;	/* Possibility for indirect block */
	word		fs_possdirb;	/* Possibility for directory block */
	word		fs_mingood;	/* Minimal limit for bad direct bnr's */
	word		fs_maxbmerr;	/* Max percentage of errors in a bitmap */
	time_t		fs_time;	/* creation time */
	char		fs_name[32];	/* filesystem name */
};

#define MAGIC_NUMBER     0x17fa341a	/* Magic number of the basic fs	*/
#define MAGIC_CG_OFFSET	     0x3f42	/* Incremental step to fs for	*/
					/* each cylinder group		*/
	
struct fs_para {			/* Parametrization with "mkfs" :*/
	word cgsize;			/* The elements of this struct	*/
	word ncg;			/* correspond to elements in the*/
	word cgoffset;			/* superblock-struct fs.	*/
	word minfree;
	word syncop;
	word maxnii;
	word psmal;
	word pmedi;
	word phuge;
	word maxpsz;
	word pscnt;
	word pmcnt;
	word phcnt;
	word possindb;
	word possdirb;
	word mingood;
	word maxbmerr;
	char name[32];
	};
	
struct sum {
	uword	s_ndir;			/* number of directories */
	uword	s_nbfree;		/* number of free blocks */
	uword	s_nffree;		/* number of free fragments */
};

#define LIMIT_BPG	3072		/* This allows cg-sizes of up 	*/
					/* 12 Mbytes			*/
struct cg {
	struct sum	cg_s;		
	time_t 		cg_time;	/* time last written */
	word		cg_cgx;		/* cgx'th cylinder group */
	word		cg_ndblk;	/* number of data blocks this cg */
	word 		cg_magic;	/* magic number */
	daddr_t		cg_rotor;	/* position of last alloced blk++ */
					/* free block map */
	byte		cg_free[LIMIT_BPG];	
};


#define LIMIT_NCG	300		/* Maximal number of cylinder	*/
					/* groups			*/

struct sum_blk {
	struct dir_elem		root_dir;	/* DirEntry of RootDir */
	struct sum		fs_sum;		/* filesys summary info */
						/* each cyl grp sum info */
	struct sum		cg_sum[LIMIT_NCG];	 
	word			sum_same;	/* sum_blk update flag */
	word			first_boot;	/* TRUE when the server */
						/* is booted the first time */
};

struct i_sum {
	struct sum	sum;			
	Semaphore	sem;			/* guardian to each sum info */
};

struct incore_s {
	struct i_sum	is_fs;			/* incore filesys summary */
	struct i_sum	is_cg[LIMIT_NCG];	/* incore sum of each cyl grp */
	word            is_sum_same;		/* incore summary flag */
	word		is_first_boot;		/* signal first server boot */
};

struct info_blk {
	struct cg		cgx;			
	struct fs		fs;
};

typedef struct pkt {
	daddr_t	bnr;			/* disc block address */
	word	bcnt;			/* contiguous blocks */
} pkt_t;

#define		BSIZE		4096	/* disc Block SIZE */

struct block {
	byte block[BSIZE];
};

/* maps : block to cyl grp */
#define btocg(bnr)	((bnr) / maxbpg)

/* maps : block to cyl grp block */
#define btocgb(bnr)	((bnr) % maxbpg)

/* maps : cyl grp block to block */
#define cgbtob(cgnr,cgbnr)	((cgbnr) + (cgnr) * maxbpg)

/* maps : cyl grp to info block */
#define cgtoib(cgnr)	cgbtob((cgnr),((2 + ((cgnr) * cgoffs)) % maxbpg))

typedef struct pref {
	daddr_t	bpref;			/* preffered block */
	daddr_t	cpref;			/* preffered cyl-grp */
} pref_t;

#endif

/* end of fs.h */
