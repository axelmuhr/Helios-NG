/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/dev.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: dev.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.2  90/09/05  07:05:04  guenter
 * format_vol added
 * 
 * Revision 2.1  90/08/31  10:53:56  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.5  90/08/08  07:58:47  guenter
 * ParentDir of RootDir is no more RootDir itself
 * 
 * Revision 1.4  90/08/06  07:47:12  guenter
 * multivolume/multipartition
 * 
 * Revision 1.3  90/05/30  15:36:05  chris
 * All sorts of things
 * Including passing back of device error code
 * 
 * Revision 1.2  90/02/01  17:36:04  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:02  chris
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
   |  dev.c								     |
   |                                                                         |
   |    Generic device interface code for file server.                       |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - O.Imbusch - 11 December 1991 - do_dev_seek: parameter "size"     |
   |                                       added to meet CF's drivers's      |
   |                                       conditions                        |
   |    1 - NHG       - 15 February 1989 - Basic version                     |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG    0
#define IN_NUCLEUS 1

#include "error.h"

#include "fserr.h"
#include "nfs.h"
#include "procname.h"

#include <device.h>

word open_dev (DiscDevInfo *ddi);
void close_dev (void);
void getsize_dev (word partition, word *size, word *blocksize);
word format_vol (VD *vol);
word do_dev_seek(word fn, word dev, word timeout, word size, word pos);
word load_volume (VD *vol);
word unload_volume (VolDescriptor *volume);
word read_dev (struct packet *);
word read_blk (void *bp, daddr_t bnr, VD *vol);
word read_blk2 (void *bp, daddr_t bnr, VD *vol);
word write_dev (struct packet *);
word write_blk (void *bp, daddr_t bnr, VD *vol);

DCB *discdcb = NULL;

word open_dev  (DiscDevInfo *ddi)
{
	if( (discdcb = OpenDevice(RTOA(ddi->Name),ddi)) == NULL) 
		return FALSE;		/* this means ok	*/
	else
		return TRUE;		/* this means error 	*/
	
}


void close_dev (void)
{
	CloseDevice(discdcb);
}


void dev_action(DiscReq *req)
{
	Signal(&req->WaitLock);
}

#define do_req(req)  InitSemaphore(&((req)->WaitLock),0);  \
			Operate(discdcb,(req));            \
			FLdebug ("Wait req");              \
			Wait(&((req)->WaitLock));          \
			FLdebug ("Waited req");




void getsize_dev (word partition, word *size, word *blocksize)

/*
 *  read size of requested partition 
 *   in req.Actual the blocksize of the MSC is returned (parameter addressing)
 *   in req.DevReq.result the number of blocks of the device is returned in MSC
 *    blocksize
 */

{
	DiscReq		req;
	
	req.DevReq.Request = FG_GetSize;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = partition;
	req.DevReq.Timeout = -1;
	
	do_req(&req);
		
	if (req.DevReq.Result < 0) {
		*size = 0;
		*blocksize = 0;	
	}
	else {
		*size = req.DevReq.Result;
		*blocksize = req.Actual;	
	}

GEPdebug (" getsize_dev : got size %d of partition %d, addressing %d",*size,partition,*blocksize);

	return;	
}


word format_vol (VD *vol)

/*
 *   format a volume with one partition physically
 */
 
{
	DiscReq		req;
	

GEPdebug (" format_dev : format volume /%s",vol->vol_name);


	if (vol->num_of_parts != 1)
	{
		Error (FSErr [OnePart2Formt], vol->vol_name);
		return (FALSE);
	}
	
	req.DevReq.Request = FG_Format;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = vol->logic_partition[0].partnum;
	req.DevReq.Timeout = -1;
	
	do_req(&req);	
	
	if (req.DevReq.Result < 0) {
		return (FALSE);		
	}

GEPdebug (" format_dev : volume /%s formatted",vol->vol_name);

	return (TRUE);		
}


word do_dev_seek(word fn, word dev, word timeout, word size, word pos)
{
	DiscReq req;

	req.DevReq.Request 	= fn;
	req.DevReq.Action 	= dev_action;
	req.DevReq.SubDevice 	= dev;
	req.DevReq.Timeout 	= timeout;
	req.Size 		= size;
	req.Pos			= pos;

	GEPdebug (" do_dev_seek() : function %d,subdevice = %d, no. %d",
				fn,req.DevReq.SubDevice,pos);

	do_req(&req);
	return req.DevReq.Result;
}


word do_dev_rdwr(word fn, word dev, word timeout, word size, word pos,
				char *buf, word *deverror )
{
	DiscReq req;

	req.DevReq.Request = fn;
	req.DevReq.Action = dev_action;
	req.DevReq.SubDevice = dev;
	req.DevReq.Timeout = timeout;
	req.Size = size;
	req.Pos  = pos;
	req.Buf = buf;

GEPdebug ("function 0x%x, subdevice = %d, no. %d, size %d", fn, req.DevReq.SubDevice, pos, size);

	do_req (&req);

	*deverror = req.DevReq.Result;

/*FIXME*/
FLdebug (">do_req, deverror (%x)", *deverror);

	return req.Actual;
}

word checksum(caddr_t b, word size)
{
	int i;
	word *wb = (word *)b;
	word wsize = size/4;
	word r = 0;

	for(i = 0; i < wsize; i++)
		r += wb[i];
	return r;
}


word load_volume (VD *vol)

/* 
 *   Tries to load volume pointed to by *volume;
 *   Returns a GSP error code if some error occurred having nothing to do with
 *   device handling, e.g. memory allocation Failure.
 *   Returns FALSE if no error occurred and TRUE if errors having to do with
 *   device handling.
 *   In the structure load_result the status of the device dependent operations
 *   is returned.
 *   If the volume consists of more than one partition (which is only allowed
 *   for structured and not loadable devices) then all load_results have to
 *   be exactly the same; otherwise an error is returned.
 * Parameter:	pointer to volume info
 * Return   :	GSP error code,TRUE or FALSE,
 *              volume info parameter filled in
 */

{
	word		partnum,num_of_parts;
	word		first_time,error;	
	load_res_type	save_result,last_result;

	num_of_parts = vol->num_of_parts;
	
	save_result.error = FALSE;
	save_result.raw = FALSE;
	save_result.loadable = FALSE;
	save_result.loaded = FALSE;
	save_result.writeprotected = FALSE;
	save_result.hard_formatted = TRUE;
	save_result.med_removable = FALSE;

	first_time = TRUE;

FLdebug ("Loading volume %d with %d partition(s)",vol->volnum,num_of_parts);

	for (partnum = 0; partnum < num_of_parts; partnum++) {
		do_dev_rdwr (FG_Open,
			     vol->logic_partition[partnum].partnum,
			     -1,
			     0,		/* not needed here */
			     0, 	/* not needed here */
			     (char *)&last_result,  /* points to result struct */
			     &error);

FLdebug ("FG_Open of partition %d performed returned:\n"
         "error (%d), raw (%d), loadable (%d), loaded (%d)\n"
         "writeprotected (%d), hard_formatted (%d)\n"
         "medium_removable (%d)\n"
         "GSP error code %x",
                              partnum,
                              last_result.error,
                              last_result.raw,
                              last_result.loadable,
                              last_result.loaded,
                              last_result.writeprotected,
                              last_result.hard_formatted,
                              last_result.med_removable,
                              error);

		if (error < 0 )   /* volume cannot be loaded */
		{
			Error (FSErr [FGOpenGSP], vol->volnum, partnum, error);
			return (error);
		}

		if (last_result.error)   /* if any device error occurs, skip */
			save_result.error = TRUE;

		if ( first_time ) {
			first_time = FALSE;
			save_result.loaded = last_result.loaded;
			save_result.raw = last_result.raw;
			save_result.loadable = last_result.loadable;
		}
		else {
			if ( save_result.raw || save_result.loadable )
			{
				Error (FSErr [MustB1Part], vol->volnum);
				save_result.error = TRUE;
			}
			else if ( save_result.raw != last_result.raw )
			{
				Error (FSErr [RawStructMix], vol->volnum);
				save_result.error = TRUE;
			}	
			else if ( save_result.loadable != last_result.loadable )
			{
				Error (FSErr [LoadUnloadMix], vol->volnum);
				save_result.error = TRUE;
			}	
			else if ( last_result.loaded != save_result.loaded )
			{
				Error (FSErr [DiffLoadStat], partnum, vol->volnum, last_result.loaded, save_result.loaded);
				save_result.error = TRUE;
			}
		}
		if ( !last_result.hard_formatted )
			save_result.hard_formatted = FALSE;
		if ( last_result.writeprotected )
			save_result.writeprotected = TRUE;
					
	}

	if ( save_result.error )
		return (TRUE);
		
	vol->raw = save_result.raw;
	vol->loaded = save_result.loaded;
	vol->loadable = save_result.loadable;
	vol->hard_formatted = save_result.hard_formatted;
	vol->writeprotected = save_result.writeprotected;
	vol->size_known = FALSE;
	vol->filesystem_found = FALSE;
	vol->unload_msg_allowed = TRUE;
				
	return (FALSE);	
}



word unload_volume (VD *vol)

/*
 *  Return : TRUE if ok, FALSE else
 */

{
	word	partnum,num_of_parts;
	word	error;	
	word	return_status;

	vol->sync_allowed = FALSE;
	vol->loaded = FALSE;
	return_status = TRUE;
			
	num_of_parts = vol->num_of_parts;
	for (partnum = 0; partnum < num_of_parts; partnum++) {
		if ( vol->loadable ) {

GEPdebug (" unload_volume() : Unload loadable volume /%s ",vol->vol_name);

			do_dev_rdwr (FG_Close,
				     vol->logic_partition[partnum].partnum,
				     -1,
				     0,		/* not needed here */
				     0, 	/* not needed here */
				     NULL,      /* not needed here */
				     &error);
			if ( error < 0 ) {  /* volume cannot be unloaded */
				Error (FSErr [UnloadFailed], vol->volnum, error);
				return_status = FALSE;
			}
		}
	}			
	return (return_status);		
}


word read_dev (struct packet *tbp)

/* Returns GSP error code if read request fails */

{
	char		*buffer;
	daddr_t		bnr;
	word		err;
	word    	bcnt, realbcnt, restbcnt, done;
	word		curpart;
	struct buf 	*lbp, *ibp;
	VolDescriptor	*curvol;
	word		reassigned_block = Err_Null;
	
	curvol = &volume[tbp->p_fbp->b_dev];

	/* first logical block # */
	bnr  = tbp->p_fbp->b_bnr;	
	/* # of blocks to transfer */
	bcnt = tbp->p_blk_cnt;		
	/* pointer to buffer */
	buffer = (char *)tbp->p_fbp->b_un.b_addr;		

	if ( (bnr < 0) || ( (bnr+bcnt) > curvol->bpcg * curvol->cgs)
	   ||
	     (bcnt <= 0) || (bcnt > phuge) )
	{
          Error (FSErr [InvalidParRd], curvol->vol_name, bcnt, bnr, curvol->bpcg, curvol->cgs);

	  curvol->sync_allowed = FALSE;
          return (EC_Fatal + EG_Parameter + EO_Server);
	}

	/* search linear for right logic partition */
	curpart = 0;
	while ( (bnr = bnr - curvol->logic_partition[curpart].size) >= 0)
		curpart++;
	/* adjust blocknumber to current partition */
	bnr += curvol->logic_partition[curpart].size;

GEPdebug ("<Read: pos %d, size %d, log_part %d",bnr,bcnt,curpart);


	/* blocks are read in a loop */
	while ( bcnt != 0 ) {
		if ( (restbcnt = bcnt + bnr - 
		     curvol->logic_partition[curpart].size) > 0 ) {
			realbcnt = bcnt - restbcnt;
		}
		else {
			realbcnt = bcnt;	
		}

		done = addr_to_block (
			do_dev_rdwr(FG_Read, 
				    curvol->logic_partition[curpart].partnum,
				    -1,
				    block_to_addr(realbcnt),
				    block_to_addr(bnr), 
				    buffer,
				    &err)
			);

		/* if couldn't succeed */
		if ( (done != realbcnt) || err ) 
		{
			if ( err == EC_Recover + SS_IOProc + EO_Medium ) 
				/* block successfully reassigned */	
				reassigned_block = err;
			else                     /* read error */
			{
				Error (FSErr [MSCRead], err, curvol->vol_name, realbcnt, bnr, done);

				curvol->sync_allowed = FALSE;
				return (err);
			}
		} 

GEPdebug (">Read: %d blocks from %d", realbcnt, bnr);

		curpart++;
		bnr = 0;
		buffer = buffer + done * BSIZE; /* increment buffer pointer */
		bcnt = bcnt - done;		
	}
	/* update valid blocks counter in the packet header */
	tbp->p_vld_cnt = tbp->p_blk_cnt;
	/* set the valid flags in the buffer headers */
	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
	for (ibp=tbp->p_fbp; ibp<=lbp; ibp++) {
		if (checksum_allowed)
			ibp->b_checksum = checksum(ibp->b_un.b_addr,BSIZE);
		ibp->b_vld = TRUE;
	}

	if (err)
	{
GEPdebug ("return (%d) (err)", err);
 		return (err);
	}
	else
	{
GEPdebug ("return (%d) (evtl. reassign) to %s", reassigned_block, CalledBy (tbp));
		return (reassigned_block);
				/* can be Err_Null  		   	     */
				/*        EC_Recover + SS_IOProc + EO_Medium */
				/*	  (means successfully reassigned block ) */
	}
}


word read_blk (void *bp, daddr_t bnr, VD *vol)

/*
 * Read block directly by avoiding the use of the buffer-cache
 * This routine has to be used in init_fs() before init_volume_info() has
 * been called and can only read from the first partition of these volume
 */

{
	word	err,curpart;
	word	read;

	curpart = 0;
	read = addr_to_block(
		do_dev_rdwr(FG_Read,
			      vol->logic_partition[curpart].partnum,
			      -1,
			      block_to_addr(1),
			      block_to_addr(bnr), 
			      (char *) bp,
			      &err));
	if (err)
		if (err != EC_Recover + SS_IOProc + EO_Medium) 
			read = 0;
	return (read);
}


word read_blk2 (void *bp, daddr_t bnr, VD *vol)

/*
 * Read block directly by avoiding the use of the buffer-cache
 * init_volume_info() has to be called before using this routine!
 */

{
	word	err,curpart;
	word	read;

	curpart = 0;
	while ( (bnr = bnr - vol->logic_partition[curpart].size) >= 0)
		curpart++;
	/* adjust blocknumber to current partition */
	bnr += vol->logic_partition[curpart].size;
	read = addr_to_block(
		do_dev_rdwr(FG_Read,
			      vol->logic_partition[curpart].partnum,
			      -1,
			      block_to_addr(1),
			      block_to_addr(bnr), 
			      (char *) bp,
			      &err));
	if (err)
		return (err);
	else
		return (read);
}



word write_dev (struct packet *tbp)

/* Returns GSP error if write request fails */

{	
	char		*buffer;
	daddr_t		bnr;
	word		err;
	word    	bcnt, realbcnt, restbcnt, done;
	word		curpart;
	struct buf 	*lbp, *ibp;
	VolDescriptor 	*curvol;

	curvol = &volume[tbp->p_fbp->b_dev];

	/* first physical block # */
	bnr  = tbp->p_fbp->b_bnr;	
	/* # of blocks to transfer */
	bcnt = tbp->p_blk_cnt;		
	/* pointer to buffer */
	buffer = tbp->p_fbp->b_un.b_addr;

	if ( (bnr < 0) || ( (bnr+bcnt) > curvol->bpcg * curvol->cgs)
	   ||
	     (bcnt <= 0) || (bcnt > phuge) )
	{
		Error (FSErr [InvalidParWr],
                          curvol->vol_name,
                          bcnt,
                          bnr,
                          curvol->bpcg,
                          curvol->cgs);
		curvol->sync_allowed = FALSE;
		return (EC_Fatal + EG_Parameter + EO_Server);
	}

	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;

	if (checksum_allowed) {
		for (ibp=tbp->p_fbp; ibp<=lbp; ibp++)
		{
			if( ibp->b_checksum != checksum(ibp->b_un.b_addr,BSIZE) )
			{
				Error (FSErr [BuffCacheChck], curvol->vol_name, bnr);
				curvol->sync_allowed = FALSE;
				return (EC_Fatal + EG_Invalid + EO_Server);
			}
		}
	}

	/* search linear for right logic partition */
	curpart = 0;
	while ( (bnr = bnr - curvol->logic_partition[curpart].size) >= 0)
		curpart++;
	/* adjust blocknumber to current partition */
	bnr += curvol->logic_partition[curpart].size;

FLdebug ("<<Write: pos %d, size %d, log_part %d",bnr,bcnt,curpart);

	/* blocks are written out in a loop */
	while ( bcnt != 0 )
	{
FLdebug (">while");
		if ( (restbcnt = bcnt + bnr - 
		     curvol->logic_partition[curpart].size) > 0 )
		{
FLdebug (">if1");
			realbcnt = bcnt - restbcnt;
		}
		else
		{
FLdebug (">else1");
			realbcnt = bcnt;	
		}
		
FLdebug ("<Write");			
		done = addr_to_block (
			do_dev_rdwr(FG_Write,
				    curvol->logic_partition[curpart].partnum,
				    -1,
				    block_to_addr(realbcnt),
				    block_to_addr(bnr), 
				    buffer,
				    &err)
			);
FLdebug (">Write, done (%d), realbcnt (%d), err (%d)", done, realbcnt, err);
	
		/* if couldn't succeed */
		if ( (done != realbcnt) || err ) 
		{
FLdebug (">if2");			
			if ( err == EC_Recover + SS_IOProc + EO_Medium ) 
			{
FLdebug (">if3");
				/* block successfully reassigned */
				err = Err_Null;
			}
			else     /* write error */
			{
FLdebug (">else2");	
				Error (FSErr [MSCWrite], 
	                        	err,
        	                        curvol->vol_name,
                	                realbcnt,
                        	        bnr,
                                	done);
				curvol->sync_allowed = FALSE;
FLdebug (">Error, return (%d) to %s", err, CalledBy (tbp));
				return (err);
			}
		} 

FLdebug (">>Write: %d blocks from %d",realbcnt,bnr);

		curpart++;
		bnr = 0;
		buffer = buffer + done * BSIZE; /* increment buffer pointer */
		bcnt = bcnt - done;		
	}
	/* update delayed-write blocks counter in packet header */
	tbp->p_dwt_cnt = 0;
	/* reset delayed-write flags in the buffer headers */
	lbp = tbp->p_fbp + tbp->p_blk_cnt - 1;
	for (ibp=tbp->p_fbp; ibp<=lbp; ibp++)
		ibp->b_dwt = FALSE;

FLdebug ("return (%d) to %s", err, CalledBy (tbp));
		
	return (err);		/* should be Err_Null    */
}



word write_blk (void *bp, daddr_t bnr, VD *vol)

/* 20/03/89 , HJE
 *
 * Write block directly by avoiding the use of the buffer-cache
 *
 */

{
	word err,curpart;
	word written = 0;

	curpart = 0;
	written = addr_to_block(
		do_dev_rdwr(FG_Write,
			      vol->logic_partition[curpart].partnum,
			      -1,
			      block_to_addr(1),
			      block_to_addr(bnr), 
			      (char *) bp,
			      &err));
	if (err)
		if ( err != EC_Recover + SS_IOProc + EO_Medium) 
			written = 0;
	return (written);
}


word write_blk2 (void *bp, daddr_t bnr, VD *vol)

/* 
 * Write block directly by avoiding the use of the buffer-cache
 * init_volume_info() has to be called before using this routine!
 */

{
	word err,curpart;
	word written = 0;

	curpart = 0;
	while ( (bnr = bnr - vol->logic_partition[curpart].size) >= 0)
		curpart++;
	/* adjust blocknumber to current partition */
	bnr += vol->logic_partition[curpart].size;
	written = addr_to_block(
		do_dev_rdwr(FG_Write,
			      vol->logic_partition[curpart].partnum,
			      -1,
			      block_to_addr(1),
			      block_to_addr(bnr), 
			      (char *) bp,
			      &err));
	if (err)
		return (err);
	else
		return (written);
}

#if 0
#include <sysdev.c>
#endif
