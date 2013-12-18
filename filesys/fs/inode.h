/* $Header: /usr/perihelion/Helios/filesys/fs/RCS/inode.h,v 1.1 90/10/05 16:28:36 nick Exp $ */

/* $Log:	inode.h,v $
 * Revision 1.1  90/10/05  16:28:36  nick
 * Initial revision
 * 
 * Revision 1.1  90/01/02  19:03:45  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                   H E L I O S   F I L E S E R V E R                  **
**                   ---------------------------------                  **
**                                                                      **
**                   Copyright (C) 1988, Parsytec GmbH                  **
**                         All Rights Reserved.                         **
**                                                                      **
** inode.h	  							**
**                                                                      **
**	Definitions of the Inode structures. 				**
**									**
**************************************************************************
** HISTORY  :								**
**-----------								**
** Author   :  25/07/88 : A.Ishan					**
** Modified :  25/01/89 : H.J.Ermen - Note: Prototypes can be found in  **
**					    the general header "nfs.h"	**
**	       22/03/89 : H.J.Ermen - Full parametrization		**
*************************************************************************/


#ifndef	__inode_h
#define __inode_h

#ifndef daddr_t
#  define daddr_t word		/* disc address type */
#endif

#ifndef dev_t
#  define dev_t word		/* device type */
#endif


#define 	NDADDR		  16 	/* direct addresses in inode */
#define 	NIADDR		   2	/* indirect addresses in inode */

#define 	DATA		Type_File	/* data file */
#define		DIR		Type_Directory	/* directory file */
#define 	LINK		Type_Link	
#define		FREE		 0		/* deleted file */
#define		UNKNOWN		-1		/* used for checking */

struct inode {	
	word		i_mode;		/* mode and type of file */
	word 		i_accnt;	/* owner identifier */
	uword		i_size;		/* number of bytes in file */
	time_t		i_atime;	/* time last accessed */
	time_t		i_mtime;	/* time last modified */	
	time_t  	i_ctime;	/* time created */
	daddr_t 	i_db[NDADDR];	/* disc block addresses */
	daddr_t 	i_ib[NIADDR];	/* indirect blocks */
	word		i_blocks;	/* # blocks actually held */
	word		i_spare;        /* nr of DirEntries in DIR files */
	word		i_cryptkey;	/* key for de-/encryption */
	Matrix		i_matrix;	/* file access permissions */
};

struct dir_elem {
	char		de_name[32];	/* name of DirEntry */
	struct inode 	de_inode;	/* inode of DirEntry */
};

#define MAXDPB 	(BSIZE/sizeof(struct dir_elem)) /* Max DirEntries in a block */

typedef struct dir {
	struct buf	*bp;		/* ptr to DirBlock in BufferCache */
	word		ofs;		/* offset of DirEntry in DirBlock */
} dir_t;

struct incore_i {
	struct incore_i		*ii_next;
	struct incore_i		*ii_prev;
	dev_t			 ii_dev;	/* device where inode resides */
	daddr_t			 ii_parbnr;     /* disc address of ParBlock */
	word			 ii_parofs;	/* offset of Entry in ParBlk */
	daddr_t			 ii_dirbnr;	/* disc address of DirBlock */
	word			 ii_dirofs;	/* offset of Entry in DirBlk */
	word			 ii_changed;	/* incore inode changes */
	word			 ii_count;	/* reference count */
	Semaphore		 ii_sem;	/* guardian of inode */
	char			 ii_name[32];	/* name of DirEntry */
	struct inode		 ii_i;		/* incore copy of file-inode */
};

struct i_hash {
	struct incore_i 	*ii_next;
	struct incore_i 	*ii_prev;
};


#define	IHSZ	64

/* inode hash function */
#if	((IHSZ&(IHSZ-1)) == 0)
#define	IHASH(blkno, offset)	\
	(&i_hash[((blkno)+(offset))&(IHSZ-1)])
#else
#define	IHASH(blkno, offset)	\
	(&i_hash[((blkno)+(offset)) % IHSZ])
#endif


struct free_ilist {
	struct incore_i	*ii_next;
	struct incore_i	*ii_prev;
	Semaphore	 fi_sem;	/* guardian of free-inode list */
};

#define 	SIADDR		1024 	/* addresses in an indirect block */
#define 	MAXDIR		16	/* direct addresses in inode */
#define 	MAXSIND		(SIADDR + MAXDIR)
#define		MAXDIND		(SIADDR * SIADDR + MAXSIND)
#define 	MAXTIND		(SIADDR * SIADDR * SIADDR + MAXDIND)

#endif

/* end of inode.h */

