/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/nfs.h,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: nfs.h,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:03:48  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.5  90/08/06  07:47:52  guenter
 * multivolume/multipartition
 * 
 * Revision 1.4  90/05/30  15:46:34  chris
 * Update function prototypes
 * 
 * Revision 1.3  90/02/01  17:37:33  chris
 * Tape support amongst other things
 * 
 * Revision 1.2  90/01/12  19:08:48  chris
 * Fix use of semaphores in make_obj
 * 
 * Revision 1.1  90/01/02  19:03:48  chris
 * Initial revision
 * 
 */

/*************************************************************************
**                                                                      **
**                   H E L I O S   F I L E S E R V E R                  **
**                   ---------------------------------                  **
**                                                                      **
**                 Copyright (C) 1988,1989 Parsytec GmbH                **
**                          All Rights Reserved.                        **
**                                                                      **
** nfs.h	  							**
**                                                                      **
**	General prototyping for the network file server and		**
**	file system checker.						**
**									**
**************************************************************************
** HISTORY  :								**
**-----------								**
** Author   :  21/03/89  H.J.Ermen 					**
*************************************************************************/


#ifndef	__nfs_h
#define __nfs_h

#define CHECKER		1

#include <helios.h>
#include <gsp.h>
#include <codes.h>
#include <protect.h>
#include <message.h>
#include <sem.h>
#include <nonansi.h>
#include <syslib.h>
#include "fservlib.h"
#include <device.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

/* Include the appropriate header files for the file server task	*/

#include "buf.h"		
#include "fs.h"
#include "inode.h"

#ifdef CHECKER
#include "check.h"
#endif

/*--------------  Some definitions for general use  --------------------*/

#define  FG_Private	0x000000f0	/* Function group in a private	*/
					/* protocol.			*/
					/* Some operation codes :	*/
#define  FO_Check       0x00000001	/* File system checking		*/
#define  FO_Terminate	0x00000002	/* File server termination	*/
#define  FO_Corrupt	0x00000003	/* File server corruption 	*/
#define  FO_Synch       0x00000004	/* Only use bwrite()		*/
#define  FO_Asynch	0x00000005	/* Use bwrite() and bdwrite()	*/
#define  FO_ForceSync   0x00000006      /* Force an immediate sync_fs()	*/
#define  FO_FSstatus	0x00000007	/* Produce an active-worker rep.*/
#define  FO_NoChanges   0x00000008	/* Check but don't correct errors*/
#define  FO_MakeFs	0x00000009	/* build structured filesystem  */
#define  FO_Unload	0x0000000a	/* unload volume		*/
#define  FO_Load	0x0000000b	/* load volume	 		*/
#define  FO_MakeSuper	0x0000000c	/* make new superblock		*/
#define  FO_Edit	0x0000000d	/* edit disk blocks		*/
#define  FO_Format	0x0000000f	/* format optical disk		*/

#define  FO_Debug	0x0000000E      /* Debug informations		*/


/*=======  GENERAL PROTOTYPES AND GLOBAL DECLARED VARIBALES  ===========*/

/*** THE SERVER ***/
 
/*-----------------------  defined in : bio.c   ------------------------*/

word		remove_free (struct packet *tbp);
struct packet  *remove_free_any (word psize);
struct buf     *getblk (dev_t dev, daddr_t bnr, word bcnt, word save);
struct buf     *bread (dev_t dev, daddr_t bnr, word bcnt, word save);
void		brelse (struct packet *tbp, word position);
void		bdwrite (struct packet *tbp);
word		bwrite (struct packet *tbp);
void		bawrite (struct packet *tbp);
struct buf     *bsearch_hash (dev_t dev, daddr_t bnr);
void		bremove_hash (struct buf *bp);

/*---------------------  defined in : inode.c  ------------------------*/

daddr_t		 get_daddr (dev_t dev, daddr_t bnr, word pos);
daddr_t		 bmap (struct incore_i *ip, uword byte_offset, word *error);
struct incore_i *iget (struct buf *bp, word offset, daddr_t parbnr,word parofs);
word		 iput (struct incore_i *ip);
dir_t		 searchi (struct incore_i *ip, string name, word len);
struct incore_i *namei (string pathname, VD *vol);
struct incore_i *iremove_free (void);
void		 iinsert_free (struct incore_i *ip);
void		 iremove_hash (struct incore_i *ip);
void		 iinsert_hash (struct incore_i *ip);

/*---------------------  defined in : alloc.c  ------------------------*/

void		 clr_buf (struct buf *bp);
pkt_t		 alloc (struct incore_i *ip, daddr_t lgbnr, word bcnt);
word		 fREE (struct incore_i *ip, daddr_t flgbnr, word dlgbnr);
void		 tidyup_cache (dev_t dev, daddr_t bnr);

/*-------------------  defined in : deal_fs.c  -------------------------*/

				/* Guardian to all hash-queues		*/
extern  Semaphore		bhash_sem;
				/* The heads of the hash-queues		*/
extern  struct bufhash		bufhash[BUFHSZ];
				/* Pointer to free-packet list		*/
extern  struct free_packet	*free_packet;
				/* Guardian to inode hash-queues	*/
extern  Semaphore		ihash_sem;
				/* Table of inode hash-queues		*/
/* OI 28 Oct 1991 */
#if 0
extern  struct i_hash		i_hash[IHSZ];
#else
extern  struct i_hash	       *i_hash;
#endif
				/* Head element of the free inode-list	*/
extern  struct free_ilist	free_ilist;

				/* The three packets areas		*/
extern  struct packet		*stbp, *mtbp, *htbp;
				/* The three buffer areas		*/
extern  struct buf		*sbp, *mbp, *hbp;
				/* The three data areas			*/
extern  struct block		*scbp, *mcbp, *hcbp;
				/* Pointer to free-ilist		*/
extern  struct incore_i		*incore_ilist;

				/* The S,M,H packet-sizes		*/
extern  word   psmal, pmedi, phuge; 
extern  word   maxpsz;		/* The maximal packet-size		*/
				/* # of S,M,H packets			*/
extern  word   pscnt, pmcnt, phcnt;
				
extern  word   maxnii;		/* Number of open files			*/
				
extern	VolDescriptor	*volume; /* global quick volume info 		*/

/* macros to convert fileserver blocksize to device addressing size 	*/
/* and vice versa							*/
extern	word	block_to_addr_shifts;	/* conversion shifting constant */
#if 0
#define addr_to_block(num)	(num)
#define block_to_addr(num)	(num)
#else
#define addr_to_block(num)	((num) >> block_to_addr_shifts)
#define block_to_addr(num)	((num) << block_to_addr_shifts)
#endif

/* function codes for edit_block() routine and editvol command */
#define	READ	1
#define	WRITE	2
#define QUIT	3


void		*load_devinfo (char *Path2DevInfo);
InfoNode	*find_info(void *devinfo, word type, char *name);
word		 init_info(FileSysInfo *fsi, DiscDevInfo *ddi);
word		 init_volume_info (VolDescriptor *curvol);

word		 memfree (void);	 /* returns free memory space */ 
word		 alloc_mem (void);	 /* memory allocation for server */
void		 init_buffer_cache (void); /* init buffer cache structures */
void		 init_incore_ilist (void); /* init incore inode list */
void		 clean_cache (VD *vol); /* clean cache from all blocks belonging to 'vol' */
void		 clean_inodelist (VD *vol); /* remove all inodes of volume from hash queue */
void		 free_root_inode (VD *vol); /* release root inode from ilist */
word		 init_fs (word full_check,word delete_hanging_links, VD *vol);
word		 make_fs (VD *vol);
word		 make_super (VD *vol);
void		 edit_block (MsgBuf *m, VD *vol);
void		 complete_super_block (struct fs *sbpt);
void		 sync_fs (void);
void		 sync_vol (VD *vol);

/*----------------------  defined in : fsyscall.c  ---------------------*/
									
void		 seek_file (MCB *m, struct incore_i *ip);
word		 setsize_file (struct incore_i *ip, uword file_size);
word		 read_file (MCB *m, struct incore_i *ip);
word		 write_file (MCB *m, struct incore_i *ip);


/*----------------------  defined in : fserver.c  ---------------------*/

extern	Semaphore	term_sem;
extern	Semaphore	checker_sem;
extern	word		checksum_allowed;

extern  word		checker_mode;
#define FULL		2		/* possible checker modes */
#define BASIC		1
#define NO		0

extern	FileSysInfo	*fsi;

extern void do_open (ServInfo *, VD *);
extern void do_create (ServInfo *, VD *);
extern void do_locate (ServInfo *, VD *);
extern void do_objinfo (ServInfo *, VD *);
extern void do_serverinfo (ServInfo *, VD *);
extern void do_delete (ServInfo *, VD *);
extern void do_rename (ServInfo *, VD *);
extern void do_link (ServInfo *, VD *);
extern void do_protect (ServInfo *, VD *);
extern void do_setdate (ServInfo *, VD *);
extern void do_refine (ServInfo *, VD *);
extern void do_closeobj (ServInfo *, VD *);


/*----------------------  defined in : fservlib.c  ---------------------*/

extern  Semaphore	sync_sem;

#define FORMAT_KEEP_OPEN	0x4711

/* status codes for makefs command */
#define	MAKE_OK		0
#define	MAKE_ERR	1
#define	MAKE_PROTECTED	2
#define	MAKE_HFS_ACTIVE	3
#define	MAKE_TAPE_OK	4
#define	MAKE_TAPE_ERR	5


word		 get_context (ServInfo *servinfo, VD *vol);
void		 fdispatch (VD *vol);
struct incore_i	*get_parent (ServInfo *servinfo,string pathname, VD *vol);
struct incore_i	*get_child  (string pathname, struct incore_i *iip);
struct incore_i *make_obj   (struct incore_i *iip, string pathname, 
			     word mode, string newname);
struct incore_i *get_target (ServInfo *servinfo);
struct incore_i *get_target_dir (ServInfo *servinfo, VD *vol);
struct incore_i *get_target_obj (ServInfo *servinfo, struct incore_i *iip, VD *vol);
void		 handle_link (struct incore_i *ip, ServInfo *servinfo);
word		 dir_server (struct incore_i *ip, struct incore_i *iip,
			     MCB *m, Port reqport);
void		 IOCRep1    (MsgBuf *r, MCB *m, struct incore_i *ip, word flags,
			     char *pathname);
void		 new_cap (Capability *cap, struct incore_i *ip, AccMask mask);
void		 marshal_ientry (MCB *m, struct incore_i *ip, char *name);
void		 marshal_dentry (MCB *m, struct dir_elem *dp, VD *vol);
word		 marshal_info (MCB *m, struct incore_i *ip);

void		 pathcat (string s1, string s2);
void 		 InvalidFn (ServInfo *servinfo);
void		 NullFn (ServInfo *servinfo);
void		 ErrorMsg (MCB *m, word err);
AccMask		 UpdMask (AccMask mask, Matrix matrix);
int		 CheckMask (AccMask mask, AccMask access);
word		 GetAccess (Capability *cap, Key key);
void		 crypt (Key key, byte *data, word size);

/*--------------  defined in : dev_driv.c / dev.c -----------------------*/

word		 open_dev (DiscDevInfo *);
void		 close_dev (void);
void		 getsize_dev (word partition, word *size, word *blocksize);
word 		 format_vol (VD *vol);
word 		 load_volume (VD *vol);
word 		 unload_volume (VD *vol);
word		 checksum (caddr_t b, word size);
word		 read_dev (struct packet *pt);
word		 write_dev (struct packet *pt);
word		 read_blk (void *bp, daddr_t bnr, VD *vol);
word		 read_blk2 (void *bp, daddr_t bnr, VD *vol);
word		 write_blk (void *bp, daddr_t bnr, VD *vol);
word		 write_blk2 (void *bp, daddr_t bnr, VD *vol);
word 		 do_dev_rdwr(word fn, word dev, word timeout,
				word size, word pos, char *buf, word *err );
word 		 do_dev_seek(word fn, word dev, word timeout, word size, word pos );

/*--------------  defined in : tserver.c --------------------------------*/

extern  void	tdispatch (VD *vol);

#define tserverSS 2000


#endif


/*------------------------- Debugging -----------------------------------*/

#define FLdebugMsgHdr(M)	FLdebug ("MsgHdr:\tDataSize (%x)\tContSize (%x)\tFlags (%x)\tDest (%x)\nReply (%x)\tFnRc (%x)", M.DataSize, M.ContSize, M.Flags, M.Reply, M.FnRc)
#define FLdebugMCB(Ptr2M)	{ \
					uword I; \
					FLdebugMsgHdr(Ptr2M->MsgHdr); \
					FLdebug ("Timeout (%x)\t", Ptr2M->Timeout); \
					for (I = 0; I < Ptr2M->MsgHdr.ContSize; ++I) \
						FLdebug ("C [%d] (%x)\t", I, Ptr2M->Control [I]); \
					for (I = 0; I < Ptr2M->MsgHdr.DataSize; ++I) \
						FLdebug ("D [%d] (%x\t", I, Ptr2M->Data [I]); \
 				 }

/* end of nfs.h */
