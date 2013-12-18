/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/deal_fs.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: deal_fs.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:05:48  guenter
 * first multivolume/multipartition HFS with tape
 * 
 * Revision 1.6  90/08/03  13:31:32  guenter
 * multivolume/multipartition
 * 
 * Revision 1.5  90/06/15  16:58:34  adruni
 * getting rid of -DDEVINFO.
 * devinfo parameter-check added.
 * 
 * Revision 1.4  90/06/11  19:54:00  adruni
 * Replacement of 'struct fs    f;' by
 * already existing 'struct fs    incore_fs;'.
 * 
 * Revision 1.3  90/05/30  15:32:25  chris
 * Convert printf's to IOdebug
 * Fix initialisation of incore_inode
 * 
 * Revision 1.2  90/02/01  17:36:18  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:04  chris
 * Initial revision
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
   |  deal_fs.c								     |
   |                                                                         |
   |	Miscellaneous routines of the FileServer.	 		     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    6 - O.Imbusch - 28 October   1991 - Incore inode hash table now      |
   |                                        created at runtime               |
   |    5 - O.Imbusch -  7 May       1991 - Error handling centralized       |
   |    4 - G.Lauven  -  3 August    1990 - Multipartition/multivolume       |
   |    3 - H.J.Ermen - 21 March     1989 - Full parametrization and dynamic |
   |                                        memory allocation                |
   |    2 - H.J.Ermen - 19 January   1989 - Init of superblock data          |
   |    1 - A.Ishan   - 17 September 1988 - Basic version                    |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define HDEBUG     0
#define IN_NUCLEUS 1

#include "error.h"

#include "nfs.h"
#include <root.h>

#include "misc.h"
#include "prime.h"

Semaphore		bhash_sem;
struct bufhash		bufhash[BUFHSZ];
struct free_packet	*free_packet;
Semaphore		ihash_sem;

/* OI 28 Oct 1991 */
#if 0
	struct i_hash		i_hash[IHSZ];
#else
	struct i_hash	       *i_hash;
#endif

struct free_ilist	free_ilist;

					/* Globals from buf.h		*/
word psmal, pmedi, phuge;
word maxpsz;
word pscnt, pmcnt, phcnt;
					/* Globals from inode.h		*/
word maxnii;
					/* Globals from fs.h		*/
VolDescriptor *volume;			/* Global storage area for quick */
					/* volume informations		 */
word block_to_addr_shifts;		/* conversion constant		*/

struct packet		*stbp, *mtbp, *htbp;
struct buf		*sbp, *mbp, *hbp;
struct block		*scbp, *mcbp, *hcbp;
struct incore_i		*incore_ilist;


/* ==================================================================== */

/* Local procedures */

static word init_Boot_Block(VD *vol);
static word init_Sum_Block(VD *vol);
static word init_Info_Block(word, VD *);
static word init_Root_Dir(VD *vol);
static word get_root_inode (VD *vol);
       void sync_root (VD *vol);
static void init_free_list (word, word, struct packet *, 
   			    struct buf *, struct block *);
static word init_incore_sum (VD *vol);
static word init_incore_fs (VD *vol);
static word get_parameter (struct info_blk *ibp, VD *vol);

/* ==================================================================== */

static word 
init_Boot_Block (VD *vol)

/* 
*  Since there isn't any BootBlock yet, simply clear the contents
*  of the first block on disc.
*  Returns FALSE if I/O-error occurres.
*/

{
     	struct buf *bp;

	bp = getblk(vol->volnum,0,1,NOSAVE);
	if (!bp)
		return FALSE;		
	clr_buf (bp);
	if (!bwrite(bp->b_tbp))
		return (FALSE);
	else
		return (TRUE);
}

/* ==================================================================== */

static word
init_Sum_Block (VD *vol)

/*
*  Initialise the SummaryBlock after formatting the hard disc.
*  Returns FALSE if any I/O-error occurred.
*/

{
	word i;
	struct sum_blk *sbp;
	Date date;
     	struct buf *bp;

	/* allocate a buffer and clear its contents */
	bp = getblk(vol->volnum,1,1,NOSAVE);
	if (!bp)
		return (FALSE);
	clr_buf (bp);
	sbp = bp->b_un.b_sum;

	/* initialise the DirEntry of the RootDirectory */
	sbp->root_dir.de_inode.i_mode = DIR; 
	sbp->root_dir.de_inode.i_size = sizeof(struct dir_elem); 
	sbp->root_dir.de_inode.i_db[0] = 3; 	
	sbp->root_dir.de_inode.i_blocks = 1; 		
	sbp->root_dir.de_inode.i_spare = 0; 			
	strcpy(sbp->root_dir.de_name,vol->vol_name);
	/* the root directory has an initial matrix giving full access */
	/* The system administrator should restrict this after setting */
	/* up any special links.				       */
	sbp->root_dir.de_inode.i_matrix = -1; 
	date = GetDate();
	sbp->root_dir.de_inode.i_ctime = date; 			
	sbp->root_dir.de_inode.i_atime = date; 			
	sbp->root_dir.de_inode.i_mtime = date; 		 			
	
	/* Initialise SummaryInformations */
	sbp->sum_same = TRUE;

	/* Initialise the first_boot flag */
	sbp->first_boot = TRUE;
	
	/* SummaryInfo of the first cylinder group */
	sbp->cg_sum[0].s_ndir = 1;
	sbp->cg_sum[0].s_nbfree = vol->bpcg - 4;
	sbp->fs_sum = sbp->cg_sum[0];

	/* SummaryInfo of the remaining cylinder groups */
	for (i=1; i<vol->cgs; i++) 
	{
		sbp->cg_sum[i].s_ndir = 0;
		sbp->cg_sum[i].s_nbfree = vol->bpcg - 1;
		sbp->fs_sum.s_ndir += sbp->cg_sum[i].s_ndir;
		sbp->fs_sum.s_nbfree += sbp->cg_sum[i].s_nbfree;		
	}
		
	/* write to hard disc */
	if (!bwrite(bp->b_tbp) )
		return (FALSE);
	else
		return (TRUE);
}

/* ==================================================================== */

static word 
init_Info_Block (cgnr, vol)

word cgnr;
VD *vol;

/*
*  Initialise InfoBlock of the given cylinder group.
*  Returns FALSE if any I/O-error occurres.
*/

{
	word i;
	struct cg  *cgp;
     	struct buf *bp;

	/* get the appropriate block and clear its contents */

DEBdebug ("init_IB :	bp = getblk(%d, %d, 1, NOSAVE);\n",vol->volnum,cgtoib(cgnr,vol));

	bp = getblk(vol->volnum,cgtoib(cgnr,vol),1,NOSAVE);
	if (!bp)
		return (FALSE);
	clr_buf (bp);
	cgp = &bp->b_un.b_info->cgx;
	
	/* calculate an "incremental" magic number of the specified cg */
	cgp->cg_magic = (MAGIC_CG_OFFSET << 16) | cgnr;
	
	/* Copy the initialized super-block data to the cg-info_blk */
	memcpy ((void *) &bp->b_un.b_info->fs, (void *) &vol->incore_fs, sizeof(struct fs));
	
	if ( cgnr==0 ) 
	{
	 	/* special treatment for the first cylinder group,
		  cause there are 3 more blocks reserved */
		cgp->cg_s.s_ndir = 1;
		cgp->cg_s.s_nbfree = vol->bpcg - 4;
		cgp->cg_rotor = 4;
	} 
	else 
	{
		cgp->cg_s.s_ndir = 0;
		cgp->cg_s.s_nbfree = vol->bpcg - 1;
		cgp->cg_rotor = 0;
	}
	cgp->cg_cgx = cgnr;
	cgp->cg_ndblk = vol->bpcg;

	/* set the right bytes in the BitMap */
	for (i=0; i<cgp->cg_rotor; i++)
		cgp->cg_free[i]	= 0xff;
	for (i=cgp->cg_rotor; i<vol->bpcg; i++)
		cgp->cg_free[i]	= 0x00; 	    	
	cgp->cg_free[btocgb(cgtoib(cgnr,vol),vol)] = 0xff;

	/* write InfoBlock out to disc */
	if (!bwrite(bp->b_tbp))
		return (FALSE);
	else
		return (TRUE);
}

/* ==================================================================== */

static word
init_Root_Dir (VD *vol)

/*
*  Allocate the first block of the RootDirectory,
*  although there aren't any DirEntries yet.
*  Returns FALSE if any I/O-error occurres.
*/

{
	struct buf *bp;

	bp = getblk(vol->volnum,3,1,NOSAVE);
	if (!bp)
		return (FALSE);
	clr_buf(bp);
	if (!bwrite(bp->b_tbp))
		return (FALSE);
	else
		return (TRUE);
}

/* ==================================================================== */

word
make_fs ( VD *vol )

/*
*  After having formatted the hard disc,
*  first of all the FileSystem-Layout should be copied out.
*  Returns FALSE if any error occurres
*/

{
	word i;

	/* Get the user's parameters and copy them to the global used	*/
	/* variables.							*/


GEPdebug (" make_fs() : init volume info of volume /%s",vol->vol_name);

	vol->cgs = vol->found_cgs;
	vol->bpcg = vol->found_bpcg;
	if (!vol->size_known) {
		if ( !init_volume_info(vol) ) {
Error (" make_fs() : Volume /%s failed to init size info !",vol->vol_name);
			return (FALSE);
		}
	}

	vol->incore_fs.fs_cgoffset	= vol->cgoffs;
	vol->incore_fs.fs_ncg		= vol->cgs;
	vol->incore_fs.fs_minfree	= vol->minfree;
	vol->incore_fs.fs_syncop     	= vol->syncwrite;
	vol->incore_fs.fs_maxnii	= maxnii;
	vol->incore_fs.fs_cgsize	= vol->bpcg;
	vol->incore_fs.fs_maxpsz	= maxpsz;
	vol->incore_fs.fs_psmal		= psmal;
	vol->incore_fs.fs_pmedi		= pmedi;
	vol->incore_fs.fs_phuge 	= phuge;
	vol->incore_fs.fs_pscnt		= pscnt;
	vol->incore_fs.fs_pmcnt		= pmcnt;
	vol->incore_fs.fs_phcnt		= phcnt;
	vol->incore_fs.fs_possindb	= fsi->PossIndir;
	vol->incore_fs.fs_possdirb	= fsi->PossDir;
	vol->incore_fs.fs_mingood	= fsi->MinGood;
	vol->incore_fs.fs_maxbmerr	= fsi->BitMapErrs;
	
	strcpy ( vol->incore_fs.fs_name, vol->vol_name );
					/* ... and complete it with	*/
	complete_super_block ( &vol->incore_fs );	/* fixed and redundant data.	*/



GEPdebug ("\n	make_fs : 	init_Boot_Block();\n");

	/* initialise the BootBlock */
	if (!init_Boot_Block(vol)) {
Error (" make_fs() : Volume /%s failed to write boot block !",vol->vol_name);
		return (FALSE);
	}

GEPdebug ("	make_fs : 	init_Sum_Block();\n");

	/* initialise the SummaryBlock */	
	if (!init_Sum_Block(vol) ) {
Error (" make_fs() : Volume /%s failed to write summary block !",vol->vol_name);
		return (FALSE);
	}

	for (i=0; i<vol->cgs; i++) 
	{

GEPdebug ("	make_fs : 	init_Info_Block(%d);\n",i);

		/* initialise the InfoBlock of each cylinder group */
		if (!init_Info_Block(i,vol)) {
Error (" make_fs() : Volume /%s failed to write info block of cyl.group %d !",vol->vol_name,i);
			return (FALSE);
		}
	}	

GEPdebug ("	make_fs : 	init_Root_Dir();\n");

	/* allocate the first block of RootDirectory */
	if (!init_Root_Dir(vol)) {
Error (" make_fs() : Volume /%s failed to write root directory !",vol->vol_name);
		return (FALSE);
	}
	
	return (TRUE);
	
}

/* ==================================================================== */

word
make_super (VD *vol) 

/*
 *  Creates a new superblock with devinfo data and writes it to disk 
 *  Return : FALSE if any error occurred, TRUE if not.
 */

{
	struct info_blk *ibp;


GEPdebug (" make_super() : init volume info of volume /%s",vol->vol_name);

	vol->cgs = vol->found_cgs;
	vol->bpcg = vol->found_bpcg;
	if (!vol->size_known) {
		if ( !init_volume_info(vol) ) {
Error (" make_super() : Volume /%s failed to init size info !",vol->vol_name);
			return (FALSE);
		}
	}
	
	vol->incore_fs.fs_cgoffset	= vol->cgoffs;
	vol->incore_fs.fs_ncg		= vol->cgs;
	vol->incore_fs.fs_minfree	= vol->minfree;
	vol->incore_fs.fs_syncop     	= vol->syncwrite;
	vol->incore_fs.fs_maxnii	= maxnii;
	vol->incore_fs.fs_cgsize	= vol->bpcg;
	vol->incore_fs.fs_maxpsz	= maxpsz;
	vol->incore_fs.fs_psmal		= psmal;
	vol->incore_fs.fs_pmedi		= pmedi;
	vol->incore_fs.fs_phuge 	= phuge;
	vol->incore_fs.fs_pscnt		= pscnt;
	vol->incore_fs.fs_pmcnt		= pmcnt;
	vol->incore_fs.fs_phcnt		= phcnt;
	vol->incore_fs.fs_possindb	= fsi->PossIndir;
	vol->incore_fs.fs_possdirb	= fsi->PossDir;
	vol->incore_fs.fs_mingood	= fsi->MinGood;
	vol->incore_fs.fs_maxbmerr	= fsi->BitMapErrs;
	
	strcpy ( vol->incore_fs.fs_name, vol->vol_name );
					/* ... and complete it with	*/
	complete_super_block ( &vol->incore_fs );	/* fixed and redundant data.	*/

	ibp = (struct info_blk *) Malloc (BSIZE);
	if ( read_blk ( (void *)ibp, 2, vol) != 1 ) {
		if ( read_blk ( (void *)ibp, 2, vol) != BSIZE ) {
			Error (" make_super() : Volume /%s, super block on disk physically destroyed!", vol->vol_name);
			Free (ibp);
			return FALSE;
		}
	}

	memset (ibp, 0, BSIZE);
		
	memcpy ((void *) &ibp->fs, (void *) &vol->incore_fs, sizeof (struct fs));
	
	if ( write_blk ( (void *)ibp, 2, vol) != 1 ) {
		Error (" make_super() : Volume /%s, failed to write super block !", vol->vol_name);
		Free (ibp);
		return FALSE;
	}
	
	Free (ibp);

	return TRUE;
}

/* ==================================================================== */

typedef struct command_type {
	word		function;
	word		block_number;
} command_type;


void
edit_block (MsgBuf *m, VD *vol) 
{
	MCB	msg;	
	word	Control_V[IOCMsgMax];
	byte	Data_V[4096];
	word	error,e,result;
	command_type	*command;
	Port	outport,inport;
	word	func;
	word	function,block_number;
	void	*data_buffer;
				
	outport = m->mcb.MsgHdr.Reply;
	inport = DebNewPort();
	func = FC_GSP+SS_HardDisk+FG_Private+FO_Edit;
	InitMCB ( &msg, MsgHdr_Flags_preserve, outport, inport, func );
	msg.Control = Control_V;
	msg.Data    = Data_V; 	   
	PutMsg ( &msg );	/* send I'm alive */
	
	forever {
		error = FALSE;
		InitMCB ( &msg, MsgHdr_Flags_preserve,inport, NullPort, 0);
		msg.Timeout = MaxInt;
		e = GetMsg (&msg);
		if (e < 0) {
			error = e;
			function = QUIT;
Error (" edit_block() : Volume /%s failed to get message (%x) !",vol->vol_name,e);
		}
		elif ( e != func) {
			error = EC_Error+EG_Unknown+EO_Message;	
			function = QUIT;
Error (" edit_block() : Volume /%s received unknown message (%x) !",vol->vol_name,e);
		}
		else {
			command = (command_type *)msg.Control;
			function = command->function;
			block_number = command->block_number;

GEPdebug (" edit_block() : function %d, blocknumber %d ",function,block_number);

			switch (function) {
			case READ : 

GEPdebug (" edit_block() : reading block");

				result = read_blk2 ((void *)Data_V,
							block_number,vol);
				if (result != 1)
					error = result;				
				break;
			
			case WRITE :

GEPdebug (" edit_block() : writing block");

				result = write_blk2 ((void *)Data_V,
							block_number,vol);
				if (result != 1)
					error = result;
				break;
				
			case QUIT :

GEPdebug (" edit_block() : quit");

				break;	

			default :
				error = EC_Error+EG_WrongFn+EO_Message;	
				break;	
			}
		}

		if (function == QUIT) {
			Free (m);
			DebFreePort (inport);
			return;	
		}
		InitMCB ( &msg, MsgHdr_Flags_preserve,outport, NullPort, func);
		msg.Control = Control_V;
		msg.Data    = Data_V; 	   
		MarshalWord (&msg,error);
		if ( function == READ )
			msg.MsgHdr.DataSize = 4096;

GEPdebug (" edit_block() : put result message");

		PutMsg (&msg);		
	}
}


/* ==================================================================== */

static void
init_free_list (psize, pcnt, itbp, ibp, jcbp)

word psize, pcnt;
struct packet *itbp;
struct buf *ibp;
struct block *jcbp;

/*
*  Initialise a free list in the Buffer Cache 
*  for the given packet-size and number of packets.
*/

{
	struct free_packet *php;
	word i,j;
	
	/* take the appropriate free-list header */
	php = &free_packet[psize-1];
	php->p_next = itbp;
	/* for the requested number of packets */
	for (i=0; i<pcnt; i++) 
	{
		/* according to the packet-size build a packet */
		for (j=0; j<psize; j++) 
		{			
			/* initialise buffer header */
			(ibp+j)->b_next = ibp+j;
			(ibp+j)->b_prev = ibp+j;
			(ibp+j)->b_bnr = 0;
			(ibp+j)->b_dev = 0;
			(ibp+j)->b_vld = FALSE;
			(ibp+j)->b_dwt = FALSE;
			(ibp+j)->b_pnr = j;
			/* pointer to the right buffer in the BufferCache */
			(ibp+j)->b_un.b_addr = (caddr_t) jcbp;
			/* pointer to the packet header */
			(ibp+j)->b_tbp = itbp;
			jcbp++;
		}
		/* insert the packet into free-list */
		itbp->p_next = itbp+1;
		itbp->p_prev = itbp-1;
		/* initialise packet header */
		itbp->p_size = psize;
		itbp->p_blk_cnt = 0;
		itbp->p_vld_cnt = 0;
		itbp->p_dwt_cnt = 0;
		itbp->p_fbp = ibp;	
		InitSemaphore(&itbp->p_sem,1);

		/* update the pointers and continue */
		ibp += psize;
		itbp++;
	}
	/* correct the free-list pointers of the first and last element */
	php->p_next->p_prev = (struct packet *)php;
	(itbp-1)->p_next = (struct packet *)php;
	php->p_prev = (itbp-1);

	/* initialise the free packet counter */
/* OI 910429 */	
	php->fp_count = pcnt;

	InitSemaphore(&php->fp_avail,0);
	InitSemaphore(&php->fp_sem,1);
}		
	
/* ==================================================================== */

void
init_buffer_cache (void)

/*
*  Initialise the BufferCache, creating the Buffer Hash Queues
*  and Free Packet Lists.
*/

{
	struct bufhash *ihp;
	word i;
	
	/* initialise the buffer hash queues */
	ihp = &bufhash[0];
	for (i=0; i<BUFHSZ; i++, ihp++) 
	{
		ihp->b_next = (struct buf *)ihp;
		ihp->b_prev = (struct buf *)ihp;
	}
	InitSemaphore(&bhash_sem,1);
	
	/* initialise the free-lists for the current three packet-sizes */
	init_free_list(psmal,pscnt,stbp,sbp,scbp);      	
	init_free_list(pmedi,pmcnt,mtbp,mbp,mcbp);
	init_free_list(phuge,phcnt,htbp,hbp,hcbp);
}      			
	
			
/* ==================================================================== */

void
init_incore_ilist (void)

/*
*  OI 28 Oct 1991: Create Incore Inode List,
*
*  Initialise the Incore Inode List, creating Inode Hash Queues
*  and Free Inode List.
*/

{
	struct i_hash  *hip;
	struct incore_i *ip;
	struct free_ilist *fip;
	word 	i;

/* OI 28 Oct 1991 */
	IHashSize = GreatestPrimeLE (2 * fsi->MaxInodes);
	FLdebug ("Creating hash table of size %d for %d inodes.", IHashSize, fsi->MaxInodes);	
	if ((i_hash = PolyNew (struct i_hash, IHashSize)) == NULL)
		FLdebug ("i_hash not created");

	/* initialise the inode hash queues */
	hip = i_hash;

	for (i=0; i<IHashSize; i++,hip++) 
	{
		hip->ii_next = (struct incore_i *) hip;
		hip->ii_prev = (struct incore_i *) hip;
	}
	InitSemaphore (&ihash_sem, 1);
	
	ip = incore_ilist;
	/* adjust the free-list header */
	fip = &free_ilist;
	fip->ii_next = ip;
	
	/* for decided number of elements in the Incore Inode List */
	for (i=0; i<maxnii; i++,ip++) 
	{
		/* insert free Incore Inode into the list */
		ip->ii_next = ip+1;
		ip->ii_prev = ip-1;
		/* initialise free Incore Inode */
		ip->ii_dev = 0;
		ip->ii_parbnr = 0;
		ip->ii_parofs = 0;
		ip->ii_dirbnr = 0;
		ip->ii_dirofs = 0;
		ip->ii_changed = FALSE;
		ip->ii_count = 0;
		memset(ip->ii_name, 0, 32);
		memset(&(ip->ii_i), 0, sizeof(struct inode));
		InitSemaphore(&ip->ii_sem,0);				
	}
	/* correct the free-list pointers of the first and last element */
	fip->ii_next->ii_prev = (struct incore_i *)fip;
	(ip-1)->ii_next = (struct incore_i *)fip;
	fip->ii_prev = (ip-1);
	/* initialise free-list header */
	InitSemaphore(&fip->fi_sem, 1);
}

/* ==================================================================== */

static word
get_root_inode (VD *vol)

/*
*  copy the inode of the RootDirectory into this list which will be kept 
*  incore for the runtime of this volume.
*  Returns FALSE if failed
*/

{
	struct incore_i *ip;
	struct buf *bp;
	


GEPdebug (" get_root_inode() : reading inode of root dir of volume %d to incore",vol->volnum);


	/* read the SummaryBlock including the DirEntry of the RootDir */
	bp = bread (vol->volnum,1,1,SAVEA);
	if ( bp == NULL )
		return (FALSE);
	/* remove a free Incore Inode from the free-list */
	ip = iremove_free();
	/* initialise the Incore Inode */
	ip->ii_dev = bp->b_dev;
	ip->ii_parbnr = bp->b_bnr;
	ip->ii_parofs = 0;
	ip->ii_dirbnr = bp->b_bnr;
	ip->ii_dirofs = 0;
	ip->ii_changed = FALSE;
	InitSemaphore(&ip->ii_sem, 1);
	/* copy inode of RootDirectory incore */
	strcpy(ip->ii_name,bp->b_un.b_dir->de_name);
	ip->ii_i = bp->b_un.b_sum->root_dir.de_inode;
	/* increment the reference count to keep
	   inode of the RootDir incore all the time */
	ip->ii_count = 1;
	/* insert Incore Inode of the RootDir into Hash Queue */	
	iinsert_hash (ip);
	/* release the buffer containing the SummaryBlock */
	brelse (bp->b_tbp, TAIL);
	return (TRUE);
}

/* ==================================================================== */

void
free_root_inode (VD *vol)

/*
*   removes inode of RootDirectory of this volume from inode list  
*/

{
	struct buf bp;
	struct incore_i *ip;
	

GEPdebug (" free_root_inode() : removing inode of root dir of volume /%s",vol->vol_name);

	sync_root (vol);

	bp.b_bnr = 1;
	bp.b_dev = vol->volnum;
	ip = iget (&bp,0,1,0);		/* get root inode pointer */

	Wait (&ihash_sem);

#if GEPDEBUG
	if ( ip->ii_count != 1 ) {	/* should be 1 here */
GEPdebug (" free_root_inode() : root inode counter <> 1 (%d)!",ip->ii_count);
	}
#endif	
	/* remove it from the inode-hash queue */
	iremove_hash (ip);

GEPdebug (" free_root_inode() : inode removed from hash queue");

			
	/* clear contents of incore-inode */
	ip->ii_count = 0;
	ip->ii_dev = 0;
	ip->ii_parbnr = 0;
	ip->ii_parofs = 0;
	ip->ii_dirbnr = 0;
	ip->ii_dirofs = 0;
	ip->ii_changed = FALSE;
	memset(ip->ii_name, 0, 32);
	memset(&(ip->ii_i), 0, sizeof(struct inode));
	InitSemaphore (&ip->ii_sem,0);
			
	/* insert the cleared incore-inode into free-list */	
	iinsert_free(ip);

GEPdebug (" free_root_inode() : inode inserted in freelist");

	
	Signal (&ihash_sem);
}


/* ==================================================================== */

void
clean_inodelist (VD *vol)

/*
*   looks for inodes still in inodelist of this volume and removes them
*   from the list; normally no inode should be found here!
*/

{
	word	i;
	struct i_hash	*hip;
	struct incore_i *ip,*ip2; 
	struct buf	*bp;


GEPdebug (" clean_inodelist() : cleaning for volume /%s",vol->vol_name);

/*	Wait (&ihash_sem); */
	hip = i_hash;
/* OI 28 Oct 1991 */
#if 0	
	for (i = 0; i < IHSZ; i++, hip++) {	/* step through inode hash queues */
#else	
	for (i = 0; i < IHashSize; i++, hip++) {/* step through inode hash queues */
#endif	
		ip2 = hip->ii_next;
		while (	ip2 != (struct incore_i *)hip ) {
			ip = ip2;
			ip2 = ip2->ii_next;
			if ( ip->ii_dev == vol->volnum ) {
				if ( ip->ii_changed ) { 

					/* read the directory block of file */
		
					bp = bread (ip->ii_dev,ip->ii_dirbnr,1,SAVEB);
					if ( bp == NULL ) {
Error (" clean_inodelist () : Volume /%s failed to read directory block !",vol->vol_name);
						continue;
					}
					/* copy incore-inode out to disc */
					strncpy ((bp->b_un.b_dir + ip->ii_dirofs)->de_name,
						ip->ii_name,NameMax);
					(bp->b_un.b_dir + ip->ii_dirofs)->de_inode = ip->ii_i;
					/* write out the directory block */
					if (!bwrite (bp->b_tbp)) {
Error (" clean_inodelist () : Volume /%s failed to write directory block !",vol->vol_name);
					}
		
				}

				/* remove it from the inode-hash queue */
				iremove_hash (ip);
			
				/* clear contents of incore-inode */
				ip->ii_count = 0;
				ip->ii_dev = 0;
				ip->ii_parbnr = 0;
				ip->ii_parofs = 0;
				ip->ii_dirbnr = 0;
				ip->ii_dirofs = 0;
				ip->ii_changed = FALSE;
				memset(ip->ii_name, 0, 32);
				memset(&(ip->ii_i), 0, sizeof(struct inode));
				InitSemaphore (&ip->ii_sem,0);
			
				/* insert the cleared incore-inode into free-list */	
				iinsert_free(ip);
			}	
		}
	}
/*	Signal (&ihash_sem);	*/

GEPdebug (" clean_inodelist() : finished cleaning for volume /%s",vol->vol_name);

}


/* ==================================================================== */

void
clean_cache (VD *vol)

/*
*   syncs this volume and clears buffer cache from its blocks  
*/

{
	word i;
	struct packet	*tbp,*itbp;
 

GEPdebug (" clean_cache() : cleaning for volume /%s",vol->vol_name);

	for (i=0; i<maxpsz; i++) 
	{
		/* for the currently available packet-sizes */
		if ((i==(psmal-1))||(i==(pmedi-1))||(i==(phuge-1))) 
		{
			/* block free-lidt of this packet-size */
			Wait (&free_packet[i].fp_sem);
			/* follow down the free-list */
			tbp = (struct packet *) &free_packet[i];
			for (itbp=tbp->p_next; 
			     itbp!=tbp; itbp=itbp->p_next) {

				/* if packet belongs to vol->volnum
				   AND is not already blocked */
				if ( (itbp->p_fbp->b_dev == vol->volnum)
				     && (TestSemaphore(&itbp->p_sem)>0) ) {
					/* if packet marked delayed_write
					   AND sync_allowed */
					if (itbp->p_dwt_cnt) {
						/* if sync allowed */
						if (vol->sync_allowed) {
							Wait (&itbp->p_sem);

DEBdebug (" clean_cache() : 	write_dev (0x%p);\n",itbp);

							/* write contents of packet out to disc */
							if (write_dev (itbp) != Err_Null) {
Error (" clean_cache() : Volume /%s failed to write block !\n"
        "   		  Write request skipped !", vol->vol_name);
							}
							Signal (&itbp->p_sem);		
						}
						else {
Error (" clean_cache() : not allowed to write %d block(s) from blocknumber %d\n"
        "                 of volume /%s , data will be lost!", itbp->p_blk_cnt,itbp->p_fbp->b_bnr, vol->vol_name);
						}	 
					}
					/* remove packet from cache */
			     		tidyup_cache (itbp->p_fbp->b_dev,itbp->p_fbp->b_bnr);	
			     
				}

			}
			/* give free-list free */
			Signal (&free_packet[i].fp_sem);
		}
	}

}
	
/* ==================================================================== */

static word
init_incore_sum (VD *vol)

/*
*  Copy the SummaryInformations into incore.
*/

{
	word i;
	struct buf *bp;
	
	/* read the SummaryBlock including the SummaryInfos */
	bp = bread (vol->volnum,1,1,SAVEA);
	if ( bp == NULL )
		return (FALSE);

	/* copy the first_boot flag */
	vol->incore_sum.is_first_boot = bp->b_un.b_sum->first_boot;

	vol->incore_sum.is_fs.sum = bp->b_un.b_sum->fs_sum;
	InitSemaphore(&vol->incore_sum.is_fs.sem,1);
	for (i=0; i<vol->cgs; i++) 
	{
		vol->incore_sum.is_cg[i].sum = bp->b_un.b_sum->cg_sum[i];
		InitSemaphore(&vol->incore_sum.is_cg[i].sem,1);
	}
	
	/* provide the Incore Summary Informations */
	if ( bp->b_un.b_sum->sum_same )
	{
		vol->incore_sum.is_sum_same = TRUE;
	}
	else
	/* Special treatment, if the summary infos are not valid */
	{
Error (" init_fs() : Volume /%s, invalid summary block found. Call the checker !",vol->vol_name);
		vol->incore_sum.is_sum_same = FALSE;
	}	

	/* First time booted : first_boot is TRUE -> set it to FALSE ! */
	if ( vol->incore_sum.is_first_boot && !vol->writeprotected )
	{
		bp->b_un.b_sum->first_boot = FALSE;
		if (!bwrite ( bp->b_tbp ))
			return (FALSE);
						
	}
	else
		brelse (bp->b_tbp, TAIL);
		
	return (TRUE);
}
	
/* ==================================================================== */

static word
init_incore_fs (VD *vol)

/*
*  Copy the Superblock into incore.
*/

{
	struct buf *bp;

	bp = bread (vol->volnum,2,1,SAVEA);
	if (bp == NULL)
		return (FALSE);
	vol->incore_fs = bp->b_un.b_info->fs;
				
	brelse (bp->b_tbp, TAIL);
	return (TRUE);
}
/* ==================================================================== */

static word
get_parameter (struct info_blk *ibp, VD *vol)

/*
 *  Tries to read or build a valid info block 0 for this volume
 *  Return : FALSE if block is physically not readable or info
 *	     block 0 could not be built, TRUE if ok.
 */

{
	word		errors = 0;
	struct fs 	*f;
	
	if ( read_blk((char *)ibp,2,vol) == 0 ) {
Error (" get_parameter() : Volume /%s, info block 0 physically destroyed !",vol->vol_name);
		return (FALSE);
	}
	f = &ibp->fs;	
	if ( f->fs_cgsize < MIN_BPG || f->fs_cgsize > LIMIT_BPG ) {
Error (" get_parameter() : Volume /%s, cgsize out of legal range (%d) !",vol->vol_name,f->fs_cgsize);
		errors++;
	}
	if ( f->fs_ncg < MIN_NCG || f->fs_ncg > LIMIT_NCG ) {
Error (" get_parameter() : Volume /%s, ncg out of legal range (%d) !",vol->vol_name,f->fs_ncg);
		errors++;
	}
	if ( f->fs_cgoffset < 1 || f->fs_cgoffset > LIMIT_BPG ) {
Error (" get_parameter() : Volume /%s, cgoffset out of legal range (%d) !",vol->vol_name,f->fs_cgoffset);
		errors++;
	}
	if (errors)
		return (FALSE);
	else	
		return (TRUE);
}


/* ==================================================================== */

word
init_fs (word check_mode,word delete_hanging_links ,VD *vol)

/*
*  Builds the FileServer structures at system-startup.
*/

{
	struct info_blk *ibp;

	ibp = (struct info_blk *) Malloc ( BSIZE );

	/* get valid info block 0 */
	if ( !get_parameter (ibp,vol) ) {
Error (" init_fs() : Volume /%s failed to get valid superblock (info block 0).\n"
        "             Use mksuper command to create a valid superblock or \n"
        "             makefs command to install a new (empty) file system.", vol->vol_name);
		return (FALSE);	
	}

	vol->bpcg	= ibp->fs.fs_cgsize;
	vol->cgs	= ibp->fs.fs_ncg;
	vol->cgoffs	= ibp->fs.fs_cgoffset;
/*	strcpy ( vol->vol_name, ibp->fs.fs_name );	*/
	Free ( ibp );

	if (!vol->size_known) {	
		if ( !init_volume_info(vol) ) {
Error (" init_fs() : Volume /%s failed to init size info !",vol->vol_name);
			return (FALSE);
		}	
	}

#ifdef CHECKER
	if ( check_mode ) {
		if ( !fscheck(check_mode,delete_hanging_links,vol) ) {
Error (" init_fs() : Volume /%s, CHECKER failed !",vol->vol_name);
			return (FALSE);
		}
	}
#endif

	/* get Incore Inode of RootDir */
	if ( ! get_root_inode (vol)) {
Error (" init_fs() : Could not get root inode of volume /%s !",vol->vol_name);
		return (FALSE);	
	}
	

DEBdebug ("	init_fs : 	init_incore_sum(%d);\n",vol->volnum);

	/* import the Summary Informations */
	if ( !init_incore_sum(vol) ) {
Error (" init_fs() : Volume /%s failed to init summary info !",vol->vol_name);
		return (FALSE);	
	}

DEBdebug ("	init_fs : 	init_incore_fs(%d);\n",vol->volnum);

	/* import the SuperBlock */
	if ( !init_incore_fs(vol)) {
Error (" init_fs() : Could not read super block of volume /%s !",vol->vol_name);
		return (FALSE);	
	}
	
	vol->filesystem_found = TRUE;
	if (vol->writeprotected )
		vol->sync_allowed = FALSE;
	else
		vol->sync_allowed = TRUE;
	vol->vol_full = FALSE;

GEPdebug (" init_fs() : filesystem on volume /%s found",vol->vol_name);

	
	return (TRUE);
}

/* ==================================================================== */

/* 
 *
 * Completion of a sample superblock with fixed or redundant information.
 *
 */

void
complete_super_block ( struct fs *s )
{
	s->fs_sblknr    = 1;
	s->fs_iblknr    = 2;
	s->fs_rblknr    = 3;
	s->fs_size      = s->fs_ncg * s->fs_cgsize;
	s->fs_dsize     = s->fs_size - s->fs_ncg - 3;
	s->fs_bsize     = BSIZE;
	s->fs_maxdpb    = s->fs_bsize / sizeof (struct dir_elem);
	s->fs_frag      = 8;
	s->fs_fsize     = s->fs_bsize / s->fs_frag;
	s->fs_maxcontig = s->fs_bsize / sizeof (daddr_t);
	s->fs_ndaddr    = NDADDR;
	s->fs_niaddr    = NIADDR;
	s->fs_magic     = MAGIC_NUMBER;
	s->fs_szcg      = sizeof (struct cg);
	s->fs_szfs      = sizeof (struct fs);
	s->fs_ncgcgoff  = s->fs_ncg * s->fs_cgoffset;
	s->fs_time      = GetDate();
}

/* ==================================================================== */

void
sync_fs (void)

/*
*   loops over all volumes and if sync_allowed is TRUE it writes out all
*   delayed write marked blocks
*/

{
	word	act_vol;
	
	for (act_vol = 0; act_vol < volume[0].num_of_vols; act_vol++) {
		if ( !volume[act_vol].raw && volume[act_vol].sync_allowed ) {

GEPdebug (" sync_fs() : working on volume /%s",volume[act_vol].vol_name);

			sync_vol (&volume[act_vol]);	
		}	
	}	
}


/* ==================================================================== */


void
sync_vol (VD *vol)

/*
*  Tries to save the consistency of the FileSystem,
*  writing all delayed-write blocks of volume 'vol' out to disc,
*  and updating the SummaryBlock on disc.
*  This routine is called every 20 seconds.
*
*  20/03/89 : A "special purpose" semaphore sync_sem is used to guarantee
*	      exclusive access to the incore data-structures during 
*	      sync-operation. This is essential, because the file system
*	      checker has to rebuild incore-data structures under certain
*	      error conditions.
*/ 

{
	word i;
	struct packet	*tbp,*itbp;

	if ( vol->writeprotected )	/* cannot sync writeprotected media */
		return;			

GEPdebug (" sync_vol() : Get access for volume /%s.",vol->vol_name);

				
	Wait ( &sync_sem );		/* Guarantee exclusive access	*/
 

GEPdebug (" sync_vol() : started on volume /%s",vol->vol_name);

	for (i=0; i<maxpsz; i++) 
	{
		/* for the currently available packet-sizes */
		if ((i==(psmal-1))||(i==(pmedi-1))||(i==(phuge-1))) 
		{

DEBdebug ("	sync_vol :	| %d |",i+1);

			/* block free-lidt of this packet-size */
			Wait (&free_packet[i].fp_sem);
			/* follow down the free-list */
			tbp = (struct packet *) &free_packet[i];
			for (itbp=tbp->p_next; 
			     itbp!=tbp; itbp=itbp->p_next) 
			{ 
				/* if packet marked as delayed-write
				   AND  not already blocked
				   AND  belongs to vol->volnum 
				   AND  sync on volume allowed */
				if ((itbp->p_dwt_cnt) 
				   && (TestSemaphore(&itbp->p_sem)>0)
				   && (itbp->p_fbp->b_dev == vol->volnum)) {
					if (vol->sync_allowed) {
						Wait (&itbp->p_sem);

DEBdebug ("	sync_vol : 	write_dev (0x%x);",itbp);

						/* write contents of packet out to disc */
						if (write_dev (itbp) != Err_Null) {
Error (" sync_vol() : Volume /%s failed to write block !",vol->vol_name);
							if (vol->unload_msg_allowed) {
								Signal (&itbp->p_sem);
								Signal (&free_packet[i].fp_sem);
								Signal ( &sync_sem );
								vol->unload_msg_allowed = FALSE;
								PutMsg(&vol->unload_mcb);
								return;
							}
							else
Error ("              Write request skipped !");
						}
						Signal (&itbp->p_sem);		
					}
					else {
Error (" sync_vol() : not allowed to write %d block(s) from blocknumber %d\n"
        "              of volume /%s !", itbp->p_blk_cnt,itbp->p_fbp->b_bnr, vol->vol_name);
					}
				}

			}
			/* give free-list free */
			Signal (&free_packet[i].fp_sem);
		}
	}
	/* update rootdir */
	sync_root (vol);

	Signal ( &sync_sem );		/* Signal others, that they can	*/
}					/* access the incore structures.*/



/* ====================================================================== */

void
sync_root (VD *vol)

/*
*   updates the root directory and summary informations
*/

{
	word i;
	struct incore_i *ip;
	struct buf *bp;
	struct buf bbp;

	if (vol->writeprotected)	/* cannot sync writeprotected media */
		return;
	/* get the pointer to the Incore Inode of the RootDir */

GEPdebug (" sync_root() : working on volume /%s",vol->vol_name);

	bbp.b_bnr = 1;
	bbp.b_dev = vol->volnum;
 	ip = iget (&bbp, 0, 1, 0);
	if ( !ip ) {
Error (" sync_root() : root inode of volume /%s not found!",vol->vol_name);
		return;
	} 

GEPdebug (" sync_root() : Volume /%s, before Wait (&ip->ii_sem)",vol->vol_name);

	Wait (&ip->ii_sem);
	if (ip->ii_changed || (!vol->incore_sum.is_sum_same) )
	{

GEPdebug (" sync_root() :	bp = bread (vol->volnum, 1, 1, SAVEA);\n");

		/* read the SummaryBlock into BufferCache */
		bp = bread (vol->volnum,1,1,SAVEA);
		if ( bp == NULL ) {
			Signal (&ip->ii_sem);
			if ( vol->unload_msg_allowed ) {
Error (" sync_root() : Volume /%s failed to read summary block !",vol->vol_name);
				vol->unload_msg_allowed = FALSE;
				PutMsg(&vol->unload_mcb);
				return;
			}
			else {
Error (" sync_root() : Volume /%s failed to read summary block !\n"
        "		Sync_root() skipped !", vol->vol_name);
				Signal (&ip->ii_sem);
				return;
			}
		}
		/* write the inode of RootDir out */
		bp->b_un.b_sum->root_dir.de_inode = ip->ii_i;
		/* update the Summary Infos on disc */
		bp->b_un.b_sum->fs_sum = vol->incore_sum.is_fs.sum;
		for (i=0; i<vol->cgs; i++) 
			bp->b_un.b_sum->cg_sum[i] = vol->incore_sum.is_cg[i].sum;
		/* validate the Summary Infos on disc */
		bp->b_un.b_sum->sum_same = TRUE;

GEPdebug (" sync_root() :	bwrite (0x%p);\n",bp->b_tbp);

		/* write the SummaryBlock out to disc */
		if (!bwrite (bp->b_tbp)) {
			Signal (&ip->ii_sem);
			if ( vol->unload_msg_allowed ) {
Error (" sync_root() : Volume /%s failed to write summary block !",vol->vol_name);
				vol->unload_msg_allowed = FALSE;
				PutMsg(&vol->unload_mcb);
				return;
			}
			else {
Error (" sync_root() : Volume /%s failed to write summary block !\n"
        "		Sync_root() skipped !, vol->vol_name);");
				Signal (&ip->ii_sem);
				return;
			}
		}

		Wait (&vol->incore_sum.is_fs.sem);
		/* ... and the incore summary flag */
		vol->incore_sum.is_sum_same = TRUE;
		Signal (&vol->incore_sum.is_fs.sem);
 
		/* reset the modified-flag of the Incore RootDir Inode */
		ip->ii_changed = FALSE;
	}

GEPdebug (" sync_root() : informations updated");


	/* give the Incore Inode free */
	Signal (&ip->ii_sem);
	if (vol->unload_msg_allowed)
		iput (ip);

GEPdebug (" sync_root() : finished");

	
}

/* ==================================================================== */

word
memfree (void)

/*
 *   Return : free memory size in bytes
 */

{
	RootStruct	*root;
	Pool		*pool;
	Memory		*m;	
	word		freesize = 0;
	word		size;

	
	root = GetRoot();
	pool = root->FreePool;
	m = (Memory *)(pool + 1);
	until ( m == (Memory *)pool->Memory.Head) {
		size = m->Size & 0xfffffff0;
		if (!(m->Size & Memory_Size_FwdBit))
			freesize += size;
		m = (Memory *)((int)m + size);	
	}
	return (freesize);
}

/* ==================================================================== */

/*
 * Dynamic allocation of all needed buffers and tables
 *
 * If one of the allocations fails, FALSE is returned to signal the
 * calling procedure, that there is not enough free space on the desired
 * node.
 *
 */
 
word
alloc_mem ( void )
{
 int v;

DEBdebug ("	alloc_mem :	Allocate memory for the buffer-cache.\n");



DEBdebug ("maxpsz = %d",maxpsz);

 scbp = (struct block *) Malloc (v = pscnt * psmal * sizeof(struct block));
 if ( scbp == (struct block *) NULL )
 {	Error ("Unable to allocate %d bytes for scbp",v);
 	return FALSE;
 }
 mcbp = (struct block *) Malloc (v = pmcnt * pmedi * sizeof(struct block));
 if ( mcbp == (struct block *) NULL )
 {	Error ("Unable to allocate %d bytes for mcbp",v);
 	return FALSE;
 }
 hcbp = (struct block *) Malloc (v = phcnt * phuge * sizeof(struct block));
 if ( hcbp == (struct block *) NULL )
 {	Error ("Unable to allocate %d bytes for hcbp",v);
 	return FALSE;
 }
 free_packet = (struct free_packet *) Malloc (v = maxpsz * sizeof(struct free_packet));
 if ( free_packet == (struct free_packet *) NULL )
 {	Error ("Unable to allocate %d bytes for free_packet",v);
	return FALSE;
 }
 stbp = (struct packet *) Malloc (v = pscnt * sizeof(struct packet));
 if ( stbp == (struct packet *) NULL )
 {	Error ("Unable to allocate %d bytes for stbp",v);
	return FALSE;
 }
 mtbp = (struct packet *) Malloc (v = pmcnt * sizeof(struct packet));
 if ( mtbp == (struct packet *) NULL )
 {	Error ("Unable to allocate %d bytes for mtbp",v);
	return FALSE;
 }
 htbp = (struct packet *) Malloc (v = phcnt * sizeof(struct packet));
 if ( htbp == (struct packet *) NULL )
 {	Error ("Unable to allocate %d bytes for htbp",v);
	return FALSE;
 }
 sbp = (struct buf *) Malloc (v = pscnt * psmal * sizeof(struct buf));
 if ( sbp == (struct buf *) NULL )
 {	Error ("Unable to allocate %d bytes for sbp",v);
 	return FALSE;
 }
 mbp = (struct buf *) Malloc (v = pmcnt * pmedi * sizeof(struct buf));
 if ( mbp == (struct buf *) NULL )
 {	Error ("Unable to allocate %d bytes for mbp",v);
 	return FALSE;
 }
 hbp = (struct buf *) Malloc (v = phcnt * phuge * sizeof(struct buf));
 if ( hbp == (struct buf *) NULL )
 {	Error ("Unable to allocate %d bytes for hbp",v);
 	return FALSE;
 }
 incore_ilist = (struct incore_i *) Malloc (v = maxnii * sizeof(struct incore_i) );
 if ( incore_ilist == (struct incore_i *) NULL )
 {	Error ("Unable to allocate %d bytes for incore_ilist",v);
	return FALSE;
 }
 return TRUE;
}

/* ====================================================================== */

#undef GEPDEBUG

#include "rdevinfo.c"


/* end of deal_fs.c */

