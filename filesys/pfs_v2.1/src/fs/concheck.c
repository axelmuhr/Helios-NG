                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                    (c) 1991 by parsytec GmbH, Aachen                    |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  concheck.c                                                             |
   |                                                                         |
   |    - After validating each entry and first check of the bit-maps, 	     |
   |	  the original bit-maps are scanned for duplicate blocks and	     |
   |	  entries for the "lost+found"-directory.			     |
   |	- Various efforts are made to handle 'lost + found' blocks and	     |
   |	  multiple claimed blocks. (The routines for resolving multiple      |
   |	  allocations are collected in condups.c)			     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 11 May   1991 - Error handling centralized           |
   |    1 - H.J.Ermen - 18 April 1989 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1

#include "error.h"
#include "check.h"

/*----------------------  Global variables  ----------------------------*/

word	ignore_links;		/* This is a flag, which signals whe-	*/
				/* 'hanging' symbolic links should be 	*/
				/* removed or not.			*/
				
/*---------------------  Local prototypes  -----------------------------*/

static word	slot_available (struct inode *lfpt);
static daddr_t	new_alloc (void);
static word	workon_lost_block (daddr_t bnr);
static word 	create_lost_entry (daddr_t lostbnr, daddr_t lfbnr, word slot,
				   word nentries);
static word	poss_lost_dir_block (daddr_t bnr);
static void     search_local_roots (void);
static void	append_lost_bnr (daddr_t lbnr);
static void     append_ref_bnr (daddr_t rbnr);

/*************************************************************************
 * HANDLE REFERENCE BIT-MAPS AND DO EVERYTHING TO LOCATE AND HANDLE
 * DOUBLE REFERRED AND 'LOST' BLOCKS
 *
 * - The following three steps are taken :
 *
 *   1. Scan reference bit-maps for new allocated or freed blocks
 *   2. Compare cg and reference bit-maps to find "lost" directory blocks.
 *      If we found some of them, we start check_inodes() with the entries
 *      in this block as a local parent directory. This is only done, if
 *	the bit-map is declared as being usable. 
 *   3. Scan reference maps for double allocated blocks. All double allocated
 *      blocks are collected in a hash-table. During a second pass over the
 *      whole directory tree, we try to find out the entries which refer 
 *      these blocks and decide which should claim the block exclusively.
 *      During the same pass over the directory-tree, each entry is searched
 *      in the hash-table for symbolic links to find out whether a valid
 *      reference exists or not. All entries found are deleted from the
 * 	hash-table. The remaining links are "hanging" abd their entries are
 *	removed during tidy-up operation.
 *
 * - For maximal safety these three steps are handled separately.
 *
 * Parameter : - nothing - 
 * Return    : TRUE   : If no error occurred
 *	       FALSE  : If a fatal error occurred, which makes it impossible
 *			to continue checking
 *
 *************************************************************************/
word
check_blocks ( void )
{
 word cgnr, bcnt, bnr, i, changes, val_entries;
 word n_alloc, n_free, off;
 struct buf *bp, *blf;
 struct lost_bnr *lpt;
 struct inode lf_inode;
 daddr_t lf_bnr;			/* Block number and offset are 	  */
 word lf_off;				/* used to describe a slot in the */
 					/* /lost+found-directory.	  */
 					
/*----------  Pass 1 : Look for freed and new allocated blocks  ----------*/

 Report ("%s***    Step 3.1 : Look for freed or allocated blocks.", S_INFO);

					/* Step through the cyl.groups	  */
 for ( cgnr = 0 , changes = 0 ;
       cgnr < i_fs.fs_ncg ; cgnr++ )
 {					
  					/* Read the info-block		  */
 	bp = bread ( cdev, info_blocks[cgnr], 1, SAVEA );
	if (!bp) {
		Report ("%scheck_blocks() : Failed to read info block %d (bnr %d) !",S_FATAL,cgnr,info_blocks[cgnr]);
		return (FALSE); 
	}
					/* If the cg bit-map is in an	  */
					/* usable state			  */
	if ( cg_bitmap_usable[cgnr] )
	{				/* Work on the actual bit-map	  */
		for ( bcnt = 0 , n_alloc = 0 , n_free = 0 ;
	      	       bcnt < i_fs.fs_cgsize ; bcnt++ )
		{				
			/* Note : Values in the cylinder-group bitmaps	  */
			/* unequal to zero and 0xff are interpreted as 	  */
			/* allocated blocks! (There is a significantly	  */
			/* higher possibility that invalid entries are    */
			/* allocated blocks than free blocks.)		  */
				
					/* We HAVE NOT touched the block  */
					/* during the first pass	  */
			if ( ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) == 0 )
			{
 					/* It was referred as being free  */
				if ( bp->b_un.b_info->cgx.cg_free[bcnt] == 0 )
				{
					n_free++;
					continue;
				}
				if ( bp->b_un.b_info->cgx.cg_free[bcnt] != 0 &&
				     ( bit_maps[cgnr][bcnt] & MASK_FREE ) )
				{

DEBdebug (" check_blocks() : free block %d, cylinder group %d",bcnt,cgnr);

					bp->b_un.b_info->cgx.cg_free[bcnt] = 0;
					bit_maps[cgnr][bcnt] &= ~MASK_FREE;
					n_free++;
					changes++;
				}
 			}
					/* If we have touched the block	  */ 
					/* during our inspection at least */
					/* one time 			  */
 			if ( ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) > 0 )
 			{		
 		     			/* Test for allocated block	  */
 				if ( bp->b_un.b_info->cgx.cg_free[bcnt] == 0xff )
 		     		{
	 	     			n_alloc++;
 		     			continue;
 	     			}
 	     			        /* Test for new allocated block or*/
 		     			/* for "illegal" value (...which  */
 		     			/* should be allocated...)	  */
 		     		if ( bp->b_un.b_info->cgx.cg_free[bcnt] != 0xff )
 		     		{
 		     			bp->b_un.b_info->cgx.cg_free[bcnt] = 0xff;
	 	     			n_alloc++;
	 	     			changes++;
 		     			continue;
 	     			}	    	
 	     		}
					/* Reset the free-bit ...	  */
			bit_maps[cgnr][bcnt] &= ~MASK_FREE;
					/* ... and reset the used-bit, be-*/
					/* cause we make use of them later*/
					/*  again.  			  */
			bit_maps[cgnr][bcnt] &= ~MASK_USED;
					
		} /* end <for (bcnt)> */

DEBdebug (">     usable : cg = %d  :  FREE  sum = %d | count = %d",
	cgnr, bp->b_un.b_info->cgx.cg_s.s_nbfree, n_free);  

	}
	else				/* If the bit-map is NOT in an	  */
	{				/* usable state:		  */
					/* Step through the reference map */
		for ( bcnt = 0 , n_alloc = 0 , n_free = 0 ;
	      	      bcnt < i_fs.fs_cgsize ; bcnt++ )
		{
					/* Strategy: Copy the complete    */
					/* allocation informations from   */
					/* the ref-maps to the cg-bitmaps.*/
			if ( ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) == 0 )
			{
				bp->b_un.b_info->cgx.cg_free[bcnt] = 0;
				n_free++;
			}
			else		/* Content unequal to zero is in- */
			{		/* terpreted as allocated.	  */
				bp->b_un.b_info->cgx.cg_free[bcnt] = 0xff;
				n_alloc++;
			}
					/* Reset the free-bit ...	  */
			bit_maps[cgnr][bcnt] &= ~MASK_FREE;
					/* ... and reset the used-bit	  */
			bit_maps[cgnr][bcnt] &= ~MASK_USED;
		}

DEBdebug ("> NOT usable : cg = %d  :  FREE  sum = %d | count = %d",
  	cgnr, bp->b_un.b_info->cgx.cg_s.s_nbfree, n_free);

			 
		changes++;		/* We always have to write this   */
					/* info-block back to disk !	  */
	}
 					/* Write the info-block back, if  */
 					/* there were any changes made:	  */
 	if ( changes )			
 	{
		Report ("%sUpdate modified info-block cg = %d on disk.",
			 S_INFO, cgnr);
	 	test_bwrite ( bp );
	}
	else				/* .. or release it simply	  */
		brelse ( bp->b_tbp, TAIL );
		 
 } /* end <for (cgnr)> */

/*-------------  Pass 2 : Look for "lost" directory-blocks  --------------*/

 Report ("%s***    Step 3.2 : Look for 'lost' directory blocks", S_INFO);

 remove_lost_hash ();			/* Free all elements in the hash- */
 					/* structures for lost blocks	  */
 remove_ref_hash ();			/* Free also all reference structs*/ 

/*------------  Collect 'lost' blocks from the cg bit-maps  --------------*/

					/* Step through all cg's :	  */
 for ( cgnr = 0 ; ( cgnr < i_fs.fs_ncg && lost_found ) ; cgnr++ )
 {					/* Is the cg-bitmap in an usable  */
 	if ( cg_bitmap_usable[cgnr] )	/* state ?			  */
 	{				
 					/* Get the appropriate info-block */
 		bp = bread ( cdev, info_blocks[cgnr], 1, SAVEA );
		if (!bp) {
			Report ("%scheck_blocks() : Failed to read info block %d (bnr %d) !", S_FATAL,cgnr,info_blocks[cgnr]);
			return (FALSE);
		}
 					/* Work on all entries of the	  */
 					/* cg - bitmap (free-map)	  */
 		for ( (cgnr==0) ? (bcnt = 1) : (bcnt = 0) , changes = 0 ;
 		       bcnt < i_fs.fs_cgsize ; bcnt++ )
 		{
				/* It could be a lost block,if we have    */
 				/* not found any reference, but find the  */
 				/* block claimed in the "original" map:	  */
 			if ( bp->b_un.b_info->cgx.cg_free[bcnt] != 0 &&
 			     ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) == 0 )
 			{
 					/* To ease handling ...		  */
 				bnr = cgnr * i_fs.fs_cgsize + bcnt ;

DEBdebug ("	check_blocks :	Block no %d is a 'lost' block", bnr );

 					/* One more lost block to be noted*/
 				fst.lost_blocks++;
 					/* Test for lost DIRECTORY-block  */
 				if ( poss_lost_dir_block ( bnr ) )
 				{

DEBdebug ("	check_blocks :	The 'lost' block no %d contains directory information",
	 bnr );

 					/* Add the 'lost' block number to */
 					/* the hash-table for 'lost' blks */
 					append_lost_bnr ( bnr );
 					/* Add all references to the hash-*/
 					/* table for block-references	  */
 					append_ref_bnr  ( bnr );
 					
 					/* Allocated blocks should be de- */
 					/* noted as 0xff!		  */
 					if ( bp->b_un.b_info->cgx.cg_free[bcnt] != 0 )
 					{
 						bp->b_un.b_info->cgx.cg_free[bcnt] = 0xff;
 						changes++;
 					}
 				}	
 			} /* end <if (lost)> */
 		} /* end <for (bcnt)> */
 					/* Give the info-block back	 */
 		if ( changes )
 		{
 			Report ("%sUpdate info-block of cg %d on disk",
 				  S_INFO, cgnr);
 			test_bwrite ( bp );
 		}
 		else
 			brelse ( bp->b_tbp, TAIL );
 	} 
 } /* end <for (cgnr)> */

/*--  Try to reintegrate 'lost' blocks which resides as 'local roots'  ---*/
 

DEBdebug ("	check_blocks :	Look for the /lost+found directory");


 bp = bread ( cdev, 1, 1, SAVEA );		/* Get the summary block	  */
 if (!bp) {
 	Report ("%scheck_blocks() : Failed to read summary block !", S_FATAL);
 	return (FALSE);
 }
 					/* Search block-number and offset */
 					/* of the /lost+found - entry 	  */
 blf = search_entry ( &bp->b_un.b_sum->root_dir.de_inode, "lost+found", &off );
 					/* Did we have success ?	  */
 if ( blf == (struct buf *) NULL )
 {					/* /lost+found is not available !! */
	if ( create_lostfound_inode ( &bp->b_un.b_sum->root_dir.de_inode ) )
	{
		Report ("%sHave created a new /lost+found directory.", S_INFO);
		lost_found = TRUE;	/* Note that we have a valid one. */
		
 					/* Search block-number and offset */
 					/* of the /lost+found - entry again*/
 		blf = search_entry ( &bp->b_un.b_sum->root_dir.de_inode, 
 				     "lost+found", &off );
 					/* Copy the /lost+found inode and  */
		memcpy ( &lf_inode, &blf->b_un.b_dir[off].de_inode, 
			 sizeof(struct inode) );
 				     	/* Write the modified root-dir	  */
		test_bwrite ( bp );	/* back to disk and release the	  */
					/* block with the /lost+found entry*/
		brelse ( blf->b_tbp, TAIL );
	}
	else				/* We were not able to create a	  */
	{				/* new /lost+found inode.	  */
		Report ("%sNo /lost+found directory available!", S_WARNING);
		brelse ( bp->b_tbp, TAIL );
 		lost_found = FALSE;
 	}
 }
 else					/* We have directly found a valid */
 {					/* /lost+found inode.		  */
					/* Copy the /lost+found inode and  */
	memcpy ( &lf_inode, &blf->b_un.b_dir[off].de_inode, sizeof(struct inode) );
					/* ... release the spec. block.	  */
	brelse ( blf->b_tbp, TAIL );
 	brelse ( bp->b_tbp, TAIL );	/* Release the summary block	  */
	lost_found = TRUE;
 }

				/* We need a valid /lost+found - directory */
 if ( lost_found &&		/* and at least one 'lost' block put into */
      fst.lost_blocks )		/* the hash-table for lost blocks to start*/
 {				/* further examinations.		  */
				/* Now compare the table of lost blocks   */
				/* with the table of referred blocks to   */
 				/* find 'local' root-blocks. These are 	  */
	search_local_roots ();	/* marked by the use of the "root"-flag   */
 

DEBdebug ("	check_blocks :	Handle lost directory-blocks which are 'local roots'");

 					/* Scan the 'lost' hash-table	  */
 	for (   i = 0 , changes = 0 ;
       	      ( i < LOSTHASHSZ && lost_found ) ; i++ )
 	{	
		lpt = lost_hash_tab[i]. lostnxt;
					/* Follow each row up to the end  */
		while ( lpt != NULL && lost_found )
		{			/* We create an entry only, if    */
					/* the block is a 'root-block'	  */
			if ( lpt->root )	
			{
				if ( ! slot_available ( &lf_inode ) )
				{
					/* We have to terminate, because  */
					/* the /lost+found-dir is full!!  */
						lost_found = FALSE;
						continue;
				}
				

DEBdebug ("	check_blocks :	Take block no %d as a 'local root block'",
	 lpt->bnr );

				/* workon_lost_block() validates all en - */
				/* tries in the lost block (eventually 	  */
				/* with sub-directory trees		  */
				
				val_entries = workon_lost_block ( lpt->bnr );
				
					/* We need at least one valid en- */
					/* try in the 'local root-block'  */
					/* to create an entry in the      */
					/* /lost+found - directory	  */
				if ( val_entries )
				{	

DEBdebug ("	check_blocks :	Create an entry for the block (valid=%d)",
	 val_entries );

				/* Allocate an empty slot from the /lost- */
				/* found-dir. We know that a free slot is */
				/* available, because we have looked for  */
				/* one with slot_available() before !	  */
				
					get_free_slot ( &lf_inode, &lf_bnr, &lf_off );
					
					/* Add the block to the reference */
					/* bitmap array			  */
					bitmap_incr ( lpt->bnr );
					found_blocks++;
					
					/* Create a new /lost+found-entry  */
					create_lost_entry ( lpt->bnr, lf_bnr, lf_off,
							    val_entries );
					
					/* Correct the /lost+found-inode  */
					lf_inode.i_spare++;
					lf_inode.i_size += sizeof (struct dir_elem);
					
					/* Note the modification	  */
					changes++;
					fst.lost_found_blocks++;
				}
				else
				{

DEBdebug ("	check_blocks :	The block does not keep enough valid entries.");

				}
			}		/* Take the next element in the	  */
					/* row ...			  */
			lpt = lpt->lostnxt;
		}
 	} /* end <for (i)> */
 } /* enf <if (lost_found)> */
					/* Write updated /lost+found-inode */
					/* on disk if there were changes: */
 if ( changes )
 {

DEBdebug ("	check_blocks :	The /lost+found-inode has to be corrected");

 	bp = bread ( cdev, 1, 1, SAVEA );	/* Get the summary-block	  */
	if (!bp) {
		Report ("%scheck_blocks() : Failed to read summary block !", S_FATAL);
		return (FALSE);	
	}
 					/* Find block /lost+found direc -  */
 					/* tory-entry again 		  */
 	blf = search_entry ( &bp->b_un.b_sum->root_dir.de_inode, "lost+found", &off );
 					/* Release the summary-block	  */
 	brelse ( bp->b_tbp, TAIL );
					/* We have found it ?		  */
	if ( blf == (struct buf *) NULL )
		Report ("%sThe /lost+found - directory is definitely not accessable",
			  S_WARNING);
	else
	{				/* Copy the modified /lost+found - */
					/* inode back to the sum - block  */
		memcpy ( &blf->b_un.b_dir[off].de_inode, &lf_inode, sizeof (struct inode) );
		Report ("%sUpdate /lost+found - inode on disk.", S_INFO);
		test_bwrite ( blf );
	}

 }

 remove_lost_hash ();			/* Free all elements in the hash- */
 					/* structures for lost blocks	  */ 
 remove_ref_hash ();			/* Free also all reference structs*/

/*----  Pass 3 : Look for multiple allocated blocks and handle links  ----*/

 Report ("%s***    Step 3.3 : Look for multiple referred blocks.", S_INFO);
 
 ignore_links = FALSE;			/* At default, we don't ignore	  */
					/* 'hanging' symbolic links.	  */

 remove_dup_hash ();			/* Free the dup hash-structures	  */
					/* Step through the cyl. groups	  */
 for ( cgnr = 0 , fst.dup_blocks = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {					/* Deal with each block number in */
 					/* every cg reference map	  */
 	for ( bcnt = 0 ; bcnt < i_fs.fs_cgsize ; bcnt++ )
 	{
 					/* A multiple allocated block	  */
 		if ( ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) > 1 )
 		{

DEBdebug ("	check_blocks :	Block No %d is multiple allocated.",
	 cgnr * i_fs.fs_cgsize + bcnt );

			fst.dup_blocks++;
					/* Put the block number and the	  */
					/* ref-counter into the hash-table*/
			append_dup_bnr ( cgnr * i_fs.fs_cgsize + bcnt,
					 bit_maps[cgnr][bcnt] & MASK_REFCNT );
 		}
 	}
 }
 				/* If there were any multiple allocated	  */
				/* blocks found or symbolic links in the  */
 if ( fst.dup_blocks || 	/* file-system, we have to perform a se - */
      found_links )		/* cond pass over the file-system to find */
 {				/* the corresponding entries		  */
	if ( fst.dup_blocks )		/* Look for dup block numbers in  */
	{				/* the root inode.		  */

DEBdebug ("	check_blocks :	Found %d multiple allocated blocks",
	 fst.dup_blocks );

					/* Get the summary - block	  */
		bp = bread ( cdev, 1, 1, SAVEA );	
		if (!bp) {
			Report ("%scheck_blocks() : Failed to read summary block !", S_FATAL);
			return (FALSE);	
		}
					/* Work on the block references	  */
					/* held in the root - inode	  */
		entry_dup_bnr ( &bp->b_un.b_sum->root_dir, 1, 0 );
		brelse ( bp->b_tbp, TAIL );	
	}
			/* Traverse the whole directory-tree to find the  */
			/* entries which refer a multiple allocated block */
			/* or are referred by a symbolic link		  */

	if ( fst.dup_blocks ) {		/* We make always a second pass,  */
					/* if we have dup blocks.	  */
 		if (!handle_dups_and_links ()) {
 			return (FALSE);	
 		}
 	}
 	else				/* If we have no dup blocks but	  */
 	{				/* symbolic links, we skip	  */
 		if ( found_links )	/* the 2nd pass.	  	  */
 		{
 			
 /* 
  * 'HANGING' SYMBOLIC LINKS ARE NOT FATAL AND DO NOT HAVE INFLUENCE ON
  * THE BASIC CONSISTENCY OF THE FILE-SYSTEM. IT TAKES EVENTUALLY A VERY
  * LONG TIME TO TRAVERSE THE DIRECTORY TREE A SECOND TIME (think of a 1 GByte
  * partition!). THIS IS NOT ACCEPTABLE FOR A BASIC CHECK WHICH IS MADE
  * E V E R Y TIME WHEN THE FILE-SYSTEM IS SET-UP.
  */
  
 			if (!handle_dups_and_links ()) {
 				return (FALSE);	
 			}
 /*
 			Report ("%s'Hanging' links are ignored.", S_WARNING);
 			ignore_links = TRUE;
 */
 		}
 
 	}
			/* After looking at all entries, we have to work  */
			/* on the generated hash-structure to decide for  */
			/* each block, which entry should claim it.	  */
	if ( fst.dup_blocks )
	{
 		decide_on_dups ();
 		remove_dup_hash ();	/* Free all memory used for sto-  */
					/* rage of dup entries		  */
	}
 }
 
 return TRUE;
}


/*===========  Procedures dealing with 'lost' directory blocks  =========*/

/*************************************************************************
 * EXAMINE THE /lost+found DIRECTORY TO FIND AN EMPTY SLOT FOR A NEW
 * ENTRY
 *
 * - The operation succeeds, if we find a direct block entry which keeps
 *   no block reference or if we find a zeored slot in an allocated block.
 *
 * Parameter :	lfpt	= Pointer to the /lost+found inode
 * Return    :  TRUE    : If a slot is available
 *		FALSE   : If no slot is available
 *
 ************************************************************************/
static word
slot_available ( struct inode *lfpt )
{
 word dbcnt, offcnt;
 struct buf *bp;
 struct dir_elem decmp;			/* Used for comparison		 */
 
					/* Clear the comparison struct	 */
 memset ( &decmp, 0, sizeof (struct dir_elem) );

/*--------------------  Work on the /lost+found - inode  -----------------*/ 


DEBdebug ("	slot_available :	Look for a free /lost+found-slot");

					/* Scan the list of direct blocks*/
 for ( dbcnt = 0 ; dbcnt < i_fs.fs_ndaddr ; dbcnt++ )
 {
 	if ( ! lfpt->i_db[dbcnt] )	/* A non allocated block	 */
 	{

DEBdebug ("	slot_available :	The %d. direct block keeps no reference",
	 dbcnt );

 		return TRUE;
 	}
 					/* Skip over invalid block-nums	 */
 					/* which are referred.		 */
 	if ( ! valid_bnr ( lfpt->i_db[dbcnt] ) )
 		continue;
 					/* Read the directory-block	 */	
 	bp = bread ( cdev, lfpt->i_db[dbcnt], 1, SAVEA );
	if (!bp) {
Report ("%sslot_available() : Failed to read directory block (bnr %d) !",S_FATAL,lfpt->i_db[dbcnt]);
		longjmp (term_jmp, DIR_BLK_ERR);
	}
 					/* Handle the inode-slots of this*/
 					/* allocated block		 */
 	for ( offcnt = 0 ; offcnt < i_fs.fs_maxdpb ; offcnt++ )
 	{				/* An empty slot ?		 */
 		if ( ! my_memcmp ( &decmp, &bp->b_un.b_dir[offcnt], 
 				sizeof(struct dir_elem) ) )
 		{

DEBdebug ("	slot_available :	Found a free slot on pos %d in the %d. direct block",
	 offcnt, dbcnt );

 			brelse ( bp->b_tbp, TAIL );
 			return TRUE;
 		}
 	}
 					/* Release the not further used	 */
 	brelse ( bp->b_tbp, TAIL );	/* directory block		 */
 } /* end <for (dbcnt)> */

 Report ("%sThe /lost+found-directory is full. All other blocks are lost!", S_WARNING);
 return FALSE;
}


/*************************************************************************
 * FIND AN EMPTY SLOT IN THE SPECIFIED DIRECTORY FOR A NEW ENTRY
 * TO BE PLACED THERE
 *
 * - At first, all allocated blocks are scanned.
 * - If they all are filled up, we try to allocate a new block.
 * - If this also fails, we know that the directory is full and
 *   we return with FALSE to signal this situation.
 * - This procedure is also used to grab free slots for new entries in 
 *   the /lost+found - directory.
 *
 * Parameter : ip       = Pointer to the inode of the parent directory
 *	       bnr	= Used to store the block in which a slot is found
 *	       off	= Used to specify where the slot resides in this block.
 * Return    : TRUE     : An empty slot was found
 *	       FALSE    : The directory is full
 *	
 *************************************************************************/
word
get_free_slot ( struct inode *ip, daddr_t *bnr, word *off )
{
 word dbcnt, offcnt;
 daddr_t new_bnr;
 struct buf *bp;
 struct dir_elem decmp;			/* Used for comparison		 */
 
					/* Clear the comparison struct	 */
 memset ( &decmp, 0, sizeof (struct dir_elem) );

/*-----------  At first examine only allocated direct blocks  -----------*/ 


DEBdebug ("	get_free_slot :	Search an unused slot in an allocated direct blocks");

 					/* Look for an allocated direct	 */
					/* block in the parent-dir	 */
 for ( dbcnt = 0 ; dbcnt < i_fs.fs_ndaddr ; dbcnt++ )
 {
 				 	/* Take only blocks which were.. */
 	if ( valid_bnr (ip->i_db[dbcnt]) )
 	{				/* ... previously allocated:	 */
 					/* Read them into cache		 */
 		bp = bread ( cdev, ip->i_db[dbcnt], 1, SAVEA );
		if (!bp) {
Report ("%sget_free_slot() : Failed to read directory block (bnr %d) !",S_FATAL,ip->i_db[dbcnt]);
			longjmp (term_jmp, DIR_BLK_ERR);
		}
 					/* Work on all directory-entries */
 		for ( offcnt = 0 ; offcnt < i_fs.fs_maxdpb ; offcnt++ )
 		{			/* Look for an empty slot	 */
 			if ( ! my_memcmp ( &decmp, &bp->b_un.b_dir[offcnt],
 					sizeof ( struct dir_elem ) ) )
 			{		

DEBdebug ("	get_free_slot :	Free slot found the %d. direct block (bnr=%d) at pos %d",
	 dbcnt, ip->i_db[dbcnt], offcnt );

 					/* Save block number and offset	 */
				*bnr = ip->i_db[dbcnt];
				*off = offcnt;
				brelse ( bp->b_tbp, TAIL );
				return TRUE;
			}
		}
					/* We have not found a free slot */
					/* in THIS block and take a next */
					/* one ...			 */
		brelse ( bp->b_tbp, TAIL );
 	}
 }
 
 /*---------  Allocate a new block and find a free slot in it  ----------*/


DEBdebug ("	get_free_slot :	Allocate a new block and search an unused slot.");

  	
  					/* Try to allocate a "new" block */
 for ( dbcnt = 0 ; dbcnt < i_fs.fs_ndaddr ; dbcnt++ )
 {
 	if( ! valid_bnr ( ip->i_db[dbcnt] ) )
 	{				/* Allocate the first available	 */
 					/* block for the inode.		 */
		new_bnr = new_alloc ();
 		if ( new_bnr == -1 )	/* Allocation failed		 */
 		{
			 Report ("%sNo more free blocks available!", S_WARNING);
 			 return FALSE;
 		}
#if DEBUG
 		else
 			 DEBdebug ("%sBlock no %d is allocated.", S_INFO, new_bnr);
#endif
		 

DEBdebug ("	get_free_slot :	Block no %d is noted in the directory on pos %d",
	 new_bnr, dbcnt );

 					/* Modify the /lost+found-inode	 */
 		ip->i_db[dbcnt] = new_bnr;
 		ip->i_blocks++;		/* One block more allocated	 */
 					
 					/* Find a free slot in the newly */
 					/* allocated block:		 */	
 		bp = bread ( cdev, ip->i_db[dbcnt], 1, SAVEA );
		if (!bp) {
Report ("%sget_free_slot() : Failed to read directory block (bnr %d) !",S_FATAL,ip->i_db[dbcnt]);
			longjmp (term_jmp, DIR_BLK_ERR );
		}
 					/* Work on all directory-entries */
 		for ( offcnt = 0 ; offcnt < i_fs.fs_maxdpb ; offcnt++ )
 		{
 					/* Zeroed entries are free slots */
 			if ( ! my_memcmp ( &decmp, &bp->b_un.b_dir[offcnt],
 						sizeof ( struct dir_elem ) ) )
 			{		

DEBdebug ("	get_free_slot :	Found a free slot in the %d. direct block at pos %d",
	 dbcnt, offcnt );

 					/* Save block number and offset  */
				*bnr = ip->i_db[dbcnt];
				*off = dbcnt;
				brelse ( bp->b_tbp, TAIL );
				return TRUE;
			}
		} 		
		brelse ( bp->b_tbp, TAIL );
	}
 }
					/* We hopely never reach this	 */
					/* code - region		 */
 Report ("%sThe specified directory is full!", S_WARNING);
 return FALSE;
}


/*************************************************************************
 * ALLOCATE A NEW BLOCK FOR NEW DIRECTORY-ENTRIES
 *
 * - The strategy to find such a block is fairly simple: The first block
 *   which is noted in reference-map AND original bitmap as free is taken.
 * - This procedure is also used to grab a new block for new entries like
 *   the /lost+found - directory itself.
 *
 * Parameter : - nothing - 
 * Return    : The number of the allocated block
 *
 ************************************************************************/
static daddr_t
new_alloc ( void )
{
 word cgnr, bcnt;
 struct buf *bip, *bp;
 

DEBdebug ("	new_alloc :	Allocate a new block for the /lost+found - dir");

					/* Deal with each cylinder-group */
 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {
 					/* Get the info-block		 */
 	bip = bread ( cdev, info_blocks[cgnr], 1, SAVEA );
	if (!bip) {
		Report ("%snew_alloc() : Failed to read info block %d (bnr %d) !",S_FATAL,cgnr,info_blocks[cgnr]);
		longjmp (term_jmp, BAD_INFO_BLK);			
	}
 					/* Deal with all blocks in 	 */
 					/* each of the cylinder-groups	 */
 	for ( bcnt = 0 ; bcnt < i_fs.fs_cgsize ; bcnt++ )
 	{				
 					/* We have found a free block	 */
		if ( ( bit_maps[cgnr][bcnt] & MASK_REFCNT ) == 0 &&
		       bip->b_un.b_info->cgx.cg_free[bcnt] == 0 &&
		       valid_bnr ( cgnr * i_fs.fs_cgsize + bcnt ) )
		{			

DEBdebug ("	new_alloc :	Block no %d is free and now allocated",
	cgnr * i_fs.fs_cgsize + bcnt );

					/* Note the allocation:		 */
			bip->b_un.b_info->cgx.cg_free[bcnt] = 0xff;
			bitmap_incr ( cgnr *  i_fs.fs_cgsize + bcnt );
			found_blocks++;
					/* Write the modified info-block */
					/* back to disk			 */
			test_bwrite ( bip );
					/* Read the "new" block ,	 */
			bp = bread ( cdev, cgnr*i_fs.fs_cgsize+bcnt, 1, SAVEA );
			if (!bp) {
Report ("%snew_alloc() : Failed to read block (bnr %d) !",S_FATAL,cgnr*i_fs.fs_cgsize+bcnt);
				longjmp (term_jmp, READ_ERR );
			}
					/* ... erase it's content and ...*/
			clr_buf ( bp );
					/* ... write it back to disk.	 */
			test_bwrite ( bp );
					/* Return with the block number	 */
		     	return ( cgnr * i_fs.fs_cgsize + bcnt );
		}
 	} /* end <for (bcnt)> */
					/* Release the non-used info-	 */
					/* block.			 */
	brelse ( bip->b_tbp, TAIL );
 } /* end <for (cgnr)> */
 					/* If we have not found any free */
 return -1; 				/* block (file-sys is full !)	 */
}


/*************************************************************************
 * VALIDATE ENTRIES WHICH ARE KEPT BY A 'LOST' DIRECTORY-BLOCK
 *
 * - Each entry is validated by the use of validate_entry()
 * - Subdirectories are scanned by using check_inodes() operating on
 *   these 'local roots'
 * - Note that there must be at least one valid entry to succeed with
 *   workon_lost_block() !
 *
 * Parameter : lbnr	= The lost block to be inspected and validated
 * Return    : The number of valid entries found in the block
 *
 ************************************************************************/
static word
workon_lost_block ( daddr_t lbnr )
{
 struct buf *bp;
 struct dir_elem de_comp, de;
 word icnt, valid_entries, changes;
 

DEBdebug ("	workon_lost_block :	Validate the entries in 'lost' block no %d",
	 lbnr );

 					/* Clear the comparison structure:*/
 memset ( &de_comp, 0, sizeof (struct dir_elem) );
 
/*-----------  Work with each entry in the directory-block  --------------*/

 					/* Read the 'lost' directory-block*/
 bp = bread ( cdev, lbnr, 1, SAVEA );
 if (!bp) {
Report ("%sworkon_lost_block() : Failed to read directory block (bnr %d) !",S_FATAL,lbnr);
	longjmp (term_jmp, DIR_BLK_ERR );
 }
					/* Validate all directory-entries:*/
 for ( icnt = 0 , valid_entries = 0 , changes = 0 ;	
       icnt < i_fs.fs_maxdpb ; icnt++ )
 {
 					/* Skip over empty slots	  */
 	if ( ! my_memcmp ( &de_comp, &bp->b_un.b_dir[icnt], 
 	                sizeof (struct dir_elem) ) )
 		continue;
 		
 	changes_de = 0;			/* At the beginning: no changes   */
 	
/*-------  We have to validate each entry and to look for sub-dirs  ------*/ 

	if ( validate_entry ( &bp->b_un.b_dir[icnt], NULL, UNKNOWN, TRUE ) )
	{

GEPdebug ("	workon_lost_block :	A valid entry '%s' is found on slot %d",
	 &bp->b_un.b_dir[icnt].de_name, icnt );

		valid_entries++;	/* Another valid entry ...	  */
		if ( changes_de )	/* Were there any changes made du-*/
		{			/* ring validate_entry() ?	  */
			changes++;
			changes_de = 0;
		}
					/* Sub-directory trees to be exa- */
					/* mined ?			  */
		if ( bp->b_un.b_dir[icnt].de_inode.i_mode == Type_Directory )
		{

			Report ("%s'%s' is a Subdirectory and will be used as 'local' root",
	 			    S_INFO, &bp->b_un.b_dir[icnt].de_name );
	 			    
					/* Before we work on the sub-dir  */
					/* tree, we have to release the   */
					/* 'lost' block (for safety...)	  */
			if ( changes_de || changes )
			{

GEPdebug ("	workon_lost_block :	Write 'lost' block temporarily on disk");

				test_bwrite ( bp );
			}
			else		
			{

GEPdebug ("	workon_lost_block :	Release 'lost' block");

				brelse ( bp->b_tbp, TAIL );
			}
				
					/* Prepare a copy of the direc -  */
					/* tory entry to be 'root - inode'*/
					/* for check_inodes()		  */
			memcpy ( &de, &bp->b_un.b_dir[icnt], sizeof (struct dir_elem) );
					/* Check this sub-tree		  */
			check_inodes ( &de, lbnr, icnt );
					/* Read again the 'lost' block	  */

GEPdebug ("	workon_lost_block :	Read 'lost' block %d again",lbnr);

			bp = bread ( cdev, lbnr, 1, SAVEA );
			if (!bp) {
Report ("%sworkon_lost_block() : Failed to read directory block (bnr %d) !",S_FATAL,lbnr);
				longjmp (term_jmp, DIR_BLK_ERR );
			}
		}
	}
	else				/* Invalid entries will be de -	  */
	{				/* leted 			  */
		memset ( &bp->b_un.b_dir[icnt], 0, sizeof (struct dir_elem) );
		changes++;
	}
 }
 
 if ( changes )				/* If we have made any changes to */
 {					/* the entries in the lost block: */
 	Report ("%sUpdate 'lost' directory-block finally on disk\n", S_INFO);
 	test_bwrite ( bp );
 }
 else
 	brelse ( bp->b_tbp, TAIL );

 return valid_entries;		/* Return with the number of valid entries*/
 				/* found in the block. At least one entry */
 				/* must be valid to allow creation of a   */
 				/* new entry in /lost+found.		  */
}


/*************************************************************************
 * CREATE A NEW ENTRY FOR THE 'LOST' BLOCK IN THE GIVEN 'SLOT'
 *
 * - We take the slot we have got from get_free_slot() and create an entry
 *   for a subdirectory with all needed information.
 *   
 * Parameter : lostbnr 	= The block to be placed
 *	       lfbnr    = The direct block in /lost+found in which we have
 *			  to put the entry 
 *	       dbslot   = The offset into this direct blocks in /lost+found
 *	       nentries = The number of valid entries found for this block
 *
 *---------------------------------------------------------------------------
 *
 * Return    : TRUE   , if an entry was succesfully created for the block
 *	       FALSE  , if we have not found a free slot to place the new
 *			entry in.
 *
 * ^^^ No sir !!!
 *
 ************************************************************************/
static word
create_lost_entry ( daddr_t lostbnr, daddr_t lfbnr, word dbslot, word nentries )
{
 char tbuf[80];
 struct buf *blfp;
 struct dir_elem *dp;
 Date date;
 	

DEBdebug ("	create_lost_entry :	Create an entry for block %d in %d at position %d",
	 lostbnr, lfbnr, dbslot );

					/* Get the specified block from	*/
 blfp = bread ( cdev, lfbnr, 1, SAVEA);	/* the /lost+found-directory.	*/
 if (!blfp) {
Report ("%screate_lost_entry() : Failed to read directory block (bnr %d) !",S_FATAL,lfbnr);
	longjmp (term_jmp, DIR_BLK_ERR );
 }		
 dp = &blfp->b_un.b_dir[dbslot];	/* To ease handling ...		*/
 					/* Clear the comparison struct	*/
 memset ( dp, 0, sizeof (struct dir_elem) );
	
					/* Build an unique entry-name	*/
 strcpy ( dp->de_name, "lf_" );
 memset ( tbuf, 0, NameMax);
 my_itoa ( tbuf, lfbnr * i_fs.fs_maxdpb + dbslot );
 strcat ( dp->de_name, tbuf ); 
 					/* Prepare the inode:		*/
 dp->de_inode.i_mode   = Type_Directory;
 dp->de_inode.i_matrix = DefDirMatrix;
 dp->de_inode.i_db[0]  = lostbnr;
 date = GetDate ();
 dp->de_inode.i_ctime  = date;					
 dp->de_inode.i_mtime  = date;					
 dp->de_inode.i_atime  = date;				
 dp->de_inode.i_spare  = nentries;
 dp->de_inode.i_size   = nentries * sizeof (struct dir_elem);
 dp->de_inode.i_blocks = 1;
	
 Report ("%sUpdate modified /lost+found directory block on disk", S_INFO);
					/* Write the block with the new	*/
 test_bwrite ( blfp );			/* entry back to disk		*/
}



/************************************************************************
 * TEST A BLOCK FOR "POSSIBLE LOST DIRECTORY BLOCK"
 *
 * - The block must be a directory-block.
 * - There must be at least on valid entry in the block
 * - Entries which refer multiple allocated blocks in their inodes are
 *   deleted.
 * - To avoid problems with non-deleted directory informations in
 *   blocks which are not touched by the 'logical' formatting routine
 *   "mkfs", the creation date of each directory entry is compared with
 *   the creation date of the actual superblock. If an entry is older
 *   than the superblock, it is ignored and it's content will be deleted.
 *
 * Parameter :	bnr 	= The block to be inspected
 * Return    :	TRUE ,  if it seems to be a lost directory-block with at 
 *		        least one valid entry. (which does not claim multiple 
 *		        allocated blocks...)
 *		FALSE, if it does not seem to be a lost directory-block
 *
 ************************************************************************/
static word
poss_lost_dir_block ( daddr_t bnr )
{
 struct buf *bp;
 struct dir_elem de;
 word icnt, mcnt, bcnt, changes, val_refs, valid_entries;
 

DEBdebug ("	poss_lost_dir_block :	Test block no %d for 'lost directory block'",
	 bnr );

					/* Clear the comparison struct  */
 memset ( &de, 0, sizeof (struct dir_elem) );
 
					/* 1. The block MUST BE OF TYPE */
					/* DIRECTORY !			*/
 bp = bread ( cdev, bnr, 1, SAVEA );	/* Read the block into cache	*/
 if (!bp) {
Report ("%sposs_lost_dir_block() : Failed to read block (bnr %d) !",S_FATAL,bnr);
	longjmp (term_jmp, READ_ERR );
 }
 if ( possible_dir_block ( bp ) < 1 )	/* We can only work with this 	*/
 {					/* block, if it is of type-dir	*/
 					/* with at least one valid en-	*/
 					/* try kept in it.		*/

DEBdebug ("	poss_lost_dir_block :	The block is NOT of type directory and will be ignored");

	brelse ( bp->b_tbp, TAIL );		
 	return FALSE;
 }
					/* 2. The block MUST NOT REFER	*/
					/* blocks (..in the inodes..), 	*/
					/* which are MULTIPLE ALLOCATED */
 for ( icnt = 0 , mcnt = 0 , val_refs = 0 , changes = 0 , valid_entries = 0 ;
       icnt < i_fs.fs_maxdpb ; icnt++ )
 {					/* Skip over zeroed entries	*/
 	if ( ! my_memcmp ( &de, &bp->b_un.b_dir[icnt], sizeof (struct dir_elem) ) )
 		continue;
 					/* Make tests for "possible"	*/
 					/* directory-entries :		*/	
 	if ( ( bp->b_un.b_dir[icnt].de_inode.i_mode == Type_Directory ||
 	       bp->b_un.b_dir[icnt].de_inode.i_mode == Type_Link ||
 	       bp->b_un.b_dir[icnt].de_inode.i_mode == Type_File ) &&
 	       valid_entry_name ( bp->b_un.b_dir[icnt].de_name , FALSE ) )
 	{
 					/* The direct blocks		*/
	 	for ( bcnt = 0 ; bcnt < i_fs.fs_ndaddr ; bcnt++ )
 		{
 			if ( valid_bnr ( bp->b_un.b_dir[icnt].de_inode.i_db[bcnt] ) )
 			{
 				if ( bitmap_get ( bp->b_un.b_dir[icnt].de_inode.i_db[bcnt] ) > 1 )
 					mcnt++;
 				else
 					val_refs++;
 			}
 		}
 					/* The levels of indirection	*/
 		for ( bcnt = 0 ; bcnt < i_fs.fs_niaddr ; bcnt++ )
 		{
 			if ( valid_bnr ( bp->b_un.b_dir[icnt].de_inode.i_db[bcnt] ) )
 			{
 				if ( bitmap_get ( bp->b_un.b_dir[icnt].de_inode.i_ib[bcnt] ) > 1 )
 					mcnt++;
 				else
 					val_refs++;
 			}
 		}
 					/* If there is one or more mul- */
 		if ( mcnt > 0 )		/* tiple allocated block, we 	*/
 		{			/* delete the directory - entry.*/
 			changes++;	/* Note the modification	*/

DEBdebug ("	poss_lost_dir_block :	'%s' claims %d dup blocks and will be deleted",
	 &bp->b_un.b_dir[icnt].de_name, mcnt );

 					/* Erase the non-usable direc-  */
 					/* tory structure.		*/
 			memset ( &bp->b_un.b_dir[icnt], 0, sizeof (struct dir_elem) );

 		}
					/* If there is any valid refe-	*/
					/* rence which does not refer to*/
					/* a dup block, we find val_refs*/
					/* greater than zero:		*/
		if ( mcnt == 0 && val_refs )
 			valid_entries++;	
	}	
 } /* end <for (icnt)> */
	
 if ( changes )				/* Any changes made to the inode*/
 {
 	Report ("%sWrite modified 'lost' directory-block to disk", S_INFO);
 	test_bwrite ( bp );
 }
 else
 	brelse ( bp->b_tbp, TAIL );
 	
 if ( valid_entries )			/* We need at least one valid 	*/
 {					/* entry to succeed.		*/

DEBdebug ("	poss_lost_dir_block :	Found %d usable entries in the lost dir-block",
	 valid_entries );

 	return TRUE;		
 }
 else
 {

DEBdebug ("	poss_lost_dir_block :	The block is not a usable 'lost directory block'");

 	return FALSE;
 }
}

/*-----------  Procedures working on 'lost' hash-tables  ----------------*/

/*************************************************************************
 * COMPARE 'LOST' BLOCKS AND THEIR REFERENCES TO FIND 'LOCAL' ROOTS
 *
 * - The two hash-tables - for 'lost' blocks and for the blocks they refer -
 *   are compared to find out, which block numbers are not referred by any
 *   other entry. 
 * - These blocks are noted as 'root' to signal, that they should be used
 *   later on as 'local root-directories' by check_inodes() 
 * - All reference block-numbers which refer to lost blocks are removed
 *   from the reference hash-table to speed-up execution.
 * 
 * Parameter : - nothing -
 * Return    : - nothing -
 *
 ************************************************************************/
static void
search_local_roots ( void )
{
 word locnt, ri;
 struct lost_bnr *lpt;
 struct ref_bnr *rpt, *rpt_last, *rpt_old;
 

DEBdebug ("	search_local_roots :	Find 'local roots' by comparison of the hash-tabs");

					/* Deal with all 'lost' blocks	*/
					/* noted in the hash-table	*/
 for ( locnt = 0 ; locnt < LOSTHASHSZ ; locnt++ )
 {					/* Begin with the first element */
 	lpt = lost_hash_tab[locnt].lostnxt;
 					/* ..and follow the row up to 	*/
 					/* it's end			*/
 	while ( lpt != (struct lost_bnr *) NULL )
 	{
			/* Now we have to take a look at the hash-table	*/
			/* for block references of 'lost' blocks to 	*/
			/* find out whether the lost block is referred. */
					
					/* Calculate the hash - index	*/
					/* based on the lost bnr	*/
		ri = calc_ref_hash ( lpt->bnr );
					/* Start with the first element */
					/* in the hash-table		*/
		rpt = ref_hash_tab[ri].refnxt;
		rpt_last = (struct ref_bnr *) NULL;
					/* ... and follow the row up to */
					/* it's end			*/
		while ( rpt != (struct ref_bnr *) NULL )
		{			
			if ( lpt->bnr == rpt->bnr )
			{		/* We can't use the block-number*/
					/* as a local root !		*/

DEBdebug ("	search_local_roots :	Block no %d is NOT a 'local root block'",
	 lpt->bnr );
 					/* Note that the block is not   */
					/* usable as a local root-block	*/
				lpt->root = FALSE;
					/* The first element has to be	*/
					/* taken from the queue ?	*/
				if ( rpt == ref_hash_tab[ri].refnxt )
					ref_hash_tab[ri].refnxt = rpt->refnxt;
				else
					rpt_last->refnxt = rpt->refnxt;
				
					/* Unchain the element from the */
					/* reference-blocks queue.	*/
				rpt_old = rpt;
				Free ( rpt_old );
				break;
			}		/* Get the next ref - block   	*/
			rpt_last = rpt;
			rpt = rpt->refnxt;
		}
					/* If we finished with unequal  */
					/* block-numbers, we have a lo- */
					/* cal root-block.		*/
		if ( lpt->bnr != rpt->bnr )
		{		

			Report ("%sBlock no %d is a non-referred 'local root block' !",
	 			    S_INFO, lpt->bnr );
		}
		
		lpt = lpt->lostnxt;	/* Take the next element in the */
 					/* chain of lost block numbers  */
 	} /* end <while (lpt)> */
 } /* end <for (locnt)> */
}


/*---------  Hash queue handling for 'lost' directory-blocks  -----------*/

/*************************************************************************
 * APPEND AN ENTRY TO THE HASH-TABLE FOR 'LOST' BLOCKS
 *
 * - It is guaranteed that we note a block number at maximum one time,
 *   because we "follow" the reference map of each cg (sequentially
 *   increased block numbers)
 *
 * Parameter :	lbnr	= The lost block-number
 * Return    :  - nothing -
 *
 ************************************************************************/
static void
append_lost_bnr ( daddr_t lbnr )
{
 word index;
 struct lost_bnr *lpt;
 	
 index = calc_lost_hash ( lbnr );	/* Calculate the hash-index	*/
 

DEBdebug ("	append_lost_bnr :	Append lost block no %d to hash-table on slot %d",
	 lbnr, index );

					/* Allocate memory for the new	*/
					/* entry			*/
 lpt = (struct lost_bnr *) Malloc ( sizeof (struct lost_bnr) );
 if ( lpt == (struct lost_bnr *) NULL )
 {

DEBdebug ("%sappend_lost_bnr() : Unable to allocate memory !", S_FATAL);

 	longjmp (term_jmp, MEMERR);
 }
 lpt->bnr    = lbnr;			/* Save the 'lost' block number */
 lpt->root   = TRUE;			/* 'root' will be set to FALSE  */
 					/* if THIS lost bnr is found in */
 					/* the reference table during   */
 					/* search_local_roots()		*/
					
					/* Insert the element on the	*/
					/* head-position of the queue	*/
 lpt->lostnxt = lost_hash_tab[index].lostnxt;
 lost_hash_tab[index].lostnxt = lpt;
}

 
/*************************************************************************
 * FREE ALL MEMORY USED FOR KEEPING OF 'LOST' BLOCKS
 *
 * Parameter :	- nothing -
 * Return    :  - nothing -
 *
 ************************************************************************/
void
remove_lost_hash ( void )
{
 word i;
 struct lost_bnr *lpt, *lpt_nxt;


DEBdebug ("	remove_lost_hash :	Free 'lost' block number hash-table");

					/* Work on each row of the hash- */
					/* table			 */
 for ( i = 0 ; i < LOSTHASHSZ ; i++ )
 {
 	lpt = lost_hash_tab[i].lostnxt;
 					/* Up to the end of the row ...	 */
 	while ( lpt != (struct lost_bnr *) NULL )
 	{
 		lpt_nxt = lpt->lostnxt;
 		Free ( lpt );
 		lpt = lpt_nxt;
 	}	
					/* Re-init the head-pointers	 */
	lost_hash_tab[i].lostnxt = (struct lost_bnr *) NULL;
 }
}

/*----------  Hash-queue handling for 'lost-referred' blocks  -----------*/

/*************************************************************************
 * APPEND AN ENTRY TO THE HASH-TABLE FOR BLOCKS WHICH ARE REFERRED BY
 * 'LOST' BLOCKS
 *
 * - The 'lost' block is read and all directory-entries are examined.
 * - All direct block references are put into the hash-table. 
 *   Block numbers which are kept in this table before performing 
 *   this pass are skipped to avoid multiple references in the hash-table.
 *
 * Parameter :	lbnr	= The lost block 
 * Return    :  - nothing -
 *
 ************************************************************************/
static void
append_ref_bnr ( daddr_t lbnr )
{
 word icnt, dbcnt, index;
 struct ref_bnr *rpt;
 struct buf *bp;
 struct dir_elem de, *dp;


DEBdebug ("	append_ref_bnr :	Examine references kept in block %d",
	 lbnr );


 bp = bread ( cdev, lbnr, 1, SAVEA );	/* Get the lost block		*/
 if (!bp) {
Report ("%sappend_ref_bnr() : Failed to read block (bnr %d) !",S_FATAL,lbnr);
	longjmp (term_jmp, READ_ERR );
 }
 					/* Clear the comparison struct	*/
 memset ( &de, 0, sizeof (struct dir_elem) );
 
 					/* Take a look at each entry	*/
 					/* in the directory-block	*/
 for ( icnt = 0 ; icnt < i_fs.fs_maxdpb ; icnt++ )
 {
 	dp = &bp->b_un.b_dir[icnt];	/* To ease handling ...		*/
 	
					/* Empty slots and non-directory*/
					/* entries are skipped.		*/
 	if ( ! my_memcmp ( &de, dp, sizeof (struct dir_elem) ) ||
 	       dp->de_inode.i_mode != Type_Directory )
 		  continue;
 		  

DEBdebug ("	append_ref_bnr :	Inspect the %d. inode held in the block",
	 icnt );

 	     				/* Otherwise we examine the	*/
 	     				/* direct blocks referred:	*/
 	for ( dbcnt = 0 ; dbcnt < i_fs.fs_ndaddr ; dbcnt++ )
 	{				/* Take the next direct block-  */
 					/* number immediately if the 	*/
 					/* block number is invalid.	*/
 		if ( ! valid_bnr ( dp->de_inode.i_db[dbcnt] ) )
 			continue;

DEBdebug ("	append_ref_bnr :	The %d. direct block refers a valid block",
	 dbcnt );

 					/* Calculate the hash-index	*/
		index = calc_ref_hash ( dp->de_inode.i_db[dbcnt] );

			/* If we find out that the block is already no- */
			/* ted in the table of reference block numbers, */
			/* we can leave it out and return directly.	*/
			
		rpt = ref_hash_tab[index].refnxt; 
					/* Follow the chain up to the	*/
					/* end of the row		*/
		while ( rpt != (struct ref_bnr *) NULL )
		{			/* The entry is contained in the*/
					/* list ?			*/
			if ( rpt->bnr == dp->de_inode.i_db[dbcnt] )
			{

DEBdebug ("	append_ref_bnr :	Reference to block %d found in the hash-table",
	 dp->de_inode.i_db[dbcnt] );

				brelse ( bp->b_tbp, TAIL );
				return;
			}
					/* Take the next element	*/
			rpt = rpt->refnxt;
		}
		

DEBdebug ("	append_ref_bnr :	Append reference to hash-table on slot %d",
	 index );

					/* Allocate memory for the NEW	*/
					/* entry			*/
		rpt = (struct ref_bnr *) Malloc ( sizeof (struct ref_bnr) );
		if ( rpt == (struct ref_bnr *) NULL )
		{

DEBdebug ("%sappend_ref_bnr() : Unable to allocate memory !", S_FATAL);

			longjmp (term_jmp, MEMERR);
		}
 		else			/* Save the block reference	*/
 		{
 			rpt->bnr = dp->de_inode.i_db[dbcnt];
					/* Insert the new element at the*/
					/* head of the hash-queue	*/
			rpt->refnxt = ref_hash_tab[index].refnxt;
			ref_hash_tab[index].refnxt = rpt;
		}
 	} /* end <for (dbcnt)> */
 } /* end <for (icnt)> */
 
 brelse ( bp->b_tbp, TAIL );
}

 
/*************************************************************************
 * FREE ALL MEMORY USED FOR KEEPING OF BLOCK-NUMBERS WHICH ARE REFERRED
 * BY LOST BLOCKS
 *
 * Parameter :	- nothing -
 * Return    :  - nothing -
 *
 ************************************************************************/
void
remove_ref_hash ( void )
{
 word i;
 struct ref_bnr *rpt, *rpt_nxt;


DEBdebug ("	remove_ref_hash :	Free lost block reference hash-table");

					/* Work on each row of the hash- */
					/* table			 */
 for ( i = 0 ; i < REFHASHSZ ; i++ )
 {
 	rpt = ref_hash_tab[i].refnxt;
 					/* Up to the end of the row ...	 */
 	while ( rpt != (struct ref_bnr *) NULL )
 	{
 		rpt_nxt = rpt->refnxt;
 		Free ( rpt );
 		rpt = rpt_nxt;
 	}	
					/* Re-init the head-pointers	 */
	ref_hash_tab[i].refnxt = (struct ref_bnr *) NULL;
 }
}

/*----------------------------------------------------------------------*/

/* end of concheck.c */
