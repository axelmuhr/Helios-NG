 static char rcsid[] = "$Header: /hsrc/filesys/fs/RCS/alloc.c,v 1.3 1991/05/08 13:40:03 al Exp $";

/* $Log: alloc.c,v $
 * Revision 1.3  1991/05/08  13:40:03  al
 * Parsytec bug fixes added. (1/5/91)  New revision 1.4
 *
 * Revision 1.2  90/02/01  17:34:29  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  18:16:41  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                  H E L I O S   F I L E S E R V E R                   **
**                  ---------------------------------                   **
**                                                                      **
**                  Copyright (C) 1988, Parsytec GmbH                   **
**                         All Rights Reserved.                         **
**                                                                      **
** alloc.c								**
**                                                                      **
**	Routines of the Allocation Strategies.	 			**
**************************************************************************
** HISTORY  :                                                           **
**-------------								**
** AUTHOR   :  30/06/88 - A.Ishan					**
** MODIFIED :  22/03/89 - H.J.Ermen   : "syncop" implementation		**
*************************************************************************/

#define DEBUG	FALSE

#include "nfs.h"

/*-----------------------  Local prototypes  ----------------------------*/

static word	cgprefb (void);
static word	cgprefd (void);
static pref_t	blkpref (struct incore_i *ip, daddr_t lgbnr);
static pkt_t    map_search (struct cg *cgp, word cgbpref, word bcnt);
static void	set_map (struct cg *cgp, word cgbnr, word allocsize);
static void 	sum_updateb (word cgnr, word allocsize);
static void	sum_updated (word cgnr, word allocsize);
static void	i_update (struct incore_i *ip, daddr_t lgbnr, pkt_t pkt);
static void	free_map (struct cg *cgp, daddr_t bnr);
static void 	tidyup_cache (dev_t dev, daddr_t bnr);

/*-----------------------------------------------------------------------*/

static word
cgprefb(void)

/*
*  Search for the cylinder-group with maximal free blocks.
*/

{
	word cgnr,i;
	uword nbfree;
	
	/* for all cylinder-groups */
	for(cgnr=-1,i=0,nbfree=0; i<maxncg; i++) 
	{
		Wait (&incore_sum.is_cg[i].sem);

		/* if (nr of free-blocks in this cyl-group > nbfree) */
		if (incore_sum.is_cg[i].sum.s_nbfree > nbfree) 
		{
			/* notice nr of free-blocks */
			nbfree = incore_sum.is_cg[i].sum.s_nbfree;
			/* notice cyl-groupnr */
			cgnr = i;
		}
		Signal (&incore_sum.is_cg[i].sem);
	}
	return (cgnr);
}	
	
/* ================================================================= */

static word
cgprefd(void)

/*
*  Search for the cylinder-group with minimal SubDirectories.
*/

{
	word cgnr,i;
	uword ndir;
	
	/* for all cylinder-groups */
	for(cgnr=-1,i=0,ndir=maxbpg; i<maxncg; i++) 
	{
		Wait (&incore_sum.is_cg[i].sem);

		/* if (nr of dirs in this cyl-group < ndir) */
		if (incore_sum.is_cg[i].sum.s_ndir < ndir) 
		{
			/* notice nr of directories */
			ndir = incore_sum.is_cg[i].sum.s_ndir;
			/* notice cyl-groupnr */
			cgnr = i;
		}
		Signal (&incore_sum.is_cg[i].sem);
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

	/* clear contents of this block */
	for (i=0;i<SIADDR;i++) 
		*(bp->b_un.b_daddr+i) = 0;
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
*/

{
	daddr_t lbnr,ibnr,iibnr;
	pref_t pref;
	word i,pos;
	pkt_t ipkt,iipkt;
	struct buf *bp;
	
	pref.bpref = -1;
	pref.cpref = -1;

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
			pref.cpref = cgprefd();
			return (pref);
		}	
		else	
		{	
			/* return ((cyl-groupnr of dir entry as preferred)) */
			pref.cpref = btocg(ip->ii_dirbnr);
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
			if (fsfull)
			{
				pref.bpref = 0;
				return(pref);
			}	
#if DEBUG
printf("	blkpref :	ipkt = alloc (0x%p, -1, 1);\n",ip);
#endif
			/* allocate an indirect block */
			ipkt = alloc (ip, -1, 1);
			if (!ipkt.bcnt)
			{
				pref.bpref = 0;
				return(pref);
			}	
			/* notice allocated blocknr */
			ip->ii_i.i_ib[0] = ipkt.bnr;
#if DEBUG
printf("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE);\n",
ip->ii_dev,ipkt.bnr);
#endif
			/* get this block into buffer cache */
			bp = getblk (ip->ii_dev,ipkt.bnr, 1, NOSAVE);
			/* clear contents of new allocated block */
			clr_buf (bp);
#if DEBUG
printf("	blkpref :	bdwrite (0x%p);\n",bp->b_tbp);
#endif
			/* write it out to disc */
			bwrite (bp->b_tbp);
			/* return ((preferred cyl-groupnr for single ind
				     data blocks)) */ 	
			pref.cpref = cgprefb();
			return (pref);
		}

		/* if (there is an indirect block) */ 
		else 
		{
sind:			

#if DEBUG
printf("	blkpref : 	bp = bread (%d, %d, 1, SAVEA);\n",
ip->ii_dev, ibnr);
#endif
			/* read indirect block to buffer cache */
			bp = bread (ip->ii_dev,	ibnr, 1, SAVEA);

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

#if DEBUG
printf("	blkpref :	bp = brelse (0x%p, TAIL);\n",bp->b_tbp);
#endif
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
			if (fsfull)
			{
				pref.bpref = 0;
				return(pref);
			}	
#if DEBUG 
printf("	blkpref :	iipkt = alloc (0x%p, -1, 1);\n",ip);
#endif
			/* allocate a double indirect block */
			iipkt = alloc (ip, -1, 1);
			if (!iipkt.bcnt)
			{
				pref.bpref = 0;
				return(pref);
			}	
			/* notice allocated blocknr */
			ip->ii_i.i_ib[1] = iipkt.bnr;
			if (fsfull)
			{
				pref.bpref = 0;
				return(pref);
			}	
#if DEBUG
printf("	blkpref :	ipkt = alloc (0x%p, -1, 1);\n",ip);
#endif
			/* allocate a single indirect block */
			ipkt = alloc (ip, -1, 1);
			if (!ipkt.bcnt)			
			{
				pref.bpref = 0;
				return(pref);
			}	
#if DEBUG
printf("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE);\n",
ip->ii_dev,iipkt.bnr);
#endif
			/* get double indirect block into buffer cache */
			bp = getblk (ip->ii_dev,iipkt.bnr, 1, NOSAVE);
			/* clear contents of new allocated block */
			clr_buf (bp);
			/* notice blocknr of single indirect block
					in double indirect block */
			*(bp->b_un.b_daddr+pos) = ipkt.bnr;
#if DEBUG
printf("	blkpref :	bwrite (0x%p);\n",bp->b_tbp);
#endif
			/* write it out to disc */
			bwrite (bp->b_tbp);
#if DEBUG
printf("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE);\n",
ip->ii_dev,ipkt.bnr);
#endif
			/* get single indirect block into buffer cache */
			bp = getblk (ip->ii_dev,ipkt.bnr, 1, NOSAVE);
			/* clear contents of new allocated block */
			clr_buf (bp);
#if DEBUG
printf("	blkpref :	bdwrite (0x%p);\n",bp->b_tbp);
#endif
			/* write it out to disc */
			bwrite (bp->b_tbp);	
			/* return ((preferred cyl-groupnr for single ind
				     data blocks)) */ 	
			pref.cpref = cgprefb();
			return (pref);
		}

		/* if (there is a double indirect block) */ 
		else 
		{
			/* adjust blocknr of single indirect block */
			ibnr = get_daddr (ip->ii_dev, iibnr, pos);

			/* if (there isn't this single indirect block) */
			if (!ibnr) 
			{
				if (fsfull)
				{
					pref.bpref = 0;
					return(pref);
				}	
#if DEBUG
printf("	blkpref :	ipkt = alloc (0x%p, -1, 1);\n",ip);
#endif
				/* allocate a single indirect block */
				ipkt = alloc (ip, -1, 1);
				if (!ipkt.bcnt)
				{
					pref.bpref = 0;
					return(pref);
				}	
#if DEBUG
printf("	blkpref : 	bp = bread (%d, %d, 1, SAVEB);\n",
ip->ii_dev, ip->ii_i.i_ib[1]);
#endif
				/* read double indirect block to buffer cache */
				bp = bread (ip->ii_dev,
					    ip->ii_i.i_ib[1], 1, SAVEB);
				/* notice blocknr of single indirect block
						in double indirect block */
				*(bp->b_un.b_daddr+pos) = ipkt.bnr;
#if DEBUG
printf("	blkpref :	bwrite (0x%p);\n",bp->b_tbp);
#endif
				/* write double indirect block out to disc */

				bwrite (bp->b_tbp);
#if DEBUG
printf("	blkpref :	bp = getblk (%d, %d, 1, NOSAVE);\n",
ip->ii_dev,ipkt.bnr);
#endif
				/* get single ind block into buffer cache */
				bp = getblk (ip->ii_dev,
					     ipkt.bnr, 1, NOSAVE);
				/* clear contents of new allocated block */
				clr_buf (bp);
#if DEBUG
printf("	blkpref :	bdwrite (0x%p);\n",bp->b_tbp);
#endif
				/* write it out to disc */
				bwrite (bp->b_tbp);	
				/* return ((preferred cyl-groupnr for single 
				     	indirect data blocks)) */ 	
				pref.cpref = cgprefb();
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
IOdebug("	blkpref : 	ERROR : TIND data blocks not implemented !");
	}		

}			

/* ================================================================= */

static pkt_t 
map_search (cgp ,cgbpref ,bcnt)

struct cg *cgp;
word cgbpref, bcnt;

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
		
	for (free_cnt=0,pkt.bcnt=0,i=0; i<maxbpg; i++) 
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
						 cgb-free_cnt+1);
				/* break if requested amount is achieved */
				if (pkt.bcnt>=bcnt)
					break;
			}
		}
finish:
		/* from search-start-block forwards */
		cgb++;
		/* modulo func : from (0) to (maxbpg-1) */
		if (cgb == maxbpg)
			cgb = 0; 

	}

	/* return (maximum amount of contig free blocks in this cyl-group */
	return(pkt);
}	

/* ================================================================= */

static void 
set_map (cgp, cgbnr, allocsize)

struct cg *cgp;
word cgbnr, allocsize;

/*
*  Sets the appropriate bytes in the BitMap of zhe pointed cylinder group 
*  as occupied and updates the SummaryInfos in the InfoBlock.
*/

{
	word i;
	struct buf *bp;
	
	/* set the blocks as occupied in the bitmap */
	for (i=0;i<allocsize;i++)
		cgp->cg_free[cgbnr+i] = 0xff;

	/* update nr of free blocks in the cyl-group summary */
	cgp->cg_s.s_nbfree -= allocsize;

	/* update cyl-group-rotor to last allocated blk++ */ 
	cgp->cg_rotor = (cgbnr+allocsize) % maxbpg;
	
}	
	
/* ================================================================= */

static void 
sum_updateb (cgnr, allocsize)

word cgnr, allocsize;

/*
*  Updates the Incore SummaryInformation for allocated data blocks.
*/

{
	struct buf *bp;
	
	/* update free blocks in incore-file-system-summary */
	Wait (&incore_sum.is_fs.sem);
	incore_sum.is_fs.sum.s_nbfree -= allocsize;
	Signal (&incore_sum.is_fs.sem);	

	/* update free blocks in incore-cyl-group-summary */
	Wait (&incore_sum.is_cg[cgnr].sem);
	incore_sum.is_cg[cgnr].sum.s_nbfree -= allocsize;	
	Signal (&incore_sum.is_cg[cgnr].sem);

	/* update summary block info, depending on the incore summary flag */
        if ( incore_sum.is_sum_same )       
        {
		Wait (&incore_sum.is_fs.sem);
        	/* ... and it's incore counterpart */
        	incore_sum.is_sum_same = FALSE;
		Signal (&incore_sum.is_fs.sem);

        	/* read summary block from disc */
        	bp = bread ( filedrive,1,1,SAVEA );
        	/* reset the summary flag of the structure */
        	bp->b_un.b_sum->sum_same = FALSE;
        	/* write the summary block's package to disc */
        	bwrite ( bp->b_tbp );
        }
}

/* ================================================================= */

static void 
sum_updated (cgnr, allocsize)

word cgnr, allocsize;

/*
*  Updates the Incore SummaryInformation for directories.
*/

{
	struct buf *bp;
	
	/* update nr of dirs in incore-file-system-summary */
	Wait (&incore_sum.is_fs.sem);
	incore_sum.is_fs.sum.s_ndir += allocsize;
	Signal (&incore_sum.is_fs.sem);	

	/* update nr of dirs in incore-cyl-group-summary */
	Wait (&incore_sum.is_cg[cgnr].sem);
	incore_sum.is_cg[cgnr].sum.s_ndir += allocsize;	
	Signal (&incore_sum.is_cg[cgnr].sem);

	/* update summary block info, depending on the incore summary flag */
        if ( incore_sum.is_sum_same )       
        {
		Wait (&incore_sum.is_fs.sem);
        	/* ... and it's incore counterpart */
        	incore_sum.is_sum_same = FALSE;
		Signal (&incore_sum.is_fs.sem);

        	/* read summary block from disc */
        	bp = bread ( filedrive,1,1,SAVEA );
        	/* reset the summary flag of the structure */
        	bp->b_un.b_sum->sum_same = FALSE;
        	/* write the summary block's package to disc */
        	bwrite ( bp->b_tbp );
        }
}

/* ================================================================= */

static void 
i_update (ip, lgbnr, pkt)

struct incore_i *ip;
daddr_t lgbnr;
pkt_t pkt;

/*
*  Updates the address information of a file in the pointed Incore Inode
*  modified by the current allocation.
*/

{
	word i,pos;
	daddr_t bnr;
	
	struct buf *bp;
	
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
				if (i==pkt.bcnt) return;
				ip->ii_i.i_db[lgbnr+i] = pkt.bnr+i;  
/*
printf("blknr == %d is in addr == %d \n",lgbnr+i,pkt.bnr+i);
*/
			}
			return;
		} 

		/* if (single indirect data block) */
		if (lgbnr < MAXSIND) 
		{
#if DEBUG 
printf("	i_update : 	bp = bread (%d, %d, 1, SAVEB);\n",
ip->ii_dev, ip->ii_i.i_ib[0]);
#endif
			/* read single indirect block into buffer cache */
			bp = bread(ip->ii_dev,
				   ip->ii_i.i_ib[0],1,SAVEB);
			/* update the inode of direct data blocks */ 
			for (i=0;(lgbnr+i)<MAXSIND;i++) 
			{
				if (i==pkt.bcnt) 
					goto finish;
				*(bp->b_un.b_daddr + (lgbnr+i-MAXDIR)) 
						= pkt.bnr+i;
/*
printf("blknr == %d is in addr == %d \n",lgbnr+i,pkt.bnr+i);
*/
			}
#if DEBUG
printf("	i_update :	bwrite (0x%p);\n",bp->b_tbp);
#endif
			/* write indirect block out to disc */
			bwrite(bp->b_tbp);
			return;
		} 

		/* if (double indirect data block) */
		if (lgbnr < MAXDIND) 
		{ 
			/* calc position of single ind block */
			pos = (lgbnr-MAXSIND)/SIADDR;
			/* adjust indirect data blocknr */
			bnr = get_daddr(ip->ii_dev,
			   		ip->ii_i.i_ib[1],pos);
#if DEBUG 
printf("	i_update : 	bp = bread (%d, %d, 1, SAVEB);\n",
ip->ii_dev, bnr);
#endif
			/* read indirect block into buffer cache */
			bp = bread(ip->ii_dev,bnr,1,SAVEB);
			/* update the inode of direct data blocks */
			for(i=0;(lgbnr+i-MAXSIND)/SIADDR==pos;i++) 
			{
				if (i==pkt.bcnt) goto finish;
				*(bp->b_un.b_daddr + ((lgbnr+i-MAXSIND)%SIADDR))
						= pkt.bnr+i;
			}
#if DEBUG
printf("	i_update :	bwrite (0x%p);\n",bp->b_tbp);
#endif
			/* write out indirect block to disc */
			bwrite(bp->b_tbp);
			return;
		} 

		/* if (triple indirect data block) */
		if (lgbnr < MAXTIND) 
		{
IOdebug("	i_update : 	ERROR : TIND data blocks not implemented !");
		}

finish:

#if DEBUG
printf("	i_update :	bwrite (0x%p);\n",bp->b_tbp);
#endif
		/* write out indirect block to disc */
		bwrite(bp->b_tbp);
	}
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
		
	/* if (allocation of data blocks) */
	if (lgbnr>=0) 
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

#if DEBUG
printf("	alloc : 	pref = blkpref (0x%p, %d);\n",
ip,lgbnr);
#endif
		/* find out preferred blocknr respectively cyl-groupnr */
		pref = blkpref (ip, lgbnr);

		/* if (preferred block) */
		if (pref.bpref>0) 
		{
			/* calc cyl-groupnr */
			cgnr = btocg(pref.bpref);
			/* calc relative blocknr in this cyl-group */
			cgbpref = btocgb(pref.bpref);
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
		cgnr = btocg(ip->ii_dirbnr);
		/* calc relative blocknr close to dir entry */
		cgbpref = btocgb(ip->ii_dirbnr+1);
	}
	
	/* for (from (requested amount of blocks) downto (1) block) */
	for (ibcnt=bcnt;
	     ibcnt>0;
	     cgnr=btocg(iibnr),cgbpref=btocgb(iibnr),ibcnt=iibcnt) 

		/* for (all cylinder-groups) */	     
	    	for (iibcnt=0,i=0;i<maxncg;i++) 
	    	{
			/* modulo func : from (0) to (maxncg-1) */
	    		icgnr = (cgnr+i) % maxncg;
#if DEBUG
printf("	alloc :		bp = bread(%d, %d, 1, SAVEA);\n",
ip->ii_dev,cgtoib(icgnr));
#endif
			/* read info-block of cyl-group into buffer-cache */
	    		bp = bread(ip->ii_dev,cgtoib(icgnr),1,SAVEA);
			/* pointer to cyl-group struct */
			cgp = &bp->b_un.b_info->cgx;
#if DEBUG
printf("	alloc :		ipkt = map_search(0x%p, %d, %d);\n",
cgp,cgbpref,ibcnt);
#endif
			/* search in cyl-group bitmap for free blocks */
	    		ipkt = map_search(cgp,cgbpref,ibcnt);

			    /* get as much as possible in the pref cyl-group */
	    		if ((ibcnt==bcnt)&&(!i)&&(ipkt.bcnt>0)
			    /* OR get as much as requested in other cyl-grps */
	 		    ||(ipkt.bcnt==ibcnt)) 
	 		{
#if DEBUG
printf("	alloc :		set_map(0x%p, %d, %d);\n",
cgp,btocgb(ipkt.bnr),ipkt.bcnt);
#endif
				/* set in the bitmap the blocks as occupied */
	    		    	set_map (cgp,btocgb(ipkt.bnr),ipkt.bcnt);
#if DEBUG
printf("	alloc :		bwrite (0x%p);\n",bp->b_tbp);
#endif
				/* write out the info-block to disc */
	    		    	bwrite(bp->b_tbp);			
				/* update free blocks in incore-summary */
	    		    	sum_updateb(icgnr,ipkt.bcnt);
				/* if ((it's a directory file) */ 
				if ((ip->ii_i.i_mode == DIR)
				    /* AND (it's the first block of dir)) */	
				    && (ip->ii_i.i_db[0] == 0))
					/* update nr of dirs in incore-sum */
				    	sum_updated(icgnr,ipkt.bcnt);
				/* update the inode in incore-ilist */
	    		    	i_update(ip,lgbnr,ipkt);
				/* return (nr of allocated blocks) */
	    		    	return(ipkt);
	    		}

#if DEBUG
printf("	alloc :		bp = brelse (0x%p, TAIL);\n",bp->b_tbp);
#endif
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
IOdebug("	alloc : 	ERROR : FileSystem is full !!!");
fsfull = TRUE;
finish:
	ipkt.bcnt = 0;
	ipkt.bnr = 0;
	return(ipkt);
}
	    	
	    		     			
/* ================================================================= */

static void
free_map (cgp, bnr)

struct cg *cgp;
daddr_t bnr;

/*
*  Marks the freed blocks in the BitMap of the pointed cylinder group
*  as free and updates the SummaryInformations.
*/

{
	/* set block free in bitmap */
	cgp->cg_free[btocgb(bnr)] = 0;
	/* update nr of free blocks in info-block */
	cgp->cg_s.s_nbfree++;
	/* update nr of free blocks in incore-summary */
	sum_updateb(btocg(bnr),-1);
}

/* ================================================================= */

static void
tidyup_cache (dev, bnr)

dev_t dev;
daddr_t bnr;

{
	struct buf *bp, *lbp, *ibp;
	struct packet *tbp;
		
	if (fsfull)
		fsfull = FALSE;
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

void 
fREE (ip, flgbnr, dlgbnr)

struct incore_i *ip;
daddr_t flgbnr;
word dlgbnr;

/*
*  Gives the requested size blocks of the pointed file free,
*  updating the Incore Inode, the Incore Summary Informations
*  and the BitMaps of affected InfoBlocks.
*/ 

{
	daddr_t bnr,ibnr,iibnr;
	word i,j,cgnr,lcgnr=-1;
	struct cg *cgp;
	struct buf *bip,*biip,*ibp;	

/* define a macro called 'chg_info_blk' to write out the
   InfoBlock of the last cylinder-group and read the 
   InfoBlock of the current cylinder-group into Buffer Cache */
#	define chg_info_blk \
	if (lcgnr!=-1) \
		bwrite(ibp->b_tbp); \
	ibp = bread(ip->ii_dev,cgtoib(cgnr),1,SAVEB); \
	cgp = &ibp->b_un.b_info->cgx; \
	lcgnr = cgnr;
	
	/* if it's a directory file so its blocks are freed only before
	   deleting the directory, therefore update the Incore SummaryInfo */
	if (ip->ii_i.i_mode == DIR)
		sum_updated (btocg(ip->ii_i.i_db[0]),-1);
		
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
				cgnr = btocg(bnr);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif
					chg_info_blk
				}	    			
				/* update InfoBlock */
				free_map(cgp,bnr);
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
#if DEBUG
printf("	free :		bwrite (0x%p);\n",ibp->b_tbp); 
#endif
					bwrite(ibp->b_tbp);
				}		
				return;
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
#if DEBUG 
printf("	free :		bip = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,ibnr);
#endif
			/* read it into Buffer Cache */
			bip = bread(ip->ii_dev,ibnr,1,SAVEB);
		
			for(i=(flgbnr>MAXDIR)?flgbnr:MAXDIR;i<MAXSIND;i++) 
			{
				/* if there is an allocated data block */
				if (*(bip->b_un.b_daddr+i-MAXDIR)) 
				{
					/* find out its disc address */
					bnr = *(bip->b_un.b_daddr+i-MAXDIR);
					
					tidyup_cache (ip->ii_dev, bnr);
					
					/* and its cylinder group */
					cgnr = btocg(bnr);
					/* change InfoBlock if necessary */
					if (cgnr!=lcgnr) 
					{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif

						chg_info_blk
					}	    			
					/* update InfoBlock */
					free_map(cgp,bnr);
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
#if DEBUG
printf("	free :		bwrite (0x%p);\n",bip->b_tbp); 
#endif
				/* write it to disc */
				bwrite(bip->b_tbp);
			} 
			else 
			{
#if DEBUG
printf("	free :		brelse (0x%p, HEAD);\n",bip->b_tbp); 
#endif
				/* release the buffer containing
				   the empty IndirectBlock */
				if (checksum_allowed) {
					bip->b_checksum = checksum(bip->b_un.b_addr,BSIZE);
				}

				brelse(bip->b_tbp, HEAD);
				
				tidyup_cache (ip->ii_dev, ibnr);
				
				/* find out the cylinder group of IndBlock */
				cgnr = btocg(ibnr);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif
					chg_info_blk
				}	    			
				/* free the IndirectBlock */
				free_map(cgp,ibnr);
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
#if DEBUG
printf("	free :		bwrite (0x%p);\n",ibp->b_tbp); 
#endif
				bwrite (ibp->b_tbp);
			}
			return;
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
#if DEBUG
printf("	free :		biip = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,iibnr);
#endif
			/* read it into BufferCache */
			biip = bread(ip->ii_dev,iibnr,1,SAVEB);
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
#if DEBUG
printf("	free :		bip = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,ibnr);
#endif
			/* read it into Buffer Cache */
			bip = bread(ip->ii_dev,ibnr,1,SAVEB);
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
					cgnr = btocg(bnr);
					/* change InfoBlock if necessary */
					if (cgnr!=lcgnr) 
					{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif
						chg_info_blk
					}	    			
					/* update BitMap in InfoBlock */
					free_map(cgp,bnr);
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
#if DEBUG
printf("	free :		bwrite (0x%p);\n",bip->b_tbp); 
#endif
				/* write it to disc */
				bwrite(bip->b_tbp);
			} 
			else 
			{
#if DEBUG
printf("	free :		brelse (0x%p, HEAD);\n",bip->b_tbp); 
#endif
				/* release the buffer containing this IndBlock */
				if (checksum_allowed) {
					bip->b_checksum = checksum(bip->b_un.b_addr,BSIZE);
				}

				brelse(bip->b_tbp, HEAD);
				
				tidyup_cache (ip->ii_dev, ibnr);
				
				/* adjust the cyl-group of this IndBlock */
				cgnr = btocg(ibnr);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif
					chg_info_blk
				}	    			
				/* free this IndirectBlock */
				free_map(cgp,ibnr);
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
#if DEBUG
printf("	free :		bwrite (0x%p);\n",biip->b_tbp); 
#endif
				/* write it to disc */
				bwrite(biip->b_tbp);
			} 
			else 
			{
#if DEBUG
printf("	free :		brelse (0x%p, HEAD);\n",biip->b_tbp); 
#endif
				/* release the buffer containing
				   the double IndirectBlock */
				if (checksum_allowed) {
					biip->b_checksum = checksum(biip->b_un.b_addr,BSIZE);
				}

				brelse(biip->b_tbp, HEAD);

				tidyup_cache(ip->ii_dev,iibnr);
				
				/* adjust its cylinder group */
				cgnr = btocg(iibnr);
				/* change InfoBlock if necessary */
				if (cgnr!=lcgnr) 
				{
#if DEBUG
if (lcgnr!=-1)
	printf("	free :		bwrite (0x%p);\n",ibp->b_tbp);
printf("	free :		ibp = bread(%d, %d, 1, SAVEB);\n",
ip->ii_dev,cgtoib(cgnr));
#endif
					chg_info_blk
				}	    			
				/* free the double IndirectBlock */
				free_map(cgp,iibnr);
				/* update the Incore Inode */
				ip->ii_i.i_ib[1] = 0;
				ip->ii_i.i_blocks--;
			}
#if DEBUG
printf("	free :		bwrite (0x%p);\n",ibp->b_tbp); 
#endif
			/* write the InfoBlock out */
			bwrite(ibp->b_tbp);			
		}
	}	

/* undefine the macro to change InfoBlocks */
#	undef chg_info_blk
}

/* ==================================================================== */

/* end of alloc.c */



