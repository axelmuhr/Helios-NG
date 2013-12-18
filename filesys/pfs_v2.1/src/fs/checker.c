 /* 15.08.90 - Basic cuts to build the "integrated" version
 * 16.08.90 - Removed INTEGRATED_VERSION, EXTENSIONS, IOS
 *	      Usage of standard printf()
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
   |  checker.c								     |
   |                                                                         |
   |    The file system checker.                                             |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    3 - O.Imbusch -  7 May     1991 -  Error handling centralized        |
   |    2 - H.J.Ermen - 24 January 1989 -  Stand-alone checker version       |
   |    1 - H.J.Ermen - 21 March   1989 -  Basic version                     |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG    0
#define GEPDEBUG 0
#define FLDEBUG  0

#include "error.h"

#include "check.h"
#include "fserr.h"

/*-----------------------  Global variables  ---------------------------*/

word cdev;				/* number of volume currently checked */

word no_corrections = FALSE;		/* If TRUE, no changes are made	*/
					/* on disk. The basic version	*/
					/* of the checker makes always	*/
					/* the required corrections!	*/
word op_mode;				/* The desired operation mode	*/
word bit_maps_allocated;		/* TRUE after succesfull alloc	*/
					/* with Malloc()		*/
word unique_err;			/* Counter for faults during    */
					/* check_unique()		*/
word corrupt_cnt;			/* Counter for faults, which	*/
					/* corrupt the file system	*/
word lost_found;			/* Is TRUE, if a valid "lost +	*/
					/* found" directory is in use	*/

word found_links;			/* Used to store the number of 	*/
					/* symbolic links found during	*/
					/* inode-inspection		*/
word found_files;			/* Number of files found during */
					/* inode inspection.		*/
word found_dirs;			/* Number of subdirectories	*/
					/* found during inode inspection*/
word found_blocks;
char actual_path[512];					
struct fs i_fs;				/* holds a validated incore copy*/
					/* of the superblock		*/
daddr_t info_blocks[LIMIT_NCG];		/* Array for storage of info-	*/
					/* block numbers (maximal size) */
byte *bit_maps[LIMIT_NCG];		/* Table with arrays for bit-	*/
					/* map controlling		*/
word cg_bitmap_usable[LIMIT_NCG];	/* Array of binary flags, which */
					/* signal, whether the bit-map	*/
					/* of a cylinder-group is usable*/
					/* or not.			*/
word n_dirs[LIMIT_NCG];			/* Number of directory-entries	*/
					/* kept in each cg		*/
					
struct de_hash de_hash_tab[DEHASHSZ];	/* Table of hash-vectors for 	*/
					/* duplicate entry names	*/
					/* Table for duplicate block -	*/
					/* number hash-vectors		*/	
struct dup_hash dup_hash_tab[DUPHASHSZ];
					/* Table of hash-vectors for	*/
					/* 'lost' directory-blocks	*/
struct lost_hash lost_hash_tab[LOSTHASHSZ];
					/* Table of hash-vectors for	*/
					/* blocks which are referred by */
					/* 'lost' directory-blocks	*/
struct ref_hash ref_hash_tab[REFHASHSZ];
					/* Table for symbolic link hash-*/
					/* vectors			*/
struct link_hash link_hash_tab[LINKHASHSZ];
					/* A table which contains over-	*/
struct fsck_stat fst;			/* all statistic informations	*/

word silent_mode;			/* TRUE: No filename output	*/
char in_buf[80];			/* Buffer for keyboard input	*/

word possdirb;				/* The variable checker limits */
word possindb;
word mingood;
word maxbmerr;

word maxbpg;				/* globals during check */
word maxncg;

jmp_buf term_jmp;

char *S_FATAL   = "FATAL ERROR   : ";
char *S_WARNING = "WARNING       : ";
char *S_INFO    = " : ";
char *S_INFOCLR = "   ";
char *S_SERIOUS = "SERIOUS ERROR : ";

/*-----------------------  Local procedures  ---------------------------*/

static void create_new_root_dir (struct dir_elem *dp);
static word alloc_ref_maps      (void);
static void free_ref_maps       (void);
static void dealloc_mem		(void);

/*************************************************************************
 * MAIN ENTRY POINT FOR THE FILE-SYSTEM CHECKER
 *
 * - The different passes of the checking process ...
 *
 * Parameter  : full	= TRUE : Full check incl. directory tree scan
 *                        FALSE: only check_unique() then full check if
 *			         any errors occurred
 * Return     : FALSE, if a fatal error-condition occurred
 *
 *************************************************************************/
word
fscheck ( word full, word delete_hanging_links, VD *vol)
{
 struct buf *bpar;			/* used for check_inodes()	*/
 struct dir_elem root_de;
 word	val;
 
 cdev = vol->volnum;	/* set number of volume to be checked 		*/
 maxbpg = vol->bpcg;
 maxncg = vol->cgs;
 Report (FSErr [ChkStart], vol->vol_name); 
			/* For fatal error handling. This longjmp -	*/
			/* location is used, if bread() to fixed block- */
			/* numbers (like 1,2,3) fails or memory alloca- */
			/* operations do not succeed.			*/ 
			
 val = setjmp ( term_jmp );
 if (val)
 {
   Error (FSErr [DueFatal], S_FATAL);
   switch (val)
   {
     case MEMERR:
       Error (FSErr [AllocMemFld], S_FATAL);
     break;

     case READ_ERR:
       Error (FSErr [HandleFld], S_FATAL);
     break;

     case BAD_INFO_BLK:
       Error (FSErr [RdIBFld], S_FATAL);
     break;

     case BAD_SUM_BLK:
       Error (FSErr [RdSBFld], S_FATAL);
     break;

     case WRITE_ERR:
       Error (FSErr [BWrBFld], S_FATAL);	 
     break;

     case DIR_BLK_ERR:
       Error (FSErr [HDBFld], S_FATAL);
     break;

     case LNK_BLK_ERR:
       Error (FSErr [HLBFld], S_FATAL);	 
     break;

     case IND_BLK_ERR:
       Error (FSErr [HIBFld], S_FATAL);	 
     break;

     case DIND_BLK_ERR:
       Error (FSErr [HDIBFld], S_FATAL);
     break;
	 
     default:
       Error (FSErr [Unknown], S_FATAL);
     break;
   }
   Report (FSErr [ReloadVol], S_FATAL);
   finish_checker ();
   return FALSE;
 }

 if ( ! init_checker () )		/* Basic initialization		*/
 	return FALSE;

/*---------------  The four steps of the checking process ---------------*/

 Report (FSErr [Step1], S_INFO );
 unique_err = 0;
 if ( ! check_unique () )		/* Basic tests on superblock	*/
 {					/* and other unique structures	*/
 	Error (FSErr [DurBasic], S_FATAL);
 	finish_checker ();
 	return FALSE;
 }	
 
 if ( full != FULL && !unique_err ) {
	 finish_checker ();		/* Finish checking, tidy up and	*/
 					/* unlock the server's ports	*/
	 return TRUE;			/* Terminate checker process	*/
 }
 				
 Report (FSErr [Step2], S_INFO);
 bpar = bread ( cdev, 1, 1, SAVEA );	/* Get sum-block with root-dir	*/
 if (!bpar) {				/* if read fails jmp to term_jmp */
	Error (FSErr [RdSumBFld], S_FATAL);
	finish_checker();
	return (FALSE);
 }
 					/* Start from the file-server's	*/
 					/* root position		*/
 memcpy ( &root_de, &bpar->b_un.b_sum->root_dir, sizeof (struct dir_elem) );
 brelse ( bpar->b_tbp, TAIL );		/* Release summary block	*/
 					
 					/* Work on the directory tree	*/
 if ( ! check_inodes ( &root_de , 1, 0 ) )
 {
 	Error (FSErr [DurSimple], S_FATAL);
	finish_checker ();
	return FALSE;
 }
 					/* Work on the bit-maps		*/
 Report (FSErr [Step3], S_INFO);
 if ( ! check_blocks () )
 {
 	Error (FSErr [DurBitMap], S_FATAL);
	finish_checker ();
	return FALSE;
 }
 actual_path[0] = '\0';
					/* Perform some final tidy-up	*/
					/* operations			*/
 Report (FSErr [Step4], S_INFO);
 if ( ! tidy_update (delete_hanging_links) )
 {
	Error (FSErr [DurTidyUp], S_FATAL);
 	finish_checker ();
 	return FALSE;
 }
 finish_checker ();			/* Finish checking, tidy up and	*/
 					/* unlock the server's ports	*/
 return TRUE;				/* Terminate checker process	*/
}

/*************************************************************************
 * BASIC INITIALIZATION OF LOCAL VARIABLES AND SIMILAR THINGS
 *
 * Parameter  : - nothing -
 * Return     : TRUE if initialization was successful, otherwise FALSE
 *
 *************************************************************************/
word
init_checker ( void )
{
 int i;

 bit_maps_allocated = FALSE;		/* First time initializations:	*/
 found_links	    = 0;
 found_files	    = 0;
 found_dirs	    = 0;
 found_blocks	    = 0;
 actual_path[0]	    = '\0';
					/* Clear the overall statistics	*/
					/* data structure.		*/  
 memset ( (void *) &fst, 0, sizeof (struct fsck_stat) );
  				
					/* Initialize the tables, which */
 for ( i = 0 ; i < LIMIT_NCG ; i++ )	/* are sized to the maximal # of*/
 {					/* cylinder-groups possible.	*/
 	info_blocks[i] = 0;
 	bit_maps[i] = (byte *) NULL;
 	cg_bitmap_usable[i] = 0;
 	n_dirs[i] = 0;
 }

 return TRUE;				/* Initialisation successfull.	*/
}

/*************************************************************************
 * FINISH THE WORK OF THE CHECKER
 *
 * - Give all temporarily allocated memory back.
 * - Perform all needed tidy-up operations
 * 
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
void
finish_checker ( void )
{					/* Deallocate memory, used for	*/
					/* bit map reference array	*/
 if ( bit_maps_allocated )
 {
DEBdebug ("	finish_checker :	De-allocate reference bit-map areas.");
	free_ref_maps ();		/* Give the memory back !	*/
 }



DEBdebug ("	finish_checker :	Free hash table structures");

					/* Give memory used by the hash-*/
 remove_de_hash ();			/* tables back			*/
 remove_dup_hash ();
 remove_lost_hash ();
 remove_link_hash ();
 
/*  dealloc_mem (); 	*/		/* buffer cache stays alive in 	*/
					/* multivolume HFS		*/
 Report (FSErr [ChkEnd], volume[cdev].vol_name);
} 

/************************************************************************
 * BASIC TEST ROUTINES TO CHECK UNIQUE DATA STRUCTURES LIKE SUPERBLOCK AND
 * ROOT DIRECTORY AND GET MEMORY FOR THE CHECKER
 *
 * - Make sure, that the superblock and all copies are valid.
 *
 *   THIS TEST HAS BEEN REMOVED FROM THE BASIC VERSION, BECAUSE THE SUPERBLOCK
 *   DATA IS ACTUALLY NOT USED BY THE FILE-SERVER. ALL DATA, DESCRIBING A FILE
 *   SYSTEM IS DERIVED FROM 'devinfo'. ON THE OTHER SIDE, IT IS A NON-TRIVIAL
 *   TASK TO FIND OTHER, EVENTUALLY PARTLY DAMAGED SUPERBLOCKS ON THE MEDIA TO
 *   BE CHECKED. TO HAVE A BASIC IDEA, WHETHER THE RELEVANT DATA (number of
 *   cyl.groups, cyl.group size and cyl.group offset) KEPT IN THE 'devinfo'-
 *   FILE AND ON DISK DESCRIBE THE SAME PHYSICAL DISK, THEY ARE COMPARED FOR
 *   ALL CYLINDER-GROUPS.
 *  
 * - Compare the # number of free blocks in the bit-maps with the
 *   values, referenced in the cylinder group info structs.
 * - Allocate all needed memory for the checker.
 * - Check the root-directory inode and look for the /lost+found - directory
 *
 * Parameter  : - nothing -
 * Return     : TRUE  = no error at all
 *		FALSE = occurrence of a fatal error
 *
 *************************************************************************/
word
check_unique ( void )
{
 struct fs tmp;			/* For intermediate operation		*/
 struct buf *bp, *bp_2;
 word cgnr, i, free_cnt, ecnt, try, off;
 VD *vol = &volume[cdev];

 /*--------- Make a basic comparison of file-system parameters ---------*/

 Report (FSErr [Step11], S_INFO);
 
					/* cgnr and ncg are the values	*/
					/* taken from devinfo. If no	*/
					/* valid superblock is found 	*/
					/* there or the disk parameters	*/
					/* are different, the checking	*/
					/* process is aborted.		*/
					
					/* Make incore copies of the 	*/
					/* superblock data into i_fs and*/
					/* incore_fs.			*/
 /*------------------ Tests on the super-block -------------------------*/

				/* We have to copy the first 		*/
				/* superblock from the cg 0 info-block. */
 bp = bread ( cdev, 2, 1, SAVEA );
 if (!bp) {
	Error (FSErr [RB2Fld], S_FATAL);
	return (FALSE); 	
 }

 memcpy ( &tmp, &bp->b_un.b_info->fs, sizeof (struct fs) );
 brelse ( bp->b_tbp, TAIL );

 				/* Run until we have a valid copy and	*/
 				/* scan the cylinder groups		*/
 for ( cgnr = 0 , try = -1 ;; cgnr++ ) 
 {	
	corrupt_cnt = 0;	/* At the beginning: no errors at all	*/
	try++;			/* Count the tries which are needed to	*/
				/* get a valid super-block.		*/

DEBdebug ("	check_unique :	Try to validate a sample superblock.");

				/*------- I.Definitely errors ----------*/
				/* Block-no 1-3 are always the same !	*/
	if ( tmp.fs_sblknr != 1 || tmp.fs_iblknr != 2 || tmp.fs_rblknr != 3 )
		corrupt_cnt++;
				/* The magic number is fixed.		*/
	if ( tmp.fs_magic != MAGIC_NUMBER )
		corrupt_cnt++;

	if ( tmp.fs_szfs != sizeof (struct fs) )
		corrupt_cnt++;
			
	if ( tmp.fs_szcg != sizeof (struct cg) )
		corrupt_cnt++;
				/*------ II.Plausibility errors	--------*/
	if ( tmp.fs_size != tmp.fs_cgsize * tmp.fs_ncg )
		corrupt_cnt++;
	if ( tmp.fs_dsize != tmp.fs_size - tmp.fs_ncg - 3 )
		corrupt_cnt++;
	if ( tmp.fs_fsize != tmp.fs_bsize / tmp.fs_frag )
		corrupt_cnt++;
	if ( tmp.fs_maxdpb != tmp.fs_bsize / sizeof (struct dir_elem) )
		corrupt_cnt++;
	if ( tmp.fs_maxcontig != tmp.fs_bsize / sizeof (daddr_t) )
		corrupt_cnt++;
	if ( tmp.fs_ncgcgoff != tmp.fs_ncg * tmp.fs_cgoffset )
		corrupt_cnt++;
	if ( tmp.fs_minfree < 0 || tmp.fs_minfree > tmp.fs_cgsize * tmp.fs_ncg - 10)
		corrupt_cnt++;		
	if ( tmp.fs_psmal > tmp.fs_maxpsz || tmp.fs_pmedi > tmp.fs_maxpsz ||
	     tmp.fs_phuge > tmp.fs_maxpsz )
		corrupt_cnt++;

					/* Were there errors detected ?	 */
	if ( corrupt_cnt )
	{
		unique_err++;
		if ( cgnr == 0 )
			cgnr++;
			
		Error (FSErr [SuperDmgd], corrupt_cnt);
					/* Is it possible to select 	 */
					/* automatically a new one ?	 */
		if ( tmp.fs_size     == tmp.fs_cgsize * tmp.fs_ncg &&
		     tmp.fs_ncgcgoff == tmp.fs_cgoffset * tmp.fs_ncg &&
		     tmp.fs_size     != 0 &&
		     tmp.fs_ncgcgoff != 0 )
		{
			Report (FSErr [BlkAsIBlk], map_cgtoib (cgnr,tmp.fs_cgoffset,tmp.fs_cgsize));
	 			    
					/* Read in info-blk from next cg */
			bp = bread ( cdev, map_cgtoib (cgnr,tmp.fs_cgoffset,tmp.fs_cgsize),
				     1, SAVEA );
			if (!bp) {
				Error (FSErr [RdBINrFld], S_FATAL,cgnr,map_cgtoib (cgnr,tmp.fs_cgoffset,tmp.fs_cgsize));
				return (FALSE);
			}	     
			/* There are possibly two reasons why we cannot */
			/* read the block. If we have used a block-num  */
			/* which is invalid, we need help from the user */
			/* to find a correct number for an info-block	*/

				     	/* Copy in temporary struct	*/
			memcpy ( &tmp, &bp->b_un.b_info->fs,
				 sizeof (struct fs) );
				 	/* Release unused packet	*/
			brelse ( bp->b_tbp, TAIL );
					/* Try to validate the new copy	*/
					/* of the Superblock.		*/
			continue;
		}
		else			/* No automatically selection:	*/
					/* Give the user the choice to	*/
		{			/* select manually a new one	*/
			Error (FSErr [NewIBNotSel]);
					/* Assisted search or manual	*/
					/* 'rebuild' of a superblock!	*/
				return FALSE;
		}
	}
	else				/* No errors were detected	*/
	{
		if ( try > 0 )
			Report (FSErr [SBNowValid]);
					/* Keep the valid data incore 	*/
					/* in a separated storage area.	*/
		memcpy ( &vol->incore_fs, &tmp, sizeof (struct fs) );
 					/* Make a copy in i_fs, which is*/
					/* for exclusive checker's use	*/
		memcpy ( &i_fs, &tmp, sizeof (struct fs) );
		break;		 	/* We can finish the search for */
					/* a valid superblock !		*/
	} /* end of <if (corrupt_cnt)> */

 } /* end of <for(cgnr)> */

			/* At this point we are sure, that the incore-	*/
			/* copy of the super-block is valid. So we can	*/
			/* use it further on, if we need file system	*/
			/* parameters!					*/
		
 { 	
 					/* Update each cylinder-group	*/
					/* if necessary.		*/
	for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 	{				/* Read appropriate info block	*/
		bp = bread ( cdev, map_cgtoib (cgnr,i_fs.fs_cgoffset,i_fs.fs_cgsize),
			     1, SAVEA );
		if (!bp) {
			Error (FSErr [RdBINrFld], S_FATAL,cgnr,map_cgtoib (cgnr,i_fs.fs_cgoffset,i_fs.fs_cgsize));
			return (FALSE);
		}	
 
DEBdebug ("	check_unique :	Compare it with the copy in cylinder-group %d.", cgnr);
 
					/* Updating is only recommended, */
					/* if changes were made.	 */
		if ( my_memcmp ( &i_fs, &bp->b_un.b_info->fs, tmp.fs_szfs ) )
		{
 
DEBdebug ("	check_unique :	Copy superblock to cylinder-group %d.", cgnr);
 
			unique_err++;
					/* Update it's superblock copy	 */
			memcpy ( &bp->b_un.b_info->fs, &i_fs, sizeof (struct fs) );
					/* ... and write it back to disk */
			fst.corrected_sb++;
			test_bwrite ( bp );
		}				
		else			/* They are the same: don't copy */
			brelse ( bp->b_tbp, TAIL );
	} /* end <for (cgnr)> */
 }
				/* Report differences between data kept  */
				/* in the superblock and in the 'devinfo'*/
				/* structures 				 */
 
DEBdebug ("	check_unique : Compare 'devinfo' data with the superblock");
 

 if (i_fs.fs_ncg != vol->cgs)
 	Report (FSErr [NCG], i_fs.fs_ncg, vol->cgs );
 if (i_fs.fs_cgsize != vol->bpcg)
 	Report (FSErr [CGSize], i_fs.fs_cgsize, vol->bpcg);
 if (i_fs.fs_cgoffset != vol->cgoffs)
 	Report (FSErr [ROffset], i_fs.fs_cgoffset, vol->cgoffs);
 if (i_fs.fs_psmal != fsi->SmallPkt)
 	Report (FSErr [SmallPkt], i_fs.fs_psmal, fsi->SmallPkt);
 if (i_fs.fs_pmedi != fsi->MediumPkt)
 	Report (FSErr [MediumPkt], i_fs.fs_pmedi, fsi->MediumPkt);
 if (i_fs.fs_phuge != fsi->HugePkt)
 	Report (FSErr [HugePkt], i_fs.fs_phuge, fsi->HugePkt);
 if (i_fs.fs_pscnt != fsi->SmallCount)
 	Report (FSErr [SmallCnt], i_fs.fs_pscnt, fsi->SmallCount);
 if (i_fs.fs_pmcnt != fsi->MediumCount)
 	Report (FSErr [MediumCnt], i_fs.fs_pmcnt, fsi->MediumCount);
 if ( i_fs.fs_phcnt != fsi->HugeCount )
 	Report (FSErr [HugeCnt], i_fs.fs_phcnt, fsi->HugeCount);
 if ( i_fs.fs_maxnii != fsi->MaxInodes )
 	Report (FSErr [IncInode], i_fs.fs_maxnii, fsi->MaxInodes);
 if ( i_fs.fs_minfree != vol->minfree )
 	Report (FSErr [FreePerc], i_fs.fs_minfree, vol->minfree);

					
 /*---- To ease addressing of info-blocks: create an info-bnr table ----*/
 
 
DEBdebug ("	check_unique :	Create an info-block number table.");
 
 					/* Calculate all block numbers	*/
 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 					/* ... and fill the table	*/
 	info_blocks[cgnr] = map_cgtoib (cgnr, i_fs.fs_cgoffset, i_fs.fs_cgsize);

 
 /*--------- Discrepancy between bit-maps and cg-info ?	----------------*/

			/* This part is only used to detect differences	*/
			/* in the bit-maps and summary-structures and   */
			/* to report them. No attempts to correct them	*/
			/* are made. This is done later by check_blocks.*/ 
			
 Report (FSErr [Step12], S_INFO);

					/* Scan all cylinder groups	*/
 for ( corrupt_cnt = 0 , cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {					/* Read the cg info-block	*/
	bp = bread ( cdev, info_blocks[cgnr], 1, SAVEA );
					/* Read error			*/
	if ( !bp ) {
		Error (FSErr [RdBINrFld], S_FATAL, cgnr, info_blocks[cgnr]);
		return FALSE;
	}

	if ( bp->b_un.b_info->cgx.cg_cgx != cgnr ) {
	Error (FSErr [InvalidCGNr], S_WARNING, bp->b_un.b_info->cgx.cg_cgx ,cgnr);	
		unique_err++;
	}
	
					/* Scan through the bit-map !	*/
	for ( i = 0 , free_cnt = 0 , ecnt = 0 ; i < i_fs.fs_cgsize ; i++ )
	{
		switch ( bp->b_un.b_info->cgx.cg_free[i] )
		{			/* Test for errors		*/
			case 0x00 : free_cnt++;  
 				    break;
			case 0xff : continue;
				    break;
			default   : ecnt++;
				    fst.bitmap_errors++;
				    corrupt_cnt++;
		}
	}
	
	if ( free_cnt != bp->b_un.b_info->cgx.cg_s.s_nbfree )
	{				/* By comparing with cg-sum	*/
		corrupt_cnt++;
		unique_err++;
		fst.summary_errors++;
	Error (FSErr [NOfFreeDiff], S_WARNING);
	Error (FSErr [CGroup1], S_INFO, cgnr, free_cnt, bp->b_un.b_info->cgx.cg_s.s_nbfree);
	}

					/* Definitely errors in maps ?	*/
	if ( ecnt )			/* ( != 0 && != 0xff )		*/
	{
		unique_err++;
		Error (FSErr [CorruptBits], S_WARNING);
		Error (FSErr [CGroup2], S_INFO, cgnr, ecnt);
					/* We have reached the limit ?	*/
		if ( ecnt > i_fs.fs_cgsize * maxbmerr / 100 )
		{			/* Note cg-bitmap as unusable	*/
			cg_bitmap_usable[cgnr] = FALSE;
			Report (FSErr [NoBitMap], S_SERIOUS);
		}
		else
			cg_bitmap_usable[cgnr] = TRUE;
 			
	}				/* We have found a bitmap in an	*/
	else				/* usable state !		*/
		cg_bitmap_usable[cgnr] = TRUE;
		
					/* Release unused block		*/
	brelse ( bp->b_tbp, TAIL );
 } /* end < for (cgnr) > */
 
 /*--------- Allocate enough memory for a reference bit-map array ------*/

 Report (FSErr [Step13], S_INFO);

 bit_maps_allocated = alloc_ref_maps ();			

 if ( ! bit_maps_allocated )	/* No success again, that's bad!  */
 {
	Error (FSErr [AllocRBMFld], S_FATAL);
	return FALSE;
 }
 					/* Blocks 0-2 always allocated !  */
 bit_maps[0][0] = 1;			/* Boot-block			  */
 found_blocks++;
 bitmap_incr (1);			/* Summary block		  */
 found_blocks++;
					/* Mark all info-blocks as being  */
					/* allocated.			  */
 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {
 	found_blocks++;
	bitmap_incr (info_blocks[cgnr]); 
 }

 /*-----------  Prepare the hash-table for directory entries  ------------*/

 
DEBdebug ("	check_unique :	Prepare directory-entry hash table.");
 

					/* We have to initialize all	*/
 for ( i = 0 ; i < DEHASHSZ ; i++ )	/* hash-pointers !		*/
	de_hash_tab[i].denxt = (struct de_name *) NULL;
 
 /*------  Prepare the hash-table for duplicate block numbers  ---------*/

 
DEBdebug ("	check_unique :	Prepare duplicate block-number hash table.");
 

					/* We have to initialize all	*/
 for ( i = 0 ; i < DUPHASHSZ ; i++ )	/* hash-pointers !		*/
	dup_hash_tab[i].dupnxt = (struct dup_bnr *) NULL;

 /*--------  Prepare the hash-table for symbolic link references  ------*/ 
 
 
DEBdebug ("	check_unique :	Prepare symbolic-link hash-table.");
 
					/* We have to initialize all	*/
 for ( i = 0 ; i < LINKHASHSZ ; i++ )	/* hash-pointers		*/
 	link_hash_tab[i].lnnxt = (struct de_link *) NULL;
 
 /*--------------- Valid root directory data ? -------------------------*/

 Report (FSErr [Step14], S_INFO);

					/* Read summary block with the	*/
 bp = bread ( cdev, 1, 1, SAVEA );		/* root-dir inode in it.	*/
 if (!bp) {
 	Error (FSErr [RdSumBFld], S_FATAL);
 	return (FALSE);	
 }
 changes_de = 0;			/* Nothing done so far ...	*/
 					/* Check the root-dir entry 	*/
 if ( ! validate_entry ( &bp->b_un.b_sum->root_dir , "/", Type_Directory, TRUE ) )
 {					/* Is it totally damaged !	*/
 
 		/* It is a very dangerous situation, if we have no root-*/
 		/* directory available. The user can create an		*/
 		/* empty one and hope, that lost subdiretory-information*/
 		/* is picked up during the "lost+found" pass over the	*/
 		/* cylinder-group bit-maps.				*/

	unique_err++;
	Error (FSErr [RootTrash], S_SERIOUS);
					/* Pioneer's work: we create a	*/
					/* new inode from scratch !	*/
	Report (FSErr [NewRoot], S_INFO);
	
 					/* Pioneer's work: we create a	*/
 					/* new root-inode from scratch  */
 	create_new_root_dir ( &bp->b_un.b_sum->root_dir );
 					/* Write the root-inode back on */
 					/* disk.			*/
 	test_bwrite ( bp );
 }
 else					/* After finding a usable root- */
 {					/* directory entry ...		*/
	if (!bp->b_un.b_sum->sum_same)
		unique_err++;
 	if ( changes_de )		/* Any changes made ?		*/
 	{
 
DEBdebug ("	check_unique :	Update root-directory inode on disk.");
 
		unique_err++;
					/* Write corrected summary-block */
 		test_bwrite ( bp );	/* directly to disk		 */
 	}
 	else
		brelse ( bp->b_tbp, TAIL );
 }

 /*----------------  Look for the "lost+found" directory  --------------*/


 Report (FSErr [Step15], S_INFO);

 bp = bread ( cdev, 1, 1, SAVEA );	/* Get root-directory		*/
 if (!bp) {
 	Error (FSErr [RdSumBFld], S_FATAL);
 	return (FALSE);	
 }
 					/* Look for the "lost+found"-dir */
 bp_2 = search_entry ( &bp->b_un.b_sum->root_dir.de_inode, "lost+found", &off );
					/* We have found it ?		*/
 if ( bp_2 != (struct buf *) NULL )
 {					/* We have found a usable 	*/
	lost_found = TRUE;		/* /lost+found - directory.	*/
	brelse ( bp_2->b_tbp, TAIL );
	brelse ( bp->b_tbp, TAIL );
 }
 					/* If we have not found the 	*/
 else					/* /lost+found-inode, we have to */
 {					/* create a new one.		*/
 	Error (FSErr [LnFNotFound], S_WARNING);
	Report (FSErr [LFECreated], S_INFO);
	unique_err++;
 	if ( create_lostfound_inode ( &bp->b_un.b_sum->root_dir.de_inode ) )
	{
		Report (FSErr [LFCreated], S_INFO);
		lost_found = TRUE; 	
 		test_bwrite ( bp );	/* Write modified root back.	*/
 	}
 	else				/* We cannot create a /lost+found*/
 	{				/* inode, because we have no    */
 					/* slot available.		*/
		Error (FSErr [LnFNotCreat], S_WARNING);
		lost_found = FALSE;
		brelse ( bp->b_tbp, TAIL );
	}
 }

 return TRUE;
}

/*--  Procedures dealing with the creation of a new basic structures ---*/

/************************************************************************
 * CREATE A TOTALLY EMPTY ROOT-DIRECTORY ENTRY FROM SCRATCH
 *
 * - This procedure is called, if the checker was not able to find a 
 *   valid root-directory.
 *
 * Parameter   : dp    = Pointer to the root directory-element
 * Return      : - nothing -
 *
 ***********************************************************************/
static void
create_new_root_dir ( struct dir_elem *dp )
{
 Date date;
 
 
DEBdebug ("	create_new_root_dir :	Create the new entry");
 
					/* Clear the directory-entry	*/
 memset ( dp, 0, sizeof (struct dir_elem) );
 					/* .. and fill in with the data	*/
 					/* for the root-directory	*/
 strcpy ( dp->de_name, i_fs.fs_name );
 dp->de_inode.i_mode   = Type_Directory;
 dp->de_inode.i_matrix = DefDirMatrix;
 date = GetDate ();
 dp->de_inode.i_ctime  = date;
 dp->de_inode.i_mtime  = date;
 dp->de_inode.i_atime  = date;
}


/************************************************************************
 * CREATE A NEW /lost+found - INODE FOR KEEPING LOST BLOCKS 
 *
 * - This procedure is called, if the checker was not able to find a 
 *   valid /lost+found - directory.
 * - It is assumed, that ip points to the inode of the root-directory
 *
 * Parameter   : ip    = Pointer to the root directory-inode
 * Return      : TRUE  : We had success by creating a new /lost+found entry
 *		 FAKSE : The attempt to create failed.
 *
 ***********************************************************************/
word
create_lostfound_inode ( struct inode *ip )
{
 daddr_t bnr;
 word offset;
 struct buf *bip;
 Date date;
 
 
DEBdebug ("	create_lostfound_inode :   Allocate a free slot to keep /lost+found");
 
					/* Got a free slot with success?*/
 if ( get_free_slot ( ip, &bnr, &offset ) )
 {					/* Read the specified block.	*/
 	bip = bread ( cdev, bnr, 1, SAVEA );
	if (!bip) {
DEBdebug("%screate_lostfound_inode() : Failed to read directory block (bnr %d) !",S_INFO,bnr);
		longjmp( term_jmp, DIR_BLK_ERR);
	}
 					/* Prepare all necessary fields */
	strcpy ( bip->b_un.b_dir[offset].de_name, "lost+found" );
 	bip->b_un.b_dir[offset].de_inode.i_mode   = Type_Directory;
 	bip->b_un.b_dir[offset].de_inode.i_matrix = DefDirMatrix;
 	date = GetDate();
 	bip->b_un.b_dir[offset].de_inode.i_ctime  = date;
 	bip->b_un.b_dir[offset].de_inode.i_mtime  = date ;
 	bip->b_un.b_dir[offset].de_inode.i_atime  = date;
 					/* Write the modified directory-*/
 	test_bwrite ( bip );		/* block back to disk.		*/

					/* Modify the parent-inode	*/
	ip->i_spare++;			/* One more entry in the dir	*/
					/* to denote.			*/
	ip->i_size += sizeof (struct dir_elem);
 	return TRUE;
 }
 else					/* We return with the message	*/
 	return FALSE;			/* that we have failed to create*/
 					/* a new /lost+found - inode.	*/
}
 
/*-------- Routines dealing with memory management for bitmaps ---------*/

/*************************************************************************
 * ALLOCATE MEMORY FOR REFERENCE BIT-MAPS
 *
 * - All non-allocated references are set to NULL-pointer
 * - If allocation fails, all previoulsy allocated buffers are freed.
 *
 * Parameter  : - nothing -
 * Return     : A flag which signals success or failure of the allocation
 *
 *************************************************************************/
static word
alloc_ref_maps ( void )
{
 int cgnr;

 
DEBdebug ("	alloc_ref_maps :	Allocate reference bit-maps areas.");
 
					/* Default: set to NULL-pointer	*/
 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
	bit_maps[cgnr] = (byte *) NULL;

					/* For each cylinder group	*/
 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {
 
DEBdebug ("	alloc_ref_maps :	Try to allocate for cg = %d", cgnr);
 
					/* Allocate for one cyl.grp	*/
	bit_maps[cgnr] = (byte *) Malloc ( i_fs.fs_cgsize );
					/* Allocation succesfully ?	*/
	if ( bit_maps[cgnr] == (byte *) NULL )
	{				/* No !				*/
		free_ref_maps ();	/* Free all previously alloca-	*/
					/* ted memory.			*/
		return FALSE;
	}				/* Set all cg-reference maps to	*/
					/* zero !			*/
	memset ( bit_maps[cgnr], 0, i_fs.fs_cgsize );
 }

 return TRUE;
}


/*************************************************************************
 * FREE MEMORY, USED BY REFERENCE BIT-MAPS
 *
 * - Freeing all allocated memory, referred by pointers unequal to the
 *   NULL-pointer
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
static void
free_ref_maps ( void )
{
 int cgnr;

 
DEBdebug ("	free_ref_maps :		Free reference bit-maps areas.");
 

 for ( cgnr = 0 ; cgnr < i_fs.fs_ncg ; cgnr++ )
 {					/* Free only allocated cg-mem	*/
	if ( bit_maps[cgnr] != (byte *) NULL )
		Free ( bit_maps[cgnr] );
 }
}


/*************************************************************************
 * FREE MEMORY, USED BY THE BUFFER-CACHE AND INODE LISTS
 *
 * - this has to be done before new buffer-cache memory can be allocated
 *   by the server
 *
 * Parameter  : - nothing -
 * Return     : - nothing -
 *
 *************************************************************************/
static void
dealloc_mem ( void )
{

 
DEBdebug ("	dealloc_mem :		Free buffer-cache memory.");
 
 if (stbp != NULL)	Free (stbp);
 if (sbp  != NULL)	Free (sbp);
 if (scbp != NULL)	Free (scbp);
 if (mtbp != NULL)	Free (mtbp);
 if (mbp  != NULL)	Free (mbp);
 if (mcbp != NULL)	Free (mcbp);
 if (htbp != NULL)	Free (htbp);
 if (hbp  != NULL)	Free (hbp);
 if (hcbp != NULL)	Free (hcbp);
 if (free_packet  != NULL)	Free (free_packet);
 if (incore_ilist != NULL)	Free (incore_ilist);

}

/*-----------------------------------------------------------------------*/

/* end of checker.c */
