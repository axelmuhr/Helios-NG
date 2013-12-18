/*************************************************************************
**                                                                      **
**            H E L I O S   F I L E S Y S T E M   C H E C K E R         **
**            -------------------------------------------------         **
**                                                                      **
**                   Copyright (C) 1989 Parsytec GmbH                   **
**                          All Rights Reserved.                        **
**                                                                      **
** check.h	  							**
**                                                                      **
**	Header definitions for the file system checker			**
**									**
**************************************************************************
** HISTORY  :								**
**-----------								**
** Author   :  21/03/89  H.J.Ermen 					**
** 									**
** modified :  05/09/90  G.Lauven 	perihelion HFS			**
*************************************************************************/


#ifndef	__check_h
#define __check_h

#ifndef __nfs_h
#include "nfs.h"
#endif
#ifndef __ctype_h
#include <ctype.h>
#endif
#ifndef __stdlib_h
#include <stdlib.h>
#endif
#ifndef __attrib_h
#include <attrib.h>
#endif

			/*-- Different error classes for internal use --*/
#define  NONE		0		/* No error at all		*/
#define  WARNING	1		/* General warning		*/
#define  SERIOUS	2		/* A condition, which needs re- */
					/* covery  (automatically or	*/
					/* by the user's choice)	*/
#define  FATAL		3		/* Condition under which the	*/
					/* checker is terminated	*/
#define  INFO		4		/* Some informations (variable  */
					/* parameters) have to be shown */

			/*----- Limits for incore structure checks -----*/

					/* Number of blocks in cache	*/
#define  MAXNBUF	(pscnt*psmal+pmcnt*pmedi+phcnt*phuge)+1
					/* Total number of packets	*/
#define  MAXNPACK	pscnt+pmcnt+phcnt+1

			/*----------- Checker opcode flags -------------*/

#define  UNKNOWN_CHECKS  -1			
#define  BASIC_CHECKS	  0		/* guarantee consistence	*/
#define  ADVANCED_CHECKS  1		/* "guru"-mode			*/
#define  DESTROY_FS	  2		/* file system destroying	*/
			/*--- Sizes for the temporarily buffer-cache ---*/

 			
#define  TEMP_PSCNT	 60		/* At first, many small packets	*/
#define  TEMP_PMCNT	  1		/* are required by the checker!	*/
#define  TEMP_PHCNT	  1

			/*---------- Global declared variables ---------*/

					/*=== sacheck.c / fservlib.c ===*/
extern word no_corrections;		/* TRUE: no changes are written */
					/* to disk.			*/
extern word silent_mode;		/* No filename output, if TRUE	*/
extern word possdirb;			/* The variable checker limits:	*/
extern word possindb;
extern word mingood;
extern word maxbmerr;

					/*========  checker.c  =========*/	
extern word unique_err;			/* errors during check_unique   */
extern word op_mode;			/* holds the actual op-code	*/
extern word lost_found;			/* is TRUE, if a valid "lost +	*/
					/* found" directory is usable	*/
extern word bit_maps_allocated;		/* TRUE, if we have succesfully	*/
					/* allocated memory for the 	*/
					/* reference-bitmaps		*/
extern word found_links;		/* Number of symbolic links	*/
					/* found during inode-inspection*/
extern word found_files;		/* Number of files found during	*/
					/* inode-inspection		*/
extern word found_blocks;
extern char actual_path[512];		/* The actual search path during*/
					/* directory tree scan.		*/
extern word found_dirs;			/* Number of subdirectories	*/
					/* found during inode inspection*/
extern struct fs i_fs;			/* holds a validated copy of 	*/
					/* the super-block		*/
extern byte *bit_maps[LIMIT_NCG];	/* RAM bit-map reference array	*/
					/* Flags to denote, whether a 	*/
					/* cg-bitmap is usable or not.	*/
extern word cg_bitmap_usable[LIMIT_NCG];
extern daddr_t info_blocks[LIMIT_NCG];	/* Array of info-block numbers	*/

extern char *S_FATAL, *S_WARNING, *S_INFO, *S_SERIOUS;

					/*========  dircheck.c  ========*/
extern word changes_de;			/* If we changed an entry	*/
extern char in_buf[80];

					/*========  concheck.c  ========*/
extern word ignore_links;		/* A flag to determine, whether */
					/* 'hanging' symbolic links are */
					/* ignored during tidyup-phase	*/
					/* or not.			*/
					
			/*--- Struct defs for directory tree handling --*/
			
struct tree	 			/* Search-tree entry		*/
{
	daddr_t		  bnr;		/* Absolute dir-block number	*/
	word		  offset;	/* Offset in this directory blk	*/
	word		  parent_used;	/* True, if it was used as a	*/
					/* parent-dir (fully scanned)	*/
	struct dir_elem   de;		/* The inode information	*/
	struct tree	 *enxt;		/* Next sub-dir in the same	*/
					/* sub-directory		*/
	struct tree	 *eprv;		/* Previous sub-dir in the same	*/
					/* same sub-directory.		*/
	struct tree	 *lnxt;		/* Pointer to a sub-dir, one	*/
					/* step beyond the actual dir	*/
	struct tree	 *lprv;		/* Pointer to a sub_dir, one	*/
					/* step above the actual dir	*/
};

typedef struct tree tree_e;		/* For easier use ...		*/

/*-------------  Hash-handling of directory-entry names  ---------------*/

#define DEHASHSZ	51		/* Size of the hash-table	*/

struct de_name				/* One element in the hash-chain*/
{
	char name[32];			/* The name itself		*/
	struct de_name *denxt;		/* Pointer to the next elements	*/
};

struct de_hash				/* One element of the hash-table*/
{
	struct de_name *denxt;		/* Head-pointer for a hash-queue*/
};
					/* Hash table for directory-	*/
					/* entries			*/
extern struct de_hash de_hash_tab[DEHASHSZ];	

			/* Calculate an hash-index, based on the first	*/
			/* two character values of the parameter string	*/

#define calc_de_hash(s) ( (s[0]+s[1]+s[2]) % DEHASHSZ )


/*----------  Hash-handling of multiple claimed block numbers  ---------*/

			/* We use a hash-table for the multiple	alloca-	*/
			/* ted blocks and queues to storea all reference*/
			/* informations.				*/
				
#define DUPHASHSZ	71		/* The size of the hash-table	*/
					/* for duplicate block-numbers	*/

struct ref
{
	daddr_t		pi_bnr;		/* Block number and offset of	*/
	word		pi_off;		/* the block which keeps the	*/
					/* inode information		*/
	word		pi_place;	/* The index into the list of	*/
					/* direct and indirect blocks	*/
					/* 0-15 , -1=sing.ind. -2=doub	*/
	word		ref_level;	/* The level in which the block	*/
					/* is referred ( 0 = direct,..)	*/
	daddr_t		ind_bnr[2];	/* The block numbers and offsets*/
	word		ind_off[2];	/* of the two levels of indi -	*/
					/* rections			*/
};

struct dup_entry			/* The chain of duplicate entry-*/
{					/* names			*/
	char		  name[32];	/* The entry-name		*/
	time_t		  mtime;	/* The modification time	*/
	word		  type;		/* The type of the entry, which	*/
					/* claims the dup block		*/
	word		  usable;	/* TRUE, if one of the following*/
					/* conditions is fulfilled:	*/
					
			/* type of the block :      mode of the inode : */
			/*     == directory     &&  == Type_Directory   */
			/*     == indirect      &&  referred as indirect*/
			/*     != directory     &&  != Type_Directory   */
			
	struct ref	  b_ref;	/* The reference informations	*/
	struct dup_entry *dupenxt;	/* Hash-pointers		*/
}; 
				
			/*-------- Type-flag for block content ---------*/
			
#define  POSSIBLE_DIR	  	1	/* It could be a directory blk	*/
#define  POSSIBLE_INDIRECT	2	/* It could be an indirect blk	*/
#define  POSSIBLE_UNKNOWN	3	/* We cannot make assumptions	*/
					/* on the block content.	*/
		
					/* The chain of duplicate block-*/
struct dup_bnr				/* numbers			*/
{
	daddr_t		  bnr;		/* The duplicate block-number	*/
	word		  cnt;		/* The replication of this bnr	*/
	word		  type;		/* Could be one of the values	*/
					/* defined above		*/
	struct dup_bnr   *dupnxt;	/* Chaining in the hash-queue	*/
	struct dup_entry *dupenxt;	/* Head into the column of en - */
					/* tries which refer a dup block*/
};
	
struct dup_hash				/* One element in the hash-tab	*/
{
	struct dup_bnr *dupnxt;
};
					/* Declaration of the hash-tab	*/
extern struct dup_hash dup_hash_tab[DUPHASHSZ];

					/* The hashing function		*/
#define calc_dup_hash(b) ( b % DUPHASHSZ )


/*--------------  Hash-handling of 'lost' block numbers  ---------------*/

			/* We work with two hash-tables: One for the 	*/
			/* lost block numbers and another for the blocks*/
			/* which are referred in inodes of lost direc-	*/
			/* tory blocks:					*/

			/* The 'lost' block numbers			*/
			
#define  LOSTHASHSZ	71		/* The size of the hash-table	*/
					/* for 'lost' block numbers	*/

struct lost_bnr 			/* Information stored for each  */
{					/* lost block :			*/
	daddr_t		  bnr;		/* The block number		*/
	word		  root;		/* A flag which signals whether */
					/* The block is a local root and*/
					/* referred by no other entry in*/
					/* a lost directory block.	*/
	struct lost_bnr *lostnxt;	/* Pointer to the next element	*/
					/* in the row			*/
};				
	
struct lost_hash			/* One element in the hash-tab	*/
{
	struct lost_bnr *lostnxt;	/* Pointer to the first element */
};					/* in a hash-queue (row)	*/

					/* Declaration of the hash-tab	*/
extern struct lost_hash lost_hash_tab[LOSTHASHSZ];

					/* The hashing function		*/
#define calc_lost_hash(b) ( b % LOSTHASHSZ )

			/* The block numbers referred by 'lost' direc-	*/
			/* tory blocks in their inodes.			*/
			
#define  REFHASHSZ	171		/* The size of the hash-table	*/
					/* for 'lost' block references	*/

struct ref_bnr	 			/* Information stored for each  */
{					/* lost block :			*/
	daddr_t		  bnr;		/* The block number		*/
	struct ref_bnr  *refnxt;	/* Pointer to the next element	*/
					/* in the row			*/
};				
	
struct ref_hash				/* One element in the hash-tab	*/
{
	struct ref_bnr *refnxt;		/* Pointer to the first element */
};					/* in a hash-queue (row)	*/

					/* Declaration of the hash-tab	*/
extern struct ref_hash ref_hash_tab[REFHASHSZ];

					/* The hashing function		*/
#define calc_ref_hash(b) ( b % REFHASHSZ )


/*----------------  Hash-handling of symbolic links  -------------------*/

#define LINKHASHSZ	71		/* The size of the hash-table	*/

struct de_link 				/* One element in a hash-row	*/
{
	daddr_t		 pi_bnr;	/* The block and the offset into*/
	word		 pi_off;	/* the block which holds the 	*/
					/* parent inode of the symbolic */
					/* link.			*/
	daddr_t		 i_bnr;		/* The block number and the off-*/
	word		 i_off;		/* set into the block which 	*/
					/* keeps the symbolic link	*/
	char		*path;		/* Points to the abs. pathname	*/
	struct de_link  *lnnxt;		/* The next element in the chain*/
};

struct link_hash 			/* One element in the hash-table*/
{				
	struct de_link *lnnxt;		/* The first entry in a row	*/
};
					/* Hash-table for symbolic link-*/
					/* paths			*/
extern struct link_hash link_hash_tab[LINKHASHSZ];

					/* The hashing-function		*/
#define calc_link_hash(p) \
	( ( *strrchr(p,'/') + *(strrchr(p,'/')+1) + *(strrchr(p,'/')+2) ) \
	% LINKHASHSZ )

/*-------------------  Overall checking statistics  --------------------*/

					/* A sample of this structure is*/
struct fsck_stat 			/* filled during checking proc	*/
{
		/* # of corrected copies of the superblock		*/
	word corrected_sb;
		/* # of blocks which were tested for directory-blocks   */
		/* with a negative result				*/
	word wrong_dir;			
		/* # of blocks which were tested for indirect blocks    */
		/* with a negative result.				*/
	word wrong_ind;			
		/* # of inodes for which some errors were detected	*/
	word damaged_inodes;
		/* # of inodes which are not usable			*/
	word wrong_inodes;		
		/* # of inodes which are not usable and which have been */
		/* deleted. (in BASIC-mode equal to wrong_inodes)	*/
	word deleted_inodes;		
		/* # of inodes for which the MIN_GODD criteria failed.	*/
	word mingood_dir;		
		/* # of errors in the summary informations		*/
	word summary_errors;
		/* # of errors in the bit-maps of all cylinder groups	*/
	word bitmap_errors;		
		/* # of multiple allocated blocks found in the file-sys */
	word dup_blocks;		
		/* # of 'lost' blocks found in the file-system		*/
	word lost_blocks;		
		/* # of 'lost' blocks which are 'local root blocks'	*/
	word lost_found_blocks;
		/* # of symbolic links which are 'hanging' links	*/
	word hanging_links;
};

extern struct fsck_stat fst;		/* Overall statistic data	*/


/*--------- Maintaining the number of directories in each cg  ----------*/

extern word n_dirs[LIMIT_NCG];		/* # of directories in each cg	*/
					/* Increment the appropriate	*/
					/* counter.			*/
#define add_dir(bnr) ( n_dirs[bnr/i_fs.fs_cgsize]++ )


/*----------------  Relate cylinder group and blocks  ------------------*/
			
					/* Mapping: cg block to block	*/
#define map_cgbtob(cgnr,cgbnr,maxbpg) \
	((cgbnr) + (cgnr) * (maxbpg))
					/* Mapping: cgnr to info block	*/
#define map_cgtoib(cgnr,cgoff,maxbpg) \
        map_cgbtob(cgnr, (2 + (cgnr) * (cgoff)), maxbpg)

        
/*-----------------  Operations on reference bit-maps ------------------*/

/* One byte in the reference bit-map :
 *
 *     7  6  5  4  3  2  1  0
 *              +- ref-cnt -+
 *           +-- block modified
 *        +----- block given free
 *     +-------- block used during scan
 */
 
#define MASK_USED	0x80
#define MASK_FREE	0x40
#define MASK_MODIFIED   0x20
#define MASK_REFCNT	0x1f

					/* Calculate position in the	*/
					/* reference bitmap array	*/
#define addr_bitmap(bnr) \
	(bit_maps[bnr/i_fs.fs_cgsize][bnr%i_fs.fs_cgsize])
	
					/* Read a specified cell 	*/    
#define bitmap_get(bnr) \
	(addr_bitmap(bnr) & MASK_REFCNT)
					/* Fill a specified cell with a	*/
					/* new value.			*/
#define bitmap_set(bnr, val) \
	(addr_bitmap(bnr) = val )
					/* Increment a specified cell	*/
#define bitmap_incr(bnr) \
	(addr_bitmap(bnr)++)
	
					/* Set the "free" bit in the	*/
					/* specified cell		*/
#define bitmap_set_free(bnr) \
	(addr_bitmap(bnr) |= MASK_FREE)
					/* Get the "free" value from 	*/
					/* the specified cell.		*/
#define bitmap_get_free(bnr) \
	(addr_bitmap(bnr) & MASK_FREE)
	
					/* Set the "used" bit in the	*/
					/* specified cell		*/
#define bitmap_set_used(bnr) \
	(addr_bitmap(bnr) |= MASK_USED)
					/* Get the "used" value from 	*/
					/* the specified cell.		*/
#define bitmap_get_used(bnr) \
	(addr_bitmap(bnr) & MASK_USED)
	
					/* Set the modification bit	*/
#define bitmap_set_modified(bnr) \
	(addr_bitmap(bnr) |= MASK_MODIFIED )
					/* Get the modification bit	*/
#define bitmap_get_modified(bnr) \
	(addr_bitmap(bnr) & MASK_MODIFIED )


/*===============  Global function prototypes for the checker  ===========*/
 
/*-----------------------  defined in : checker.c  -----------------------*/

word		 fscheck (word full);
word		 init_checker (void);
void		 finish_checker (void);
word		 check_unique (void);
word		 create_lostfound_inode (struct inode *ip);

/*-----------------------  defined in : misc_chk.c  ----------------------*/

void		 test_bwrite (struct buf *bp);
int		 my_memcmp (const void *a, const void *b, size_t n);
void		 my_itoa (char *buffer, word val);
int		 my_strcmp (char *s1,char *s2);

/*-----------------------  defined in : dircheck.c  ----------------------*/

word		 validate_entry (struct dir_elem *dp, char *path,  
				 word should_be, word ref_update);
word		 valid_bnr (daddr_t bnr);
word		 valid_entry_name (char *entry, word garbage);
word 		 valid_pathname (char *path);
word		 possible_indirect_block (struct buf *bp, word make_changes,
					  word *changes);
word 		 possible_dir_block (struct buf *bp);
word 		 possible_link_block (struct buf *bp);
void		 append_de_hash (char *de_name);
word		 isin_de_hash (char *de_name);
void		 remove_de_hash (void);

/*-----------------------  defined in : xtdcheck.c  ----------------------*/

word		 check_inodes (struct dir_elem *root_de, daddr_t root_bnr,
			       word root_offs);
struct dir_elem *get_next_entry (struct inode *parent, word first_time,
				 daddr_t *block, word *off);
struct buf	*search_entry (struct inode *parent, char *compstr, word *off);
daddr_t	    	 get_next_bnr (struct inode *parent, word first_time,
			       word termination);
tree_e 		*append_level (tree_e *elem);
tree_e 		*append_entry (tree_e *elem);
tree_e 		*remove_level (tree_e *elem);
tree_e 		*remove_entry (tree_e *elem);
void		 append_link_hash (daddr_t bnr, word off, daddr_t pi_bnr,
				   word pi_off, char *path);
word		 takefrom_link_hash (char *path);
void    	 remove_link_hash (void);

/*-----------------------  defined in : concheck.c  ----------------------*/

word		 check_blocks (void);
word		 get_free_slot (struct inode *ip, daddr_t *bnr, word *offset);
void		 remove_lost_hash (void);
void		 remove_ref_hash (void);
void		 remove_dup_hash (void);
void		 append_dup_bnr (daddr_t bnr, word refcnt);
void		 append_dup_entry (daddr_t dupbnr, char *name, time_t mtime,
				   word type, struct ref *refinfos);

/*----------------------  defined in : condups.c  ------------------------*/ 

void		 handle_dups_and_links (void);
void		 entry_dup_bnr (struct dir_elem *de, daddr_t bnr, word off);
void		 decide_on_dups (void);

/*-----------------------  defined in : tidyup.c  ------------------------*/

word		 tidy_update (void);
word		 update_inode_totals (struct inode *ip);

#endif

/* end of check.h */
