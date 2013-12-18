/* 16.08.90 - Make all required cuts to build the "integrated" version
 *
 */

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
   |  xtdcheck.c							     |
   |                                                                         |
   |    Traversing the whole directory tree				     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - H.J.Ermen - 21 March 1989 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define	 DEBUG	    0
#define	 GEPDEBUG   0
#define	 FLDEBUG    0
#define  IN_NUCLEUS 1

#include "error.h"

#include "check.h"

/*---------------------  Local variables  ------------------------------*/

static char path[512];		/* For temporary pathname storage	*/
static struct dir_elem dir_cmp;	/* Used for comparison operation and is	*/
				/* therefore zeroed.			*/


/*************************************************************************
 * TESTS ON SINGLE DATA STRUCTURES LIKE INODES
 *
 * - This procedure is the central routine for working on the directory
 *   tree. A backtracking mechanism with no recursive procedure-overhead
 *   is used to avoid memory allocation and stack overflow problems.
 * - It is called as "step 2" during the checking process after working
 *   on unique data-structures and allocation all needed buffers for
 *   tables etc. 
 * - The root-inode was validated during a separate step in check_unique().
 *
 * Parameter  : root_de    = The directory-entry of the local root which
 *			     has to be examined
 *		root_bnr   = The number of the directory block which keeps
 *			     the root entry
 *		root_offs  = The offset in this directory block
 * Return     : TRUE    : under all normal checking conditions
 *		FALSE   : if a fatal error occurs
 *
 *************************************************************************/
word
check_inodes ( struct dir_elem *root_de, daddr_t root_bnr, word root_offs )
{
 word spare;				/* # of entries in the direct.	*/
 daddr_t bnr;				/* For temporary storage	*/
 word offset;
 word depth;				/* To keep track about the dis-	*/
 					/* tance from the root-direct.	*/

 struct buf *bpar;			/* Points to the current parent	*/
					/* directory - block		*/
 struct buf *blpt;			/* For symbolic-link reference- */
 					/* path block			*/
 tree_e head;
 word first_time;			/* Used to signal, if an entry	*/
					/* is scanned the first time	*/
 tree_e *parent;			/* Pointer to the actual parent */
					/* tree-element			*/
 tree_e *tpt;				/* Pointer to the actual wor-	*/
					/* king element.		*/
 struct dir_elem *de_pt;		/* Used for temporary storage	*/

 /*----------------------  Init operations  ----------------------------*/

 head.bnr = root_bnr;			/* Init the head element	*/
 head.offset = root_offs;
 head.parent_used = TRUE;
					/* Copy the root entry		*/
 memcpy ( (void *) &head.de, (void *) root_de, sizeof (struct dir_elem) );
 
 head.enxt = (tree_e *) NULL;		/* Zero the head-pointers	*/
 head.eprv = (tree_e *) NULL;
 head.lnxt = (tree_e *) NULL;
 head.lprv = (tree_e *) NULL;
					/* Clear the comparison struct	*/
 memset ( &dir_cmp, 0, sizeof (struct dir_elem) );

 strcpy ( path, "/" );			/* Set the name of the root	*/
 strcat ( path, head.de.de_name );	/* of the filesystem		*/

 spare = 0;				/* No entry at the beginning	*/
 first_time = TRUE;			/* Start up value		*/
 parent = &head;			/* At the beginning the head 	*/
 tpt    = &head;			/* element is the only one.	*/

 remove_de_hash ();			/* Free previously used hash-	*/
					/* table elements		*/
 changes_de = 0;			/* Nothing changed so far...	*/
 depth = 0;				/* We start on root-position	*/

 /*---------------  Traverse the whole directory tree  -----------------*/

 for ( ;; )
 {
					/* Get the next entry to check  */
	de_pt = get_next_entry ( &parent->de.de_inode, first_time, 
				 &bnr, &offset );
	first_time = FALSE;		/* We want to get all other en-	*/
					/* tries from this directory	*/
					/* We have got an entry ?	*/
	if ( de_pt != (struct dir_elem *) NULL )
	{
		strncpy (actual_path, path, 512);
#if 0
		Report ("%sChecking  %s / [%s]",
			    S_INFO, path, de_pt->de_name);
#endif		
					/* An entry was extracted. Now	*/
					/* check it's validity		*/
		if ( validate_entry ( de_pt, path, UNKNOWN, TRUE ) )
		{			
					/* Put the entry name into the	*/
					/* appropriate hash-queue	*/
			append_de_hash ( de_pt->de_name );
			
					/* We have to increment the 	*/
			spare++;	/* total count of entries	*/
			
/*--------------  Storage of symbolic-link information  ----------------*/

					/* Was it a symbolic link ?	*/
			if ( de_pt->de_inode.i_mode == Type_Link )
			{		/* Read the block with the re-	*/
					/* ference path in buffer-cache	*/
				blpt = bread ( cdev, de_pt->de_inode.i_db[0], 
					       1, SAVEA );
				if (!blpt) {
Report ("%scheck_inodes() : Failed to read link data block (bnr %d) !",S_FATAL,de_pt->de_inode.i_db[0]);
					longjmp ( term_jmp, LNK_BLK_ERR);
				}
					/* Append the link to the hash-	*/
					/* table for symbolic links	*/
				append_link_hash ( bnr, offset, 
						   parent->bnr, parent->offset,
						   blpt->b_un.b_link->name);
					/* One more symbolic link found */
				found_links++;
					/* The ref-block is not used	*/
					/* anymore			*/
				brelse ( blpt->b_tbp, TAIL );
			}

/*-----  Appending new elements to the hierarchical directory-tree  ----*/

			if ( de_pt->de_inode.i_mode == Type_Directory )
			{
					/* Increment the counter for 	*/
					/* the number of valid directo- */
					/* ries in a cylinder group.	*/
				add_dir ( bnr );
				found_dirs++;
								
				if ( tpt == parent )
					/* Add a new directory level	*/
					tpt = append_level ( tpt );	
				else
					/* .. or a new entry		*/
					tpt = append_entry ( tpt );
					/* Save the entry's location	*/
				tpt->bnr = bnr;
				tpt->offset = offset;
					/* Save the entry's content	*/
				memcpy ( &tpt->de, de_pt, sizeof (struct dir_elem) );	
					/* At this time NOT used as a 	*/
					/* parent-dir			*/
				tpt->parent_used = FALSE;

DEBdebug ("	check_inodes :	New element created for %s (bnr= %d, off= %d)",
	 tpt->de.de_name, tpt->bnr, tpt->offset );

			}
					/* Update the actual number of	*/
					/* file checked.		*/
			if ( de_pt->de_inode.i_mode == Type_File )
				found_files++;
		}
		else			/* If the validation does not	*/
		{			/* succeeds:			*/
					/* We have to make a note, if	*/
					/* /lost+found will be deleted ! */
			if ( depth == 0 &&
			     ! strcmp ( de_pt->de_name, "lost+found" ) )
			{
				Report ("%sUnusable /lost+found-dir !", S_WARNING);
				lost_found = FALSE;
			}
								
			Report ("%sThe entry is invalid and will be deleted!",
				  S_SERIOUS);
				  
			if ( ! no_corrections )
			{
				    	/* We cannot make use anymore	*/
				    	/* of the corrupted entry and	*/
				    	/* delete it.			*/
				memset ( (void *) de_pt, 0, sizeof (struct dir_elem) );
					/* Note the deletion		*/
				fst.deleted_inodes++;
				changes_de++;
			}
		}
	}

/*------  After checking a complete directory with all entries: --------*/

	else				/* One directory fully scanned:	*/
	{			

DEBdebug ("	check_inodes :	Have examined all entries. Change the directory");
DEBdebug ("			Current parent = %s     'used' = %d",
	 parent->de.de_name, parent->parent_used);
DEBdebug ("			Current entry  = %s     'used' = %d",
	 tpt->de.de_name, tpt->parent_used);


 /*-------  Go on with the last directory of the current sub-dir  ------*/

		if ( tpt->enxt == (tree_e *) NULL && 
		     tpt->lnxt == (tree_e *) NULL &&
		     tpt != parent &&
		     ! tpt->parent_used )		     
		{			

DEBdebug ("	check_inodes :	Take the last sub-dir as parent directory");



DEBdebug ("	check_inodes :  Spare value found = %d , counted = %d",
	 parent->de.de_inode.i_spare, spare );

					/* Different # of entries ? 	*/
			if ( spare != parent->de.de_inode.i_spare )
			{		/* Save the new spare value	*/
					/* Read the parent-directory 	*/

DEBdebug ("	check_inodes :	Correct number of entries (spare value)");

				bpar = bread ( cdev, parent->bnr, 1, SAVEA );
				if (!bpar) {
Report ("%scheck_inodes() : Failed to read directory block (bnr %d) !",S_FATAL,parent->bnr);
					longjmp ( term_jmp, DIR_BLK_ERR );					
				}
					/* Change the entry		*/
				if ( parent->bnr == 1 )
				{	/* Special handling for root	*/
					bpar->b_un.b_sum->root_dir.de_inode.i_spare = 
					spare;
					bpar->b_un.b_sum->root_dir.de_inode.i_size =
					spare * sizeof(struct dir_elem);
				}
				else
				{
					bpar->b_un.b_dir[parent->offset].de_inode.i_spare =
					spare;		
					bpar->b_un.b_dir[parent->offset].de_inode.i_size =
					spare * sizeof(struct dir_elem);		
				}
			 		/* ... and write it back.	*/
				test_bwrite ( bpar );
			}
					/* Prepare the tree-element	*/
					/* for a new pass as "parent":	*/
			parent = tpt;
			parent->parent_used = TRUE;
			first_time = TRUE;
			spare = 0;
					/* Append to the pathname-string*/
			strcat ( path , "/" );
					/* ... a new sub-dir branch	*/
			strcat ( path , parent->de.de_name );
					/* One level lower than the	*/
			depth++;	/* root-directory		*/
					/* Clear the name-hash-table	*/
			remove_de_hash ();
					/* Begin with the first entry..	*/
			continue;	/* and scan again all entries.	*/
		}

 /*--------------------  Backtracking conditions  ----------------------*/

		if ( tpt->enxt == (tree_e *) NULL && 
		     tpt->lnxt == (tree_e *) NULL &&
		     tpt == parent &&
		     tpt->parent_used )
		{

DEBdebug ("	check_inodes :	Use backtracking to get a new parent-dir");
DEBdebug ("	check_inodes :  Spare value found = %d , counted = %d",
	 parent->de.de_inode.i_spare, spare );

					/* Different # of entries ?	*/
			if ( spare != parent->de.de_inode.i_spare )	
			{		

DEBdebug ("	check_inodes :	Correct number of entries (spare value)");

					/* Save the new spare value	*/
					/* Read the directory block	*/
				bpar = bread ( cdev, parent->bnr, 1, SAVEA );
				if (!bpar) {

Report ("%scheck_inodes() : Failed to read directory block (bnr %d) !",S_FATAL,parent->bnr);

					longjmp ( term_jmp, DIR_BLK_ERR );
				}
					/* Change the entry		*/
				if ( parent->bnr == 1 )
				{	/* Special handling for root	*/
					bpar->b_un.b_sum->root_dir.de_inode.i_spare = 
					spare;
					bpar->b_un.b_sum->root_dir.de_inode.i_size =
					spare * sizeof(struct dir_elem);
				}
				else
				{
					bpar->b_un.b_dir[parent->offset].de_inode.i_spare =
					spare;		
					bpar->b_un.b_dir[parent->offset].de_inode.i_size =
					spare * sizeof(struct dir_elem);		
				}
					/* ... and write it back.	*/
				test_bwrite ( bpar );
			}
					/* Look for a directory, which 	*/
					/* wasn't used as a "parent"	*/
			do
			{		/* THE MAIN EXIT-POINT !	*/
				if ( tpt->eprv == (tree_e *) NULL &&
				     tpt->lprv == (tree_e *) NULL )
					return TRUE;

					/* The "move-back" operation	*/
				if ( tpt->lprv != (tree_e *) NULL )
				{
					/* Remove the lowest level	*/
					parent = remove_level ( tpt );
					depth--;
				}
				else	/* Remove the last entry from	*/
					/* the directory		*/
					parent = remove_entry ( tpt );
					/* Adjust the work-pointer	*/
				tpt = parent;
					/* Cut the pathname string and	*/
				*( strrchr ( path , '/' ) ) = '\0';

DEBdebug ("	check_inodes :	Backtracking to path = %s (depth: %d)",
	 path, depth );

			}		/* try it again until an unused	*/
					/* dir-entry is found		*/
			while ( tpt->parent_used );

					/* Append the new sub-dir	*/
			if ( tpt->de.de_name )
			{
				strcat ( path, "/" );
				strcat ( path, tpt->de.de_name );
			}
					/* Prepare the tree-element as	*/
					/* the new parent-directory	*/
			tpt->parent_used = TRUE;
			first_time = TRUE;
			spare = 0;
					/* Clear the hash-table		*/
			remove_de_hash ();
					/* ... and go to the beginning	*/
			continue;	/* Now we will examine the en-	*/
					/* tries of the "new" directory.*/
		}
	}
 } /* end of <for (;;)> */

}

/*----------  Procedures to extract entries from a directory  ----------*/

/*************************************************************************
 * GET THE NEXT DIRECTORY ENTRY FROM A DIRECTORY BLOCK
 *
 * - This procedure uses static local variables
 * - These variables are initialized by setting "first_time" to TRUE
 * - Entries, which are zeroed, are skipped.
 * - If we get a new block-number which was previously used as an active
 *   directory-block and is referred more than one time in the reference
 *   bit-map, we skip it.
 * - All blocks used are marked as being used
 *
 * Parameters : parent     = Pointer to the parent directory entry
 *		first_time = Init flag
 *		block      = pointer to var to hold the actual block number
 *		off	   = pointer to var to hold the actual offset (index)
 * Return     : A pointer to the entry or NULL, if no more valid blocks
 *		are available. This signals the end of the inspection
 *
 *************************************************************************/
struct dir_elem *
get_next_entry ( struct inode *parent, word first_time, daddr_t *block, word *off )
{
 static word index;
 static struct buf *bp;
 static word bnr, empty;
					/* get_next_entry is called the */
 if ( first_time )			/* first time			*/
 {

DEBdebug ("	get_next_entry :	First time initialization.");

	index  = 0;			/* Initialization of index	*/
	empty  = 0;			/* No empty entry at first	*/
 }


DEBdebug ("	get_next_entry :	Get a new entry.");

					/* An unknown number of blocks	*/
 for ( ;; )				/* referred by the dir-blks 	*/
 {					/* Scan all directory entries	*/
	for ( ; index < i_fs.fs_maxdpb ; index++ )
 	{
		if ( index == 0 )	/* Beginning of a new block ?	*/
		{			/* After first try, we have to	*/
					/* release previously used blks */
			if ( ! first_time )
			{
				if ( changes_de )
				{

DEBdebug ("	get_next_entry :	Update modified directory-block on disk.");

					test_bwrite ( bp );
				}
				else
				{

DEBdebug ("	get_next_entry :	Release non modified directory-block");

					brelse ( bp->b_tbp, TAIL );
				}
			}
					/* Loop until we get an "unused"*/
					/* and single referred block-num*/
			for ( ;; )
			{
					/* Get a "new" blocknumber	*/ 
				bnr = get_next_bnr ( parent, first_time, FALSE );
					/* We have reached the last blk	*/
				if ( bnr == 0 )	
					/* The last entry in this di -	*/
					/* rectory was reached:		*/
					
					return (struct dir_elem *) NULL;
					
					/* Multiple use of this block ?	*/
				if ( bitmap_get_used (bnr) &&
				     bitmap_get (bnr) > 1 )
				{

DEBdebug ("	get_next_entry :	Skip over multiple referred dir-block.");

					first_time = FALSE;
					continue;
				}
				else
					/* We have a usable block-num	*/
					break;
			}
					/* Mark the block as being used	*/
			bitmap_set_used (bnr);					
					/* Read the block into memory	*/
			bp = bread ( cdev, bnr, 1, SAVEA );
			if (!bp) {

Report ("%sget_next_entry() : Failed to read block (bnr %d) !",S_FATAL,bnr);

				longjmp (term_jmp, READ_ERR );
			}
			changes_de = 0;	/* Nothing changed so far...	*/
		}
				/* Wiped out content ? ( A mode-field	*/
				/* set to FREE is ignored, because it	*/
				/* could be an error. )			*/
		if ( ! my_memcmp ( (void *) &dir_cmp, (void *) &bp->b_un.b_dir[index], 
	    			 sizeof (struct dir_elem) ) )
	    	{
	    		empty++;	/* Another empty slot found	*/
			continue;	/* Get the next entry		*/
		}
	
		*block = bnr;		/* Save block-number and index	*/
		*off   = index;
		index++;		/* Next entry			*/
					/* Return the new entry		*/
		return &bp->b_un.b_dir[*off];
		
 	} /* end of <for (index)> */
					/* After examination of a whole	*/
					/* block we check whether we	*/
					/* have found any non-zeroed	*/
					/* directory entry.		*/
	if ( empty == i_fs.fs_maxdpb )
	{

DEBdebug ("	get_next_entry :	Block %d is totally empty",
	 bnr );

		first_time = FALSE;	/* To prevent get_next_bnr()	*/
					/* from another initialization	*/
	}
	empty = 0;			/* We have no empty entries at	*/
					/* first again.			*/
 	index = 0;			/* Start-position in a new	*/
 					/* directory block		*/
 } /* end of <for (;;)> */
 
}


/**************************************************************************
 * SEARCH FOR A SPECIFIED ENTRY IN A DIRECTORY
 *
 * - In contrast to get_next_entry(), all entries are examined one after
 *   another until the desired entry is found or the last entry was
 *   reached.
 * - If the entry is found, a pointer to the block is returned and "off"
 *   contains the index of the entry. After doing all work, the block has
 *   to be written/released by the calling procedure!
 *
 * Parameters : parent   = Pointer to the inode of the parent-directory
 *	        compstr  = Character-string used for search-operation
 *		off      = Pointer to a variable to keep the offset position
 * Return     : A pointer to the current directory-block or NULL, if the
 *		entry is not found
 *
 **************************************************************************/
struct buf *
search_entry ( struct inode *parent, char *compstr, word *off )
{
 word offset;
 struct buf *bp;
 word bnr, first_time;


DEBdebug ("	search_entry :	Look for an entry named '%s'", compstr);


 first_time = TRUE;
 offset = 0;
					/* An unknown number of blocks	*/
 for ( ;; )				/* have to be inspected		*/
 {					/* Get the next available bnr	*/
	bnr = get_next_bnr ( parent, first_time, FALSE );
	if ( bnr == 0 )			/* No more blocks available	*/
					/* "off" will be of undefined	*/
					/* value.			*/
		return (struct buf *) NULL;
	
	first_time = FALSE;		/* For all other passes:	*/
					/* Read the specified block	*/
	bp = bread ( cdev, bnr, 1, SAVEA );
	if (!bp) {
Report ("%ssearch_entry() : Failed to read block (bnr %d) !",S_FATAL,bnr);
		longjmp (term_jmp, READ_ERR);			
	}
					/* Scan all directory entries	*/
	for ( offset = 0 ; offset < i_fs.fs_maxdpb ; offset++ )
	{				/* Look for the desired string	*/

DEBdebug ("	search_entry :	Compare it in block %d at pos. %d with '%s'",
	 bnr, offset, bp->b_un.b_dir[offset].de_name );

		if ( ! strcmp ( bp->b_un.b_dir[offset].de_name, compstr ) )
		{			/* Release all previously re-	*/
					/* leased blocks		*/
			get_next_bnr ( NULL, FALSE, TRUE );
			*off = offset;	/* Save the offset and return..	*/
			return bp;	/* a pointer to the block	*/
		}
	}
					/* Release the not further used	*/
	brelse ( bp->b_tbp, TAIL );	/* directory block		*/
 }
}


/**************************************************************************
 * EXTRACT THE NEXT AVAILABLE DIRECTORY BLOCK FROM THE ALLOCATION SCHEME
 *
 * - This procedure uses static local variables.
 * - These variable are initialized by setting first_time to TRUE.
 * - To break during block number scan, call get_next_bnr() with
 *   the terminate-flag set to TRUE (used by search_entry() only). This
 *   will guarantee, that all blocks which are in use will be correctly
 *   released.
 * - Important: The calling procedures get_next_entry() and search_entry()
 *   have to decide whether to use the block or not - especially to 
 *   look for multiple allocations.
 * - If an indirect block is multiple allocated and was previously used
 *   by another entry, it is skipped.
 *
 * Parameters : parent     = Pointer to the parent directory entry
 *		first_time = Init flag
 *		terminate  = Flag which signals finishing of operation
 *			     BEFORE the last block number was extracted
 * Return     : The block number or 0, if an error occurred
 *
 **************************************************************************/
daddr_t
get_next_bnr ( struct inode *parent, word first_time, word terminate )
{
 static word db_done, ib1_done, ib2_done;
 static word db_cnt,  ib1_cnt,  ib2_cnt;
 static struct buf *bip1, *bip2;

 if ( terminate )			/* Test the termination flag	*/
 {					/* If there are any blocks in	*/

DEBdebug ("	get_next_bnr :	Premature termination of block-scan");

	if ( bip1 )			/* use, release them		*/
	{
		if ( changes_de )
			test_bwrite ( bip1 );
		else
			brelse ( bip1->b_tbp, TAIL );
	}
	if ( bip2 )
	{
		if ( changes_de )
			test_bwrite ( bip2 );
		else
			brelse ( bip2->b_tbp, TAIL );
	}
	return 0;			/* Directly exit		*/
 }

 if ( first_time )			/* The first time, we enter	*/
 {					/* this procedure		*/

DEBdebug ("	get_next_bnr :	First time initialization.");

	db_done   = FALSE;		/* The "finish-flags"		*/
	if ( parent->i_ib[0] )
		ib1_done  = FALSE;
	else
		ib1_done  = TRUE;
	if ( parent->i_ib[1] )
		ib2_done  = FALSE;
	else
		ib2_done  = TRUE;
	db_cnt    = 0;			/* The block number and offset	*/
	ib1_cnt   = 0;			/* counters			*/
	ib2_cnt   = 0;
	bip1      = (struct buf *) NULL;
	bip2	  = (struct buf *) NULL;
 } 

 /*-----------------  Handle direct block numbers  ---------------------*/

 if ( ! db_done )
 {					
					/* Scan all direct block numbers*/
	for ( ; db_cnt < i_fs.fs_ndaddr ; db_cnt++ )
  	{				

DEBdebug ("	get_next_bnr :	Direct blk : index=%d  bnr=%d",
	 db_cnt, parent->i_db[db_cnt]);

					/* Skip over zeroed block nums	*/
		if ( ! parent->i_db[db_cnt] )
			continue;
		else			/* Return the next block number	*/
		{			/* which is valid		*/
			db_cnt++;
			return parent->i_db[db_cnt-1];
		}
  	}
	db_done = TRUE;			/* After dealing with all di-	*/	
 }					/* rect block numbers		*/

 /*--------------  Handle single indirect block numbers  ---------------*/

 if ( ! ib1_done )			/* If not done so far ...	*/
 {
 					/* We only use this indirect	*/
 					/* block if it is not multiple	*/
 					/* allocated.			*/
 	if ( ! bitmap_get_used ( parent->i_ib[0] ) )
 	{
					/* Get the indirection block	*/
		if ( ib1_cnt == 0 )	/* for the first time		*/
		{			/* Read the indirect block	*/
			bip1 = bread ( cdev, parent->i_ib[0], 1, SAVEA );
			if (!bip1) {
Report ("%sget_next_bnr() : Failed to read indirect block (bnr %d) !",S_FATAL,parent->i_ib[0]);
				longjmp (term_jmp, IND_BLK_ERR);
			}
		}
					/* Scan the indirect entries	*/
		for ( ; ib1_cnt < i_fs.fs_maxcontig ; ib1_cnt++ )
		{
			if ( ! bip1->b_un.b_daddr[ib1_cnt] )
				continue;
			else
			{

DEBdebug ("	get_next_bnr :	Single indirect blk : index=%d  bnr=%d",

	 ib1_cnt, bip1->b_un.b_daddr[ib1_cnt]);

				ib1_cnt++;
				return bip1->b_un.b_daddr[ib1_cnt-1];
			}
		}
		ib1_done = TRUE;	/* All single indirect entries	*/
					/* handled			*/
					/* Release the indirect block	*/
		brelse ( bip1->b_tbp, TAIL );
		bitmap_set_used ( parent->i_ib[0] );
	}
	else				/* The single indirect block is	*/
		ib1_done = TRUE;	/* multiple allocated!		*/
 }

 /*--------------  Handle double indirect block numbers  ---------------*/

 if ( ! ib2_done )
 {					/* We only use the double ind.	*/
 					/* block if it is not multiple	*/
 					/* allocated.			*/
 	if ( ! bitmap_get_used ( parent->i_ib[1] ) )
 	{
		if ( ib2_cnt == 0 )	/* Get the double indirect blk	*/
		{			/* Read the indirect block	*/
			bip2 = bread ( cdev, parent->i_ib[1], 1, SAVEA );
			if (!bip2) {

Report ("%sget_next_bnr() : Failed to read indirect block (bnr %d) !",S_FATAL,parent->i_ib[1]);
				longjmp (term_jmp, IND_BLK_ERR);
			}
			ib1_cnt = 0;
		}
					/* Work on an unknown # of	*/
		do			/* single indirect blocks	*/
		{			/* Read the single indirect	*/
					/* block into buffer-cache	*/
			if ( ib1_cnt == 0 )	
			{		/* Skip over zeroed entries and */
					/* entries which were previous-	*/
					/* ly used for inspection.	*/
				while ( bip2->b_un.b_daddr[ib2_cnt] == 0 ||
					bitmap_get_used (bip2->b_un.b_daddr[ib2_cnt] ) )
					ib2_cnt++;

DEBdebug ("	get_next_bnr :	Handle double indirect block numbers. index=%d",
	 ib2_cnt);

				bip1 = bread ( cdev, bip2->b_un.b_daddr[ib2_cnt], 1, SAVEA );
				if (!bip1) {
Report ("%sget_next_bnr() : Failed to read double indirect block (bnr %d) !",S_FATAL,bip2->b_un.b_daddr[ib2_cnt]);
					longjmp (term_jmp, DIND_BLK_ERR);
				}
					/* New offset in double ind. bl.*/
				ib2_cnt++;
			}
					/* Scan the directory block -	*/
					/* numbers			*/
			for ( ; ib1_cnt < i_fs.fs_maxcontig ; ib1_cnt++ )
			{			

DEBdebug ("	get_next_bnr :	Handle double ind. block. Index in single=%d",
	 ib1_cnt);

					/* Skip over zero block-numbers	*/
				if ( ! bip1->b_un.b_daddr[ib1_cnt] )
					continue;
				else
				{
					ib1_cnt++;
					return bip1->b_un.b_daddr[ib1_cnt-1];
				}
			}
					/* Release single indirect blk	*/
			brelse ( bip1->b_tbp, TAIL );
			bitmap_set_used ( bip2->b_un.b_daddr[ib2_cnt] );
			
			ib1_cnt = 0;	/* Reset single-indirect index	*/
					/* Work on all single indirect	*/
					/* blocks, referred in the 	*/
					/* double indirect block	*/
		} while ( ib2_cnt < i_fs.fs_maxcontig );
					/* Release the double indirect	*/
					/* block			*/
		brelse ( bip2->b_tbp, TAIL );
		ib2_done = TRUE;
		bitmap_set_used ( parent->i_ib[1] );
	}
	else				/* The double indirect block is	*/
		ib2_done = TRUE;	/* multiple allocated!		*/
 }

 return 0;				/* No more block numbers are	*/
					/* available			*/
}


/*-------  Procedures, dealing with directory-tree-handling  -----------*/

/*************************************************************************
 * APPEND AN ELEMENT WHICH BUILDS A NEW DIRECTORY-LEVEL
 *
 * Parameter : elem = Pointer to the actual element structure
 * Return    : Pointer to the new element or NULL, if an error occurred
 *
 *************************************************************************/
tree_e *
append_level ( tree_e *elem )
{
 tree_e *tep;

 tep = (tree_e *) Malloc ( sizeof (tree_e) );
 if ( tep == (tree_e *) NULL )
 {

DEBdebug ("%sappend_level() : Unable to allocate memory !", S_FATAL);

	longjmp (term_jmp, MEMERR);
 }
				
 elem->lnxt = tep;			/* Chain in directory tree	*/
 tep->lnxt  = (tree_e *) NULL;		/* Adjust the pointers		*/
 tep->lprv  = elem;
 tep->enxt  = (tree_e *) NULL;
 tep->eprv  = (tree_e *) NULL;


DEBdebug ("	append_level :	Add a new subdirectory branch");

 return tep;
}

/*************************************************************************
 * APPEND AN ELEMENT ON THE SAME DIRECTORY-LEVEL
 *
 * Parameter : elem = Pointer to the actual element structure
 * Return    : Pointer to the new element or NULL, if an error occurred
 *
 *************************************************************************/
tree_e *
append_entry ( tree_e *elem )
{
 tree_e *tep;

 tep = (tree_e *) Malloc ( sizeof (tree_e) );
 if ( tep == (tree_e *) NULL )
 {

DEBdebug ("%sappend_entry() : Unable to allocate memory !", S_FATAL);

	longjmp (term_jmp, MEMERR);
 }

 elem->enxt = tep;
 tep->enxt  = (tree_e *) NULL;
 tep->eprv  = elem;
 tep->lnxt  = (tree_e *) NULL;
 tep->lprv  = (tree_e *) NULL;


DEBdebug ("	append_entry :	Add a new subdirectory entry");


 return tep;
}

/*************************************************************************
 * REMOVE AN ELEMENT AS THE LAST ONE FROM A DIRECTORY-LEVEL
 *
 * Parameter : elem = Pointer to the element to be removed
 * Return    : Pointer to the "new" element of the last position
 *
 *************************************************************************/
tree_e *
remove_level ( tree_e *elem )
{
 tree_e *temp = elem->lprv;

 elem->lprv->lnxt = (tree_e *) NULL;
 Free ( elem );


DEBdebug ("	remove_level :	Cut a subdirectory branch");


 return temp;
}

/*************************************************************************
 * REMOVE AN ELEMENT FROM THE SAME DIRECTORY-LEVEL
 *
 * Parameter : elem = Pointer to the element to be removed
 * Return    : Pointer to the "new" element of the last position
 *
 *************************************************************************/
tree_e *
remove_entry ( tree_e *elem )
{
 tree_e *temp = elem->eprv;

 elem->eprv->enxt = (tree_e *) NULL;
 Free ( elem );


DEBdebug ("	remove_entry :	Remove from a subdirectory");


 return temp;
}

/*--------------  Operations on symbolic link hash-structures  -----------*/

/**************************************************************************
 * APPEND A PATHNAME TO THE SYMBOLIC-LINK HASH-TABLE
 *
 * Parameter	: bnr     = The block number which keeps the entry
 *	  	  off	  = The offset into this block
 *		  pi_bnr  = The block of the parent-dir inode
 *		  pi_off  = The offset into this block
 *		  path	  = A pointer to the pathname string
 * Return       : - nothing -
 *
 *************************************************************************/
void
append_link_hash ( daddr_t bnr, word off, daddr_t pi_bnr, word pi_off, 
		   char *path )
{
 word index;
 char buffer[512];
 struct de_link *lpt;	
 
 index = calc_link_hash ( path );	/* Calculate the hash-index	*/
 
					/* Allocate memory for the new	*/
					/* entry			*/
 lpt = (struct de_link *) Malloc ( sizeof (struct de_link) );
 if ( lpt == (struct de_link *) NULL )
 {

DEBdebug ("%sappend_link_hash() : Unable to allocate memory !", S_FATAL);

 	longjmp (term_jmp, MEMERR);
 }
 					/* Extract the pathname, based  */
 					/* on the file-server's root	*/
 strncpy (buffer, ( strstr ( path, i_fs.fs_name ) ) - 1, 512 );
 					/* Allocate memory for the path-*/
 					/* name string			*/
 lpt->path = (char *) Malloc ( strlen (buffer) + 1 );
if ( lpt->path == (char *) NULL )
 {

DEBdebug ("%sappend_link_hash() : Unable to allocate memory !", S_FATAL);

 	longjmp (term_jmp, MEMERR);
 }
 					/* Save the parameters		*/
 lpt->pi_bnr = pi_bnr;
 lpt->pi_off = pi_off;
 lpt->i_bnr = bnr;
 lpt->i_off = off;
 strcpy ( lpt->path, buffer );
 

DEBdebug ("	append_link_hash :	Append '%s' on slot %d",
	  buffer, index);

					/* Insert the new element on the*/
					/* head - position		*/
 lpt->lnnxt = link_hash_tab[index].lnnxt;
 link_hash_tab[index].lnnxt = lpt;					
} 


/**************************************************************************
 * LOOK FOR A SYMBOLIC LINK-PATH IN THE HASH-TABLE AND REMOVE IT
 *
 * - If the element is found, it is unlinked from the appropriate queue and
 *   the reserved memory is deallocated.
 * - Because of the fact that one object may be referred by more than one
 *   symbolic link, the queue is always followed up to the end.
 *
 * Parameter	: path	  = Pointer to the pathname string
 * Return       : The number of links which hold this pathname as a
 *		  reference.
 *
 *************************************************************************/
word
takefrom_link_hash ( char *path )
{
 word index, lnk_cnt;
 struct de_link *lpt, *lpt_last, *lpt_old;
 
 index = calc_link_hash ( path );	/* Calculate the hash-index	*/
 lnk_cnt = 0;
 

DEBdebug ("	takefrom_link_hash :	Look for link-path '%s' (slot %d)",
	  path, index);

					/* Begin with the first element	*/
 lpt = link_hash_tab[index].lnnxt;	/* in a row 			*/
 lpt_last = (struct de_link *) NULL;
 
 while ( lpt != (struct de_link *) NULL )
 {					/* The pathname is found !	*/
 	if ( ! strcmp ( lpt->path, path ) )
 	{				/* Deallocate memory, used by	*/
		Free ( lpt->path );	/* the pathname string		*/
					/* The first element has to be	*/
					/* unchained ?			*/
		if ( lpt == link_hash_tab[index].lnnxt )
			link_hash_tab[index].lnnxt = lpt->lnnxt;
		else
			lpt_last->lnnxt = lpt->lnnxt;
			
		lpt_old = lpt;		/* Save the old pt for freeing	*/
		lpt = lpt->lnnxt;	/* Step to the next element	*/
		Free ( lpt_old );
		lnk_cnt++;		/* Note the entry		*/
 	}
	else
	{
		lpt_last = lpt;		/* We take the next one with -	*/
		lpt = lpt->lnnxt;	/* out leaving an element out	*/
	} 	
 }


DEBdebug ("	takefrom_link_hash :	The object is referred by %d symbolic links",
	 lnk_cnt );

					/* Return the number of symbolic*/ 
 return lnk_cnt;			/* links to the spec. object	*/
}

 
/**************************************************************************
 * REMOVE ALL ELEMENTS FROM THE SYMBOLIC-LINK HASH-TABLE
 *
 * Parameter	: - nothing -
 * Return       : - nothing -
 *
 *************************************************************************/
void
remove_link_hash ( void )
{
 word i;
 struct de_link *lpt, *lpt_nxt;	
 

DEBdebug ("	remove_link_hash :	Clear symbolic-link hash-table");

					/* Walk through all rwos of the	*/
 for ( i = 0 ; i < LINKHASHSZ ; i++ )	/* hash-table			*/
 {	
 	lpt = link_hash_tab[i].lnnxt;
 	
 	while ( lpt != (struct de_link *) NULL )
 	{
 		lpt_nxt = lpt->lnnxt;
 		Free ( lpt->path );	/* Release all used memory	*/
 		Free ( lpt );
 		lpt = lpt_nxt;
 	}
					/* Re-init the head-pointer	*/
 	link_hash_tab[i].lnnxt = (struct de_link *) NULL;
 }
}

/*-----------------------------------------------------------------------*/

/* end of xtdcheck.c */
