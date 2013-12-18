/* 16.08.90 - Made the basic cuts to build the integrated version
 *            Usage of the standard printf()
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
   |  dircheck.c							     |
   |                                                                         |
   |	Error detection and recovery on single directory-entries	     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 11 May   1991 - Error handling centralized           |
   |    1 - H.J.Ermen - 21 March 1989 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1

#include "error.h"

#include "check.h"

/*--------------------	Global variables  ------------------------------*/

word changes_de;	/* This is a counter-flag used to determine,	*/
			/* whether to write a block or not,		*/
			/* depending on modifications made.		*/

/*---------------------  Local variables  ------------------------------*/

static word val_blks;			/* To count the valid blocks	*/
static word data_blks;			/* during validation of an entry*/
static word dup_count	= 0;		/* Counter for duplicate names	*/
static char cur_path[512];		/* Current path 		*/
static char cur_name[32];		/* Current name			*/
static word cur_printed;		/* Was current path printed ?	*/

static void *my_bsearch(const void *key, const void *base,
       	       	        size_t nmemb, size_t size,
              	        int (*compar)(const void *, const void *));
              	     
/*---------------------  Local prototypes  -----------------------------*/

static void set_cur (char *path, char *name);
static void print_cur (void);

static void correct_type (struct dir_elem *dp, word should_be);
static word handle_direct (struct inode *ip);
static word handle_single_indirect (struct buf *bp, word mode, word ref_update);
static word handle_double_indirect (struct buf *bp, word mode, word ref_update);

static int comp_func (daddr_t *a, daddr_t *b);

/*************************************************************************
 * ANALYSIS OF THE CONTENT OF A DIRECTORY ENTRY (INODE)
 *
 * - Check and correct type, name and time-stamps
 * - Check the block numbers referred by the inode for invalid ones
 * - Work on the allocation scheme to find out invalid block numbers
 *   referred by indirect blocks and examine the content of directory-
 *   and link-path blocks.
 * - For each modification made by validate_entry(), "changes_de" will be
 *   incremented. "changes_de" is inspected by the procedure which calls
 *   called validate_entry() to decide whether to write the block with
 *   the modified inode back to disk or not.
 *
 * Parameter  : dp	   = Pointer to the sample directory entry
 *		path	   = Additional pathname string or NULL
 *		should_be  = Optionally given type (especially root-dir)
 *		ref_update = If this flag is set to TRUE, all valid blocks
 *			     which are found are noted in the ref-maps by a
 *			     bitmap_incr()-operation.
 * Return     : TRUE  : Analysis resulted in an usable directory entry
 *		FALSE : The entry is irreparable corrupted. Therefore it
 *			has to be deleted by the calling routine
 *
 *************************************************************************/
word
validate_entry ( struct dir_elem *dp, char *path, word should_be, word ref_update )
{
 Date time;
 word bcnt;
					/* Two inode block-counters:	*/
 word free_cnt, free_slots;		/* Inode-slots occupied with 0	*/
 word inval_cnt;			/* .. or irregular block-nums	*/
 word de_errors, result_ok;
 word fatal;				/* flag to signal fatal errors	*/
					/* during inode inspection	*/
 word first_free;			/* To keep track on the first	*/
					/* empty (zeroed) slot		*/
 struct buf *bp;
 struct dir_elem de;
					/* Some local error flags	*/
 word mode_err		= NONE;
 word name_err		= NONE;
 word times_err		= NONE;
 word dup_name_err	= NONE;
 word blocks_err	= NONE;
 word size_err		= NONE;
 word matrix_err	= NONE;
 de_errors = 0;				/* At first: no errors found	*/
					/* Clear the comparison struct	*/
 memset ( &de, 0, sizeof (struct dir_elem) );
					/* We can stop the examination, */
					/* if a totally cleared entry is*/
					/* found.			*/

 if ( ! my_memcmp ( &de, dp, sizeof (struct dir_elem) ) ) {
	if ((should_be == Type_Directory) && my_strcmp(path,"/")) {
		return FALSE;	/* root must not be empty */
	}
	else		
		return TRUE;
 }
	
 {
	if ( dp->de_name[0] == '\0' &&
			/* A totally empty inode is following	*/
	     ! my_memcmp ( &de.de_inode, &dp->de_inode, sizeof (struct inode)) )
	{
				/* Set the complete entry to zero and	*/
				/* note the modification.		*/
		memset ( dp, 0, sizeof (struct dir_elem) );
		changes_de++;
		if ((should_be == Type_Directory) && my_strcmp(path,"/")) {
			return FALSE;
		}
		return TRUE;	/* The inode is kept alive !		*/
	}
 }

 set_cur (path, dp->de_name);	/* set up current name for printing	*/
 

DEBdebug ("	validate_entry : Working on [%s]", dp->de_name );


					/*=========  CHECKS  ===========*/

/*---------------------  Is it a valid entry name ? --------------------*/


DEBdebug ("	validate_entry :	Check the entry name syntactically.");


					/* Now check up to the first	*/
					/* occurrence of a \0-character */

 if ( valid_entry_name ( dp->de_name , TRUE ) )
 {
					/* The entry name is syntacti-	*/
					/* cally correct.		*/

DEBdebug ("	validate_entry :	Look for duplicate entry name.");

					/* Search in the list of	*/
					/* directory entries.		*/
	if ( isin_de_hash ( dp->de_name ) )
	{
		dup_name_err |= SERIOUS;	/* The name was found in the	*/
		de_errors++;		/* table!			*/
	}
 }
 else					/* We have to correct the name	*/
 {
	name_err |= SERIOUS;
	de_errors++;
 }

/*----------------  Are the time stamps plausibel ?  -------------------*/


DEBdebug ("	validate_entry :	Check the time-stamps for zero.");

					/* Compare creation-time with	*/
					/* access and mofification-time */
 if ( dp->de_inode.i_ctime == 0 ||
      dp->de_inode.i_mtime == 0 ||
      dp->de_inode.i_atime == 0 )
 {					/* This is a serious error and	*/
					/* we have to reinitialize the	*/
	times_err |= SERIOUS;		/* time-stamps afterwards	*/
	de_errors++;			/* Note it as a serious error	*/
 }
 else
 {					/* Non-plausibel time-stamps	*/
					/* have to be corrected, but	*/
					/* we don't increment the over-	*/
					/* all error counter.		*/

DEBdebug ("	validate_entry :	Check the time-stamps for plausibility.");

	if ( /* dp->de_inode.i_ctime > dp->de_inode.i_atime || */
	     /* dp->de_inode.i_ctime > dp->de_inode.i_mtime || */
	     dp->de_inode.i_mtime > dp->de_inode.i_atime )
		times_err |= WARNING;

 }

/*--------------- Is the access matrix unequal to zero ?  --------------*/


DEBdebug ("	validate_entry :	Check the access matrix");

					/* At least one alter or dele-	*/
					/* bit must be set!		*/
 if ( ! dp->de_inode.i_matrix & 0xc0c0c0c0 )
 {
	matrix_err |= SERIOUS;
	de_errors++;
 }

/*---  Are the number of blocks and the size of the entry plausibel ? --*/


DEBdebug ("	validate_entry :	Check # of blocks and entry size.");

				/* It is a true error, if the number of	*/
				/* blocks run totally out of range.	*/
 if ( dp->de_inode.i_blocks > i_fs.fs_size ||
      dp->de_inode.i_blocks < 0 )
 {
	blocks_err |= SERIOUS;
	de_errors++;
 }
				/* Because of the fact that it is actu-	*/
				/* ally not possible to seek over non	*/
				/* allocated blocks, file sizes which	*/
				/* lay beyond the block-frame are not	*/
				/* allowed.				*/
 if ( dp->de_inode.i_size > dp->de_inode.i_blocks * i_fs.fs_bsize ||
      dp->de_inode.i_size < 0 )
 {
	size_err |= SERIOUS;
	de_errors++;
 }

/*---------  Check the validity of the referred block numbers  ---------*/


DEBdebug ("	validate_entry :	Check the validity of the block-numbers.");


					/* The direct data blocks	*/
 for ( bcnt = 0 , free_cnt = 0 , inval_cnt = 0 ;
       bcnt < i_fs.fs_ndaddr ; bcnt++ )
 {					/* Zero signals an empty slot	*/
	if ( dp->de_inode.i_db[bcnt] == 0 )
	{
		free_cnt++;
		continue;
	}
					/* We have to keep track on in-	*/
					/* valid block numbers		*/
	if ( ! valid_bnr ( dp->de_inode.i_db[bcnt] ) )
	{				/* Zero the invalid block nums	*/
					/* to guarantee integrity	*/
		dp->de_inode.i_db[bcnt] = 0;
		inval_cnt++;
		changes_de++;		/* Note the modification	*/
					/* The chain is broken now !	*/
		continue;
	}
					/* Handling of symbolic links	*/
	if ( dp->de_inode.i_mode == Type_Link &&
	     bcnt > 0 &&		/* Only i_db[0] is allowed !!	*/
	     dp->de_inode.i_db[bcnt] )
	{
		dp->de_inode.i_db[bcnt] = 0;
		inval_cnt++;
		changes_de++;
		bitmap_set_free (dp->de_inode.i_db[bcnt]);
	}
 }
					/* The indirect blocks		*/
 for ( bcnt = 0 ; bcnt < i_fs.fs_niaddr ; bcnt++ )
 {					/* Zero signals an empty slot	*/
	if ( dp->de_inode.i_ib[bcnt] == 0 )
	{
		free_cnt++;
		continue;
	}				/* We have to keep track on in-	*/
					/* valid block numbers		*/
	if ( ! valid_bnr ( dp->de_inode.i_ib[bcnt] ) )
	{				/* Set invalid entries to zero	*/
		dp->de_inode.i_ib[bcnt] = 0;
		inval_cnt++;
		changes_de++;		/* Note the modification	*/
		continue;
	}
					/* Handling of symbolic links	*/
	if ( dp->de_inode.i_mode == Type_Link &&
	     dp->de_inode.i_ib[bcnt] )	/* Symbolic link information is	*/
	{				/* not kept in indirect blocks!	*/
		dp->de_inode.i_ib[bcnt] = 0;
		inval_cnt++;
		changes_de++;
		bitmap_set_free (dp->de_inode.i_db[bcnt]);
	}
 }

			/* At this point, we are sure that all block-	*/
			/* numbers in the inode are zeroed or refer to	*/
			/* a valid block on disk.			*/

 /*-----------	Check the type of the directory entry  -----------------*/

			/* Look for the type-flag and prove, that the	*/
			/* data kept for this entry is valid, if it is	*/
			/* of Type_Directory or Type_Link.		*/

DEBdebug ("	validate_entry :	Check the type of the entry");

			/* Note that FREE is not recognized, because it */
			/* could be a destroyed flag.			*/
 switch ( dp->de_inode.i_mode )
 {
	case Type_Directory :		/* Directory entry-mode		*/
				for ( bcnt = 0 , free_slots = 0 ;
				      bcnt < i_fs.fs_ndaddr ; bcnt++ )
				{	/* A non-zero reference	?	*/
					if ( dp->de_inode.i_db[bcnt] )
					{
					/* The first non-zeroed ref.	*/
					/* Read a directory-block	*/
						bp = bread ( cdev, dp->de_inode.i_db[bcnt], 1, SAVEA );
						if (!bp) {
Report ("%svalidate_entry() : Failed to read directory block (bnr %d) !",S_FATAL,dp->de_inode.i_db[bcnt]);
							longjmp (term_jmp, DIR_BLK_ERR );
						}
					/* Is it really a directory -	*/
					/* block ? (also a totally empty*/
					/* block is valid...)		*/
						if ( ! possible_dir_block ( bp ) )
						{
							mode_err |= SERIOUS;
							de_errors++;
						}

					/* The block is not used anymore*/
						brelse ( bp->b_tbp, TAIL );

					/* We finish our inspection at	*/
					/* this point, nevertheless,	*/
					/* what the results are.	*/
						break;
					}
					else
						free_slots++;
				}
				break;

	case Type_Link	    :		/* Symbolic link entry-mode	*/
					/* A block number referred ?	*/
				if ( dp->de_inode.i_db[0] )
				{
					/* Get the link path reference	*/
					bp = bread ( cdev, dp->de_inode.i_db[0], 1, SAVEA );
					if (!bp) {
Report ("%svalidate_entry() : Failed to read directory block (bnr %d) !",S_FATAL,dp->de_inode.i_db[bcnt]);
						longjmp (term_jmp, DIR_BLK_ERR );
					}
					/* Test for valid pathname	*/
					if ( ! possible_link_block ( bp ) )
					{
						mode_err |= SERIOUS;
						de_errors++;
					}

					brelse ( bp->b_tbp , TAIL );
				}
				else
				{
					mode_err |= SERIOUS;
					de_errors++;
				}
				break;
	case Type_File	    :		/* Ordinary files does not need */
					/* to be inspected.		*/
				break;
					/* We have to find out, which	*/
					/* type of file was used !	*/
	default		    :	mode_err |= SERIOUS;
				de_errors++;
 } /* end of <switch> */

 /*-----------------------  Report the results	------------------------*/

					/* Note that we have a damaged	*/
					/* directory-entry.		*/
 if ( times_err || de_errors || inval_cnt )
	fst.damaged_inodes++;
					/* Print the entry name		*/
 if ( times_err || de_errors || inval_cnt || ! silent_mode )
 	print_cur ();

					/* Print a detailed error-report*/
					/* if any error occurred:	*/
 if ( de_errors )
 {
	if ( de_errors == 1 )
		Report ("%s1 error was detected for this entry:", S_SERIOUS);
	else
		Report ("%s%d errors were detected for this entry:",
			 S_SERIOUS, de_errors);

	if ( mode_err )
		Report ("%s: The type inspection has failed. (type-flag found: 0x%x)",
			 S_INFO, dp->de_inode.i_mode);
	if ( dup_name_err )
		Report ("%s: Entry name found twice in the same directory.",
			 S_INFO);
	if ( times_err & SERIOUS )
		Report ("%s: The time-stamps of the entry are damaged.\n", S_INFO);
	elif ( times_err & WARNING )
		Report ("%s: Relations between the time-stamps are bad.\n", S_INFO);
	if ( matrix_err )
		Report ("%s: The access matrix is corrupted.\n", S_INFO);
	if ( blocks_err )
		Report ("%s: The block-count for the entry is totally wrong.\n", S_INFO);
	if ( size_err )
	{
		Report ("%s: The size-field of the entry (%d) is suspicious:"
	                "%s  it should be between %d and %d",
			  S_INFO, 
			  dp->de_inode.i_size, 
			  S_INFOCLR,
			  dp->de_inode.i_blocks * i_fs.fs_bsize, 
			  (dp->de_inode.i_blocks + 1) * i_fs.fs_bsize );
	}
	if ( inval_cnt )
		Report ("%s: Found %d invalid direct block number references.",
			S_INFO, inval_cnt);
}

 /*-----------------  Break on fatal conditions  -----------------------*/

 fatal = FALSE;			/* We start with this assumption and try*/
				/* to find out now whether a fatal	*/
				/* error-condition has occurred.	*/

					/* Too many invalid block nums	*/
 if ( (i_fs.fs_ndaddr + i_fs.fs_niaddr - free_cnt) * mingood / 100 + 1
      <= inval_cnt )			/* Yes !!! - That's bad...	*/
 {
	fatal = TRUE;
	fst.mingood_dir++;
	Report ("%sToo many invalid block numbers (%d) found !",
		    S_SERIOUS, inval_cnt);
 }

 if ( de_errors > 2 )			/* Too many other errors	*/
 {
	fatal = TRUE;
	Report ("%sToo many inode-errors found !",
		    S_SERIOUS, de_errors);
					/* We have the decision :	*/
 }

 if ( fatal )				/* One of the fatal conditions	*/
 {					/* araised ?			*/
	fst.wrong_inodes++;		/* One more inode, which seems	*/
					/* to be not usable.		*/
					/* Always delete in basic-mode!	*/
	return FALSE;
 }


/*===================== POSSIBLE CORRECTIONS ===========================*/

/*-----------------  Correct the mode of the entry  --------------------*/

					/* Find out the file-type, if	*/
 if ( mode_err )			/* it is corrupted.		*/
 {
	if ( should_be == Type_Directory && my_strcmp(path, "/") ) {
		dp->de_inode.i_mode = Type_Directory; 	/* special treatment */
		changes_de++;				/* of root dir       */
	}	 
	else {
		Report ("%sI try to find out the correct type of the entry.",
		 	   S_SERIOUS);
					/* correct_type() always comes	*/
					/* to a solution!		*/
		correct_type ( dp, should_be );
		changes_de++;		/* Note the modification	*/
	}
 }

/*-------------------  Correct the name of the entry  ------------------*/

					/* If the filename is corrup-	*/
 if ( name_err || dup_name_err )	/* ted: type in a new one !	*/
 {
	do
	{				/* Try it again and again ...	*/
		if ( name_err )
			Report ("%sThe entry [%s] has an invalid name.",
				  S_SERIOUS, dp->de_name);
		elif ( dup_name_err )
			Report ("%sThe entry [%s] has a duplicate name.",
				  S_SERIOUS, dp->de_name);

					/* Different "generation" of a	*/
					/* new name depending on the	*/
					/* actual operation - mode.	*/
		{
			char tbuf[NameMax];
			
			memset ( tbuf, 0, NameMax);
			memset ( in_buf, 0, NameMax);
			strcpy ( in_buf, "DUPCORR_" );
			my_itoa ( tbuf, dup_count );
			strcat ( in_buf, tbuf );
			dup_count++;
		}

		Report ("%sGenerated a new entry name : '%s'.", S_INFO, in_buf );

					/* ... and perform the checks	*/
		if ( valid_entry_name ( in_buf , FALSE ) )
			name_err = NONE;
		else
			name_err = SERIOUS;

		if ( isin_de_hash ( in_buf ) )
			dup_name_err = SERIOUS;
		else
			dup_name_err = NONE;

	} while ( name_err || dup_name_err );
					/* Save the correct name	*/
	strncpy ( dp->de_name, in_buf, NameMax );
	changes_de++;			/* Note the modification	*/
 }

/*---------------------  Correct the time-stamps  ----------------------*/

 if ( times_err )
 {
	time = GetDate ();
					/* The following conditions sig-*/
					/* nal some damaged time-flags.	*/
	if ( dp->de_inode.i_ctime == 0 )
	{
		dp->de_inode.i_ctime = time;
		Report ("%sThe creation time-stamp is corrected.", S_INFO);
		changes_de++;		/* Note the modification	*/
	}

/*
	if ( dp->de_inode.i_ctime > dp->de_inode.i_mtime )
	{
		dp->de_inode.i_mtime = dp->de_inode.i_ctime;
		Report ("%sThe modification time-stamp is corrected.", S_INFO);
		changes_de++;
	}
*/

	if ( dp->de_inode.i_mtime > dp->de_inode.i_atime )
	{
		dp->de_inode.i_atime = dp->de_inode.i_mtime;
		Report ("%sThe access time-stamp is corrected.", S_INFO);
		changes_de++;
	}
 }

/*--------------------	Allow access to the entry  ---------------------*/

 if ( matrix_err )
 {
	Report ("%sThe default 'da' permissions are added to the matrix.", S_INFO);
	switch ( dp->de_inode.i_mode )
	{
		case Type_Directory : dp->de_inode.i_matrix |= DefDirMatrix;
				      break;
		case Type_File	    : dp->de_inode.i_matrix |= DefFileMatrix;
				      break;
		case Type_Link	    : dp->de_inode.i_matrix |= DefLinkMatrix;
	}
	changes_de++;
 }

/*----------------  Work on the allocation scheme  ---------------------*/

/* General strategy :
 *
 *  - Too many errors in the list of direct blocks cause the deletion
 *    of the inode.
 *  - Too many errors in single or double indirect blocks cause deletion
 *    of these blocks and their entries. The corresponding fields in the
 *    inode are set to zero but the inode itself is 'kept alive' !
 *
 */

 val_blks  = 0;				/* Actually no valid block	*/
 data_blks = 0;				/* Clear the reference structure*/


DEBdebug ("	validate_entry :	Work on the allocation scheme.");


 /*-----------------------  Direct block level -------------------------*/

				/* Inspect the DIRECT BLOCKS, if the	*/
				/* entry is a directory or link. The	*/
				/* data blocks cannot be validated.	*/

 if ( dp->de_inode.i_mode == Type_Directory ||
      dp->de_inode.i_mode == Type_Link )
 {
	if ( ! handle_direct ( &dp->de_inode ) )
					/* If there are too many errors */
					/* found in the list of direct	*/
					/* blocks.			*/
		return FALSE;
 }

 /* Note: Actually, the server is not able to seek over non allocated	*/
 /* blocks. Therefore we have to spend some extra efforts, if we have	*/
 /* found an invalid block and set it to zero. The strategy is, to fill	*/
 /* all these gaps by copying block references from higher to lower	*/
 /* "addresses". This is done with the list of direct blocks and with	*/
 /* single and double indirect blocks.					*/

					/* Update the informations in	*/
					/* the reference bit-maps.	*/

 for ( bcnt = 0 , first_free = -1 ; bcnt < i_fs.fs_ndaddr ; bcnt++ )
 {					/* If we have found any block-	*/
					/* number, we can be sure that	*/
					/* it is a valid one!		*/
	if ( dp->de_inode.i_db[bcnt] )
	{

DEBdebug ("	validate_entry :	Note at pos %d block number %d",
	 bcnt, dp->de_inode.i_db[bcnt]);

					/* Update the reference map	*/
		if ( ref_update )
		{
			found_blocks++;
			bitmap_incr (dp->de_inode.i_db[bcnt]);
		}
		val_blks++;		/* One more valid block...	*/
		data_blks++;
					/* Is there a free slot, we have*/
					/* previously found ?		*/
		if ( first_free != -1 )
		{			/* Copy reference to the free	*/
					/* slot.			*/
			dp->de_inode.i_db[first_free] = dp->de_inode.i_db[bcnt];
			dp->de_inode.i_db[bcnt] = 0;
			changes_de++;
					/* The next free slot is conti-	*/
					/* guous to the previous one.	*/
			first_free++;
		}
	}
	else				/* An empty slot was found!	*/
	{				/* Note it, if it is the first	*/
					/* zeroed slot we have found.	*/

DEBdebug ("	validate_entry :	Found free slot at pos %d", bcnt );

		if ( first_free == -1 )	/* Note the free slot.		*/
			first_free = bcnt;
	}
 }

 /*-------------------	The levels of indirection  ---------------------*/

					/* Handle INDIRECT BLOCKS	*/
 for ( bcnt = 0 ; bcnt < i_fs.fs_niaddr ; bcnt++ )
 {
					/* A non-zeroed block number is	*/
	if ( dp->de_inode.i_ib[bcnt] )	/* a valid block number.	*/
	{				/* Multiple allocated ?		*/
		if ( bitmap_get (dp->de_inode.i_ib[bcnt]) > 0 )
		{
			/* If the indirect block is multiple allocate,	*/
			/* we can be sure that all blocks referred by	*/
			/* this block were checked. So we can skip over	*/
			/* it.						*/

DEBdebug ("	validate_entry :	Indirection block (level=%d) is multiple allocated.",
	 bcnt + 1 );

			if ( ref_update )
			{
				bitmap_incr (dp->de_inode.i_ib[bcnt]);
				found_blocks++;
			}
		}
		else
		{

DEBdebug ("	validate_entry :	Inspect the indirection block (level=%d)",
	 bcnt + 1 );

					/* Read the indirect block and..*/
			bp = bread ( cdev, dp->de_inode.i_ib[bcnt], 1, SAVEA );
			if (!bp) {
Report ("%svalidate_entry() : Failed to read indirect block (bnr %d) !",S_FATAL,dp->de_inode.i_ib[bcnt]);
				longjmp (term_jmp, IND_BLK_ERR );
			}
					/* .. check the content.	*/
					/* of the single indirect block */
			if ( bcnt == 0 )
			{
				result_ok = handle_single_indirect ( bp, dp->de_inode.i_mode,
								     ref_update );
			}
					/* and the double indirect blk.	*/
			else
			{
				result_ok = handle_double_indirect ( bp, dp->de_inode.i_mode,
								     ref_update );
			}
					/* Validation succeeds ?	*/
			if ( result_ok )
			{		/* Note the single indirect blk	*/

DEBdebug ("	validate_entry :	Note indirect block (level=%d) in bit-map",
	 bcnt + 1 );

				if ( ref_update )
				{
					bitmap_incr (dp->de_inode.i_ib[bcnt]);
					found_blocks++;
				}
				val_blks++;
			}
			else		/* The validation of an indirect*/
			{		/* block failed.		*/

DEBdebug ("	validate_entry :	Clear indirect reference (level=%d)",
	 bcnt + 1 );

					/* Mark as being freed		*/
				bitmap_set_free (dp->de_inode.i_ib[bcnt]);
				dp->de_inode.i_ib[bcnt] = 0;
				changes_de++;
			}
		}
	 }
 } /* end of <for (bcnt)> */

/*----------------------  General justifications  ----------------------*/

					/* Different number of valid	*/
					/* blocks after testing ?	*/
 if ( dp->de_inode.i_blocks != val_blks )
 {
	print_cur ();
	Report ("%sA different number of (valid) blocks is counted for this entry.", S_WARNING);
	Report ("%s# of blocks counted = %d  |	referred = %d",
		    S_INFO, val_blks, dp->de_inode.i_blocks);
	changes_de++;
	Report ("%sThe number of blocks will be corrected.", S_INFO);
	dp->de_inode.i_blocks = val_blks;
 }
		/* Correct sizes of entries which don't fit into the	*/
		/* "address frame". Note that the corrected size is	*/
		/* only a first assumption for directory-entries!. The	*/
		/* true size of directory-entries will be set at the	*/
		/* moment, when the entry is used a parent-directory by	*/
		/* check_inodes().					*/

 if ( ( dp->de_inode.i_mode == Type_File ||
	dp->de_inode.i_mode == Type_Link )
      &&
      ( ( data_blks > 0 && dp->de_inode.i_size == 0 ) ||
	( data_blks == 0 && dp->de_inode.i_size > 0 ) ||
	dp->de_inode.i_size < 0 ||
	( dp->de_inode.i_size + i_fs.fs_bsize - 1 ) / i_fs.fs_bsize != data_blks
      )
    )
 {
	print_cur ();
	Report ("%sThe calculated and stored file sizes differ too much.", S_WARNING);
	Report ("%sSize calculated = %d (approx.) |  referred = %d",
		    S_INFO, data_blks * i_fs.fs_bsize, dp->de_inode.i_size);
	Report ("%sThe size field will be corrected.", S_INFO);
				/* Because the exact size is not known,	*/
				/* we calculate the new one approxi-	*/
				/* mately.				*/
	dp->de_inode.i_size = data_blks * i_fs.fs_bsize;

					/* Note the modification	*/
	changes_de++;
 }

 return TRUE;				/* validate_entry succeeds !	*/
}


/*************************************************************************
 * CHECK AND - IF NECESSARY - CORRECT THE TYPE OF THE SPECIFIED
 * DIRECTORY ENTRY
 *
 * - The type-field is corrected under all conditions. In the worst case,
 *   it is simply set to Type_File!
 * - To find out whether an entry is of Type_Directory or Type_Link, we
 *   use possible_dir_block() and possible_link_block() to verify the
 *   content of the blocks kept by the entry.
 *
 * Parameter  : dp	  = Pointer to the directory element
 *		should_be = The expected type of file (for example:
 *			    if we check the root-dir) or UNKNOWN
 * Return     : - nothing -
 *
 ************************************************************************/
static void
correct_type ( struct dir_elem *dp , word should_be )
{
 word bcnt, dir_flag, link_flag;
 struct buf *bp;

/*------------------  Interpret a valid direct block  ------------------*/

 dir_flag  = FALSE;			/* Actually, we don't know any-	*/
 link_flag = FALSE;			/* thing about the mode of the	*/
					/* entry			*/
					/* Scan the direct blocks	*/
 for ( bcnt = 0 ; bcnt < i_fs.fs_ndaddr ; bcnt++ )
 {

DEBdebug ("	correct_type :	Look for a valid direct block number.");

	if ( dp->de_inode.i_db[bcnt] != 0 )
	{
					/* A valid block number was	*/
					/* found. Use it !		*/
					/* Read the block in memory	*/
		bp = bread ( cdev, dp->de_inode.i_db[bcnt], 1, SAVEA );

DEBdebug ("	correct_type :	Analysis of the content of block no %d",
	 dp->de_inode.i_db[bcnt]);

					/* A read-error occurred ?	*/
		if ( !bp )
		{
			Report ("%sRead error occurred", S_SERIOUS);
			continue;	/* Take directly the next block */
		}
					/* Perform some statistic tests:*/
					/* Look, if it is a directory	*/
		if ( possible_dir_block ( bp ) > 0 )
					/* We need at least one valid	*/
					/* entry to succeed.		*/
			dir_flag = TRUE;

					/* ... or a link-path block	*/
		if ( bcnt == 0 )	/* Only the first direct block	*/
		{			/* is allowed.			*/
			if ( possible_link_block ( bp )  &&
			     dp->de_inode.i_blocks == 1 )
					link_flag = TRUE;
		}
					/* Release the non further	*/
					/* used block			*/
		brelse ( bp->b_tbp ,TAIL);
					/* We have got a solution and	*/
					/* can interrupt the search	*/
		if ( dir_flag || link_flag )
			break;
	}
 }

 /*---------------  Fix the results and set the new type  --------------*/

 print_cur ();
 if ( dir_flag )
	Report ("%sI interpret the inode as an entry of type DIRECTORY.", S_INFO);
 if ( link_flag )
	Report ("%sI interpret the inode as an entry of type LINK.", S_INFO);

 if ( ! dir_flag && ! link_flag )
	Report ("%sI can't interpret the type of the given entry.", S_INFO);

					/* Are there any handicaps	*/
 if ( should_be != UNKNOWN )		/* about the type of the entry?	*/
 {
					/* These are SAFE alternatives:	*/
	if ( ( should_be == Type_Directory && dir_flag ) ||
	     ( should_be == Type_Link && link_flag ) )
	{
		dp->de_inode.i_mode = should_be;
		return;
	}

	if ( should_be == Type_File && (!dir_flag) && (!link_flag) )
	{
		dp->de_inode.i_mode = Type_File;
		return;
	}
	
 }
					/* Otherwise, take a straight-	*/
					/* forward approach		*/
 if ( dir_flag )
	dp->de_inode.i_mode = Type_Directory;
 else
 {
	if ( link_flag )
		dp->de_inode.i_mode = Type_Link;
	else
					/* If we have no hint, we set	*/
					/* the type to "File" !		*/
		dp->de_inode.i_mode = Type_File;
 }
	

 Report (">   The type of the entry is now assumed to be :");
 switch ( dp->de_inode.i_mode )
 {
	case Type_Directory : Report ("	DIRECTORY\n");	break;
	case Type_File	    : Report ("	FILE\n");	break;
	case Type_Link	    : Report ("	LINK\n");	break;
 }


}

/*-----------------------------------------------------------------------*/

/*************************************************************************
 * EXAMINE DIRECT BLOCKS IF THEY BELONG TO DIRECTORY- AND LINK-ENTRIES
 *
 * - Check the content of the direct blocks by reading them sequentially
 *   and try to find out, whether they contain inode- or link-informations.
 * - Make all needed corrections or delete the entry.
 * - For symbolic links we have only look for the first direct block, which
 *   keeps the reference path information. If this block does not keep
 *   a valid pathname, we return directly with FALSE. The validity of the
 *   pathname is checked during another phase of the checking process.
 * - Note that the reference bitmap-entries for the list of direct blocks
 *   are updated by validate_entry() which calls this procedure.
 *
 * Parameters	: ip   = Pointer to the describing inode
 * Return	: TRUE : The entry is still usable
 *		  FALSE: There were so many errors, that the entry is in
 *			 an unusable state.
 *************************************************************************/
static word
handle_direct ( struct inode *ip )
{
 word bcnt;
 word wrong_cnt, free_cnt;		/* Corrupted dir- or link-blocks */
 struct buf *bp;

 /*---------  Try to interpret the content of each direct block  -------*/

 for ( bcnt = 0 , wrong_cnt = 0 , free_cnt = 0 ;
       bcnt < i_fs.fs_ndaddr ; bcnt++ )
 {					/* Examine the direct blocks	*/
	if ( ip->i_db[bcnt] == 0 )	/* No block referred ?		*/
	{
		free_cnt++;
		continue;
	}

DEBdebug ("	handle_direct :	Examine the content of block no.: %d",
	 ip->i_db[bcnt]);

					/* Read the desired block	*/
	bp = bread ( cdev, ip->i_db[bcnt], 1, SAVEA );
	if (!bp) {
Report ("%shandle_direct() : Failed to read block (bnr %d) !",S_FATAL,ip->i_db[bcnt]);
		longjmp (term_jmp, READ_ERR );
	}
	switch ( ip->i_mode )
	{				/* Could it be a directory-blk?	*/
		case Type_Directory :
		{
			if ( ! possible_dir_block ( bp ) )
			{
				print_cur ();
 Report ("%sDirectory-block on inode slot %d is invalid.", S_SERIOUS, bcnt );
				if ( ! no_corrections )
				{
					wrong_cnt++;
					/* Mark as being freed		*/
					bitmap_set_free (ip->i_db[bcnt]);
					ip->i_db[bcnt] = 0;
					fst.wrong_dir++;
					changes_de++;
					/* Erase the block and ...	*/
					clr_buf ( bp );
					/* write it back to disk	*/
					test_bwrite ( bp );
				}
				else
					brelse ( bp->b_tbp, TAIL );
			}
			else		/* Yes, it seems so ...		*/
				brelse ( bp->b_tbp, TAIL );
			break;
		}
					/* Could it be a link-blk ?	*/
		case Type_Link :
		{			/* Only the first direct block	*/
					/* is of interest for us !	*/
			if ( bcnt == 0 )
			{
				if ( ! possible_link_block ( bp ) )
				{
					print_cur ();
					if ( no_corrections )
 Report ("%sLink-block on inode slot %d is invalid.", S_SERIOUS, bcnt );
 					else
 					{
 Report ("%sInvalid link-block on inode slot %d will be deleted.",
		    S_SERIOUS, bcnt );
						wrong_cnt++;
						/* Mark as beeing freed		*/
						bitmap_set_free ( ip->i_db[0]);
						ip->i_db[0] = 0;
						changes_de++;
						/* Delete it and write it back	*/
						clr_buf ( bp );
						test_bwrite ( bp );
					}
					/* --> Finish the operation	*/
					return FALSE;
				}
				else
					brelse ( bp->b_tbp, TAIL );
			}
			break;
		}
	} /* end of <switch> */

 } /* end of <for (;;)> */

 /*---------------- The results of the inspection  ----------------------*/

			/* Note : If we have inspected the direct block	 */
			/* references of a link and have found an error, */
			/* we have finished before and returned FALSE!	 */

DEBdebug ("	handle_direct :	  The results : free = %d    wrong = %d",
	 free_cnt, wrong_cnt);


					/* Too many wrong blocks ?	*/
 if ( ( ( i_fs.fs_ndaddr - free_cnt ) * mingood / 100 ) + 1
	<=  wrong_cnt )
 {					/* In advanced (= manual) mode, */
					/* we can decide to keep the	*/
					/* damaged entry.		*/
	print_cur ();
	Report ("%sToo many bad direct blocks kept by the entry! (-> mingood)",
		 S_SERIOUS);
	if ( ! no_corrections ) 
	{
		fst.mingood_dir++;
		return FALSE;
	}
	else
		return TRUE;
 }
					/* If there are only a few	*/
 if ( wrong_cnt )			/* directory blocks with a	*/
 {					/* wrong content.		*/
 	print_cur ();
	Report ("%sThere are a few bad direct blocks kept by the entry !",
		  S_SERIOUS);
	if ( no_corrections )
		return TRUE;
	else
		return FALSE;
 }
					/* There are enough or even all	*/
 return TRUE;				/* direct blocks valid		*/
}


/*************************************************************************
 * HANDLE SINGLE INDIRECT BLOCKS AND THE DATA-BLOCKS THEY REFER
 *
 * - Check the block content for "indirect"-type
 * - Check the noted block numbers and test the block content. ( only if a
 *   directory block is assumed)
 * - If an indirect block is valid and there are no changes made, it is
 *   simply released. If we have done any changes, the block is written
 *   directly to disk. Only if the block is totally corrupted, FALSE is
 *   returned.
 * - Because of the fact that the file-server is not able to seek over
 *   non-allocated blocks, we have to fill all gaps by copying the entries
 *   from higher adresses.
 *
 * Parameters	: bp	     = Pointer to the buffer of the indirect block
 *		  mode	     = The expected type of the entry
 *		  ref_update = If this flag is set to TRUE, all valid blocks
 *			       are noted in the ref-maps by bitmap_incr().
 * Return	: TRUE : It is a valid indirect block
 *		  FALSE: It doesn't seem to be a valid indirect block
 *
 *************************************************************************/
static word
handle_single_indirect ( struct buf *bp, word mode, word ref_update )
{
 word bcnt, changes;
 word first_free;
 daddr_t *bpt;
 struct buf *bp_dir;

 /*------- Try to find out whether it is an indirect block or not ------*/

					/* Is it an indirect block ?	*/
 if ( ! possible_indirect_block ( bp, TRUE, &changes ) )
 {
 	print_cur ();
	Report ("%sToo many invalid block numbers in indirect block found.",
		  S_SERIOUS);

	if ( no_corrections )		/* cannot write to disk :	*/
		return FALSE;		/* no further inspection needed	*/
	else	
	{
		Report ("%sInvalid single indirect block will be deleted.",
			  S_INFO);
		clr_buf ( bp );
		fst.wrong_ind++;
		test_bwrite ( bp );
		return FALSE;
	}
 }

 /*-------------------- Deal with each block referred ------------------*/

 for ( bcnt = 0 , first_free = -1 , changes = 0 , bpt = bp->b_un.b_daddr ;
       bcnt < i_fs.fs_maxcontig ; bcnt++ )
 {
	if ( ! bpt[bcnt] )		/* Skip over entries, marked as	*/
	{				/* "free".			*/
					/* If it is the first zeroed	*/
		if ( first_free == -1 ) /* block we have found, we keep	*/
					/* the block-number.		*/
			first_free = bcnt;
		continue;		/* Go on directly.		*/
	}

	if ( mode == Type_Directory )	/* Examine the content of di-	*/
	{				/* rectory blocks.		*/

DEBdebug ("	handle_single_indirect :	Examine dir-block at index-pos %d bnr = %d",
	 bcnt, bpt[bcnt]);

					/* Read it into memory		*/
		bp_dir = bread ( cdev, bpt[bcnt], 1, SAVEA );
		if (!bp_dir) {
Report ("%shandle_single_indirect() : Failed to read block (bnr %d) !",S_FATAL,bpt[bcnt]);
			longjmp (term_jmp, READ_ERR );
		}
					/* Is it a directory block ?	*/
		if ( ! possible_dir_block ( bp_dir ) )
		{
		 	print_cur ();
		 	if ( no_corrections )
		 		Report ("%sInvalid directory block kept by indirect block.",
		 			  S_SERIOUS);
		 	else
		 	{
		 		Report ("%sInvalid directory block kept by ind. will be deleted",
		 			  S_SERIOUS);
					/* Mark as being freed		*/
				bitmap_set_free (bpt[bcnt]);
				bpt[bcnt] = 0;
				fst.wrong_dir++;
				changes++;
				clr_buf ( bp_dir );
					/* Write the zeroed block back	*/
					/* to disk.			*/
				test_bwrite ( bp_dir );
					/* Is it the first free slot ?	*/
				if ( first_free == -1 )
					/* Then note it !		*/
					first_free = bcnt;
			}
		}
		else			/* It is a valid directory-blk	*/
		{
					/* A valid dir-block has to be	*/
					/* noted in the ref-maps	*/
			if ( ref_update )
			{
				found_blocks++;
				bitmap_incr (bpt[bcnt]);
			}
					/* We have an empty slot ?	*/
			if ( first_free != -1 )
			{		/* Copy the actual reference to	*/
					/* the empty slot.		*/
				bpt[first_free] = bpt[bcnt];
				bpt[bcnt] = 0;
				first_free++;
				changes++;
			}
			val_blks++;
			data_blks++;
					/* We can simply release the	*/
					/* block after the inspection.	*/
			brelse ( bp_dir->b_tbp, TAIL );
		}
	}				/* If we have other entries (of */
	else				/* type file..), we simply in-	*/
	{				/* crement the ref-map directly.*/

DEBdebug ("	handle_single_indirect : Note at pos %d block number %d",
	 bcnt, bpt[bcnt]);

					/* A valid dir-block has to be	*/
					/* noted in the ref-maps	*/
		if ( ref_update )
		{
			bitmap_incr (bpt[bcnt]);
			found_blocks++;
		}
					/* An empty slot available ?	*/
		if ( first_free != -1 )
		{
					/* Copy the actual reference to */
					/* the empty slot.		*/
			bpt[first_free] = bpt[bcnt];
			bpt[bcnt] = 0;
			first_free++;
			changes++;
		}
		val_blks++;		/* Another valid block ...	*/
		data_blks++;
	}
 } /* end of <for (;;)> */
					/* Were there any changes made	*/
 if ( changes )				/* to keep the integrity of the */
	 test_bwrite ( bp );		/* entry ?			*/
 else					/* Then write the block to disk */
	 brelse ( bp->b_tbp, TAIL );	/* otherwise simply release it!	*/

 return TRUE;
}


/*************************************************************************
 * HANDLE DOUBLE INDIRECT BLOCKS AND THE SINGLE INDIRECT BLOCKS THEY REFER
 *
 * - Read block and check it for "indirect"-type
 * - Check the denoted block numbers and test the block content for
 *   "indirect"-blocks
 * - Scan the indirect blocks by calling handle_single_indirect()
 * - Because of the fact that the file-server is not able to seek over
 *   non-allocated blocks, we have to fill all gaps by copying the entries
 *   from higher adresses.
 *
 * Parameters	: bp	     = Pointer to the indirect block buffer
 *		  mode	     = The expected mode
 *		  ref_update = If this flag is set to TRUE, all valid blocks
 *			       are noted in the ref-maps by bitmap_incr()
 * Return	: TRUE : It is an double indirect block
 *		  FALSE: It doesn't seem to be an indirect block
 *
 *************************************************************************/
static word
handle_double_indirect ( struct buf *bp, word mode, word ref_update )
{
 word bcnt, changes;
 word first_free;
 daddr_t *bpt;
 struct buf *bp_ind;

 /*--- Try to find out, whether it is a double indirect block or not ----*/


DEBdebug ("	handle_double_indirect :	Is it really an indirect block ?");

					/* Is it an indirect block ?	*/
 if ( ! possible_indirect_block ( bp, TRUE, &changes ) )
 {
 	print_cur ();
	Report ("%sInvalid data in double indirect block found.", S_SERIOUS);
	if ( no_corrections )
		return FALSE;
	else
	{
		Report ("%sInvalid double indirect block will be deleted.",
			  S_INFO);
		clr_buf ( bp );
		fst.wrong_ind++;
		test_bwrite ( bp );
		return FALSE;
	}
 }

 /*-------------------- Deal with each block referred ------------------*/

 for ( bcnt = 0 , changes = 0 , first_free = -1 , bpt = bp->b_un.b_daddr ;
       bcnt < i_fs.fs_maxcontig ; bcnt++ )
 {
	if ( bpt[bcnt] == 0 )		/* Skip over entries, marked as	*/
	{				/* "free".			*/
		if ( first_free == -1 )	/* If it is the first free block*/
					/* we have to note it !		*/
			first_free = bcnt;
		continue;
	}
					/* Multiple allocated ?		*/
	if ( bitmap_get (bpt[bcnt]) > 0 )
	{				/* We skip over multiple allo-	*/
					/* cated blocks, because they	*/
					/* were examined before...	*/

DEBdebug ("	handle_double_indirect : Single indirect block is multiple allocated");

					/* We have to increment the	*/
					/* number of references ?	*/
		if ( ref_update )
		{
			found_blocks++;
			bitmap_incr (bpt[bcnt]);
		}

		if ( first_free != -1 )
		{			/* Copy the actual reference to	*/
					/* the given free slot		*/
			bpt[first_free] = bpt[bcnt];
			bpt[bcnt] = 0;
			first_free++;
			changes++;
		}
		continue;		/* Go on directly with the next	*/
					/* block number!		*/
	}


DEBdebug ("	handle_double_indirect :	Single indirect block at offset %d bnr = %d",
	 bcnt, bpt[bcnt]);

					/* Read it into memory		*/
	bp_ind = bread ( cdev, bpt[bcnt], 1, SAVEA );
	if (!bp_ind) {
Report ("%shandle_double_indirect() : Failed to read block (bnr %d) !",S_FATAL,bpt[bcnt]);
		longjmp (term_jmp, READ_ERR );
	}
					/* Scan the entries  of the	*/
					/* indirect block.		*/
	if ( handle_single_indirect ( bp_ind, mode, ref_update ) )
	{				/* We have done it with success */
					/* Update the specified cell in	*/
		if ( ref_update )	/* the reference-map.		*/
		{
			found_blocks++;
			bitmap_incr (bpt[bcnt]);
		}

		if ( first_free != -1 )	/* Is there an empty slot ?	*/
		{
					/* Copy the reference to the	*/
					/* empty slot.			*/
			bpt[first_free] = bpt[bcnt];
			bpt[bcnt] = 0;

			first_free++;
			changes++;
		}
					/* One more valid block to be	*/
		val_blks++;		/* noted!			*/
	}
	else				/* Invalid blocks will be de-	*/
	{				/* leted.			*/
					/* Note as being freed		*/
	 	print_cur ();
	 	if ( ! no_corrections )
	 	{
			Report ("%sInvalid single ind. block will be deleted in double ind. block.", S_INFO);
			bitmap_set_free ( bpt[bcnt] );
			bpt[bcnt] = 0;
					/* To be noted as the first	*/
			if ( first_free == -1 )	/* empty slot ?		*/
				first_free = bcnt;
			changes++;
		}
	}
 } /* end of <for (bcnt)> */
					/* Were there any changes made	*/
 if ( changes )				/* to keep the integrity of the */
	 test_bwrite ( bp );		/* entry ?			*/
 else					/* Write it to disk or simply	*/
	 brelse ( bp->b_tbp, TAIL );	/* release it			*/

 return TRUE;
}


/*-----------------------------------------------------------------------*/

/*************************************************************************
 * TEST A BLOCK-NUMBER FOR VALIDITY
 *
 * - The absolut limits are checked.
 * - It is searched in the info-block number table to guarantee that the
 *   block number doesn't refer to an info-block. Because the table is
 *   sorted, a binary search algorithm is used.
 *
 * Parameter  : bnr   = The block number to be inspected
 * Return     : TRUE  : It is a valid block number
 *		FALSE : The block number is invalid
 *
 ************************************************************************/
word
valid_bnr ( daddr_t bnr )
{
 if ( bnr < 3 || bnr > i_fs.fs_size )
	return FALSE;
					/* Binary search on the table of*/
					/* sorted info-block numbers	*/
 if ( my_bsearch ( (void *) &bnr, (void *) info_blocks, (size_t) i_fs.fs_ncg,
	  	   (size_t) sizeof (daddr_t), comp_func ) != NULL )
	return FALSE;

 return TRUE;
}

/*---------------------  The comparison procedure  ---------------------*/

static int				/* Such a procedure is expected	*/
comp_func ( daddr_t *a , daddr_t *b )	/* by bsearch().		*/
{
 if ( *a == *b )  return  0;
 if ( *a <  *b )  return -1;
 if ( *a >  *b )  return  1;
}


/*************************************************************************
 * TEST A CHARACTER STRING TO DETERMINE WHETHER IT IS A VALID DIRECTORY-
 * ENTRY NAME OR NOT
 *
 * - Single characters are tested up to the first occurrence of a '\0'
 *   character.
 * - The remaining buffer space is searched for garbage. These are all
 *   characters unequal to '\0'
 *
 * Parameter  : name	= Pointer to the sample character string
 *		garbage = A garbage collection is only performed
 *			  if this flag is set TRUE
 * Return     : TRUE	: It is a valid name
 *		FALSE	: It is not a valid name
 *
 *************************************************************************/

word
valid_entry_name ( char *name , word garbage )
{
 word i, ok, finished, ng;
 char buf[NameMax];


DEBdebug ("	valid_entry_name :	Test '%32s' for valid entry name",
	 name );

					/* Get a copy of the name buffer*/
 memcpy ((void *) buf, (void *) name, NameMax);

 finished  = FALSE;
 ok = TRUE;
 if ( *name == '\0' )			/* A zero string is not a valid */
 {					/* entry name			*/
	ok = FALSE;
	finished  = TRUE;
 }
					/* Scan the whole buffer	*/
 for ( i = 0 , ng = 0 ; i < NameMax ; i++ , name++ )
 {					/* We make our inspection, until*/
	if ( ! finished )		/* we find an error or reach the*/
	{				/* end of the string		*/
					/* Termination character ?	*/
		if ( *name == '\0' )
		{
			finished = TRUE;
			continue;
		}
					/* Illegal characters in an	*/
					/* entry-name			*/
		if ( *name == '/' || *name == ' ' || *name < 0x20 || *name > 0x80 )
			ok = FALSE;
#if 0
					/* Classes			*/
		if ( ! isalnum (*name) && ! isdigit (*name) && ! ispunct (*name) )
			ok = FALSE;
#endif
		continue;
	}
					/* Now check the rest of the	*/
					/* buffer for garbage		*/
	if ( garbage && *name != '\0' )
	{
		ng++;
		*name = '\0';
	}
 }
 
 if ( garbage && ng )
 {
	print_cur ();
	Report ("   : Found %d garbage characters in the name buffer", ng );
#if 0
	Report ("     [");
	for ( i = 0 ; i < NameMax ; i++ )
	{
		if ( buf[i] == '\0' )
			Report ("_");
		elif ( isalnum (buf[i]) || isdigit (buf[i]) || ispunct (buf[i]) )
			Report ("%c", buf[i]);
		else
			Report (".");
	}
	Report ("]\n");
#endif
	changes_de++;
 }

 if ( finished && ok )
	return TRUE;
 else
	return FALSE;
}


/*************************************************************************
 * TEST A CHARACTER STRING TO DETERMINE WHETHER IT CONTAINS A VALID 
 * PATHNAME OR NOT
 *
 * - The pathname is only syntactically checked. There are no efforts made
 *   to follow the path and prove on this way it's validity!
 *
 * Parameter  : name  = Pointer to the sample character string
 * Return     : TRUE  : It is a valid name
 *		FALSE : It is not a valid name
 *
 *************************************************************************/
word
valid_pathname ( char *path )
{
 int pos = 0;


DEBdebug ("	valid_pathname :	Test '%40s' for valid pathname",
	 path );


 if ( *path == '\0' )
	return FALSE;

 while ( *path != '\0' )
 {
	if ( *path < 0x20 || *path > 0x80 || *path == ' ' )
		return FALSE;
#if 0
	if ( ! isalnum (*path)	&& ! isdigit (*path)  && ! ispunct (*path) )
		return FALSE;
#endif
	if ( pos > 511 )
		return FALSE;
	pos++;
	path++;
 }

 return TRUE;
}

/*---------  Some procedures with 'heuristic characteristics'  ---------*/

/*************************************************************************
 * STATISTICAL OPERATIONS ON A DATA-BLOCK TO FIND OUT WHETHER IT COULD BE
 * AN INDIRECT BLOCK
 *
 * - We measure in the following way: The percentage of wrong block numbers
 *   has to be below the limit given by "possindb". All zeroed entries
 *   are skipped.
 * - All invalid block-numbers found are set to zero
 * - The procedures handle_single_indirect() and handle_double_indirect()
 *   which make use of this routine have to write back modified blocks.
 *   The variable "changes" is used to keep track on any modifications made.
 * - Note that "changes" reflects only the number of invalid block references
 *   which are found (..and NOT corrected), if we have set "make_changes"
 *   to FALSE.
 *
 * Parameter  : bp	     = Pointer to block structure
 *		make_changes = If TRUE, all invalid block-numbers will
 *			       be set to zero. This guarantees that the
 *			       block is usable after this operation.
 *		changes      = Pointer to modification-counter which keeps
 *			       the number of slots which are zeroed.
 * Return     : TRUE  : It is possible
 *		FALSE : It is not possible
 *
 *************************************************************************/
word
possible_indirect_block ( struct buf *bp, word make_changes, word *changes )
{
 word i, error_cnt, valid, zero;


DEBdebug ("	possible_indirect_block :	Scan block content.");

					/* Scan all entries		*/
 for ( i = 0 , valid = 0 , error_cnt = 0 , zero = 0;
       i < i_fs.fs_maxcontig ; i++ )
 {
	if ( bp->b_un.b_daddr[i] == 0 )	/* Zero is handled separately	*/
	{
		zero++;
		continue;
	}
					/* But all invalid block-numbers*/
					/* have to be reported.		*/
	if ( ! valid_bnr ( bp->b_un.b_daddr[i] ) )
	{				/* We have to correct invalid	*/
		if ( make_changes )	/* block numbers ...		*/
		{			/* ... set it to zero		*/
			bp->b_un.b_daddr[i] = 0;
					/* Increment the number of modi-*/
			(*changes)++;	/* fications made		*/
		}
		error_cnt++;
	}
	else				/* A valid reference in a single*/
					/* indirect block has to be de- */
		valid++;		/* noted.			*/
 }


DEBdebug ("	possible_indirect_block :	Errors = %d of %d entries.",
	 error_cnt, i_fs.fs_bsize / sizeof (daddr_t) );

					/* Test the limit - at least one*/
					/* valid bnr has to be found.	*/
 if ( error_cnt*100 <=
      (i_fs.fs_bsize / sizeof (daddr_t) - zero ) * (100 - possindb) &&
      valid )

	return TRUE;			/* Possible a valid block	*/
 else
	return FALSE;			/* It doesn't seem to be a	*/
					/* valid block.			*/
}

/*************************************************************************
 * STATISTICAL OPERATIONS ON A DATA-BLOCK TO FIND OUT WHETHER IT COULD BE
 * A DIRECTORY BLOCK
 *
 * - Zeroed entries are treated separately.
 * - Note that totally cleared blocks (error_cnt=0, valid=0) are treated
 *   as valid blocks! This is not avoidable, because the file-server does
 *   not free directory-blocks which contain NO ENTRIES.
 *
 * Parameter  : bp = Pointer to block structure
 * Return     :  0	: The block does not seem to be a directory-block
 *		-1	: It could be a directory-block, but we have not
 *			  found any valid entry.
 *		>1	: The number of valid entries found.
 *
 *************************************************************************/
word
possible_dir_block ( struct buf *bp )
{
 word i, error_cnt, valid, zero;
 struct dir_elem *dp;
 struct dir_elem sample_dir;

 dp = bp->b_un.b_dir;			/* To ease handling		*/
					/* Clear the comparison struct	*/
 memset ( &sample_dir, 0, sizeof (struct dir_elem) );


DEBdebug ("	possible_dir_block :	Scan block content.");

					/* Scan all entries		*/
 for ( i = 0 , valid = 0 , error_cnt = 0 , zero = 0;
       i < i_fs.fs_maxdpb ; i++ , dp++ )
 {
					/* A totally cleared entry is	*/
					/* treated as a valid entry	*/
/*
	if ( ! my_memcmp ( dp, &sample_dir, sizeof (struct dir_elem) ) )
	{
	     zero++;
	     continue;
	}
*/				/* Because of a PFS 1.1 bug, we actu-	*/
				/* ally only take the inode content for	*/
				/* comparison.				*/

					/* A zeroed inode is not further*/
					/* inspected.			*/
	if ( ! my_memcmp ( &dp->de_inode, &sample_dir.de_inode, sizeof (struct inode) ) )
	{
	     zero++;
	     continue;
	}
					/* The three conditions which	*/
					/* must be ALL fulfilled:	*/
					/* i) A valid type flag and ...	*/
	if ( ( dp->de_inode.i_mode == Type_Directory ||
	       dp->de_inode.i_mode == Type_File ||
	       dp->de_inode.i_mode == Type_Link
	     ) &&
					/* ii) plausible time-stamps .. */
	     ( ( dp->de_inode.i_ctime != 0 ||
		 dp->de_inode.i_atime != 0 ||
		 dp->de_inode.i_mtime != 0
	       ) &&
		 dp->de_inode.i_matrix & 0xc0c0c0c0
	     ) &&
					/* iii) and a valid entry-name. */
	       valid_entry_name ( dp->de_name , FALSE )
	   )
		valid++;		/* One more valid entry		*/
	else
		error_cnt++;
 }


DEBdebug (" possible_dir_block :	%d entries : errors= %d  valid= %d  free= %d.",
	 i_fs.fs_maxdpb, error_cnt, valid, zero);

					/* Test the limit		*/
 if ( (error_cnt * 100) <= ( i_fs.fs_maxdpb - zero ) * (100 - possdirb) )
 {
	if ( valid )
		return valid;		/* Possible a valid block	*/
	else
		return -1;		/* A zeroed dir-block		*/
 }
 else
	return FALSE;			/* It doesn't seem to be a	*/
					/* valid block.			*/
}

/*************************************************************************
 * OPERATION ON A DATA-BLOCK TO FIND OUT WHETHER IT COULD BE A LINKPATH-
 * BLOCK, CONNECTING A LINK WITH A "REAL" ELEMENT
 *
 * - It must be an absolute pathname (beginning with a '/')
 * - Note that we make no efforts to prove the validity of the pathname. It
 *   is only syntactically checked.
 *
 * Parameter  : bp = Pointer to block structure
 * Return     : TRUE  : It is possible
 *		FALSE : It is not possible
 *
 *************************************************************************/
word
possible_link_block ( struct buf *bp )
{

DEBdebug ("	possible_link_block :	Look at link-path '%.40s'",
       bp->b_un.b_link->name );


 if ( valid_pathname ( bp->b_un.b_link->name ) &&
      bp->b_un.b_link->name[0] == '/' )
	return TRUE;
 else
	return FALSE;

}


/*************************************************************************
 * SAVE A NAME AND A PATH FOR LATER PRINTING
 *
 * - Copy both values into static variables.
 * - Reset the cur_printed flag.
 *
 * Parameter  : path = path to the current inode
 *		name = name of the current inode
 * Return     : - nothing -
 *
 *************************************************************************/
static void
set_cur ( char *path, char *name )
{
 cur_printed = FALSE;
 strncpy ( cur_path, path, 512 );
 strncpy ( cur_name, name, 32 );
}


/*************************************************************************
 * PRINT A PREVIOUSLY SAVED PATHNAME
 *
 * - If the cur_printed flag is not set, print the saved values
 *   and set the flag.
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
static void
print_cur ( void )
{
 if ( cur_printed ) 
 	return;
 Report ("%sActual entry = %s : [%s]", S_INFO, cur_path, cur_name);
 cur_printed = TRUE;
}


/*------------	Handling of directory-entry hash-queues  -----------------*/

/**************************************************************************
 * APPEND A NAME-STRING TO THE DIRECTORY-ENTRY HASH-TABLE
 *
 * - The hash index is calculated and the element is added to the
 *   specified queue.
 *
 * Parameter : name = Pointer to the directory entry-name of an inode
 * Return    : - nothing -
 *
 **************************************************************************/
void
append_de_hash ( char *name )
{
 word index;
 struct de_name *hpt;

 index = calc_de_hash ( name );		/* Calculate the hash-index	*/
					/* based on the first three	*/
					/* characters of the name	*/

DEBdebug ("	append_de_hash :	Append '%s' to hash-table on slot : %d",
	 name, index);

					/* Allocate space for the new	*/
					/* element.			*/
 hpt = (struct de_name *) Malloc ( sizeof (struct de_name) );
 if ( hpt == (struct de_name *) NULL )
 {

DEBdebug ("%sappend_de_hash() : Unable to allocate memory !", S_FATAL);

	longjmp (term_jmp, MEMERR);
 }

 strcpy ( hpt->name, name );		/* Save the entry name		*/

					/* Insert at the front of the	*/
					/* hash-queue			*/
 hpt->denxt = de_hash_tab[index].denxt;
 de_hash_tab[index].denxt = hpt;

}


/***************************************************************************
 * SEARCH FOR AN ENTRY-NAME IN THE HASH-QUEUES
 *
 * - The hash index is calculated and the specified queue is followed to
 *   get the desired element
 *
 * Parameter : name = Pointer to the element we are looking for
 * Return    : TRUE  , if the element was found
 *	       FALSE , if the element is not in the hash-table
 *
 **************************************************************************/
word
isin_de_hash ( char *name )
{
 word index;
 struct de_name *hpt;

					/* Calculate hash-index		*/
 index = calc_de_hash ( name );


DEBdebug ("	isin_de_hash :	Look for '%s' in hash-table on slot : %d",
	 name, index );

					/* "Following" the appropriate	*/
					/* hash-queue			*/
 hpt = de_hash_tab[index].denxt;	/* Init the start element	*/

					/* Go to the end of the queue	*/
 while ( hpt != (struct de_name *) NULL )
 {
	if ( ! strcmp ( hpt->name , name ) )
		return TRUE;
	hpt = hpt->denxt;		/* The next element		*/
 }

 return FALSE;
}


/****************************************************************************
 * "RESET" THE HASH-TABLE BY DEALLOCATING ALL PREVIOUSLY USED ENTRIES
 *
 * - The hash queues are scanned one after another and they are
 *   followed via the ->denxt - chain. All elements found are deallocated.
 *
 * Parameter : - nothing -
 * Return    : - nothing -
 *
 ****************************************************************************/
void
remove_de_hash ( void )
{
 word i;
 struct de_name *hpt, *hpt_nxt;


DEBdebug ("	remove_de_hash :	Clear entry name hash-table.");


 for ( i = 0 ; i < DEHASHSZ ; i++ )
 {
	hpt = de_hash_tab[i].denxt;	/* Head initialization		*/
					/* Following the hash-chain	*/
	while ( hpt != (struct de_name *) NULL )
	{
		hpt_nxt = hpt->denxt;
		Free ( hpt );		/* Release buffer memory	*/
		hpt = hpt_nxt;
	}
					/* Re-Init the head-pointers	*/
	de_hash_tab[i].denxt = (struct de_name *) NULL;
 }
}

/*------------------------------------------------------------------------*/

static void *my_bsearch(const void *key, const void *base,
              	        size_t nmemb, size_t size,
              	        int (*compar)(const void *, const void *))
{
/* Binary search for given key in a sorted array (starting at base).     */
/* Comparisons are performed using the given function, and the array has */
/* nmemb items in it, each of size bytes.                                */
    int midn, c;
    void *midp;
    for (;;)
    {   if (nmemb<=0) return NULL;      /* not found at all.             */
        else if (nmemb==1)
/* If there is only one item left in the array it is the only one that   */
/* needs to be looked at.                                                */
        {   if ((*compar)(key, base)==0) return base;
            else return NULL;
        }
        midn = nmemb>>1;                /* index of middle item          */
/* I have to cast bast to (char *) here so that the addition will be     */
/* performed using natural machine arithmetic.                           */
        midp = (char *)base + midn * size;
        c = (*compar)(key, midp);
        if (c==0) return midp;          /* item found (by accident)      */
        else if (c<0) nmemb = midn;     /* key is to left of midpoint    */
        else                            /* key is to the right           */
        {   base = (char *)midp + size; /* exclude the midpoint          */
            nmemb = nmemb - midn - 1;
        }
        continue;
    }
}

/*------------------------------------------------------------------------*/

/* end of dircheck.c */


