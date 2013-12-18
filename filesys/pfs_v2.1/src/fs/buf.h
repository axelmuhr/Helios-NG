/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/buf.h,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: buf.h,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:03:37  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.2  90/02/01  17:37:45  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:50  chris
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
** buf.h								**
**                                                                      **
**	Definitions of Buffer Cache structures.				**
**									**
**************************************************************************
** HISTORY  :             						**
**-----------                                                           **
** Author   :  7/07/88  A.Ishan						**
** Modified : 20/03/89  H.J.Ermen  - Note: Prototypes can be found in	**
**					   the general header "nfs.h"	**
**	      24/03/89  H.J.Ermen  - Full Parametrization		**
*************************************************************************/


#ifndef	__buf_h
#define __buf_h

#ifndef	__inode_h
#  include "inode.h"
#endif

#ifndef daddr_t
#  define daddr_t word		/* disc address type */
#endif

#ifndef dev_t
#  define dev_t word		/* device type */
#endif

typedef char *caddr_t;		/* core address type */


struct buf {	/* buffer header */
	struct buf     *b_next;
	struct buf     *b_prev;
	daddr_t		b_bnr;	/* blocknr on disc	*/
	dev_t		b_dev;	/* device		*/
	word		b_vld;  /* flag for valid buffer */
	word 		b_dwt;  /* flag for delayed_write buffer */
	word		b_pnr;  /* offset in the packet */ 
	union {
		caddr_t b_addr;
		struct dir_elem *b_dir;
		struct sum_blk *b_sum;
		struct info_blk *b_info;
		struct link_blk *b_link;
		daddr_t *b_daddr;
	} 		b_un;	
	struct packet  *b_tbp;	/* pointer to packet_header */
	word	b_checksum;
};

struct link_blk {
	char 		name[IOCDataMax];
	Capability	cap;
};

struct bufhash {
	struct buf 	*b_next;
	struct buf 	*b_prev;
};

#define	BUFHSZ	512

/* buffer hash function */	
#if	((BUFHSZ&(BUFHSZ-1)) == 0)
#define	BUFHASH(dev, blkno)	\
	(&bufhash[((dev)+(blkno))&(BUFHSZ-1)])
#else
#define	BUFHASH(dev, blkno)	\
	(&bufhash[((dev)+(blkno)) % BUFHSZ])
#endif

/* packetheader 
 *
 * A packet is the basic unit for disc transfer. This header describes
 * a contigous set of bufferheader which themself describe the
 * contigous buffers.
 */ 
struct packet {
	struct packet	*p_next;
	struct packet	*p_prev;
	Semaphore	 p_sem;
	word		 p_size;	/* index into free_packet */
	word 		 p_blk_cnt;     /* # of blocks in packet */
	word		 p_vld_cnt;	/* # of valid blocks */
	word 		 p_dwt_cnt;	/* # of delayed_write blocks */
	struct buf	*p_fbp;		/* ptr to the first buffer_header */
};

struct free_packet {
	struct packet	*p_next;
	struct packet	*p_prev;
	Semaphore	 fp_sem;	/* guardian of this free_packet list */
/* OI 910429 */	
	word		 fp_count;	/* ctr of free_p                     */
	Semaphore	 fp_avail;	/* to signal waiting procs           */
};

/* results of bio.c/remove_free(tbp) function */
#define PKTFND  	1
#define PKTNFD  	0
#define PKTDWT 	       -1

/* position values */
#define HEAD	1
#define TAIL	2

/* device_write mode */
#define SYNCHR 	1
#define ASYNCH 	0

/* swapping modes */
#define NOSAVE	0
#define SAVEF	1
#define SAVEL	2
#define SAVEB	3
#define SAVEA	4

/* stack size of bio.c/bawrite routine */
#define bawriteSS 	1000

#endif

/* end of buf.h */

