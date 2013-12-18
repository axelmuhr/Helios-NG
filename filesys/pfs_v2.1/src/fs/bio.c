/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/bio.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: bio.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  10:54:12  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.3  90/05/30  15:28:38  chris
 * BUFCSUM flag added where it should have
 * been.
 * 
 * Revision 1.2  90/02/01  17:35:10  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  20:00:10  chris
 * Initial revision
 * 
 * Revision 1.1  90/01/02  18:35:13  chris
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
   |  bio.c     						             |
   |                                                                         |
   |    Routines of buffer cache.                                            |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 30 April 1991 - Error handling centralized           |
   |    1 - A.Ishan   - 10 June  1988 - Basic version                        |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define	DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1
#include "error.h"

#define  PROCCNT 1
#include "proccnt.h"

#include "fserr.h"
#include "nfs.h"
#include "procname.h"

/* ================================================================= */

/* Local procedures */

static struct buf *binsert_hash (struct buf *);
static void pinsert_free (struct packet *, word);
static void premove_free (struct packet *);

/* ================================================================= */

word 
remove_free (tbp)

struct packet *tbp;

/*
 * Try to remove the pointed packet from the free_packet list.
 *
 * The packet you're going to remove is already blocked after 
 * finding it in the buffer_hash list.
 * 
 * It's possible that some other process has already removed
 * the packet from the free_packet list before you had blocked it.
 * In this case the packet will be released so that the process
 * having removed it can access the packet.
 * 
 * Return the following results :
 * 		PKTFND =  1 : packet found 
 * 		PKTNFD =  0 : packet not found
 * 		PKTDWT = -1 : packet blocked for device_write
 */

{	
	word result;
	struct packet	*ftbp,*itbp;

	/* wait for specific free_packet list */
	Wait ( &free_packet[tbp->p_size-1].fp_sem );

	/* initialize the pointer to the packet_header */
	ftbp = (struct packet *) &free_packet[tbp->p_size-1];	
	
	 /* while ((not end of list) & (not the pointed packet)) */
	for ( itbp = ftbp->p_next; (itbp != ftbp) && (itbp != tbp);
	      itbp = itbp->p_next )
		;

	/* if (the pointed packet is found) */
	if (itbp == tbp) 
	{
		/* dec the counter and remove packet from free_packet list */

/* OI 910429 */	--free_packet [tbp->p_size - 1].fp_count;

		premove_free (tbp);
		result = PKTFND;
	} 
	else 
	{
		/* if the pointed packet is blocked for device_write */
		if (tbp->p_dwt_cnt > 0) 
			result = PKTDWT;
		else 
			result = PKTNFD;
		/* release the packet cause other process
		   already removed it from free_packet list */
		Signal (&tbp->p_sem);
	}
	
	/* signal specific free_packet list */
	Signal ( &free_packet[tbp->p_size-1].fp_sem );
	return (result);
}	

/* ================================================================= */

struct packet *
remove_free_any (psize)

word psize;

/*
 * Try to remove any packet from the free_packet list with the given
 * packet_size. Free_packet is an array of header to double linked cyclic
 * list of packet_header. Each packet in the list has the same size.
 * Different lists contain packets with different sizes. The parameter
 * 'psize' is the index in the header array 'free_packet'.
 * This alpha version does not convert any packets across list boundaries.
 *
 * The packet you're going to remove isn't blocked yet, this
 * will be done after having removed it.
 *
 * There can be already blocked packets in the free_packet list
 * which aren't removed yet, cause the processes having blocked
 * them are waiting to access the free_packet list.
 *
 * Return the packet pointer if there was at least one packet in that list.
 * If the free_packet list was empty return NULL.
 */

{	
	struct packet	*tbp,*itbp;

	/* wait for specific free_packet list */
	Wait ( &free_packet[psize-1].fp_sem );

	/* initialize the pointer to the packet_header */
	tbp = (struct packet *) &free_packet[psize-1];	

	/* if (any free_packet is available) */

/* OI 910429 */	
	if (free_packet [psize - 1].fp_count > 0) 

	{
		/* while ((not end of list)
		               & (p_header already blocked)) */ 	
		for ( itbp = tbp->p_next; 
		      (itbp != tbp) && (TestSemaphore(&itbp->p_sem) <= 0);
		      itbp = itbp->p_next )
			;

		/* if (any unblocked free_packet found) */
		if (itbp != tbp) 
		{
			/* mark the available p_header */
			tbp = itbp;
			/* dec the counter and remove from free_packet list */
/* OI 910429 */			
			--free_packet [psize - 1].fp_count;

			premove_free (tbp);
		} 
		else 
		{ 
			/* NULL pointer */
			tbp = (struct packet *)NULL;
		}
	} 
	else 
	{ 
		/* NULL pointer */
		tbp = (struct packet *)NULL;	
	}
	
	/* signal specific free_packet list */
	Signal ( &free_packet[psize-1].fp_sem );
	return (tbp);
}	

/* ================================================================= */

daddr_t
swap_free (tbp, bnr, lbnr, save)

struct packet *tbp;
daddr_t bnr, lbnr;
word save;

/* 
 * Clear the packet which contents some of the blocks you're
 * going to map into the buffer_cache.
 * Returns -1 if write_dev fails.
 */
   
{	
	daddr_t nbnr;
	struct buf *lbp,*ibp;
	
	/* initialize pointer to the first block in the packet */
	ibp = tbp->p_fbp;
		
	/* calc the pointer to the last block in the packet */
	lbp = ibp + tbp->p_blk_cnt - 1;

	/* note the block number of the last block */
	nbnr = lbp->b_bnr;
	
	/* if ((the packet is marked as delayed_write) & 
	       ((it's a read_request) OR (there are more bytes
		than you're going to actualize))) */    
	if ( (tbp->p_dwt_cnt > 0) &&
	     ( (save==SAVEA)
	       ||(ibp->b_bnr<bnr)||(lbp->b_bnr>lbnr)
	       ||((save==SAVEF||save==SAVEB)&&(ibp->b_bnr==bnr))
	       ||((save==SAVEL||save==SAVEB)&&(lbp->b_bnr==lbnr)) ) ) 
	{  

DEBdebug ("	swap_free :	write_dev (0x%p);\n",tbp);

		/* synchron write the packet to device */
		if (write_dev (tbp) != Err_Null)
			return (-1);
	} 

	/* clear the buffer_headers of the packet */
	for (; ibp <= lbp; ibp++) 
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
	
	/* release the cleared packet & insert it at the HEAD
	   of the free_packet list */
	brelse (tbp, HEAD);
	return (nbnr);
	
}
			
/* ================================================================= */

struct buf *
getblk (dev, bnr, bcnt, save)

dev_t dev;
daddr_t bnr;
word bcnt, save;

/*
 * Look for the requested blocks if their in buffer_cache
 * and available .
 * If all of them are available in one packet, return this
 * packet with a pointer to the first requested block; 
 * else clear the contents of each packet including one or
 * more requested blocks.
 *
 * After having removed these blocks from the buffer_cache,
 * allocate a new packet from the free_packet list,
 * according to the number of the requested blocks.
 * 
 * Note that there are only three different sizes of the
 * packets in the alpha version. Therefore the number of 
 * the requested packets must be less equal than the 
 * maximal packet_size "phuge". 
 */  

{
	struct buf *bp, *lbp, *ibp;
	struct packet *tbp;
	daddr_t nbnr, lbnr, ibnr;
	word psize;
	
	IncPC (84);
	
	/* calc the number of the last requested block */
	lbnr = bnr + bcnt - 1;

loop:
	for (nbnr = bnr; nbnr <= lbnr; nbnr++) 
	{
		/* search for the block in the buffer_hash list */
		bp = bsearch_hash (dev, nbnr);
		/* if (block in the buffer_cache) */
		if (bp) 
		{
			/* note the packet_header of the block */
			tbp = bp->b_tbp;
			lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
retry:		
			/* block the packet */
			Wait (&tbp->p_sem); 
			/* if ((block_nr doesn't match the requested one)
			       OR (device_nr doesn't match)) */			
			if ((bp->b_bnr != nbnr) || (bp->b_dev != dev)) 
			{
				/* release the packet */
				Signal (&tbp->p_sem);
				continue;
			}
			/* remove this packet from the free_packet list */
			switch (remove_free (tbp)) 
			{
			/* packet is being written to device */
			case PKTDWT: 
				goto retry;
			/* packet is not found */
			case PKTNFD:
				break;
			/* packet is found */
			default:
				/* if (packet includes all the
				       requested blocks) */	
				if ((nbnr == bnr) && /*(bcnt <= tbp->p_blk_cnt))*/
					(nbnr+bcnt-1 <= lbp->b_bnr))
				{
					DecPC (84); return (bp);
				}
				else 
				{
					/* clear the packet */
					nbnr = swap_free(tbp, bnr, lbnr, save);
					if ( nbnr == -1 ) {
Error (FSErr [SwapPktFailed], volume [bp->b_dev].vol_name);
						Signal (&tbp->p_sem);
						PutMsg (&volume[bp->b_dev].unload_mcb);
						DecPC (84); return (NULL);
					}
				}
			}
		}
	}

/* Allocate a suitable packet from the free_packet list */

	/* note the packet_size */
	if (bcnt > pmedi) 
		psize = phuge;
	elif (bcnt > psmal) 
		psize = pmedi;
	else 
		psize = psmal;

	/* initialize pointer to the packet_header */
	tbp = (struct packet *)NULL;
	while (tbp == NULL) 
	{
		/* remove a packet from the free_packet list */
		tbp = remove_free_any (psize);
		/* if (no free_packets available) */
		if (tbp == NULL) 
		{
			/* wait for any free_packets */
			Wait (&free_packet[psize-1].fp_avail);
			for (nbnr=bnr; nbnr<=lbnr; nbnr++) 
			{
				bp = bsearch_hash (dev, nbnr);
				if (bp) {
					goto loop;
				}
			}		
		} 
		else 
		{
			/* block the removed packet */
			Wait (&tbp->p_sem);
			/* if (packet marked as delayed_write) */
			if (tbp->p_dwt_cnt > 0) 
			{
				/* asynchron_write to the device */
				Fork(bawriteSS,bawrite,4,tbp);
				/* get back to while loop */
				tbp = (struct packet *)NULL;
			}
		}
	}
	
	/* if (packet not cleared) */
	if (tbp->p_blk_cnt > 0) 
	{
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
	}

	/* set the block# in the buffer_headers
	   and insert them into buffer_hash lists */
	for (ibnr = bnr, ibp = tbp->p_fbp; ibnr <= lbnr;
	     ibnr++, ibp++) 
	{
	     	ibp->b_dev = dev;
	        ibp->b_bnr = ibnr;
	     	ibp = binsert_hash (ibp);
	     	if (!ibp) 
	     	{
			brelse (tbp, HEAD);	
	     		goto loop;
	     	}
	}
	/* set the packet_header block counter */
	tbp->p_blk_cnt = bcnt;
	DecPC (84); return (tbp->p_fbp);			
}						
					
/* ================================================================= */

struct buf *
bread (dev, bnr, bcnt, save)

dev_t dev;
daddr_t bnr;
word bcnt, save;

/*
 * Try to find the requested blocks in the buffer_cache,
 * allocating a new packet and actualizing its contents
 * if necessary.
 * Returns NULL if any error occurred during read.
 */

{
	struct buf *bp;

	IncPC (81);
	
	bp = getblk (dev, bnr, bcnt, save);
	if (!bp) {
Error (FSErr [GetBlockFailed], volume [dev].vol_name);
		DecPC (81); return (NULL);
	}
	if (!bp->b_vld) 
	{

FLdebug ("read_dev (0x%p);\n",bp->b_tbp);

		if ( read_dev (bp->b_tbp) )
		{
			DecPC (81); return (NULL);
		}
	}
	elif (checksum_allowed)
	{
		if( bp->b_checksum != checksum(bp->b_un.b_addr,BSIZE) )
		{
Error (FSErr [CheckSum], volume [dev].vol_name, bp->b_bnr);
			DecPC (81); return (NULL);
		}
	}
	DecPC (81); return (bp);
}

/* ================================================================= */
/* ================================================================= */


#include <module.h>
static char *procname(word *x)
{
  	x = (word *)(((word)x)&~3);
  	while( (*x & T_Mask) != T_Valid ) x--;
  	
  	switch( *x )
  	{
  	case T_Proc:
  		return ((Proc *)x)->Name;
  	case T_Module:
  	case T_Program:
  		return ((Module *)x)->Name;
        default:
                Error (FSErr [IllegalType], _FILE_);
  	}
}


void
brelse (tbp, position)

struct packet *tbp;
word position;

/*
 * Insert the packet into the free_packet list
 * and release the packet.
 *
 * If other processes are waiting for any available 
 *  free_packets, signal the first of them.
 */ 

{	
  	struct buf *lbp, *ibp;

	IncPC (83);

	Wait (&free_packet[tbp->p_size-1].fp_sem);

#if GEPDEBUG	
  	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
  	if (checksum_allowed) {
	  	for (ibp = tbp->p_fbp; ibp <= lbp; ibp++) 
  		{ 
	  		if (ibp->b_checksum != checksum(ibp->b_un.b_addr,BSIZE) )
  			  GEPdebug (" brelse() : released block %d has incorrect checksum!\n"
                                    "            called from %s", ,ibp->b_bnr, procname(((word **)&tbp)[-2]));
  		}
  	}
#endif

	pinsert_free (tbp, position);

/* OI 910429 */	
	++free_packet [tbp->p_size - 1].fp_count;
	
	Signal (&tbp->p_sem);
	
/* OI 910429 */
	if (TestSemaphore (&free_packet [tbp->p_size - 1].fp_avail) < 0) 
		Signal (&free_packet[tbp->p_size-1].fp_avail);
	
	Signal (&free_packet[tbp->p_size-1].fp_sem);
	
	DecPC (83);	
}

/* ================================================================= */

void
bdwrite (tbp)

struct packet *tbp;

/* 
 * Mark packet as delayed_write.
 *
 * Insert into free_packet list and
 * release the packet.
 */

{
	struct buf *lbp, *ibp;
	
	IncPC (82);
	
	tbp->p_vld_cnt = tbp->p_blk_cnt;
	tbp->p_dwt_cnt = tbp->p_blk_cnt;	
	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
	for (ibp = tbp->p_fbp; ibp <= lbp; ibp++) 
	{ 
		if (checksum_allowed) 
			ibp->b_checksum = checksum(ibp->b_un.b_addr,BSIZE);
		ibp->b_vld = TRUE;
		ibp->b_dwt = TRUE;
	}
	brelse (tbp, TAIL);
	DecPC (82);
}

/* ================================================================= */

word
bwrite (tbp)

struct packet *tbp;


/*
 * Synchron_write the packet to the device.
 *
 * Insert into free_packet list and
 * release the packet.
 * Returns FALSE if any write error occurred
 */

{
	struct buf *lbp, *ibp;
	
	tbp->p_vld_cnt = tbp->p_blk_cnt;
	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
	for (ibp = tbp->p_fbp; ibp <= lbp; ibp++) 
	{
		if (checksum_allowed)
			ibp->b_checksum = checksum(ibp->b_un.b_addr,BSIZE);
		ibp->b_vld = TRUE;
	}
FLdebug ("<write_dev");	
	if ( write_dev (tbp) != Err_Null)
	{
FLdebug (">write_dev, <brelse (!= Err_Null)");	
		brelse (tbp, TAIL);		
FLdebug (">brelse, return (FALSE) to %s", CalledBy (tbp));	
		return (FALSE);	
	}
	else 
	{
FLdebug (">write_dev, <brelse (== Err_Null)");	
		brelse (tbp, TAIL);
FLdebug (">brelse, return (TRUE) to %s", CalledBy (tbp));	
		return (TRUE);
	}
}


/* ================================================================= */

void
bawrite (tbp)

struct packet *tbp;

/*
 * Asynchron_write the packet to the device,
 * inserting into free_packet list and releasing
 * the packet afterwards.
 * The packet is already marked for delayed write.
 * The checksum is therefore correct and does
 * not need to be performed here.
 */
 
{
	if ( write_dev (tbp) != Err_Null ) {
Error (FSErr [AsynWriteFailed], volume [tbp->p_fbp->b_dev].vol_name);
		PutMsg(&volume[tbp->p_fbp->b_dev].unload_mcb);
	}
	brelse (tbp, HEAD);	
}

/* ================================================================= */
/*************************************************************************
**                                                                      **
**	Operations of buffer header. 					**
**                                                                      **
**	Author:  M.Clauss 7/6/88					**
**                                                                      **
*************************************************************************/
/* ================================================================= */

struct buf *
bsearch_hash (dev, bnr)

dev_t dev;
daddr_t bnr;

/*
 * Search for a bufferheader of block 'bnr' on device 'dev' in
 * the buffer cache.
 * Return a pointer to the found bufferheader or NULL if that
 * block is not cached.
 */
 
{	
	struct buf	*bp;
	struct bufhash	*hp;
	
	hp = BUFHASH (dev, bnr);

	/* wait for buffer hash */
	Wait (&bhash_sem);	
	
	/* search through cyclic hash queue */
	bp = hp->b_next;
	while ( bp != (struct buf *) hp )
	{
		if ( bp->b_bnr == bnr && bp->b_dev == dev ) {
			Signal (&bhash_sem);	
			return (bp);
		}
		bp = bp->b_next;
	}
	
	/* Signal buffer hash */
	Signal (&bhash_sem);	
	return ( (struct buf *)NULL );
}


	
/* ================================================================= */


static struct buf *
binsert_hash (bp)

struct buf	*bp;

/*
 * Insert the bufferheader into the according buffer hash queue (inserted
 * at the head). The hash function is computed from
 *	bp->b_bnr  and  bp->b_dev.
 */
 
{	
	struct bufhash	*hp;
	struct buf *ibp;
	
	/*
	 * Check bp->b_bnr and bp->b_dev for plausible values.
	 * LATER ON
	 */
	hp = BUFHASH (bp->b_dev, bp->b_bnr);
	
	/* wait for buffer hash */
	Wait (&bhash_sem);	

	/* search through cyclic hash queue */
	ibp = hp->b_next;
	while ( ibp != (struct buf *) hp )
	{
		if ( ibp->b_bnr == bp->b_bnr && ibp->b_dev == bp->b_dev )
			break;
		ibp = ibp->b_next;
	}

	if ( (ibp != (struct buf *)hp) &&
	     (ibp->b_bnr == bp->b_bnr) && (ibp->b_dev == bp->b_dev) ) 
	{
		/* buffer meanwhile in hash queue */
		Signal (&bhash_sem);
		return ( (struct buf *)NULL );
	}


	/* insert bufferheader 'bp' at head of hash queue 'hp' */
 	bp->b_next = hp->b_next;
	bp->b_prev = (struct buf *)hp;
	
	hp->b_next->b_prev = bp;
	hp->b_next = bp;
	
	Signal (&bhash_sem);
	return (bp);
}


/* ================================================================= */

void 
bremove_hash (bp)

struct buf	*bp;

/*
 * Remove the bufferheader 'bp' from its hash queue
 */
 
{
	/*
	 * LATER : Check whether bufferheader is in any hash queue.
	 * The bufferheader is not in a hash queue iff
	 *
	 *  bp == bp->b_next	AND
	 *  bp == bp->b_prev
	 */
	 
	/* wait for buffer hash */
	Wait (&bhash_sem);	

	/* remove bufferheader from hash queue */
	bp->b_prev->b_next = bp->b_next;
	bp->b_next->b_prev = bp->b_prev;
	
	/* mark bufferheader  'not in cache' */
	bp->b_next = bp;
	bp->b_prev = bp;
	
	/* signal buffer hash */
	Signal (&bhash_sem);	
}


/* ================================================================= */

/* packet handling routines */

static void 
pinsert_free (tbp, position)

struct packet	*tbp;
word	position;	/* possible values HEAD or TAIL */

/*
 * Insert the packet into the according free_packet list at the
 * given position.
 * This routines assumes that the free_packet list is already locked
 */

{	
	/* LATER : Check valid tbp->p_size_index and position (HEAD/TAIL)
	 */
	struct free_packet	*php;
	
	/* set php to head of according free_packet list */
	php = &free_packet[tbp->p_size-1];
	
	if (position == HEAD ) 
	{
		/* insert packet at head of free_packet list */
		php->p_next->p_prev = tbp;
		tbp->p_next = php->p_next;
		php->p_next = tbp;
		tbp->p_prev = (struct packet *) php;
	} 
	else 
	{
		/* insert packet at tail of free_packet list */
		php->p_prev->p_next = tbp;
		tbp->p_prev = php->p_prev;
		php->p_prev = tbp;
		tbp->p_next = (struct packet *)php;
	}
}

/* ================================================================= */

static void 
premove_free (tbp)

struct packet	*tbp;

/*
 * remove this packet from its free_packet list.
 * Note: All free_packet lists must have been locked !
 *
 */
 
{
	/* LATER : Perhaps check whether this packet is in any freelist
	*/
	tbp->p_prev->p_next = tbp->p_next;
	tbp->p_next->p_prev = tbp->p_prev;
}

/*
	USE MACRO definition of this function !!!

#define	premove_free(tbp) \ 
{ \
	(tbp)->p_prev->p_next = (tbp)->p_next; \
	(tbp)->p_next->p_prev = (tbp)->p_prev; \
}
*/
/* ================================================================= */

/* end of bio.c */
