static char rcsid[] = "$Header: /giga/HeliosRoot/Helios/filesys/fs/RCS/deal_fs.c,v 1.1 1990/10/05 16:27:40 nick Exp $";

/* $Log: deal_fs.c,v $
 * Revision 1.1  1990/10/05  16:27:40  nick
 * Initial revision
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

/*************************************************************************
**                                                                      **
**                   H E L I O S   F I L E S E R V E R                  **
**                   ---------------------------------                  **
**                                                                      **
**                   Copyright (C) 1988, Parsytec GmbH                  **
**                          All Rights Reserved.                        **
**                                                                      **
** deal_fs.c								**
**                                                                      **
**	Miscellaneous routines of the FileServer.	 		**
**                                                                      **
**************************************************************************
** HISTORY  :								**
**-----------								**
** Author   :  12/07/88  A.Ishan					**
** Modified :  19/01/89  H.J.Ermen  - Init of superblock data		**
**	       21/03/89  H.J.Ermen  - Full parametrization and dynamic	**
**				      memory allocation			**
*************************************************************************/

#define	DEBUG		0
#define GEPDEBUG	0

#include "nfs.h"

Semaphore		bhash_sem;
struct bufhash		bufhash[BUFHSZ];
struct free_packet	*free_packet;
struct incore_s		incore_sum;
struct fs		incore_fs;
Semaphore		ihash_sem;
struct i_hash		i_hash[IHSZ];
struct free_ilist	free_ilist;
					/* Globals from buf.h		*/
word psmal, pmedi, phuge;
word maxpsz;
word pscnt, pmcnt, phcnt;
					/* Globals from inode.h		*/
word maxnii;
					/* Globals from fs.h		*/
word maxbpg;
word maxncg;
word cgoffs;

word minfree;				/* Percentage of blocks to be	*/
					/* kept free			*/
char fs_name[32];			/* Global storage area for the	*/
					/* file-server's name		*/

struct packet		*stbp, *mtbp, *htbp;
struct buf		*sbp, *mbp, *hbp;
struct block		*scbp, *mcbp, *hcbp;
struct incore_i		*incore_ilist;

word filedrive;				/* which driveid the fs is on	*/

/* ==================================================================== */

/* Local procedures */

static void init_Boot_Block(void);
static void init_Sum_Block(void);
static void init_Info_Block(word);
static void init_Root_Dir(void);

static void init_incore_sum (void);

void init_buffer_cache (void);
void init_free_list (word, word, struct packet *, struct buf *, struct block *);
void init_incore_fs (void);
word alloc_mem (void);

/* ==================================================================== */

static void 
init_Boot_Block (void)

/* 
*  Since there isn't any BootBlock yet, simply clear the contents
*  of the first block on disc.
*/

{
     	struct buf *bp;

	bp = getblk(filedrive,0,1,NOSAVE);
	clr_buf (bp);
	bwrite(bp->b_tbp);						
}

/* ==================================================================== */

static void 
init_Sum_Block (void)

/*
*  Initialise the SummaryBlock after formatting the hard disc.
*/

{
	word i;
	struct sum_blk *sbp;
	Date date;
     	struct buf *bp;

	/* allocate a buffer and clear its contents */
	bp = getblk(filedrive,1,1,NOSAVE);
	clr_buf (bp);
	sbp = bp->b_un.b_sum;

	/* initialise the DirEntry of the RootDirectory */
	sbp->root_dir.de_inode.i_mode = DIR; 
	sbp->root_dir.de_inode.i_size = sizeof(struct dir_elem); 
	sbp->root_dir.de_inode.i_db[0] = 3; 	
	sbp->root_dir.de_inode.i_blocks = 1; 		
	sbp->root_dir.de_inode.i_spare = 0; 			
	strcpy(sbp->root_dir.de_name,fs_name);
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
	sbp->cg_sum[0].s_nbfree = maxbpg - 4;
	sbp->fs_sum = sbp->cg_sum[0];

	/* SummaryInfo of the remaining cylinder groups */
	for (i=1; i<maxncg; i++) 
	{
		sbp->cg_sum[i].s_ndir = 0;
		sbp->cg_sum[i].s_nbfree = maxbpg - 1;
		sbp->fs_sum.s_ndir += sbp->cg_sum[i].s_ndir;
		sbp->fs_sum.s_nbfree += sbp->cg_sum[i].s_nbfree;		
	}
		
	/* write to hard disc */
	bwrite(bp->b_tbp);						
}

/* ==================================================================== */

static void 
init_Info_Block (cgnr)

word cgnr;

/*
*  Initialise InfoBlock of the given cylinder group.
*/

{
	word i;
	struct cg  *cgp;
     	struct buf *bp;

	/* get the appropriate block and clear its contents */
#if DEBUG
IOdebug ("init_IB :	bp = getblk(%d, %d, 1, NOSAVE);\n",filedrive,cgtoib(cgnr));
#endif
	bp = getblk(filedrive,cgtoib(cgnr),1,NOSAVE);
	clr_buf (bp);
	cgp = &bp->b_un.b_info->cgx;
	
	/* calculate an "incremental" magic number of the specified cg */
	cgp->cg_magic = (MAGIC_CG_OFFSET << 16) | cgnr;
	
	/* Copy the initialized super-block data to the cg-info_blk */
	memcpy ((void *) &bp->b_un.b_info->fs, (void *) &incore_fs, sizeof(struct fs));
	
	if ( cgnr==0 ) 
	{
	 	/* special treatment for the first cylinder group,
		  cause there are 3 more blocks reserved */
		cgp->cg_s.s_ndir = 1;
		cgp->cg_s.s_nbfree = maxbpg - 4;
		cgp->cg_rotor = 4;
	} 
	else 
	{
		cgp->cg_s.s_ndir = 0;
		cgp->cg_s.s_nbfree = maxbpg - 1;
		cgp->cg_rotor = 0;
	}
	cgp->cg_cgx = cgnr;
	cgp->cg_ndblk = maxbpg;

	/* set the right bytes in the BitMap */
	for (i=0; i<cgp->cg_rotor; i++)
		cgp->cg_free[i]	= 0xff;
	for (i=cgp->cg_rotor; i<maxbpg; i++)
		cgp->cg_free[i]	= 0x00; 	    	
	cgp->cg_free[btocgb(cgtoib(cgnr))] = 0xff;

	/* write InfoBlock out to disc */
	bwrite(bp->b_tbp);						
}

/* ==================================================================== */

static void
init_Root_Dir (void)

/*
*  Allocate the first block of the RootDirectory,
*  although there aren't any DirEntries yet.
*/

{
	struct buf *bp;

	bp = getblk(filedrive,3,1,NOSAVE);
	clr_buf(bp);
	bwrite(bp->b_tbp);
}

/* ==================================================================== */

word getdrivenum(DiscDevInfo *ddi, VolumeInfo *vvi)
{
	Partition *p;
/*	word partnum;
	PartitionInfo *pi;
	word driveindex;
	DriveInfo *di;
*/
	p = (Partition *)RTOA(vvi->Partitions);
	return p->Partition;
/*	pi = (PartitionInfo *)RTOA(ddi->Partitions);
	while(partnum--) pi = (PartitionInfo *)RTOA(pi->Next);
	driveindex = pi->Drive;
	di = (DriveInfo *)RTOA(ddi->Drives);
	while(driveindex--) di = (DriveInfo *)RTOA(di->Next);
	return di->DriveId; */
}

word
make_fs ( FileSysInfo *fsi, DiscDevInfo *ddi, int volnum )

/*
*  After having formatted the hard disc,
*  first of all the FileSystem-Layout should be copied out.
*/

{
	VolumeInfo *vvi = (VolumeInfo *)RTOA(fsi->Volumes);
	word i;

	while(volnum--) vvi = (VolumeInfo *)RTOA(vvi->Next);

	filedrive  = getdrivenum(ddi,vvi);
		
	/* Get the user's parameters and copy them to the global used	*/
	/* variables.							*/

	psmal		= fsi->SmallPkt;	
	pmedi		= fsi->MediumPkt;	
	phuge		= fsi->HugePkt;
	maxpsz		= fsi->MaxPktSize;
	pscnt		= fsi->SmallCount;
	pmcnt		= fsi->MediumCount;
	phcnt		= fsi->HugeCount;

	maxnii		= fsi->MaxInodes;
	minfree		= vvi->MinFree;
	syncop          = fsi->SyncOp;

	maxbpg		= vvi->CgSize;
	maxncg		= vvi->CgCount;
	cgoffs		= vvi->CgOffset;
	

	strcpy ( fs_name, RTOA(vvi->Name) );

#if GEPDEBUG
IOdebug("pkt sizes %d %d %d max %d",psmal,pmedi,phuge,maxpsz);
IOdebug("pkt counts %d %d %d",pscnt,pmcnt,phcnt);
IOdebug("inodes %d",maxnii);
IOdebug("maxbpg %d maxncg %d cgoffs %d",maxbpg,maxncg,cgoffs);
IOdebug("syncop = %d",syncop);
#endif
					/* Fill a sample superblock	*/	
	incore_fs.fs_cgoffset	= cgoffs;
	incore_fs.fs_ncg	= maxncg ;
	incore_fs.fs_minfree	= minfree;
	incore_fs.fs_syncop     = syncop;
	incore_fs.fs_maxnii	= maxnii;
	incore_fs.fs_cgsize	= maxbpg ;
	incore_fs.fs_maxpsz	= maxpsz;
	incore_fs.fs_psmal	= psmal;
	incore_fs.fs_pmedi	= pmedi;
	incore_fs.fs_phuge 	= phuge;
	incore_fs.fs_pscnt	= pscnt;
	incore_fs.fs_pmcnt	= pmcnt;
	incore_fs.fs_phcnt	= phcnt;
	incore_fs.fs_possindb	= fsi->PossIndir;
	incore_fs.fs_possdirb	= fsi->PossDir;
	incore_fs.fs_mingood	= fsi->MinGood;
	incore_fs.fs_maxbmerr	= fsi->BitMapErrs;
	
	strcpy ( incore_fs.fs_name, fs_name );
					/* ... and complete it with	*/
	complete_super_block ( &incore_fs );	/* fixed and redundant data.	*/
	
	if ( ! alloc_mem () )
	{
		IOdebug ("Unable to allocate memory for the server!");
		close_dev();
		return FALSE;
	}

#if GEPDEBUG
IOdebug("	make_fs : 	init_buffer_cache();");
#endif
	/* initialise the BufferCache */ 
	init_buffer_cache();

#if GEPDEBUG
IOdebug("	make_fs : 	init_Boot_Block();");
#endif
	/* initialise the BootBlock */
	init_Boot_Block();
#if GEPDEBUG
IOdebug("	make_fs : 	init_Sum_Block();");
#endif
	/* initialise the SummaryBlock */	
	init_Sum_Block();

	for (i=0; i<maxncg; i++) 
	{
#if GEPDEBUG
IOdebug("	make_fs : 	init_Info_Block(%d);",i);
#endif
		/* initialise the InfoBlock of each cylinder group */
		init_Info_Block(i);
	}	
#if GEPDEBUG
IOdebug("	make_fs : 	init_Root_Dir();");
#endif
	/* allocate the first block of RootDirectory */
	init_Root_Dir();

	/* check "devinfo.src" parameters */
	{
		struct buf *bp;
			
		bp = bread(0,(maxbpg*maxncg-1),1,SAVEA);
		brelse (bp->b_tbp, TAIL);
	}

#if GEPDEBUG
IOdebug("	make_fs : 	close_dev();");
#endif
	/* finish communication to device-drivers */
	close_dev();
	
	return TRUE;
}

/* ==================================================================== */

word
make_super ( FileSysInfo *fsi, DiscDevInfo *ddi,int volnum )

{
	VolumeInfo *vvi = (VolumeInfo *)RTOA(fsi->Volumes);
	word i;
	struct info_blk *ibp;

	while(volnum--) vvi = (VolumeInfo *)RTOA(vvi->Next);

	filedrive  = getdrivenum(ddi,vvi);
		
	/* Get the user's parameters and copy them to the global used	*/
	/* variables.							*/

	psmal		= fsi->SmallPkt;	
	pmedi		= fsi->MediumPkt;	
	phuge		= fsi->HugePkt;
	maxpsz		= fsi->MaxPktSize;
	pscnt		= fsi->SmallCount;
	pmcnt		= fsi->MediumCount;
	phcnt		= fsi->HugeCount;

	maxnii		= fsi->MaxInodes;
	minfree		= vvi->MinFree;
	syncop          = fsi->SyncOp;

	maxbpg		= vvi->CgSize;
	maxncg		= vvi->CgCount;
	cgoffs		= vvi->CgOffset;
	

	strcpy ( fs_name, RTOA(vvi->Name) );

#if GEPDEBUG
IOdebug("pkt sizes %d %d %d max %d",psmal,pmedi,phuge,maxpsz);
IOdebug("pkt counts %d %d %d",pscnt,pmcnt,phcnt);
IOdebug("inodes %d",maxnii);
IOdebug("maxbpg %d maxncg %d cgoffs %d",maxbpg,maxncg,cgoffs);
IOdebug("syncop = %d",syncop);
#endif
					/* Fill a sample superblock	*/	
	incore_fs.fs_cgoffset	= cgoffs;
	incore_fs.fs_ncg	= maxncg ;
	incore_fs.fs_minfree	= minfree;
	incore_fs.fs_syncop     = syncop;
	incore_fs.fs_maxnii	= maxnii;
	incore_fs.fs_cgsize	= maxbpg ;
	incore_fs.fs_maxpsz	= maxpsz;
	incore_fs.fs_psmal	= psmal;
	incore_fs.fs_pmedi	= pmedi;
	incore_fs.fs_phuge 	= phuge;
	incore_fs.fs_pscnt	= pscnt;
	incore_fs.fs_pmcnt	= pmcnt;
	incore_fs.fs_phcnt	= phcnt;
	incore_fs.fs_possindb	= fsi->PossIndir;
	incore_fs.fs_possdirb	= fsi->PossDir;
	incore_fs.fs_mingood	= fsi->MinGood;
	incore_fs.fs_maxbmerr	= fsi->BitMapErrs;
	
	strcpy ( incore_fs.fs_name, fs_name );
					/* ... and complete it with	*/
	complete_super_block ( &incore_fs );	/* fixed and redundant data.	*/

	ibp = (struct info_blk *) Malloc (BSIZE);
	if ( read_blk ( (void *)ibp, 2) != BSIZE ) {
		IOdebug(" fserver : Info block on disk irreversibly physically destroyed!");
		Free (ibp);
		close_dev ();
		return FALSE;
	}
	
	memcpy ((void *) &ibp->fs, (void *) &incore_fs, sizeof (struct fs));
	
	if ( write_blk ( (void *)ibp, 2) != BSIZE ) {
		IOdebug(" fserver : Info block on disk not written!");
		Free (ibp);
		close_dev ();
		return FALSE;
	}
	
	Free (ibp);
	/* finish communication to device-drivers */
	close_dev();

	return TRUE;
}

/* ==================================================================== */
/* ==================================================================== */

void
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
	/* initialise the free-list header */
	php->fp_waiting = 0;
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
*  Initialise the Incore Inode List, creating Inode Hash Queues
*  and Free Inode List, and copying the inode of the RootDirectory
*  into this list which will be kept incore for the runtime.
*/

{
	struct i_hash  *hip;
	struct incore_i *ip;
	struct free_ilist *fip;
	word 	i,j;
	struct buf *bp;
	
	/* initialise the inode hash queues */
	hip = i_hash;
	for (i=0; i<IHSZ; i++,hip++) 
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

	/* read the SummaryBlock including the DirEntry of the RootDir */
	bp = bread (filedrive,1,1,SAVEA);
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
}
	
/* ==================================================================== */

static void
init_incore_sum (void)

/*
*  Copy the SummaryInformations into incore.
*/

{
	word i;
	struct buf *bp;
	
	/* read the SummaryBlock including the SummaryInfos */
	bp = bread (filedrive,1,1,SAVEA);
	/* copy the first_boot flag */
	incore_sum.is_first_boot = bp->b_un.b_sum->first_boot;

	incore_sum.is_fs.sum = bp->b_un.b_sum->fs_sum;
	InitSemaphore(&incore_sum.is_fs.sem,1);
	for (i=0; i<maxncg; i++) 
	{
		incore_sum.is_cg[i].sum = bp->b_un.b_sum->cg_sum[i];
		InitSemaphore(&incore_sum.is_cg[i].sem,1);
	}
	
	/* provide the Incore Summary Informations */
	if ( bp->b_un.b_sum->sum_same )
	{
		incore_sum.is_sum_same = TRUE;
	}
	else
	/* Special treatment, if the summary infos are not valid */
	{
		IOdebug ("ERROR : Invalid summary block found. Call the checker !");
		longjmp (term_jmp,1);
		incore_sum.is_sum_same = FALSE;
	}	

	/* First time booted : first_boot is TRUE -> set it to FALSE ! */
	if ( incore_sum.is_first_boot )
	{
		bp->b_un.b_sum->first_boot = FALSE;
		bwrite ( bp->b_tbp );		
	}
	else
		brelse (bp->b_tbp, TAIL);
}
	
/* ==================================================================== */

void
init_incore_fs (void)

/*
*  Copy the Superblock into incore.
*/

{
	struct buf *bp;

	bp = bread (filedrive,2,1,SAVEA);
	incore_fs = bp->b_un.b_info->fs;
				
	brelse (bp->b_tbp, TAIL);
}
/* ==================================================================== */

word
init_fs (FileSysInfo *fsi, DiscDevInfo *ddi, int volnum)

/*
*  Builds the FileServer structures at system-startup.
*/

{
	struct info_blk *ibp;
	VolumeInfo *vvi = (VolumeInfo *)RTOA(fsi->Volumes);
	word drv;
	
/* Find VolumeInfo structure */
	while(volnum--)
	{	if(vvi->Next == -1) return -1;
		vvi = (VolumeInfo *)RTOA(vvi->Next);
	}
#if GEPDEBUG
IOdebug(" init_fs() : init volume %d",volnum);
#endif

	drv = getdrivenum(ddi,vvi);
	
	if( vvi->Type == vvt_raw )
	{
/* Hopefully the discdevice, the file system and buffer cache have
   already be initialised, so all we do here is to return
   the fact that there is a tape to be handled 				*/
#if DEBUG
	IOdebug("Raw volume found");
#endif
		 strcpy(tape_name, RTOA(vvi->Name));
		return drv;
	}
	filedrive = drv;


#if 1
	/* Get the user's parameters and copy them to the global used	*/
	/* variables.							*/

	psmal		= fsi->SmallPkt;	
	pmedi		= fsi->MediumPkt;	
	phuge		= fsi->HugePkt;
	maxpsz		= fsi->MaxPktSize;
	pscnt		= fsi->SmallCount;
	pmcnt		= fsi->MediumCount;
	phcnt		= fsi->HugeCount;

	maxnii		= fsi->MaxInodes;
	minfree		= vvi->MinFree;
	syncop          = fsi->SyncOp;

	maxbpg		= vvi->CgSize;
	maxncg		= vvi->CgCount;
	cgoffs		= vvi->CgOffset;
	

	strcpy ( fs_name, RTOA(vvi->Name) );

#else
					/* Read the info-block	(cg 0)  */
					/* directly by avoiding the use */
					/* of the buffer cache		*/
	ibp = (struct info_blk *) Malloc ( BSIZE );
	read_info_blk ( ibp );
					/* Copy all needed parameters	*/
	psmal 	= ibp->fs.fs_psmal;	/* into global variables.	*/
	pmedi   = ibp->fs.fs_pmedi;
	phuge   = ibp->fs.fs_phuge;
	maxpsz  = ibp->fs.fs_maxpsz;
	pscnt   = ibp->fs.fs_pscnt;
	pmcnt	= ibp->fs.fs_pmcnt;
	phcnt   = ibp->fs.fs_phcnt;

	maxnii	= ibp->fs.fs_maxnii;
	syncop  = ibp->fs.fs_syncop;

	maxbpg	= ibp->fs.fs_cgsize;
	maxncg	= ibp->fs.fs_ncg;
	cgoffs	= ibp->fs.fs_cgoffset;
	strcpy ( fs_name, ibp->fs.fs_name );

	Free ( ibp );
#endif
#if GEPDEBUG
IOdebug("pkt sizes %d %d %d max %d",psmal,pmedi,phuge,maxpsz);
IOdebug("pkt counts %d %d %d",pscnt,pmcnt,phcnt);
IOdebug("inodes %d",maxnii);
IOdebug("maxbpg %d maxncg %d cgoffs %d",maxbpg,maxncg,cgoffs);
IOdebug("syncop = %d",syncop);
#endif		
#if DEBUG
IOdebug("Allocating memory for fs");
#endif
	if ( ! alloc_mem () )
	{
		IOdebug ("Unable to allocate memory for the server!");
		close_dev ();
		return -1;
	}
	
#if DEBUG
IOdebug("\n	init_fs : 	init_buffer_cache();\n");
#endif
	/* initialise the BufferCache */
	init_buffer_cache();
#if DEBUG
IOdebug("	init_fs : 	init_incore_ilist();\n");
#endif
	/* build Incore Inode List including the Incore Inode of RootDir */
	init_incore_ilist();
#if DEBUG
IOdebug("	init_fs : 	init_incore_sum();\n");
#endif
	/* import the Summary Informations */
	init_incore_sum();
#if DEBUG
IOdebug("	init_fs : 	init_incore_fs();\n");
#endif
	/* import the SuperBlock */
	init_incore_fs();
	
	return drv;
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
*  Tries to save the consistency of the FileSystem,
*  writing all delayed-write blocks out to disc,
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
	struct incore_i *ip;
	struct buf *bp;
		
#if DEBUG
IOdebug("	sync_fs :	Get access.\n");
#endif
				
	Wait ( &sync_sem );		/* Guarantee exclusive access	*/
 
	for (i=0; i<maxpsz; i++) 
	{
		/* for the currently available packet-sizes */
		if ((i==(psmal-1))||(i==(pmedi-1))||(i==(phuge-1))) 
		{
#if DEBUG
IOdebug("	sync_fs :	| %d |\n",i+1);
#endif
			/* block free-lidt of this packet-size */
			Wait (&free_packet[i].fp_sem);
			/* follow down the free-list */
			tbp = (struct packet *) &free_packet[i];
			for (itbp=tbp->p_next; 
			     itbp!=tbp; itbp=itbp->p_next) 
			{ 
				/* if packet marked as delayed-write
				   AND  not already blocked */
				if ((itbp->p_dwt_cnt) 
				    && (TestSemaphore(&itbp->p_sem)>0))
				{
					Wait (&itbp->p_sem);
#if DEBUG
IOdebug("	sync_fs : 	write_dev (0x%p);\n",itbp);
#endif
					/* write contents of packet out to disc */
					write_dev (itbp);
					Signal (&itbp->p_sem);		
				}
			}
			/* give free-list free */
			Signal (&free_packet[i].fp_sem);
		}
	}
#if DEBUG
IOdebug("\n");
#endif

	/* get the pointer to the Incore Inode of the RootDir */
	ip = iget ((struct buf *)NULL, 0, 1, 0);
	Wait (&ip->ii_sem);
#if DEBUG
IOdebug("	sync_fs : 	bp = bread (filedrive, 1, 1, SAVEA);\n");
#endif
	if (ip->ii_changed || (!incore_sum.is_sum_same) )
	{
		/* read the SummaryBlock into BufferCache */
		bp = bread (filedrive,1,1,SAVEA);
		/* write the inode of RootDir out */
		bp->b_un.b_sum->root_dir.de_inode = ip->ii_i;
		/* update the Summary Infos on disc */
		bp->b_un.b_sum->fs_sum = incore_sum.is_fs.sum;
		for (i=0; i<maxncg; i++) 
			bp->b_un.b_sum->cg_sum[i] = incore_sum.is_cg[i].sum;
		/* validate the Summary Infos on disc */
		bp->b_un.b_sum->sum_same = TRUE;
#if DEBUG
IOdebug("	sync_fs : 	bwrite (0x%p);\n",bp->b_tbp);
#endif
		/* write the SummaryBlock out to disc */
		bwrite (bp->b_tbp);

		Wait (&incore_sum.is_fs.sem);
		/* ... and the incore summary flag */
		incore_sum.is_sum_same = TRUE;
		Signal (&incore_sum.is_fs.sem);
 
		/* reset the modified-flag of the Incore RootDir Inode */
		ip->ii_changed = FALSE;
	}
	/* give the Incore Inode free */
	Signal (&ip->ii_sem);
	iput (ip);
	
	Signal ( &sync_sem );		/* Signal others, that they can	*/
}					/* access the incore strutures.	*/

/* ==================================================================== */

void
term_fs (void)

/*
 *  Writes the contents of the Buffer Cache out a last time
 *  and terminates the device-driver routines.
 */
{
	sync_fs();
#if DEBUG
IOdebug("	test_fs : 	close_dev();\n");
#endif
	close_dev();
}

/* ====================================================================== */

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
#if DEBUG
IOdebug ("	alloc_mem :	Allocate memory for the buffer-cache.\n");
#endif

#if DEBUG
IOdebug("maxpsz = %d",maxpsz);
#endif
 free_packet = (struct free_packet *) Malloc (v = maxpsz * sizeof(struct free_packet));
 if ( free_packet == (struct free_packet *) NULL )
 {	IOdebug("Unable to allocate %d bytes for free_packet",v);
	return FALSE;
 }
 stbp = (struct packet *) Malloc (v = pscnt * sizeof(struct packet));
 if ( stbp == (struct packet *) NULL )
 {	IOdebug("Unable to allocate %d bytes for stbp",v);
	return FALSE;
 }
 mtbp = (struct packet *) Malloc (v = pmcnt * sizeof(struct packet));
 if ( mtbp == (struct packet *) NULL )
 {	IOdebug("Unable to allocate %d bytes for mtbp",v);
	return FALSE;
 }
 htbp = (struct packet *) Malloc (v = phcnt * sizeof(struct packet));
 if ( htbp == (struct packet *) NULL )
 {	IOdebug("Unable to allocate %d bytes for htbp",v);
	return FALSE;
 }
 sbp = (struct buf *) Malloc (v = pscnt * psmal * sizeof(struct buf));
 if ( sbp == (struct buf *) NULL )
 {	IOdebug("Unable to allocate %d bytes for sbp",v);
 	return FALSE;
 }
 mbp = (struct buf *) Malloc (v = pmcnt * pmedi * sizeof(struct buf));
 if ( mbp == (struct buf *) NULL )
 {	IOdebug("Unable to allocate %d bytes for mbp",v);
 	return FALSE;
 }
 hbp = (struct buf *) Malloc (v = phcnt * phuge * sizeof(struct buf));
 if ( hbp == (struct buf *) NULL )
 {	IOdebug("Unable to allocate %d bytes for hbp",v);
 	return FALSE;
 }
 scbp = (struct block *) Malloc (v = pscnt * psmal * sizeof(struct block));
 if ( scbp == (struct block *) NULL )
 {	IOdebug("Unable to allocate %d bytes for scbp",v);
 	return FALSE;
 }
 mcbp = (struct block *) Malloc (v = pmcnt * pmedi * sizeof(struct block));
 if ( mcbp == (struct block *) NULL )
 {	IOdebug("Unable to allocate %d bytes for mcbp",v);
 	return FALSE;
 }
 hcbp = (struct block *) Malloc (v = phcnt * phuge * sizeof(struct block));
 if ( hcbp == (struct block *) NULL )
 {	IOdebug("Unable to allocate %d bytes for hcbp",v);
 	return FALSE;
 }
 incore_ilist = (struct incore_i *) Malloc (v = maxnii * sizeof(struct incore_i) );
 if ( incore_ilist == (struct incore_i *) NULL )
 {	IOdebug("Unable to allocate %d bytes for incore_ilist",v);
	return FALSE;
 }
 return TRUE;
}


/* ==================================================================== */

#include "rdevinfo.c"


/* end of deal_fs.c */

