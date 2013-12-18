/* $Header: /giga/HeliosRoot/Helios/filesys/fs/RCS/nfs.h,v 1.2 1991/03/21 15:16:18 nick Exp $ */

/* 15.08.90 - replaced read_info_blk() and write_info_blk() by read_blk()
 *	      and write_blk()
 *
 */
 
/* $Log: nfs.h,v $
 * Revision 1.2  1991/03/21  15:16:18  nick
 * New source from Parsytec
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
/*
#include "check.h"
*/

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

/*=======  GENERAL PROTOTYPES AND GLOBAL DECLARED VARIBALES  ===========*/

/*** THE SERVER ***/
 
/*-----------------------  defined in : bio.c   ------------------------*/

word		remove_free (struct packet *tbp);
struct packet  *remove_free_any (word psize);
struct buf     *getblk (dev_t dev, daddr_t bnr, word bcnt, word save);
struct buf     *bread (dev_t dev, daddr_t bnr, word bcnt, word save);
void		brelse (struct packet *tbp, word position);
void		bdwrite (struct packet *tbp);
void		bwrite (struct packet *tbp);
void		bawrite (struct packet *tbp);
struct buf     *bsearch_hash (dev_t dev, daddr_t bnr);
void		bremove_hash (struct buf *bp);

/*---------------------  defined in : inode.c  ------------------------*/

daddr_t		 get_daddr (dev_t dev, daddr_t bnr, word pos);
daddr_t		 bmap (struct incore_i *ip, uword byte_offset);
struct incore_i *iget (struct buf *bp, word offset, daddr_t parbnr,word parofs);
void		 iput (struct incore_i *ip);
dir_t		 searchi (struct incore_i *ip, string name, word len);
struct incore_i *namei (string pathname);
struct incore_i *iremove_free (void);
void		 iinsert_hash (struct incore_i *ip);

/*---------------------  defined in : alloc.c  ------------------------*/

void		 clr_buf (struct buf *bp);
pkt_t		 alloc (struct incore_i *ip, daddr_t lgbnr, word bcnt);
void		 fREE (struct incore_i *ip, daddr_t flgbnr, word dlgbnr);

/*-------------------  defined in : deal_fs.c  -------------------------*/

				/* Guardian to all hash-queues		*/
extern  Semaphore		bhash_sem;
				/* The heads of the hash-queues		*/
extern  struct bufhash		bufhash[BUFHSZ];
				/* Pointer to free-packet list		*/
extern  struct free_packet	*free_packet;
				/* Incore copy of the summary infos	*/
extern  struct incore_s		incore_sum;
				/* Incore copy of the superblock	*/
extern  struct fs		incore_fs;
				/* Guardian to inode hash-queues	*/
extern  Semaphore		ihash_sem;
				/* Table of inode hash-queues		*/
extern  struct i_hash		i_hash[IHSZ];
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
extern  word   minfree;		/* Percentage of disk-space to be kept	*/
				/* free.				*/
extern  word   maxbpg;		/* Number of blocks per cg		*/
extern  word   maxncg;		/* Number of cylinder groups		*/
extern  word   cgoffs;		/* Relative offset into each cg		*/
extern  char   fs_name[32];	/* The name of the root`s node		*/

extern word filedrive;		/* drive for the filing system.		*/
void		*load_devinfo(void);
InfoNode	*find_info(void *devinfo, word type, char *name);
void		 init_info(FileSysInfo *fsi, DiscDevInfo *ddi);
void		 init_buffer_cache (void);
void		 init_incore_ilist (void);
word		 init_fs (FileSysInfo *fsi, DiscDevInfo *ddi, int volnum);
word		 make_fs (FileSysInfo *fsi, DiscDevInfo *ddi,int volnum);
word		 make_super (FileSysInfo *fsi, DiscDevInfo *ddi,int volnum);
void		 complete_super_block (struct fs *sbpt);
void		 sync_fs (void);
void		 term_fs (void);
void 		 init_free_list (word, word, struct packet *, struct buf *, struct block *);
void 		 init_incore_fs (void);
word 		 alloc_mem (void);

/*----------------------  defined in : fserver.c  ---------------------*/
extern word		checksum_allowed;
extern FileSysInfo 	*fsi;
extern DiscDevInfo 	*ddi;	
extern VolumeInfo 	*vvi;

/*----------------------  defined in : fsyscall.c  ---------------------*/
extern bool	fsfull;
extern jmp_buf  close_jmp;
									
void		 seek_file (MCB *m, struct incore_i *ip);
void		 setsize_file (struct incore_i *ip, uword file_size);
void		 read_file (MCB *m, struct incore_i *ip);
void		 write_file (MCB *m, struct incore_i *ip);

/*----------------------  defined in : fservlib.c  ---------------------*/

extern  Semaphore	sync_sem;
extern	Port		term_port;
extern  word		syncop;

extern jmp_buf  term_jmp;

word		 get_context (ServInfo *servinfo);
void		 fdispatch (DispatchInfo *info);
struct incore_i	*get_parent (ServInfo *servinfo, string pathname);
struct incore_i	*get_child  (string pathname, struct incore_i *iip);
struct incore_i *make_obj   (struct incore_i *iip, string pathname, 
			     word mode, string newname);
struct incore_i *get_target (ServInfo *servinfo);
struct incore_i *get_target_dir (ServInfo *servinfo);
struct incore_i *get_target_obj (ServInfo *servinfo, struct incore_i *iip);
void		 handle_link (struct incore_i *ip, ServInfo *servinfo);
void		 dir_server (struct incore_i *ip, struct incore_i *iip,
			     MCB *m, Port reqport);
void		 IOCRep1    (MsgBuf *r, MCB *m, struct incore_i *ip, word flags,
			     char *pathname);
void		 new_cap (Capability *cap, struct incore_i *ip, AccMask mask);
void		 marshal_ientry (MCB *m, struct incore_i *ip, char *name);
void		 marshal_dentry (MCB *m, struct dir_elem *dp);
void		 marshal_info (MCB *m, struct incore_i *ip);

void		 pathcat (string s1, string s2);
void 		 InvalidFn (ServInfo *servinfo);
void		 NullFn (ServInfo *servinfo);
void		 ErrorMsg (MCB *m, word err);
AccMask		 UpdMask (AccMask mask, Matrix matrix);
int		 CheckMask (AccMask mask, AccMask access);
word		 GetAccess (Capability *cap, Key key);
void		 crypt (Key key, byte *data, word size);

/*--------------  defined in : dev_driv.c / dev.c -----------------------*/

void		 open_dev (DiscDevInfo *);
void		 close_dev (void);
word		 checksum (caddr_t b, word size);
void		 read_dev (struct packet *pt);
void		 write_dev (struct packet *pt);
word		 read_blk (void *bp, daddr_t bnr);
word		 write_blk (void *bp, daddr_t bnr);
word 		 do_dev_rdwr(word fn, word dev, word timeout,
				word size, word pos, char *buf, word *err );
word 		 do_dev_seek(word fn, word dev, word timeout, word pos );

/*--------------  defined in : tserver.c --------------------------------*/

extern  void	tserver(void);
extern	word	tapedrive;
extern  char    tape_name[32];

#define tserverSS 2000

#endif

/* end of nfs.h */
