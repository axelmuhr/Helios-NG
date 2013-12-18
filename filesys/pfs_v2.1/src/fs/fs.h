/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/fs.h,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: fs.h,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  10:54:05  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.3  90/08/08  08:06:40  guenter
 * VolumeDescriptor struct extended
 * 
 * Revision 1.2  90/08/03  10:55:45  guenter
 * multivolume/multipartition
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
**	       03/08/90  G.Lauven   - Multipartition/Multivolume	**
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

#define CG_OFFSET	3		/* constant cylindergroup offset*/

#define MIN_BPG		32		/* minimum cg-size */
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

#define MIN_NCG		2		/* minimum number of cylinder	*/
					/* groups			*/
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

#if 0
/* maps : block to cyl grp */
#define btocg(bnr)	((bnr) / maxbpg)

/* maps : block to cyl grp block */
#define btocgb(bnr)	((bnr) % maxbpg)

/* maps : cyl grp block to block */
#define cgbtob(cgnr,cgbnr)	((cgbnr) + (cgnr) * maxbpg)

/* maps : cyl grp to info block */
#define cgtoib(cgnr)	cgbtob((cgnr),((2 + ((cgnr) * cgoffs)) % maxbpg))

#else

/* maps : block to cyl grp */
#define btocg(bnr,vol)		((bnr) / (vol)->bpcg)

/* maps : block to cyl grp block */
#define btocgb(bnr,vol)		((bnr) % (vol)->bpcg)

/* maps : cyl grp block to block */
#define cgbtob(cgnr,cgbnr,vol)	((cgbnr) + (cgnr) * (vol)->bpcg)

/* maps : cyl grp to info block */
#define cgtoib(cgnr,vol)	cgbtob((cgnr),((2 + ((cgnr) * (vol)->cgoffs)) % (vol)->bpcg), (vol))

#endif

typedef struct pref {
	daddr_t	bpref;			/* preffered block */
	daddr_t	cpref;			/* preffered cyl-grp */
} pref_t;



typedef struct PartDescriptor {
	word		border;		/* logical blocknumber border */
	word		size;		/* size of partition in blocks */
	word		partnum;	/* number of related partition */
} PartDescriptor;


typedef struct TapeDescriptor {
	struct incore_i tapeinode;	/* inode for tape server */
	char 	*tapebuf;		/* buffer for tape data */
	word 	tapebufsize;		/* size of tape buffer */
	word	bufdata;		/* buffer data flag */
	word 	tape_position;		/* tape position pointer */
} TapeDescriptor;


typedef struct VolDescriptor {
	word 		volnum;			/* number of this volume */
	word		num_of_vols;		/* number of volumes */
	word		num_of_parts;		/* number of logic partitions in this volume */
	PartDescriptor	*logic_partition;	/* pointer to array of logic partitions */
	struct fs	incore_fs;		/* incore fs for volume */
	struct incore_s	incore_sum;		/* incore summary block for volume */
	TapeDescriptor	*tape;			/* for tape server only */
	word		raw;			/* raw volume if true */
	word		loaded;			/* medium loaded status (true if loaded) */
	word		loadable;		/* loadable media status */
	word		writeprotected;		/* medium writeprotect status */
	word		hard_formatted;		/* medium physically formatted */
	word		size_known;		/* init_volume_info() succeeded */
	word		filesystem_found;	/* init_fs() succeeded */
	word		sync_allowed;		/* flag for sync operations on volume */
	word		found_bpcg;		/* bpcg found in devinfo */
	word		bpcg;			/* blocks per cylindergroup */
	word		found_cgs;		/* cgs found in devinfo */
	word		cgs;			/* number of cylindergroups */
	word		cgoffs;			/* cg offset */
	word		minfree;		/* minimum blocks to keep free */
	char		vol_name[32];		/* volumes rootdir name */
	word		vol_full;		/* TRUE if no more space */
	word		unload_msg_allowed;	/* TRUE if allowed */
	MCB		unload_mcb;		/* this message will tell the */
						/* dispatcher to terminate all */
						/* workers and unload the volume */
	Semaphore	servreq_sem;		/* Semaphore to block server */
						/* requests */
	Semaphore	dircnt_sem;		/* Used to count the workers */
						/* dealing with dir-operations */
	Semaphore	streamcnt_sem;		/* Used to count the workers */
						/* operating on streams */
	Semaphore	streammsg_sem;		/* Used for stream port locking */
	word		terminate_flag;		/* TRUE if termination desired */
	word		syncwrite;		/* TRUE if in synchronous mode	*/
} VolDescriptor;

#define	VD	VolDescriptor


typedef struct load_res_type {
	word		error;
	word		raw;
	word		loadable;
	word		loaded;
	word		writeprotected;
	word		hard_formatted;
	word		med_removable;
} load_res_type;


typedef struct load_data {		/* control vector of a private load command */
	IOCCommon	Common;
	word		touched;
	word		checker_info;
	word		make_info;
	word		verbose;
	word		delete_hanging_links;
} load_data;


#endif

/* end of fs.h */
