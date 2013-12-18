/* 16.08.90 - Some basic cuts to build the "integrated" version
 *
 *
 */

/*************************************************************************
**									**
**	     H E L I O S   F I L E S Y S T E M	 C H E C K E R		**
**	     -------------------------------------------------		**
**									**
**		    Copyright (C) 1989 Parsytec GmbH			**
**			   All Rights Reserved. 			**
**									**
** condups.c								**
**									**
**	- Collection of routines to resolve multiple allocation 	**
**	  problems. (They build a combined set with the routines	**
**	  defined in concheck.c)					**
**									**
**************************************************************************
** HISTORY   :								**
**------------								**
** Author    :	 18/04/89 : H.J.Ermen					**
*************************************************************************/

#include "check.h"

#define   DEBUG		FALSE

/*---------------------  Local variables  ------------------------------*/

static char path[512];		/* For temporary pathname storage	*/
static struct dir_elem dir_cmp; /* Is zeroed and used for comparison	*/
				/* directory entries read.		*/

/*---------------------  Local prototypes  -----------------------------*/

static void	dup_select_entry (struct dup_bnr *col_pt, char mode);


/*************************************************************************
 * TRAVERSE THE DIRECTORY-TREE TO FIND THE ENTRIES WHICH CORRESPOND TO
 * MULTIPLE CLAIMED INODES AND LINK-PATHS NOTED IN SYMBOLIC LINKS
 *
 * - This procedure is the central routine for working on the directory
 *   tree. A backtracking mechanism with no recursive procedural-overhead
 *   is used to avoid memory allocation and stack overflow problems.
 * - It is similar to check_inodes() with the exceptions that it starts
 *   always on the file-system's root-position and performs no validation
 *   checks but looks for multiple referred block numbers.
 * - The procedure entry_dup_bnr() is called to pick up the multiple
 *   allocated block-numbers which are referred by the actual entry.
 * - If an entry with a multiple claimed block-number is found, it is
 *   integrated into the hash-table data structures.
 * - Another check is done with the entries in the link-path hash table: All
 *   entries are searched in this table and - if one is found - removed from
 *   it.
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
void
handle_dups_and_links ( void )
{
 word bnr;				/* For temporary storage; used	*/
 word offset;				/* by get_next_entry()		*/

 struct buf *bpar;			/* Points to the current parent	*/
					/* directory - block		*/
 tree_e head;
 word first_time;			/* Used to signal, if an entry	*/
					/* is scanned the first time	*/
 tree_e *parent;			/* Pointer to the actual parent */
					/* tree-element			*/
 tree_e *tpt;				/* Pointer to the actual wor-	*/
					/* king element.		*/
 struct dir_elem *de_pt;		/* Used for temporary storage	*/
 word lnk_cnt;				/* For counting symbolic link -	*/
					/* references			*/
 char full_path[512];			/* The complete pathname to the */
					/* entry examined		*/

 /*----------------------  Init operations  ----------------------------*/

 bpar = bread ( 0, 1, 1, SAVEA );	/* Get the summary-block with	*/

					/* the root-directory entry	*/
 head.bnr = 1;				/* Init the head element	*/
 head.offset = -1;			/* An offset in the sum-block	*/
					/* makes no sense !		*/
 head.parent_used = TRUE;
					/* Copy the root-dir entry	*/
 memcpy ( (void *) &head.de, (void *) &bpar->b_un.b_sum->root_dir,
	  sizeof (struct dir_elem) );
 brelse ( bpar->b_tbp, TAIL );		/* Release the summary-block	*/

 head.enxt = (tree_e *) NULL;		/* Zero the pointers		*/
 head.eprv = (tree_e *) NULL;
 head.lnxt = (tree_e *) NULL;
 head.lprv = (tree_e *) NULL;
					/* Clear the comparison struct	*/
 memset ( &dir_cmp, 0, sizeof (struct dir_elem) );

 strcpy ( path, "/" );			/* Get the name of the root	*/
 strcat ( path, head.de.de_name );	/* of the filesystem		*/

 first_time = TRUE;			/* Start up value		*/
 parent = &head;			/* At the beginning the head	*/
 tpt	= &head;			/* element is the only one.	*/


IOdebug ("%sTraversing the dir-tree a second time", S_INFO);
 if ( fst.dup_blocks )
IOdebug ("%s - LOOKING FOR MULTIPLE ALLOCATED BLOCKS", S_INFO);
 if ( found_links )
IOdebug ("%s - LOOKING FOR SYMBOLIC LINK REFERENCES", S_INFO);

 /*---------------  Traverse the whole directory tree  -----------------*/

 for ( ;; )
 {
					/* Get an appropriate entry	*/
	de_pt = get_next_entry ( &parent->de.de_inode, first_time,
				 &bnr, &offset );
	first_time = FALSE;		/* We want to get all other en-	*/
					/* tries from this directory	*/

					/* We have got an entry ?	*/
	if ( de_pt != (struct dir_elem *) NULL )
	{
		strncpy (actual_path, path, 512);

		if ( ! silent_mode )
			IOdebug ("%sActual object = %s / [%s]", S_INFO, 
				    path, de_pt->de_name);

/*-------  The validating operations to be done with this entry : ------*/

					/* If there are any multiple	*/
		if ( fst.dup_blocks )	/* allocated blocks, ...	*/
					/* Look, whether it refers mul-	*/
					/* tiple allocated blocks.	*/
			entry_dup_bnr ( de_pt, bnr, offset );

		if ( found_links )	/* If there are any symbolic	*/
		{			/* links found ...		*/
					/* Build the complete pathname	*/
			strcpy ( full_path, path );
			strcat ( full_path, "/" );
			strcat ( full_path, de_pt->de_name );

					/* Look whether the entry (in-	*/
					/* clusive the whole path) is	*/
					/* referred by a symbolic link	*/
			if ( ( lnk_cnt = takefrom_link_hash ( full_path ) ) > 0 )
			{
				IOdebug ("%sThe entry '%s' is referred by %d symbolic links",
					    S_INFO, full_path, lnk_cnt );
			}
		}

/*------------	Appending new elements to the sort-tree  ---------------*/

		if ( de_pt->de_inode.i_mode == Type_Directory )
		{
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
					/* At this time NOT used as a	*/
					/* parent-dir			*/
			tpt->parent_used = FALSE;
		}
	}

/*-----  After traversing a complete directory with all entries: -------*/

	else				/* One directory fully scanned:	*/
	{
#if DEBUG
IOdebug ("	handle_dups_and_links : Have examined all entries. Change the directory");
IOdebug  ("	Current parent = %s	'used' = %d",
	 parent->de.de_name, parent->parent_used);
IOdebug ("	Current entry  = %s	'used' = %d",
	 tpt->de.de_name, tpt->parent_used);
#endif

 /*-------  Go on with the last directory of the current sub-dir  ------*/

		if ( tpt->enxt == (tree_e *) NULL &&
		     tpt->lnxt == (tree_e *) NULL &&
		     tpt != parent &&
		     ! tpt->parent_used )
		{
#if DEBUG
IOdebug ("	handle_dups_and_links : Take last sub-dir as parent directory");
#endif
					/* Prepare the tree-element	*/
					/* for a new pass as "parent":	*/
			parent = tpt;
			parent->parent_used = TRUE;
			first_time = TRUE;
					/* Append to the pathname-string*/
			strcat ( path , "/" );
					/* ... a new sub-dir branch	*/
			strcat ( path , parent->de.de_name );
					/* Begin with the first entry..	*/
			continue;	/* and scan again all entries.	*/
		}

 /*--------------------  Backtracking conditions  ----------------------*/

		if ( tpt->enxt == (tree_e *) NULL &&
		     tpt->lnxt == (tree_e *) NULL &&
		     tpt == parent &&
		     tpt->parent_used )
		{
#if DEBUG
IOdebug ("	handle_dups_and_links : Use backtracking to get a new parent-dir");
#endif
					/* Look for a directory, which	*/
					/* wasn't used as a "parent"	*/
			do
			{		/* THE MAIN EXIT-POINT !	*/
				if ( tpt->eprv == (tree_e *) NULL &&
				     tpt->lprv == (tree_e *) NULL )
					return;

					/* The "move-back" operation	*/
				if ( tpt->lprv != (tree_e *) NULL )
					/* Remove the lowest level	*/
					parent = remove_level ( tpt );
				else	/* Remove the last entry from	*/
					/* the directory		*/
					parent = remove_entry ( tpt );
					/* Adjust the work-pointer	*/
				tpt = parent;
					/* Cut the pathname string and	*/
				*( strrchr ( path , '/' ) ) = '\0';
#if DEBUG
IOdebug ("	handle_dups_and_links : Backtracking to path = %s", path );
#endif
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
					/* ... and go to the beginning	*/
			continue;	/* Now we will examine the en-	*/
					/* tries of the "new" directory.*/
		}
	}
 } /* end of <for (;;)> */
}


/*************************************************************************
 * TRY TO FIND OUT WHETHER AN ENTRY CLAIMS MULTIPLE ALLOCATED BLOCKS
 * AND CREATE AN APPROPRIATE ENTRY IN THE HASH-TABLE FOR EACH BLOCK.
 *
 * - The direct and indirect block numbers are compared with the ref-
 *   counts in the reference bit-maps
 * - If a block number is more than one time referred, it is put into
 *   the prepared linked list.
 * - There are especially two informations kept in these list-entries:
 *   - The block number and offset of the inode which claims the dup-block
 *   - The block number and offset of the block which refers the dup-block.
 *     These are different if the block is claimed by an indirect block !
 *     (single or double indirect level)
 *   - The "level" on which the multiple allocation was found is also noted.
 *     (0 = inode, 1 = single indirect, 2 = double indirect)
 *
 * Parameter	: de   = The directory-element to be examined
 *		  bnr  = The block which keeps this directory element
 *		  off  = Index into this block
 * Return	: - nothing -
 *
 ************************************************************************/
void
entry_dup_bnr ( struct dir_elem *de, daddr_t bnr, word off )
{
 struct ref rf;
 struct buf *bip1, *bip2;
 word bcnt, bcnt_d;

#if DEBUG
IOdebug ("	entry_dup_bnr :	Check this entry for multiple allocated blocks.");
#endif

 rf.pi_bnr = bnr;			/* Save the block number and the */
 rf.pi_off = off;			/* offset for this dir-entry	 */

/*-----------------  Examine direct block references  -------------------*/

#if DEBUG
IOdebug ("	entry_dup_bnr :	Scan the direct block table.");
#endif

 for ( bcnt = 0 ; bcnt < i_fs.fs_ndaddr ; bcnt++ )
 {					/* Skip over zeroed block nums	*/
	if ( ! de->de_inode.i_db[bcnt] )
		continue;
					/* More than one time referred?	*/
	if ( bitmap_get ( de->de_inode.i_db[bcnt] ) > 1 )
	{
					/* Prepare the reference para-	*/
					/* meter struct			*/
		rf.pi_place  = bcnt;
		rf.ref_level = 0;	/* "Inode level"		*/
					/* Find the appropriate chain	*/
					/* and link the entry into it	*/
		append_dup_entry ( de->de_inode.i_db[bcnt], de->de_name,
				   de->de_inode.i_mtime, de->de_inode.i_mode,
				   &rf );
	}
 }

/*---------------  Examine single indirect block references  -----------*/

 if ( de->de_inode.i_ib[0] )
 {
#if DEBUG
IOdebug ("	entry_dup_bnr : Scan the single indirect block numbers.");
#endif
	rf.pi_place = -1;		/* Single indirect block	*/
					/* Is the single indirect block */
					/* itself multiple referred ?	*/
	if ( bitmap_get ( de->de_inode.i_ib[0] ) > 1 )
	{				/* The parent inode block and	*/
					/* the current are identically	*/
		rf.ref_level = 0;	/* "Inode level"		*/
					/* Link into appropriate queue	*/
		append_dup_entry ( de->de_inode.i_ib[0], de->de_name,
				   de->de_inode.i_mtime, de->de_inode.i_mode,
				   &rf );
	}

	/* If we have found the indirect block as being multiple allo-	*/
	/* we can continue inspecting the data blocks, because during	*/
	/* inode-check we have made all needed tests on indirect blocks	*/
	/* and skipped over them if they were more than one time allo-	*/
	/* cated. So we are sure that all data blocks are not referred	*/
	/* twice or more because of the multiple allocated ind. block !	*/

					/* Read the indirect block	*/
	bip1 = bread ( 0, de->de_inode.i_ib[0], 1, SAVEA );

					/* Scan the block content	*/
	for ( bcnt = 0 ; bcnt < i_fs.fs_maxcontig ; bcnt++ )
	{				/* Skip over zero block numbers	*/
		if ( ! bip1->b_un.b_daddr[bcnt] )
			continue;
					/* Look for multiple allocations*/
		if ( bitmap_get ( bip1->b_un.b_daddr[bcnt] ) > 1 )
		{			/* Prepare reference information*/
			rf.ind_bnr[0] = de->de_inode.i_ib[0];
			rf.ind_off[0] = bcnt;
					/* "Single indirect level"	*/
			rf.ref_level  = 1;
					/* Link into appropriate queue	*/
			append_dup_entry ( bip1->b_un.b_daddr[bcnt],
					   de->de_name, de->de_inode.i_mtime,
					   de->de_inode.i_mode, &rf );
		}

	}
					/* ... and release indirect blk */
	brelse ( bip1->b_tbp, TAIL );
 }

/*---------------  Examine double indirect block references  -----------*/

 if ( de->de_inode.i_ib[1] )
 {
#if DEBUG
IOdebug ("	entry_dup_bnr : Scan the double indirect block numbers.");
#endif
	rf.pi_place = -2;		/* Double indirect block	*/
					/* Is the double indirect block */
					/* itself multiple referred ?	*/
	if ( bitmap_get ( de->de_inode.i_ib[1] ) > 1 )
	{				/* The parent inode block and	*/
					/* the current are identically	*/
		rf.ref_level = 0;	/* "Inode level"		*/
					/* Link into appropriate queue	*/
		append_dup_entry ( de->de_inode.i_ib[1], de->de_name,
				   de->de_inode.i_mtime, de->de_inode.i_mode,
				   &rf );
	}
					/* --> Look above !		*/
					/* Read the indirect block	*/
	bip2 = bread ( 0, de->de_inode.i_ib[1], 1, SAVEA );

					/* Scan the block content	*/
	for ( bcnt = 0 ; bcnt < i_fs.fs_maxcontig ; bcnt++ )
	{				/* Skip over zero block numbers	*/
		if ( ! bip2->b_un.b_daddr[bcnt] )
			continue;

					/* Look for multiple allocations*/
		if ( bitmap_get ( bip2->b_un.b_daddr[bcnt] ) > 1 )
		{			/* Prepare reference information*/
			rf.ind_bnr[0] = bip2->b_un.b_daddr[bcnt];
			rf.ind_off[0] = bcnt;
					/* "Single indirect level"	*/
			rf.ref_level  = 1;
					/* Link into appropriate queue	*/
			append_dup_entry ( bip2->b_un.b_daddr[bcnt],
					   de->de_name, de->de_inode.i_mtime,
					   de->de_inode.i_mode, &rf );
		}

		bip1 = bread ( 0, bip2->b_un.b_daddr[bcnt], 1, SAVEA );

					/* The next level of indirection*/
		for ( bcnt_d = 0 ; bcnt_d < i_fs.fs_maxcontig ; bcnt_d++ )
		{
			if ( ! bip1->b_un.b_daddr[bcnt_d] )
				continue;

					/* Look for multiple allocations*/
			if ( bitmap_get ( bip1->b_un.b_daddr[bcnt] ) > 1 )
			{		/* Prepare reference information*/
				rf.ind_bnr[0] = bip2->b_un.b_daddr[bcnt];
				rf.ind_off[0] = bcnt;
				rf.ind_bnr[1] = bip1->b_un.b_daddr[bcnt_d];
				rf.ind_off[1] = bcnt_d;
					/* "Double indirect level"	*/
				rf.ref_level  = 2;
					/* Link into appropriate queue	*/
				append_dup_entry ( bip1->b_un.b_daddr[bcnt_d],
						   de->de_name,
						   de->de_inode.i_mtime,
						   de->de_inode.i_mode, &rf );
			}
		}
					/* release single indirect and	*/
		brelse ( bip1->b_tbp, TAIL );
	}
					/* ... double indirect block	*/
	brelse ( bip2->b_tbp, TAIL );
 }
}


/*************************************************************************
 * DO EVERYTHING NEEDED TO REARRANGE DOUBLE ALLOCATED BLOCKS
 *
 * - The hash-table of multiple allocated block numbers is analyzed
 *   and the corresponding entries are inspected.
 * - Note that only directory-entries can hold directory-blocks whereas
 *   non-directory-blocks can be hold by all other types of entries. The
 *   decision whether to keep a block in the hash-table or not is made by
 *   append_dup_entry().
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
void
decide_on_dups ( void )
{
 struct dup_bnr	  *hpt;
 struct dup_entry *ept;
 word i, usable_cnt;
 char choice;

#if DEBUG
IOdebug ("	decide_on_dups :	Examine the entries which refer a dup block.");
#endif
					/* Work on the hash-table	*/
 for ( i = 0 ; i < DUPHASHSZ ; i++ )
 {					/* Start with the first element */
	hpt = dup_hash_tab[i].dupnxt;	/* in a row			*/
					/* "Follow" the row :		*/
	while ( hpt != (struct dup_bnr *) NULL )
	{

/*---------  Examine the entries which refer to the dup block  ---------*/

		ept = hpt->dupenxt;	/* Initialization to access the */
					/* first element in a column	*/
					/* Print a report		*/
		IOdebug ("%sMultiple referred block = %d | ref-count = %d | claimed by :",
			    S_INFO, hpt->bnr, hpt->cnt );

					/* If no entry refers this block*/
					/* we can set it free:		*/
		if ( hpt->dupenxt == (struct dup_entry *) NULL )
		{
					/* Mark it as "set-free"	*/
			bitmap_set_free ( hpt->bnr );
			IOdebug ("%sThe block is claimed by no entry!", 
				  S_WARNING);

			hpt = hpt->dupnxt;
			continue;	/* Get directly the next ele -	*/
					/* ment in the row.		*/
		}
					/* Print a short info		*/
		switch ( hpt->type )	/* Of which type is the block ?	*/
		{
			case POSSIBLE_DIR :
				IOdebug ("%sThe block %d seems to be of type DIRECTORY",
					    S_INFO, hpt->bnr );
				break;
			case POSSIBLE_INDIRECT :
				IOdebug ("%sThe block %d seems to be an indirect block.",
					    S_INFO, hpt->bnr );
				break;
			case POSSIBLE_UNKNOWN :
				IOdebug ("%sThe block %d seems to be a data-block",
					    S_INFO, hpt->bnr );
				break;
		}

		usable_cnt = 0;		/* No "usable" entries at first	*/
					/* "Follow" the column		*/
		while ( ept != (struct dup_entry *) NULL )
		{
					/* Display the entry for selec- */
					/* tion, if it is marked as	*/
					/* "usable".			*/
			if ( ept->usable )
			{		/* Another "usable" entry	*/
				char nbuf[80];
				
				usable_cnt++;
					/* Print the report		*/
				strcpy (nbuf, "[");
				strcat (nbuf, ept->name );
				strcat (nbuf, "] ");
				switch ( ept->type )
				{
					case Type_Directory :
						strcat (nbuf, "(Type_Directory)" );
						break;
					case Type_Link :
						strcat (nbuf, "(Type_Link)" );
						break;
					case Type_File :
						strcat (nbuf, "(Type_File)" );
						break;
				}
			}
			else
			{
#if DEBUG
IOdebug ("	decide_on_dups :	Note : The entry [%s] is not regarded",
	 ept->name );
#endif
			}
					/* Get the next name-entry	*/
		ept = ept->dupenxt;

		} /* end of <while (ept)> */

/*-------------  Solve the problem of multiple allocation  -------------*/

					/* Decide with a view on the #	*/
		switch ( usable_cnt )	/* of 'usable' entries found.	*/
		{
			case 0 :	/* If no usable entry is avai-	*/
			{		/* lable:			*/
#if DEBUG
IOdebug ("	decide_on_dups :	No 'usable' entry found");
#endif
				dup_select_entry ( hpt, 'N' );
				break;
			}
					/* One usable entry does not	*/
			case 1 :	/* need a selection!		*/
			{
#if DEBUG
IOdebug ("	decide_on_dups :	There is the only one entry");
#endif
				dup_select_entry ( hpt, 'O' );
				break;
			}
					/* If more than one 'usable'	*/
			default :	/* entries are found to select: */
			{		/* We take the last modified	*/
					/* one.				*/
					
				 dup_select_entry ( hpt, 'M' );
				 break;
			}
		} /* end of <switch (usable_cnt)> */
					/* "Follow" the row to the next	*/
					/* multiple allocated block num */
		hpt = hpt->dupnxt;

	} /* end of <while (hpt) */
 } /* end of <for ()> */
}


/*************************************************************************
 * OPERATIONS TO DECIDE WHICH ENTRY GETS THE MULTIPLE ALLOCATED BLOCK
 * AND CORRECTION OF ALL INODES WHICH HAVE REFERRED THIS BLOCK
 *
 * - This procedure offers two possible solutions to solve the
 *   problem of multiple allocated blocks. (given with 'mode')
 *	mode = 'N' : No entry with the 'usable'-mark set is found
 *	mode = 'O' : We have found exactly one entry marked as 'usable'
 *	mode = 'M' : Take the entry with the most recent modification date
 *	mode = 'R' : Remove all entries which claim multiple allocated blocks
 *
 * Parameter	: col_pt     = Pointer to the column of selectable entry
 *			       names
 *		  mode	     = The operation mode ('N' , 'O' or 'M')
 * Return	: - nothing -
 *
 ************************************************************************/
static void
dup_select_entry ( struct dup_bnr *col_pt, char mode )
{
 struct dup_entry *ept;
 struct buf *bp, *bind;
 struct dir_elem *dpt;
 char ename[80];
 time_t lastmod;

/*--------------  Decide which entry shall get the block  ----------------*/

					/* Select an entry by making use  */
					/* of the mode - flag.		  */
 switch ( mode )
 {
	case 'N'  :			/* No entry with the 'usable' flag*/
	{				/* set is available.		  */
#if DEBUG
IOdebug ("	dup_select_entry :	No 'usable' entry is available");
#endif
		strcpy ( ename, "" );	/* A zero length string signals:  */
		break;			/* no 'usable' entry!		  */
	}

	case 'O'  :			/* We have exactly one 'usable'	  */
	{				/* entry which must be extracted. */
#if DEBUG
IOdebug ("	dup_select_entry :	Find the single entry marked as 'usable'");
#endif
					/* Start with the first element   */
					/* in the row.			  */
		ept = col_pt->dupenxt;
					/* Work on all elements contained */
					/* in the specified row.	  */
		while ( ept != (struct dup_entry *) NULL )
		{			/* The 'usable'-flag is set ?	  */
			if ( ept->usable )
			{		/* Get the name of the valid elem */
				strcpy ( ename, ept->name );
				break;
			}
					/* Take the next element in the	  */
					/* row				  */
			ept = ept->dupenxt;
		}
		break;
	}

	case 'M' :		/* Walk through the list of entry-names   */
	{			/* and look for the entry with the most   */
				/* recent modification date.		  */
#if DEBUG
IOdebug ("	dup_select_entry :	The entry with the most recent date is chosen.");
#endif
		lastmod = 0;
		ept = col_pt->dupenxt;
					/* Follow the column		  */
		while ( ept != (struct dup_entry *) NULL )
		{
					/* Later modified ?		  */
			if ( ept->mtime > lastmod )
			{
				lastmod = ept->mtime;
				strcpy ( ename, ept->name );
			}
			ept = ept->dupenxt;
		}
		break;
	}

	case 'R' :			/* Set an illegal entry-name to	*/
	{				/* guarantee that all entries	*/
					/* will be deleted.		*/
		strcpy ( ename, "..." );
		break;
	}
 } /* end of switch() */

/*---------------  Delete all unused block references  ------------------*/

#if DEBUG
IOdebug ("	dup_select_entry :	The entry '%s' gets the block",
	 ename );
#endif

 ept = col_pt->dupenxt;			/* We begin with the first ele-	 */
					/* ment in the column		 */

 while ( ept != (struct dup_entry *) NULL )
 {					/* We skip over the entry, which */
					/* shall be owner of the block.	 */
	if ( ! strcmp ( ename, ept->name ) )
	{
		ept = ept->dupenxt;	/* Get directly the next entry	 */
		continue;		/* and continue with operation.	 */
	}
					/* ... and delete all other block*/
					/* references.			 */
#if DEBUG
IOdebug ("	dup_select_entry :	Delete reference in entry : %s (ref-lev=%d)",
	 ept->name, ept->b_ref.ref_level );
#endif
					/* Read the specified directory- */
					/* block into memory and ...	 */
	bp = bread ( 0, ept->b_ref.pi_bnr, 1, SAVEA );

					/* .. pick the appropriate direc-*/
					/* tory element			 */
	dpt = &bp->b_un.b_dir[ept->b_ref.pi_off];

					/* Decide by looking at the le-	 */
	switch ( ept->b_ref.ref_level )	/* vel on which the dup-block	 */
	{				/* is referred.			 */

		case 0 :		/* INODE level			 */
		{
#if DEBUG
IOdebug ("	dup_select_entry :	Dup block on inode-level. (place=%d)",
	 ept->b_ref.pi_place );
#endif
					/* Where does the dup block re-	 */
					/* sides in the inode ?		 */
			switch ( ept->b_ref.pi_place )
			{
					/* A single indirect block	 */
				case -1 :
					dpt->de_inode.i_ib[0] = 0;
					break;
					/* A double indirect block	 */
				case -2 :
					dpt->de_inode.i_ib[1] = 0;
					break;
					/* A direct block		 */
				default :
					dpt->de_inode.i_db[ept->b_ref.pi_place] = 0;
			}
			break;
		}
		case 1 :		/* SINGLE INDIRECT level	 */
		{
#if DEBUG
IOdebug ("	dup_select_entry :	Dup block on single indirect level. (offset=%d)",
	 ept->b_ref.ind_off[0] );
#endif
					/* Read the indirection block	 */
			bind = bread ( 0, ept->b_ref.ind_bnr[0], 1, SAVEA );

					/* Clear the reference and ...	 */
			bind->b_un.b_daddr[ept->b_ref.ind_off[0]] = 0;
					/* write the block back.	 */
			test_bwrite ( bind );
			break;
		}
		case 2 :		/* DOUBLE INDIRECT level	 */
		{
#if DEBUG
IOdebug ("	dup_select_entry :	Dup block on double indirect level. (offset=%d)",
	 ept->b_ref.ind_off[1] );
#endif
					/* Read the indirection block	 */
			bind = bread ( 0, ept->b_ref.ind_bnr[1], 1, SAVEA );

					/* Clear the reference and ...	 */
			bind->b_un.b_daddr[ept->b_ref.ind_off[1]] = 0;
					/* write the block back.	 */
			test_bwrite ( bind );
			break;
		}
	} /* end of <switch (ref_level)> */

					/* Correct the number of blocks	 */
					/* and some other total counters */
					/* kept in the inode:		 */
	if ( update_inode_totals ( &dpt->de_inode ) )
					/* No changes were made		 */
		brelse ( bp->b_tbp, TAIL );
	else				/* There were some modifications */
	{				/* made				 */
#if DEBUG
IOdebug ("	dup_select_entry :	Update inode for %s on disk",
	 ept->name );
#endif
		test_bwrite ( bp );	/* Write the modified directory- */
	}				/* block back to disk.		 */

	ept = ept->dupenxt;		/* Take the next element	 */

 } /* end of <while (ept)> */
}


/*-----------  Operations on block number hash-table and queues  --------*/

/*************************************************************************
 * APPEND A NEW ELEMENT TO THE APPROPRIATE DUPLICATE BLOCK-NUMBER HASH-QUEUE
 *
 * - The block is inspected an the 'type' - flag is set according to the
 *   type of the data kept in the block.
 * - A new element is created and linked into the appropriate hash-queue.
 *
 * Parameter  : bnr	 = The absolute block-number
 *		refcnt	 = The number of references to this block
 * Return     : - nothing -
 *
 *************************************************************************/
void
append_dup_bnr ( daddr_t bnr, word refcnt )
{
 word index, dummy;
 struct dup_bnr *hpt;
 struct buf *bp;

 index = calc_dup_hash ( bnr );		/* Calculate the hash-index	*/

 IOdebug ("%sBlock no %d is multiple allocated.", S_WARNING, bnr );

#if DEBUG
IOdebug ("	append_dup_bnr :	Append block %d to hash table on slot %d",
	 bnr, index);
#endif
					/* Allocate space for the new	*/
					/* hash-table element		*/
 hpt = (struct dup_bnr *) Malloc ( sizeof (struct dup_bnr) );
 if ( hpt == (struct dup_bnr *) NULL )
 {
#if DEBUG
IOdebug ("	append_dup_bnr :	Unable to allocate memory !");
#endif
	longjmp (term_jmp, 1);
 }

 hpt->bnr      = bnr;			/* Save the block number	*/

 bp = bread ( 0, bnr, 1, SAVEA );	/* Get the block and examine	*/
					/* it's content:		*/

					/* We have to note especially,	*/
 if ( possible_dir_block ( bp ) > 0 )	/* if the block is a directory- */
 {					/* block ...			*/
#if DEBUG
IOdebug ("	append_dup_bnr :	This block seems to be a directory-block.");
#endif
	hpt->type = POSSIBLE_DIR;
 }
 else					/* ... or if it is an indirect	*/
 {					/* block.			*/
	if ( possible_indirect_block ( bp, FALSE, &dummy ) )
	{
#if DEBUG
IOdebug ("	append_dup_bnr :	This block seems to be an indirect block.");
#endif
		hpt->type = POSSIBLE_INDIRECT;
	}
	else
	{
#if DEBUG
IOdebug ("	append_dup_bnr :	The block is neither a directory nor an indirect block");
#endif
		hpt->type = POSSIBLE_UNKNOWN;
	}
 }
					/* The block is not used any -	*/
 brelse ( bp->b_tbp, TAIL );		/* more at the moment.		*/

 hpt->cnt      = refcnt;		/* Save the number of references*/
					/* At the moment: no entries !	*/
 hpt->dupenxt = (struct dup_entry *) NULL;
					/* Insert the element on the	*/
					/* first position in the queue	*/
 hpt->dupnxt = dup_hash_tab[index].dupnxt;
 dup_hash_tab[index].dupnxt = hpt;
}


/*************************************************************************
 * APPEND A NEW ELEMENT TO THE APPROPRIATE DUPLICATE ENTRY QUEUE
 *
 * - An exact specification of an entry which keeps a multiple allocated
 *   block is stored in a queue which is associated with the block-number
 *   of this block.
 * - The entry is only marked as 'usable' for selection, if one of the
 *   following conditions is true:
 *   o The dup block seems to contain directory informations AND is
 *     referred by an entry of Type_Directory.
 *   o The dup block does not contain directory informations AND it
 *     is referred by an entry which is NOT of Type_Directory.
 *   o The dup block could be an indirect block and is referred by
 *     an entry on indirect block-level.
 *
 * Parameter  : dupbnr = The duplicate block-number which is referred
 *		name   = Pointer to the entry name
 *		type   = The type of the entry which claims the block
 *		mtime  = The time of the last modification
 *		r      = Pointer to a structure which keeps all reference
 *			 informations
 * Return     : - nothing -
 *
 *************************************************************************/
void
append_dup_entry ( daddr_t dupbnr, char *name, time_t mtime,
		   word type, struct ref *r )
{
 word index;
 struct dup_entry *ept;
 struct dup_bnr   *hpt;
					/* Calculate the hash-index to	*/
 index = calc_dup_hash ( dupbnr );	/* get the position in the table*/
					/* Start with the first entry	*/
 hpt = dup_hash_tab[index].dupnxt;	/* in the specified row		*/

					/* Look for the duplicate number*/
					/* in the appropriate queue	*/
 while ( hpt != (struct dup_bnr *) NULL &&
	 hpt->bnr != dupbnr )
	     hpt = hpt->dupnxt;
					/* If we have found the bnr	*/
 if ( hpt->bnr == dupbnr )
 {
	IOdebug ("%sEntry '%s' refers multiple allocated block no %d",
		    S_INFO, name, dupbnr );

					/* Allocate memory for the entry*/
	ept = (struct dup_entry *) Malloc ( sizeof (struct dup_entry) );
	if ( ept == (struct dup_entry *) NULL )
	{
#if DEBUG
IOdebug ("	append_dup_entry :	Unable to allocate memory !");
#endif
		longjmp (term_jmp, 1);
	}

					/* Save the entry's name, ...	*/
	strcpy ( ept->name, name );
	ept->mtime = mtime;		/* the modification date and ...*/
	ept->type  = type;		/* the type of the entry which	*/
					/* claims the block		*/
	ept->b_ref = *r;		/* Copy the reference infos	*/

			/* Mark the block as uable, if the type of the	*/
			/* entry (dir, file) or the reference position	*/
			/* (indirect) correspond with the content-flag	*/

	if ( ( hpt->type == POSSIBLE_DIR      && type == Type_Directory ) ||
	     ( hpt->type == POSSIBLE_UNKNOWN  && type != Type_Directory ) ||
	     ( hpt->type == POSSIBLE_INDIRECT && r->pi_place < 0 ) )
	{
#if DEBUG
IOdebug ("	append_dup_entry :	Matching block type and entry mode-flag");
#endif
		ept->usable = TRUE;
	}
	else
	{
#if DEBUG
IOdebug ("	append_dup_entry :	The type of the block and the mode-flag divert");
#endif
		ept->usable = FALSE;
	}
					/* Insert the element on first	*/
					/* position in the hash-queue	*/
	ept->dupenxt = hpt->dupenxt;
	hpt->dupenxt = ept;
 }
}


/*************************************************************************
 * REMOVE THE DUPLICATE BLOCK-NUMBER HASH TABLE AND ALL THEIR ELEMENTS
 * BY DEALLOCATING THEM
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
void
remove_dup_hash ( void )
{
 word i;
 struct dup_bnr   *hpt, *hpt_nxt;
 struct dup_entry *ept, *ept_nxt;

#if DEBUG
IOdebug ("	remove_dup_hash :	Clear duplicate bnr hash-table.");
#endif
					/* Work on the hash-table	*/
 for ( i = 0 ; i < DUPHASHSZ ; i++ )
 {					/* Initialization to the first	*/
	hpt = dup_hash_tab[i].dupnxt;	/* element in a row		*/

					/* Follow the row		*/
	while ( hpt != (struct dup_bnr *) NULL )
	{
		ept = hpt->dupenxt;	/* Initialization to the first	*/
					/* element in a column		*/
					/* Follow the column		*/
		while ( ept != (struct dup_entry *) NULL )
		{
			ept_nxt = ept->dupenxt;
			Free ( ept );	/* Free the element		*/
			ept = ept_nxt;
		}

		hpt_nxt = hpt->dupnxt;	/* Save the last pointer	*/
		Free ( hpt );		/* Free the element		*/
		hpt = hpt_nxt;		/* Go on with the saved pointer	*/
	}
	dup_hash_tab[i].dupnxt = (struct dup_bnr *) NULL;
 } /* end <for (i)> */
}

/*----------------------------------------------------------------------*/

/* end of condups.c */
