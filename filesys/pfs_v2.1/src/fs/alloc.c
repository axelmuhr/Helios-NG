/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/alloc.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: alloc.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:05:00  guenter
 * first multivolume/multipartition HFS with tape
 * 
 * Revision 1.2  90/02/01  17:34:29  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  18:16:41  chris
 * Initial revision
 * 
 */


                                                                                /*
  []-------------------------------------------------------------------------[]
   |                                                                         |
   |                  (c) 1988-91 by parsytec GmbH, Aachen                   |
   |                          All rights reserved.                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |                          Parsytec File System                           |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  alloc.c   						             |
   |                                                                         |
   |    Routines of the Allocation Strategies.                               |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    3 - O.Imbusch - 17 April 1991 - Error handling centralized           |
   |    2 - H.J.Ermen - 22 March 1989 - "syncop" implementation              |
   |    1 - A.Ishan   - 30 June  1988 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */


#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1
#include "error.h"

#define PROCCNT 1
#include "proccnt.h"

#include "fserr.h"
#include "nfs.h"

/*-----------------------  Local prototypes  ----------------------------*/

static word	cgprefb (VD *vol);
static word	cgprefd (VD *vol);
static pref_t	blkpref (struct incore_i *ip, daddr_t lgbnr);
static pkt_t    map_search (struct cg *cgp, word cgbpref, word bcnt, VD *vol);
static void	set_map (struct cg *cgp, word cgbnr, word allocsize, VD *vol);
static word 	sum_updateb (word cgnr, word allocsize, VD *vol);
static word	sum_updated (word cgnr, word allocsize, VD *vol);
static word	i_update (struct incore_i *ip, daddr_t lgbnr, pkt_t pkt);
static word	free_map (struct cg *cgp, daddr_t bnr, VD *vol);


/*-----------------------------------------------------------------------*/

static word
cgprefb(VD *vol)

/*
*  Search for the cylinder-group with maximal free blocks.
*/

{
	word cgnr,i;
	uword nbfree;
	
	/* for all cylinder-groups */
	for(cgnr=-1,i=0,nbfree=0; i<vol->cgs; i++) 
	{
		Wait (&vol->incore_sum.is_cg[i].sem);

		/* if (nr of free-blocks in this cyl-group > nbfree) */
		if (vol->incore_sum.is_cg[i].sum.s_nbfree > nbfree) 
		{
			/* notice nr of free-blocks */
			nbfree = vol->incore_sum.is_cg[i].sum.s_nbfree;
			/* notice cyl-groupnr */
			cgnr = i;
		}
		Signal (&vol->incore_sum.is_cg[i].sem);
	}
	return (cgnr);
}	
	
/* ================================================================= */

static word
cgprefd(VD *vol)

/*
*  Search for the cylinder-group with minimal SubDirectories.
*/

{
	word cgnr,i;
	uword ndir;
	
	/* for all cylinder-groups */
	for(cgnr=-1,i=0,ndir=vol->bpcg; i<vol->cgs; i++) 
	{
		Wait (&vol->incore_sum.is_cg[i].sem);

		/* if (nr of dirs in this cyl-group < ndir) */
		if (vol->incore_sum.is_cg[i].sum.s_ndir < ndir) 
		{
			/* notice nr of directories */
			ndir = vol->incore_sum.is_cg[i].sum.s_ndir;
			/* notice cyl-groupnr */
			cgnr = i;
		}
		Signal (&vol->incore_sum.is_cg[i].sem);
	}
	return (cgnr);
}	
	
/* ================================================================= */

void 
clr_buf (bp)

struct buf *bp;

/*
*  Clear contents of the block pointed by 'bp'.
*/

{	
	word i;
	
	IncPC (86);

	/* clear contents of this block */
	for (i=0;i<SIADDR;i++) 
		*(bp->b_un.b_daddr+i) = 0;
	DecPC (86);		
}

/* ================================================================= */

static pref_t 
blkpref (ip, lgbnr)

struct incore_i *ip;
daddr_t lgbnr;

/*
*  According to the global policies of allocation strategies
*  find out a preferred block to be allocated next.
*  Returns the incremented disc address of the last allocated block,
*  if the new block should be allocated contigious to the previous one,
*  else the number of a new cylinder group.
*  Returns err_pref.bpref = -1
*	   err_pref.cpref = -1
*  if any I/O-error occurres
*/

{
	daddr_t lbnr,ibnr,iibnr;
	pref_t pref,err_pref;
	word i,pos;
	pkt_t ipkt,iipkt;
	struct buf *bp;
	VD *vol;
	
	vol = &volume[ip->ii_dev];	
	pref.bpref = -1;
	pref.cpref = -1;

	err_pref.bpref = -1;
	err_pref.cpref = -1;
	
	/* if (direct data block) */
	if (lgbnr < MAXDIR) 
	{
		for (i=0; i<(MAXDIR-1); i++) 
		{
			/* from previous logical block backwards */
			lgbnr--;
			/* modulo func : from (MAXDIR-1) downto (0) */
			if (lgbnr == -1)
				lgbnr = MAXDIR - 1;
			/* notice blocknr of logical block */ 
			lbnr = ip->ii_i.i_db[lgbnr];	
			/* if (logical block is mapped to a physical one) */
			if (lbnr)
			{
				/* return (physical blknr++ as preferred ) */
				pref.bpref = lbnr + 1;
				return (pref);
			}
		}

		/* it's the first direct data block of a file */
		/* if (directory file) */
		if (ip->ii_i.i_mode == DIR)
		{
			/* return ((preferred cyl-groupnr for a new subdir)) */
			pref.cpref = cgprefd(vol);
			return (pref);
		}	
		else	
		{	
			/* return ((cyl-groupnr of dir entry as preferred)) */
			pref.cpref = btocg(ip->ii_dirbnr,vol);
			return (pref);
		}
	} 


	/* if (single indirect data block) */
	if (lgbnr < MAXSIND) 
	{
		/* assign blocknr of indirect block */
		ibnr = ip->ii_i.i_ib[0];

		/* if (there isn't any indirect block yet) */ 
		if (!ibnr) 
		{
			if (vol->vol_full)
			{
				pref.bpref = 0;
				return(pref);
			}	

DEBdebug ("	blkpref :	ipkt = alloc (0x%p, -1, 1); ",ip);

			/* allocate an indirect block */
			ipkt = alloc (ip, -1, 1);
			if (!ipkt.bcnt)
			{
				pref.bpref = 0;
				return(pref);
			}	
			/* notice allocated blocknr */
			ip->ii_i.i_ib[0] = ipkt.bnr;

DEBdebug ("bp = getblk (%d, %d, 1, NOSAVE); ", ip->ii_dev,ipkt.bnr);

			/* get this block into buffer cache */
			bp = getblk (ip->ii_dev,ipkt.bnr, 1, NOSAVE);
			if (!bp) {
Error (FSErr [GetBlkFailed], vol->vol_name);
				return (err_pref);				
			}
			/* clear contents of new allocated block */
			clr_buf (bp);

DEBdebug ("	blkpref :	bdwrite (0x%p); ",bp->b_tbp);

			/* write it out to disc */
			if (!bwrite (bp->b_tbp)) {
Error (FSErr [WriteBlkFailed], vol->vol_name);
				goto finish;
			}
			/* return ((preferred cyl-groupnr for single ind
				     data blocks)) */ 	
			pref.cpref = cgprefb(vol);
			return (pref);
		}

		/* if (there is an indirect block) */ 
		else 
		{
sind:			


DEBdebug ("	blkpref : 	bp = bread (%d, %d, 1, SAVEA); ", ip->ii_dev, ibnr);

			/* read indirect block to buffer cache */
			bp = bread (ip->ii_dev,	ibnr, 1, SAVEA);
			if (bp == NULL) {
Error (FSErr [ReadSIBlkFailed], vol->vol_name);
				goto finish;
			}
			for (i=0; i<(SIADDR-1); i++) 
			{
				/* from previous logical block backwards */
				lgbnr--;
				/* modulo func : from (MAXSIND-1) 
						 downto (MAXDIR) */
				if (lgbnr == MAXDIR - 1)
			     		lgbnr = MAXSIND - 1;
				/* notice blocknr of logical block */ 
				lbnr = *(bp->b_un.b_daddr + (lgbnr - MAXDIR));
				/* if (logical block is mapped 
						to a physical one) */		
				if (lbnr) 
					break;
			}


DEBdebug ("	blkpref :	bp = brelse (0x%p, TAIL); ",bp->b_tbp);

			/* release indirect block */			
			brelse (bp->b_tbp, TAIL);
			/* return (physical blknr++ as preferred ) */
			pref.bpref = lbnr + 1;
			return (pref);
		}
	}


	/* if (double indirect data block) */
	if (lgbnr < MAXDIND) 
	{
		/* assign blocknr of double indirect block */
		iibnr = ip->ii_i.i_ib[1];
		/* calc position of single indirect blocknr 
				in double indirect block */
		pos = (lgbnr - MAXSIND) / SIADDR;

		/* if (there isn't any double indirect block yet) */ 
		if (!iibnr) 
		{
			if (vol->vol_full)
			{
				pref.bpref = 0;
				return(pref);
			}	

DEBdebug ("	blkpref :	iipkt = alloc (0x%p, -1, 1); ",ip);

			/* allocate a double indirect block */
			iipkt = alloc (ip, -1, 1);
			if (!iipkt.bcnt)
			{
				pref.bpref = 0;
				return(pref);
			}	
			/* notice allocated blocknr */
			ip->ii_i.i_ib[1] = iipkt.bnr;
			if (vol->vol_full)
			{
				pref.bpref = 0;
				return(pref);
			}	

DEBdebug ("	blkpref :	ipkt = alloc (0x%p, -1, 1); ",ip);

			/* allocate a single indirect block */
			ipkt = alloc (ip, -1, 1);
			if (!ipkt.bcnt)			
			{
				pref.bpref = 0;
				return(pref);
			}	

DEBdebug ("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE); ", ip->ii_dev,iipkt.bnr);

			/* get double indirect block into buffer cache */
			bp = getblk (ip->ii_dev,iipkt.bnr, 1, NOSAVE);
			if (!bp) {
Error (FSErr [GetBlkFailed], vol->vol_name);
				return (err_pref);				
			}
			/* clear contents of new allocated block */
			clr_buf (bp);
			/* notice blocknr of single indirect block
					in double indirect block */
			*(bp->b_un.b_daddr+pos) = ipkt.bnr;

DEBdebug ("	blkpref :	bwrite (0x%p); ",bp->b_tbp);

			/* write it out to disc */
			if (!bwrite (bp->b_tbp)) {
Error (FSErr [WriteBlkFailed], vol->vol_name);
				goto finish;
			}

DEBdebug ("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE); ",ip->ii_dev,ipkt.bnr);

			/* get single indirect block into buffer cache */
			bp = getblk (ip->ii_dev,ipkt.bnr, 1, NOSAVE);
			if (!bp) {
Error (FSErr [GetBlkFailed], vol->vol_name);
				return (err_pref);				
			}
			/* clear contents of new allocated block */
			clr_buf (bp);

DEBdebug ("	blkpref :	bdwrite (0x%p); ",bp->b_tbp);

			/* write it out to disc */
			if (!bwrite (bp->b_tbp)) {
Error (FSErr [WriteBlkFailed], vol->vol_name);
				goto finish;
			}
			/* return ((preferred cyl-groupnr for single ind
				     data blocks)) */ 	
			pref.cpref = cgprefb(vol);
			return (pref);
		}

		/* if (there is a double indirect block) */ 
		else 
		{
			/* adjust blocknr of single indirect block */
			ibnr = get_daddr (ip->ii_dev, iibnr, pos);
			if (ibnr < 0)
				return (err_pref);
			/* if (there isn't this single indirect block) */
			if (!ibnr) 
			{
				if (vol->vol_full)
				{
					pref.bpref = 0;
					return(pref);
				}	

DEBdebug ("	blkpref :	ipkt = alloc (0x%p, -1, 1); ",ip);

				/* allocate a single indirect block */
				ipkt = alloc (ip, -1, 1);
				if (!ipkt.bcnt)
				{
					pref.bpref = 0;
					return(pref);
				}	

DEBdebug ("	blkpref : 	bp = bread (%d, %d, 1, SAVEB); ", ip->ii_dev, ip->ii_i.i_ib[1]);

				/* read double indirect block to buffer cache */
				bp = bread (ip->ii_dev,
					    ip->ii_i.i_ib[1], 1, SAVEB);
				if (bp == NULL) {
Error (FSErr [ReadDIBlkFailed], vol->vol_name);
					goto finish;
				}
				/* notice blocknr of single indirect block
						in double indirect block */
				*(bp->b_un.b_daddr+pos) = ipkt.bnr;

DEBdebug ("	blkpref :	bwrite (0x%p); ",bp->b_tbp);

				/* write double indirect block out to disc */
				if (!bwrite (bp->b_tbp)) {
Error (FSErr [WriteDIBlkFailed], vol->vol_name);
					goto finish;
				}


DEBdebug ("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE); ", ip->ii_dev,ipkt.bnr);

				/* get single ind block into buffer cache */
				bp = getblk (ip->ii_dev,ipkt.bnr, 1, NOSAVE);
				if (!bp) {
Error (FSErr [GetBlkFailed], vol->vol_name);
					return (err_pref);
				}
				/* clear contents of new allocated block */
				clr_buf (bp);

DEBdebug ("	blkpref :	bdwrite (0x%p); ",bp->b_tbp);

				/* write it out to disc */
				if (!bwrite (bp->b_tbp)) {
Error (FSErr [WriteBlkFailed], vol->vol_name);
					goto finish;
				}
				/* return ((preferred cyl-groupnr for single 
				     	indirect data blocks)) */ 	
				pref.cpref = cgprefb(vol);
				return (pref);
			}

			/* if (there is this single indirect block) */
			else 
			{
				/* adjust relative logical blocknr */
				lgbnr -= (pos + 1) * SIADDR;
				/* handle it like first single ind block */
				goto sind;
			}
		}
	}


	/* if (triple indirect data block) */
	if (lgbnr < MAXTIND) 
	{
Error (FSErr [NotImplemented], vol->vol_name);
	}		
	else {
Error (FSErr [Catastrophe], vol->vol_name);
	}

finish:
	PutMsg (&vol->unload_mcb);
	return (err_pref);	
}			

/* ================================================================= */

static pkt_t 
map_search (cgp ,cgbpref ,bcnt, vol)

struct cg *cgp;
word cgbpref, bcnt;
VD *vol;

/*
*  According to the local policies of the allocation strategies
*  tries to find out requested size contigious free blocks
*  in the BitMap of the pointed cylinder group.
*  Returns the number of the first free block and the
*  size of the contigious free blocks which might be
*  less equal to the requested size.
*/

{
	pkt_t pkt;
	word i,cgb,free_cnt;
	
	/* if (there is a preferred relative blocknr) */
	if (cgbpref>=0)
		/* assign it to the search-start-block */
		cgb = cgbpref;
	else
		/* assign the cyl-group-rotor 
			which points to last allocated block++ */
		cgb = cgp->cg_rotor;
		
	for (free_cnt=0,pkt.bcnt=0,i=0; i<vol->bpcg; i++) 
	{
		
		/* reset free_cnt */
		if (!cgb) 
			free_cnt = 0;

		/* reset free-cnt if block isn't free in bitmap & continue */
		if (cgp->cg_free[cgb]) 
		{
			free_cnt = 0;
			goto finish;
		} 

		/* if there's a free block in the bitmap */
		else 
		{
			/* increment free-count */
			free_cnt++;
			/* notice maximum amount of contigous free blocks */
			if (free_cnt>pkt.bcnt) 
			{
				pkt.bcnt = free_cnt;
				pkt.bnr = cgbtob(cgp->cg_cgx,
						 cgb-free_cnt+1,vol);
				/* break if requested amount is achieved */
				if (pkt.bcnt>=bcnt)
					break;
			}
		}
finish:
		/* from search-start-block forwards */
		cgb++;
		/* modulo func : from (0) to (vol->bpcg-1) */
		if (cgb == vol->bpcg)
			cgb = 0; 

	}

	/* return (maximum amount of contig free blocks in this cyl-group */
	return(pkt);
}	

/* ================================================================= */

static void 
set_map (cgp, cgbnr, allocsize, vol)

struct cg *cgp;
word cgbnr, allocsize;
VD *vol;

/*
*  Sets the appropriate bytes in the BitMap of zhe pointed cylinder group 
*  as occupied and updates the SummaryInfos in the InfoBlock.
*/

{
	word i;
	
	/* set the blocks as occupied in the bitmap */
	for (i=0;i<allocsize;i++)
		cgp->cg_free[cgbnr+i] = 0xff;

	/* update nr of free blocks in the cyl-group summary */
	cgp->cg_s.s_nbfree -= allocsize;

	/* update cyl-group-rotor to last allocated blk++ */ 
	cgp->cg_rotor = (cgbnr+allocsize) % vol->bpcg;
	
}	
	
/* ================================================================= */

static word 
sum_updateb (cgnr, allocsize, vol)

word cgnr, allocsize;
VD *vol;

/*
*  Updates the Incore SummaryInformation for allocated data blocks.
*  Return : FALSE if any I/O-error occurred, TRUE else
*/

{
	struct buf *bp;
	
	/* update free blocks in incore-file-system-summary */
	Wait (&vol->incore_sum.is_fs.sem);
	vol->incore_sum.is_fs.sum.s_nbfree -= allocsize;
	Signal (&vol->incore_sum.is_fs.sem);	

	/* update free blocks in incore-cyl-group-summary */
	Wait (&vol->incore_sum.is_cg[cgnr].sem);
	vol->incore_sum.is_cg[cgnr].sum.s_nbfree -= allocsize;	
	Signal (&vol->incore_sum.is_cg[cgnr].sem);

	/* update summary block info, depending on the incore summary flag */
        if ( vol->incore_sum.is_sum_same )       
        {
		Wait (&vol->incore_sum.is_fs.sem);
        	/* ... and it's incore counterpart */
        	vol->incore_sum.is_sum_same = FALSE;
		Signal (&vol->incore_sum.is_fs.sem);

        	/* read summary block from disc */
        	bp = bread ( vol->volnum,1,1,SAVEA );
		if (bp == NULL) {
Error (FSErr [ReadSumBlkFailed], vol->vol_name);
			return (FALSE);
		}
        	/* reset the summary flag of the structure */
        	bp->b_un.b_sum->sum_same = FALSE;
        	/* write the summary block's package to disc */
		if (!bwrite ( bp->b_tbp )) {
Error (FSErr [WriteSumBlkFailed], vol->vol_name);
			return (FALSE);
		}
        }
        return (TRUE);
}

/* ================================================================= */

static word
sum_updated (cgnr, allocsize, vol)

word cgnr, allocsize;
VD *vol;

/*
*  Updates the Incore SummaryInformation for directories.
*  Return : FALSE if any I/O-error occurred, TRUE else
*/

{
	struct buf *bp;

	/* update nr of dirs in incore-file-system-summary */
	Wait (&vol->incore_sum.is_fs.sem);
	vol->incore_sum.is_fs.sum.s_ndir += allocsize;
	Signal (&vol->incore_sum.is_fs.sem);	

	/* update nr of dirs in incore-cyl-group-summary */
	Wait (&vol->incore_sum.is_cg[cgnr].sem);
	vol->incore_sum.is_cg[cgnr].sum.s_ndir += allocsize;	
	Signal (&vol->incore_sum.is_cg[cgnr].sem);

	/* update summary block info, depending on the incore summary flag */
        if ( vol->incore_sum.is_sum_same )       
        {
		Wait (&vol->incore_sum.is_fs.sem);
        	/* ... and it's incore counterpart */
        	vol->incore_sum.is_sum_same = FALSE;
		Signal (&vol->incore_sum.is_fs.sem);

        	/* read summary block from disc */
        	bp = bread ( vol->volnum,1,1,SAVEA );
		if (bp == NULL) {
Error (FSErr [ReadSumBlkFailed], vol->vol_name);
			return (FALSE);
		}
        	/* reset the summary flag of the structure */
        	bp->b_un.b_sum->sum_same = FALSE;
        	/* write the summary block's package to disc */
		if (!bwrite ( bp->b_tbp )) {
Error (FSErr [WriteSumBlkFailed], vol->vol_name);
			return (FALSE);
		}
        }
        return (TRUE);
}

/* ================================================================= */

static word 
i_update (ip, lgbnr, pkt)

struct incore_i *ip;
daddr_t lgbnr;
pkt_t pkt;

/*
*  Updates the address information of a file in the pointed Incore Inode
*  modified by the current allocation.
*  Return : FALSE if any I/O-error occurred, TRUE if not.
*/

{
	word i,pos;
	daddr_t bnr;
	struct buf *bp;
	VD *vol;
	
	vol = &volume[ip->ii_dev];
	/* update the amount of blocks held by this file (incl. ind blocks) */
	ip->ii_i.i_blocks += pkt.bcnt;

	/* if (data blocks) */
	if (lgbnr>=0) 
	{
		/* if (direct data block) */
		if (lgbnr < MAXDIR) 
		{
			/* update the inode of direct data blocks */ 
			for (i=0;(lgbnr+i)<MAXDIR;i++) 
			{	
				if (i==pkt.bcnt)
					return (TRUE);
				ip->ii_i.i_db[lgbnr+i] = pkt.bnr+i;  
			}
			return (TRUE);
		} 

		/* if (single indirect data block) */
		if (lgbnr < MAXSIND) 
		{

DEBdebug ("	i_update : 	bp = bread (%d, %d, 1, SAVEB); ",ip->ii_dev, ip->ii_i.i_ib[0]);

			/* read single indirect block into buffer cache */
			bp = bread(ip->ii_dev,ip->ii_i.i_ib[0],1,SAVEB);
			if (bp == NULL) {
Error (FSErr [ReadSIBlkFailed], vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
			/* update the inode of direct data blocks */ 
			for (i=0;(lgbnr+i)<MAXSIND;i++) 
			{
				if (i==pkt.bcnt) 
					goto finish;
				*(bp->b_un.b_daddr + (lgbnr+i-MAXDIR)) 
						= pkt.bnr+i;
/*
Report (FSErr [DebugBNr], lgbnr+i,pkt.bnr+i);
*/
			}

DEBdebug ("	i_update :	bwrite (0x%p); ",bp->b_tbp);

			/* write indirect block out to disc */
			if (!bwrite(bp->b_tbp)) {
Error (FSErr [WriteIBlkFailed], vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
			return (TRUE);
		} 

		/* if (double indirect data block) */
		if (lgbnr < MAXDIND) 
		{ 
			/* calc position of single ind block */
			pos = (lgbnr-MAXSIND)/SIADDR;
			/* adjust indirect data blocknr */
			bnr = get_daddr(ip->ii_dev,ip->ii_i.i_ib[1],pos);
			if (bnr < 0) {
				return (FALSE);	
			}

DEBdebug ("	i_update : 	bp = bread (%d, %d, 1, SAVEB); ",ip->ii_dev, bnr);

			/* read indirect block into buffer cache */
			bp = bread(ip->ii_dev,bnr,1,SAVEB);
			if (bp == NULL) {
Error (FSErr [ReadDIBlkFailed], vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
			/* update the inode of direct data blocks */
			for(i=0;(lgbnr+i-MAXSIND)/SIADDR==pos;i++) 
			{
				if (i==pkt.bcnt) goto finish;
				*(bp->b_un.b_daddr + ((lgbnr+i-MAXSIND)%SIADDR))
						= pkt.bnr+i;
			}

DEBdebug ("	i_update :	bwrite (0x%p); ",bp->b_tbp);

			/* write out indirect block to disc */
			if (!bwrite(bp->b_tbp)) {
Error (FSErr [WriteIBlkFailed], vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
			return (TRUE);
		} 

		/* if (triple indirect data block) */
		if (lgbnr < MAXTIND) 
		{
Error (FSErr [NotImplemented], vol->vol_name);
			return (FALSE);
		}

finish:


DEBdebug ("	i_update :	bwrite (0x%p); ",bp->b_tbp);

		/* write out indirect block to disc */
		if (!bwrite(bp->b_tbp))
		{
Error (FSErr [WriteIBlkFailed], volume [ip->ii_dev].vol_name);
			PutMsg (&vol->unload_mcb);
			return (FALSE);
		}
	}
	return (TRUE);
}
				   
			   	   
/* ================================================================= */

pkt_t 
alloc (ip, lgbnr, bcnt)

struct incore_i *ip;
daddr_t lgbnr;
word bcnt;

/*
*  Tries to allocate requested size of new blocks for the pointed file,
*  with the help of global and local policies of allocation strategies.
*  Returns the address of the first allocated block and the size  of
*  the contigious allocated blocks which might be less equal than
*  the requested size. There is an exception message if there isn't
*  any free blocks left on the whole hard disc.
*/

{
	pkt_t ipkt;
	pref_t pref;
	word cgnr,cgbpref,ibcnt,iibnr,iibcnt,i,icgnr,pos;
	struct cg *cgp;
	struct buf *bp;
	VD *vol;
	
	IncPC (85);
	
	vol = &volume[ip->ii_dev];		
	/* is there enough space for allocating? */
	if ( vol->incore_sum.is_fs.sum.s_nbfree < vol->minfree ) {
		vol->vol_full = TRUE;
Error (FSErr [MinFreeReached], vol->vol_name, vol->minfree);
		goto finish;
	}
	/* if (allocation of data blocks) */
	elif (lgbnr>=0) 
	{
		/* if (direct data blocks) */
		if (lgbnr < MAXDIR) 
		{
			/* if (ending in single ind level) */
			if ((lgbnr+bcnt-1) >= MAXDIR)
				/* trunc block count to avoid level-exceeding */
				bcnt = MAXDIR - lgbnr;
		} 

		/* if (single indirect data blocks) */
		elif (lgbnr < MAXSIND) 
		{
			/* if (ending in double ind level) */
			if ((lgbnr+bcnt-1) >= MAXSIND)
				/* trunc block count to avoid level-exceeding */
				bcnt = MAXSIND - lgbnr;
		} 

		/* if (double indirect data blocks) */
		elif (lgbnr < MAXDIND) 
		{
			/* adjust position of single indirect block
				in double indirect block */
			pos  = ((lgbnr - MAXSIND) / SIADDR);
			/* if (ending in another single ind level) */
			if ( ((lgbnr+bcnt-1 - MAXSIND) / SIADDR) != pos)	
				/* trunc block count to avoid level-exceeding */
				bcnt = (pos+1) * SIADDR + MAXSIND - lgbnr;
		}	


DEBdebug ("	alloc : 	pref = blkpref (0x%p, %d); ",ip,lgbnr);

		/* find out preferred blocknr respectively cyl-groupnr */
		pref = blkpref (ip, lgbnr);

		/* if (preferred block) */
		if (pref.bpref>0) 
		{
			/* calc cyl-groupnr */
			cgnr = btocg(pref.bpref,vol);
			/* calc relative blocknr in this cyl-group */
			cgbpref = btocgb(pref.bpref,vol);
		} 
		/* if (preferred cyl-group) */
		elif (pref.cpref >= 0)
		{
			/* notice preferred cyl-group */
			cgnr = pref.cpref;
			/* init relative blocknr */
			cgbpref = -1;
		}
		else
			goto finish;		
	} 


	/* if (allocation of an indirect block) */	
	else 
	{
		/* adjust cyl-groupnr as the one of the directory entry */
		cgnr = btocg(ip->ii_dirbnr,vol);
		/* calc relative blocknr close to dir entry */
		cgbpref = btocgb(ip->ii_dirbnr+1,vol);
	}
	
	/* for (from (requested amount of blocks) downto (1) block) */
	for (ibcnt=bcnt;
	     ibcnt>0;
	     cgnr=btocg(iibnr,vol),cgbpref=btocgb(iibnr,vol),ibcnt=iibcnt) 

		/* for (all cylinder-groups) */	     
	    	for (iibcnt=0,i=0;i<vol->cgs;i++) 
	    	{
			/* modulo func : from (0) to (vol->cgs-1) */
	    		icgnr = (cgnr+i) % vol->cgs;

DEBdebug ("	alloc :		bp = bread(%d, %d, 1, SAVEA); ",ip->ii_dev,cgtoib(icgnr,vol));

			/* read info-block of cyl-group into buffer-cache */
	    		bp = bread(ip->ii_dev,cgtoib(icgnr,vol),1,SAVEA);
			if (bp == NULL) {
Error (FSErr [ReadInfoBlkFailed], vol->vol_name);
				PutMsg (&vol->unload_mcb);
				goto finish;	
			}
			/* pointer to cyl-group struct */
			cgp = &bp->b_un.b_info->cgx;

DEBdebug ("	alloc :		ipkt = map_search(0x%p, %d, %d); ", cgp,cgbpref,ibcnt);


			/* search in cyl-group bitmap for free blocks */
	    		ipkt = map_search(cgp,cgbpref,ibcnt,vol);

			    /* get as much as possible in the pref cyl-group */
	    		if ((ibcnt==bcnt)&&(!i)&&(ipkt.bcnt>0)
			    /* OR get as much as requested in other cyl-grps */
	 		    ||(ipkt.bcnt==ibcnt)) 
	 		{

DEBdebug ("	alloc :		set_map(0x%p, %d, %d); ", cgp,btocgb(ipkt.bnr,vol),ipkt.bcnt);

				/* set in the bitmap the blocks as occupied */
	    		    	set_map (cgp,btocgb(ipkt.bnr,vol),ipkt.bcnt,vol);

DEBdebug ("	alloc :		bwrite (0x%p); ",bp->b_tbp);

				/* write out the info-block to disc */
	    		    	if (!bwrite(bp->b_tbp)) {
Error (FSErr [WriteInfoBlkFailed], vol->vol_name);
					PutMsg (&vol->unload_mcb);
					goto finish;	
	    		    	}
				/* update free blocks in incore-summary */
	    		    	if ( !sum_updateb(icgnr,ipkt.bcnt,vol) ) {
Error (FSErr [UpdateSumInfoFailed], vol->vol_name);
					PutMsg (&vol->unload_mcb);
					goto finish;
				}
				/* if ((it's a directory file) */ 
				if ((ip->ii_i.i_mode == DIR)
				    /* AND (it's the first block of dir)) */	
				    && (ip->ii_i.i_db[0] == 0)) {
					/* update nr of dirs in incore-sum */
					if (!sum_updated(icgnr,ipkt.bcnt,vol)) {
Error (FSErr [UpdateSumInfoFailed], vol->vol_name);
						PutMsg (&vol->unload_mcb);
						goto finish;
					}
				}
				/* update the inode in incore-ilist */
	    		    	if (!i_update(ip,lgbnr,ipkt)) {
					goto finish;
	    		    	}
	    		    	
				/* return (nr of allocated blocks) */
	    		    	DecPC (85); return(ipkt);
	    		}


DEBdebug ("	alloc :		bp = brelse (0x%p, TAIL); ",bp->b_tbp);

			/* release the info-block since there weren't
				enough free blocks in this cyl-group */
	    		brelse(bp->b_tbp,TAIL);
			/* reset relative blocknr for preferred cyl-group */
	    		cgbpref=-1;
			/* notice maximum amount of contigous free blocks */
	    		if (ipkt.bcnt>iibcnt) 
	    		{
	    			iibcnt = ipkt.bcnt;
	    			iibnr = ipkt.bnr;
	    		}
	    	}

/* If You have reached this label, congratulations Your FileSystem is full ! */

Error (FSErr [FSFull], vol->vol_name);
vol->vol_full = TRUE;
finish:
	ipkt.bcnt = 0;
	ipkt.bnr = 0;
	DecPC (85); return(ipkt);
}
	    	
	    		     			
/* ================================================================= */

static word
free_map (cgp, bnr, vol)

struct cg *cgp;
daddr_t bnr;
VD *vol;

/*
*  Marks the freed blocks in the BitMap of the pointed cylinder group
*  as free and updates the SummaryInformations.
*  Return : FALSE if any I/O-error occurred, TRUE if not
*/

{
	/* set block free in bitmap */
	cgp->cg_free[btocgb(bnr,vol)] = 0;
	/* update nr of free blocks in info-block */
	cgp->cg_s.s_nbfree++;
	/* update nr of free blocks in incore-summary */
    	if ( !sum_updateb(btocg(bnr,vol),-1,vol) ) {
Error (FSErr [UpdateSumInfoFailed], vol->vol_name);
		return (FALSE);
	}
	return (TRUE);
}

/* ================================================================= */

void
tidyup_cache (dev, bnr)

dev_t dev;
daddr_t bnr;

{
	struct buf *bp, *lbp, *ibp;
	struct packet *tbp;
		
	if (volume[dev].vol_full)
		volume[dev].vol_full = FALSE;
	bp = bsearch_hash (dev, bnr);
		
	if (bp)	
	{
		tbp = bp->b_tbp;
		Wait (&tbp->p_sem);
		/* calc the pointer to the last block in the packet */
		lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
		/* clear all the buffer_headers */
		for (ibp = tbp->p_fbp; ibp <= lbp; ibp++) 
		{
			bremove_hash (ibp);
			ibp->b_dev = 0;	
			ibp->b_bnr = 0;
			ibp->b_vld = FALSE;
			ibp->b_dwt = FALSE;
		}				
		/* clear the packet_header */
		tbp->p_blk_cnt = 0;
		tbp->p_vld_cnt = 0;
		tbp->p_dwt_cnt = 0;
		Signal (&tbp->p_sem);	
	}
}

/* ================================================================= */

word 
fREE (ip, flgbnr, dlgbnr)

struct incore_i *ip;
daddr_t flgbnr;
word dlgbnr;

/*
*  Gives the requested size blocks of the pointed file free,
*  updating the Incore Inode, the Incore Summary Informations
*  and the BitMaps of affected InfoBlocks.
*  Return : FALSE if any I/O-error occurred, TURE if not
*/ 

{
	daddr_t bnr,ibnr,iibnr;
	word i,j,cgnr,lcgnr=-1;
	struct cg *cgp;
	struct buf *bip,*biip,*ibp;	
	VD *vol;

/* define a macro called 'chg_info_blk' to write out the
   InfoBlock of the last cylinder-group and read the 
   InfoBlock of the current cylinder-group into Buffer Cache */

#	define chg_info_blk \
	if (lcgnr!=-1) \
		if (!bwrite(ibp->b_tbp)) { \
Error (FSErr [WriteInfoBlkFailed], vol->vol_name); \
			Signal ( &ip->ii_sem); \
			return (FALSE); \
		} \
	ibp = bread(ip->ii_dev,cgtoib(cgnr,vol),1,SAVEB); \
	if (ibp == NULL) { \
Error (FSErr [ReadInfoBlkFailed], vol->vol_name); \
		Signal ( &ip->ii_sem); \
		return (FALSE); \
	} \
	cgp = &ibp->b_un.b_info->cgx; \
	lcgnr = cgnr;
	
	vol = &volume[ip->ii_dev];
	/* if it's a directory file so its blocks are freed only before
	   deleting the directory, therefore update the Incore SummaryInfo */
	if (ip->ii_i.i_mode == DIR) {
		if (!sum_updated (btocg(ip->ii_i.i_db[0],vol),-1,vol)) {
Error (FSErr [UpdateSumInfoFailed], vol->vol_name);
			Signal (&ip->ii_sem);
			return (FALSE);
		}
	}
		
	/* if the first block to be freed is a direct block */
	if (flgbnr<MAXDIR) 
	{
		for(i=flgbnr;i<MAXDIR;i++) 
		{
			/* if there is an allocated block */
			if (ip->ii_i.i_db[i]) 
			{
				/* adjust the disc address */
				bnr = ip->ii_i.i_db[i];
				
				tidyup_cache (ip->ii_dev, bnr);
				
				/* calculate the cylinder group */
				cgnr = btocg(bnr,vol);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free()  1 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free()  2 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif
					chg_info_blk
				}	    			
				/* update InfoBlock */
				if (!free_map(cgp,bnr,vol)) {
Error (FSErr [UpdateInfoBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
				/* update Incore Inode */
				ip->ii_i.i_db[i] = 0;
				ip->ii_i.i_blocks--;
			}
			/* decrement the size of blocks to be freed */	
			dlgbnr--;
			/* if all requested blocks freed,
			   write InfoBlock out and return */	
			if(!dlgbnr) 
			{
				if (lcgnr!=-1) 
				{

DEBdebug (" free()  3 : bwrite (0x%x); ",ibp->b_tbp);

		    		    	if (!bwrite(ibp->b_tbp)) {
Error (FSErr [WriteInfoBlkFailed], vol->vol_name);
						Signal (&ip->ii_sem);
						return (FALSE);
	    			    	}
				}		
				return (TRUE);
			}
		}
	}

	/* if there are single indirect data blocks to be freed */
	if (flgbnr<MAXSIND) 
	{
		/* if the single IndirectBlock is allocated */
		if (ip->ii_i.i_ib[0]) 
		{
			/* find out the disc address of it */
			ibnr = ip->ii_i.i_ib[0];

DEBdebug (" free()  4 : bip = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,ibnr);

			/* read it into Buffer Cache */
			bip = bread(ip->ii_dev,ibnr,1,SAVEB);
			if (bip == NULL) {
Error (FSErr [ReadSIBlkFailed], vol->vol_name);
				Signal (&ip->ii_sem);
				return (FALSE);
			}		
			for(i=(flgbnr>MAXDIR)?flgbnr:MAXDIR;i<MAXSIND;i++) 
			{
				/* if there is an allocated data block */
				if (*(bip->b_un.b_daddr+i-MAXDIR)) 
				{
					/* find out its disc address */
					bnr = *(bip->b_un.b_daddr+i-MAXDIR);
					
					tidyup_cache (ip->ii_dev, bnr);
					
					/* and its cylinder group */
					cgnr = btocg(bnr,vol);
					/* change InfoBlock if necessary */
					if (cgnr!=lcgnr) 
					{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free()  5 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free()  6 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif

						chg_info_blk
					}	    			
					/* update InfoBlock */
					if (!free_map(cgp,bnr,vol)) {
Error (FSErr [UpdateInfoBlkFailed], vol->vol_name);
						Signal (&ip->ii_sem);
						return (FALSE);
					}
					/* update IndirectBlock */
					*(bip->b_un.b_daddr+i-MAXDIR) = 0;
					/* update Incore Inode */
					ip->ii_i.i_blocks--;
				}

				/* decrement the size of blocks to be freed */	
				dlgbnr--;
				if(!dlgbnr) 
					break;
			}
			/* if the IndirectBlock isn't emptied */
			if (flgbnr>MAXDIR) 
			{

DEBdebug (" free()  7 : bwrite (0x%x); ",bip->b_tbp);

				/* write it to disc */
				if (!bwrite(bip->b_tbp)) {
Error (FSErr [WriteIBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
			} 
			else 
			{

DEBdebug (" free()  8 : brelse (0x%x, HEAD); ",bip->b_tbp);

				/* release the buffer containing
				   the empty IndirectBlock */
				if (checksum_allowed)
	  				bip->b_checksum = checksum(bip->b_un.b_addr,BSIZE);

				brelse(bip->b_tbp, HEAD);
				
				tidyup_cache (ip->ii_dev, ibnr);
				
				/* find out the cylinder group of IndBlock */
				cgnr = btocg(ibnr,vol);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free()  9 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free() 10 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif
					chg_info_blk
				}	    			
				/* free the IndirectBlock */
				if (!free_map(cgp,ibnr,vol)) {
Error (FSErr [UpdateInfoBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
				/* update Incore Inode */
				ip->ii_i.i_ib[0] = 0;
				ip->ii_i.i_blocks--;
			}
		}

		/* if the single IndirectBlock isn't allocated */
		else 
		{
			for(i=(flgbnr>MAXDIR)?flgbnr:MAXDIR;i<MAXSIND;i++) 
			{
				/* decrement the size of blocks to be freed */
				dlgbnr--;
				if (!dlgbnr)
					break;
			}
		}			
		/* if all requested blocks have been freed */
		if(!dlgbnr) 
		{
			/* write the modified InfoBlock to disc, if any */	
			if (lcgnr!=-1) 
			{

DEBdebug (" free() 11 : bwrite (0x%x); ",ibp->b_tbp);

	    		    	if (!bwrite(ibp->b_tbp)) {
Error (FSErr [WriteInfoBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
    			    	}
			}
			return (TRUE);
		}
	}
	
	/* if there are double indirect data blocks to be freed */
	if (flgbnr<MAXDIND) 
	{
		/* if the double IndirectBlock is allocated */
		if (ip->ii_i.i_ib[1]) 
		{
			/* adjust its disc address */
			iibnr = ip->ii_i.i_ib[1];

DEBdebug (" free() 12 : biip = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,iibnr);

			/* read it into BufferCache */
			biip = bread(ip->ii_dev,iibnr,1,SAVEB);
			if (biip == NULL) {
Error (FSErr [ReadDIBlkFailed], vol->vol_name);
				Signal (&ip->ii_sem);
				return (FALSE);
			}		
			/* for all the single IndirectBlocks */
			for(j=1;j<=SIADDR;j++) 
			{

/* !!!
*  ATTENTION :  For the NEXT lines of the listing
*               the INDENTATION is three times less
*		than it normally should be !!!
*
*  The end of this REGION is marked similarly !
*/

	/* if there are data blocks of this IndirectBlock to be freed */
	if (flgbnr<MAXSIND+j*SIADDR) 
	{
		/* if this single IndirectBlock is allocated */
		if (*(biip->b_un.b_daddr+j-1)) 
		{
			/* adjust its disc address */
			ibnr = *(biip->b_un.b_daddr+j-1);

DEBdebug (" free() 13 : bip = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,ibnr);

			/* read it into Buffer Cache */
			bip = bread(ip->ii_dev,ibnr,1,SAVEB);
			if (bip == NULL) {
Error (FSErr [ReadSIBlkFailed], vol->vol_name);
				Signal (&ip->ii_sem);
				return (FALSE);
			}		
			/* for all the data blocks in this IndBlock */
			for(i=(flgbnr>(MAXSIND+(j-1)*SIADDR))
			      ?flgbnr:(MAXSIND+(j-1)*SIADDR);
			    i<(MAXSIND+j*SIADDR);
			    i++) 
			{
				/* if data block allocated */
				if (*(bip->b_un.b_daddr
				     + i-(MAXSIND+(j-1)*SIADDR)
				     )) 
				{
					/* adjust its block number */
					bnr = *(bip->b_un.b_daddr
				     		+ i-(MAXSIND+(j-1)*SIADDR));
				     		
				     	tidyup_cache (ip->ii_dev, bnr);
				     	
					/* and its cylinder group */
					cgnr = btocg(bnr,vol);
					/* change InfoBlock if necessary */
					if (cgnr!=lcgnr) 
					{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free() 14 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free() 15 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif
						chg_info_blk
					}	    			
					/* update BitMap in InfoBlock */
					if (!free_map(cgp,bnr,vol)) {
Error (FSErr [UpdateInfoBlkFailed], vol->vol_name);
						Signal (&ip->ii_sem);
						return (FALSE);
					}
					/* update IndirectBlock */
					*(bip->b_un.b_daddr
				     	  + i-(MAXSIND+(j-1)*SIADDR)) = 0;
					/* update incore inode */
					ip->ii_i.i_blocks--;
				}
				/* decrement the counter of blocks to be freed */
				dlgbnr--;
				if(!dlgbnr) 
					break;
			}
			/* if this single IndirectBlock hasn't been emptied */
			if (flgbnr>(MAXSIND+(j-1)*SIADDR)) 
			{

DEBdebug (" free() 16 : bwrite (0x%x); ",bip->b_tbp);

				/* write it to disc */
				if (!bwrite(bip->b_tbp)) {
Error (FSErr [WriteSIBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
			} 
			else 
			{

DEBdebug (" free() 17 : brelse (0x%x, HEAD); ",bip->b_tbp);

				/* release the buffer containing this IndBlock */
				if (checksum_allowed)
	  				bip->b_checksum = checksum(bip->b_un.b_addr,BSIZE);

				brelse(bip->b_tbp, HEAD);
				
				tidyup_cache (ip->ii_dev, ibnr);
				
				/* adjust the cyl-group of this IndBlock */
				cgnr = btocg(ibnr,vol);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free() 18 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free() 19 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif

					chg_info_blk
				}	    			
				/* free this IndirectBlock */
				if (!free_map(cgp,ibnr,vol)) {
Error (FSErr [FreeIBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
				/* update double IndirectBlock */
				*(biip->b_un.b_daddr+j-1) = 0;
				/* update incore inode */	
				ip->ii_i.i_blocks--;
			}
		}

		/* if this single IndirectBlock not allocated */
		else 
		{
			/* for all logical blocks corresponding to this IndBlock */
			for(i=(flgbnr>(MAXSIND+(j-1)*SIADDR))
			      ?flgbnr:(MAXSIND+(j-1)*SIADDR);
			    i<(MAXSIND+j*SIADDR);
			    i++) 
			{
				/* decrement counter of the blocks to be freed */
				dlgbnr--;
				if (!dlgbnr)
					break;
			}
		}			
		if (!dlgbnr) 
			break;
	}
/* !!!
*  ATTENTION :  For the PREVIOUS lines of the listing
*               the INDENTATION is three times less
*		than it normally should be !!!
*/

			}	
			/* if the double IndirectBlock isn't emptied */
			if (flgbnr>MAXSIND) 
			{

DEBdebug (" free() 20 : bwrite (0x%x); ",biip->b_tbp);

				/* write it to disc */
				if (!bwrite(biip->b_tbp)) {
Error (FSErr [WriteDIBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
			} 
			else 
			{

DEBdebug (" free() 21 : brelse (0x%x, HEAD); ",biip->b_tbp);

				/* release the buffer containing
				   the double IndirectBlock */
				if (checksum_allowed)
	  				biip->b_checksum = checksum(biip->b_un.b_addr,BSIZE);

				brelse(biip->b_tbp, HEAD);

				tidyup_cache(ip->ii_dev,iibnr);
				
				/* adjust its cylinder group */
				cgnr = btocg(iibnr,vol);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{

#if DEBUG
if (lcgnr!=-1)
	DEBdebug (" free() 22 : bwrite (0x%x); ",ibp->b_tbp);
DEBdebug (" free() 23 : ibp = bread(%d, %d, 1, SAVEB); ", ip->ii_dev,cgtoib(cgnr,vol));
#endif
					chg_info_blk
				}	    			
				/* free the double IndirectBlock */
				if (!free_map(cgp,iibnr,vol)) {
Error (FSErr [FreeDIBlkFailed], vol->vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}
				/* update the Incore Inode */
				ip->ii_i.i_ib[1] = 0;
				ip->ii_i.i_blocks--;
			}

DEBdebug (" free() 24 : bwrite (0x%x); ",ibp->b_tbp);

			/* write the InfoBlock out */
			if (!bwrite(ibp->b_tbp)) {
Error (FSErr [WriteInfoBlkFailed], vol->vol_name);
				Signal (&ip->ii_sem);
				return (FALSE);
			}
		}
	}	
	return (TRUE);
/* undefine the macro to change InfoBlocks */
#	undef chg_info_blk
}

/* ==================================================================== */

/* end of alloc.c */
