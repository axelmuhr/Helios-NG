/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/fsyscall.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: fsyscall.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:04:22  guenter
 * first multivolume/multipartition PFS with tape
 * 
 * Revision 1.4  90/06/20  16:12:39  adruni
 * compared const was greater than ULONG_MAX.
 * 
 * Revision 1.3  90/05/30  15:40:41  chris
 * Fix tidyup operations after disk full
 * 
 * Revision 1.2  90/02/01  17:37:00  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:03:16  chris
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
   |  fsyscall.c							     |
   |                                                                         |
   |    Routines handling the Stream Operations. 			     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    2 - H.J.Ermen -  9 February 1989 - Stream port locking implementation|
   |    1 - A.Ishan   - 17 October  1988 - Basic version                     |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define  TIMING     0

#define  DEBUG      0
#define  GEPDEBUG   0
#define  FLDEBUG    0
#define  IN_NUCLEUS 1
#include "error.h"

#define  PROCCNT    1
#include "proccnt.h"

#include "fserr.h"
#include "nfs.h"

word DebugErr;

#define DebPutMsg(mcb) (IncPC (70), (DebugErr = PutMsg (mcb)), DecPC (70), DebugErr)
#define DebGetMsg(mcb) (IncPC (71), (DebugErr = GetMsg (mcb)), DecPC (71), DebugErr)


/* ================================================================= */

word
setsize_file (ip, file_size)

struct incore_i *ip;
uword file_size;

/*
*  Sets the size of the pointed file to the given value.
*  Besides handling the SetFileSize Stream Operation,
*  this routine is also called to deal with the Delete
*  Directory Operation.
*  Return : FALSE if any I/O-error occurred, TRUE if not.
*/

{
	daddr_t llgbnr,flgbnr,dlgbnr,bnr;
	word i, error;
	struct buf *bp;	
		
	Wait (&ip->ii_sem);	

	/* if the given size is equal to the file-size return */
	if (file_size==ip->ii_i.i_size)		
		goto finish;
	
	/* calculate the logical number of the last block */
	llgbnr = (ip->ii_i.i_size-1)/BSIZE;	
	/* find out the first logical block of the file to be freed */
	if (!file_size)
		flgbnr = 0;
	else 
	{
		flgbnr = (file_size-1)/BSIZE + 1;
		/* if there will be still some data in the new last block */
		if (file_size%BSIZE) 
		{
			/* adjust the block address on disc */
			bnr = bmap(ip, (flgbnr-1)*BSIZE, &error);
			if ( error ) {
				Signal (&ip->ii_sem);
				return (FALSE);
			}
			elif (bnr>0) 
			{

DEBdebug ("	setsize :	bp = bread(%d, %d, 1, SAVEB);", ip->ii_dev, bnr);

				/* read this block into Buffer Cache */
				bp = bread(ip->ii_dev,bnr,1,SAVEB);
				if (bp == NULL) {
Error (FSErr [BlkReadFld], volume[ip->ii_dev].vol_name);
					Signal (&ip->ii_sem);
					return (FALSE);
				}				
				/* delete data in this block,
				   starting with the new end of the file */
				for (i=(file_size%BSIZE);i<BSIZE;i++)
					*(bp->b_un.b_addr+i) = 0;

DEBdebug ("	setsize :	bdwrite (0x%p);\n",bp->b_tbp);

				/* write the modified block out */
				if ( volume [ip->ii_dev].syncwrite ) {
					if (!bwrite (bp->b_tbp)) {
Error (FSErr [BlkWriteFld], volume[ip->ii_dev].vol_name);
						Signal (&ip->ii_sem);
						return (FALSE);
					}
				}
				else
					bdwrite (bp->b_tbp);
			}
		}
	}
	/* calculate number of blocks to be freed */
	dlgbnr = llgbnr - flgbnr + 1;
	
	/* call the appropriate routine to give those blocks free,
	   updating necessary informations */
	if (!fREE (ip, flgbnr, dlgbnr)) {
		DebPutMsg(&volume[ip->ii_dev].unload_mcb);		
		Signal (&ip->ii_sem);		
		return (FALSE);
	}
finish:
	/* update inode */
	ip->ii_i.i_size = file_size;
	ip->ii_changed = TRUE;
	Signal (&ip->ii_sem);
	return (TRUE);
}

/* ==================================================================== */

void
seek_file (m, ip)

MCB *m;
struct incore_i *ip;

/*
*  Handles the SeekFile Stream Operation.
*/

{
	SeekRequest *req = (SeekRequest *)m->Control;
	word curpos = req->CurPos;
	word mode = req->Mode;
	word newpos = req->NewPos;

	switch( mode )
	{
	case S_Beginning:	break;

	/* if the new position should be relative to the current position */
	case S_Relative:	newpos += curpos; break;

	/* if the new position should be relative to the end of the file */
	case S_End:		newpos += ip->ii_i.i_size; break;
	}

	/* form the reply message */
	InitMCB(m,0,m->MsgHdr.Reply,NullPort,Err_Null);

	/* append the new position to the reply message */
	MarshalWord(m,newpos);

	/* send the message to client */
	DebPutMsg(m);
}

/* ==================================================================== */

word
read_file (m, ip)

MCB *m;
struct incore_i *ip;

/*
*  Reads data from a file starting from the given byte-offset
*  up to the given byte-size, in order to handle the ReadFile 
*  Stream Operation.
*  Return : FALSE if any read error occurred, TRUE if no
*/

{
	word first_bytes,last_bytes,write_size,
	     req_size,bcnt,i,first_blk_bytes, sent=0;
	word error;
	caddr_t write_offset;

#if TIMING
	clock_t start,end;
#endif	
	struct buf *bp;
	daddr_t bnr,ibnr,lbnr;

	ReadWrite *rw = (ReadWrite *)m->Control;
	Port reply = m->MsgHdr.Reply;

	uword byte_offset = rw->Pos; 
	uword byte_size = rw->Size;
	word timeout = rw->Timeout;
	word seq = 0;
	word e = Err_Null;
	
	VD *vol = &volume[ip->ii_dev];	
			
	Wait (&ip->ii_sem);
#if TIMING
	start = clock();
#endif
	/* check the validity of the byte-offset in the file */
	if (byte_offset < 0) 
	{
		ErrorMsg(m, EC_Error|EG_Parameter|1);
		Signal(&ip->ii_sem);
		return (TRUE);
	}
	if (byte_size < 0) 
	{
		ErrorMsg(m, EC_Error|EG_Parameter|2);
		Signal(&ip->ii_sem);
		return (TRUE);
	}
	if (byte_offset >= ip->ii_i.i_size) 
	{
		m->MsgHdr.FnRc = ReadRc_EOF;
		ErrorMsg(m,0);
		Signal(&ip->ii_sem);
		return (TRUE);
	}

	/* correct the byte-size if there's less data than required */
	if ((byte_offset+byte_size) > ip->ii_i.i_size)
		byte_size = ip->ii_i.i_size - byte_offset;

	/* initialise the reply message */
	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,ReadRc_More|seq);

	/* if no data was required than send a dummy reply */
	if (byte_size == 0) 
	{
		Delay(timeout);
		m->MsgHdr.Flags = 0;
		m->MsgHdr.FnRc = EC_Recover+EG_Timeout+EO_File;
		DebPutMsg(m);
		Signal(&ip->ii_sem);
		return (TRUE);
	}			

	/* calculate the first-bytes to the block limit */
	first_bytes = BSIZE - (byte_offset % BSIZE);

	/* if required data is included in one data block */
	if (first_bytes >= byte_size) 
	{
		/* find out block address on disc */
		ibnr  = bmap(ip,byte_offset, &error);
		if (ibnr<=0) 
		{
Error (FSErr [ReadReqFld], vol->vol_name);
			ErrorMsg(m, EC_Error+EG_Invalid+EO_File );
			Signal(&ip->ii_sem); 
			return (FALSE);
		}

DEBdebug ("	read_req :	bp = bread(%d, %d, 1, SAVEA);", ip->ii_dev, ibnr);

		/* read this block into Buffer Cache */
		bp = bread(ip->ii_dev,ibnr,1,SAVEA);
		if (bp == NULL) {
Error (FSErr [BlkReadFld], vol->vol_name);
			Signal(&ip->ii_sem);
			DebPutMsg(&vol->unload_mcb);
			ErrorMsg(m, EC_Error+EG_Broken+EO_Medium );
			return (FALSE);
		}
		/* mark the pointer in this buffer */
		write_offset = bp->b_un.b_addr + (byte_offset % BSIZE);
		/* adjust the size of data to be written to client */
		write_size = byte_size;
		/* form the reply message */
		m->MsgHdr.Flags = 0;
		m->MsgHdr.FnRc = ReadRc_EOD|seq;
		m->Data = write_offset;
		m->MsgHdr.DataSize = write_size;
		
 		/* send it to client */
		DebPutMsg(m);

DEBdebug ("	read_req :	brelse(0x%p, TAIL);", bp->b_tbp);

		/* release the block in Buffer Cache */
		brelse (bp->b_tbp, TAIL);
		sent += write_size;
	} 
	/* if required data is included in more than one data blocks */
	else 
	{
		/* find out the required size of data blocks */
		req_size = ((byte_size - first_bytes) - 1) / BSIZE + 2;
		/* calc the last-bytes in the last required data block */
		last_bytes = (byte_offset + byte_size - 1) % BSIZE + 1;

		/* try to build packets of contigious data blocks */
		for (i=0,bcnt=0;i<req_size;i++) 
		{
			/* adjust disc address */
			ibnr = bmap(ip,byte_offset + i * BSIZE, &error);
			if (ibnr<=0) 
			{
Error (FSErr [ReadReqFld], vol->vol_name);
				ErrorMsg(m, EC_Error+EG_Invalid+EO_File );
				Signal(&ip->ii_sem); 
				return (FALSE);
			}
			/* if next block on disc not contigious */
			if ((bcnt)&&(ibnr!=lbnr+1)) 
			{
				/* decrement loop variables */
				ibnr = lbnr;
				i--;
				/* go to read the gathered contigious blocks */
				goto read_pkt;			
			}
			/* notice the first block of the packet */
			if (!bcnt) 
				bnr = ibnr;
			/* increment size of contigious blocks */
			bcnt++;    
			/* notice the address of the last block */
			lbnr = ibnr;
			/* if huge packet-size 
			   OR requested size of blocks gathered */
			if ( (bcnt==phuge) || (i==req_size-1) ) 
			{
read_pkt:


DEBdebug ("	read_req :	bp = bread(%d, %d, %d, SAVEA);", ip->ii_dev, bnr,bcnt);

				/* read the packet of blocks */
			    	bp = bread (ip->ii_dev,bnr,bcnt,SAVEA);
			    	if (bp == NULL) {
Error (FSErr [BlkReadFld], vol->vol_name);
					Signal(&ip->ii_sem);
					DebPutMsg(&vol->unload_mcb);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium );
					return (FALSE);
			    	}
				/* adjust pointer in the buffer
				   and the byte-size of the first block */
			    	if (bcnt==i+1) 
			    	{
			    		write_offset = bp->b_un.b_addr +
			    			       (byte_offset % BSIZE);
			    		first_blk_bytes = first_bytes;
			    	} 
			    	else 
			    	{
			    		write_offset = bp->b_un.b_addr;
			    		first_blk_bytes = BSIZE;
			    	}
				/* calculate the message data size 
				   and mark the last message */
			    	if (i==req_size-1) 
			    	{
			    		write_size = first_blk_bytes
			    			     + (bcnt-2) * BSIZE
			    			     + last_bytes;
					m->MsgHdr.Flags = 0;
					m->MsgHdr.FnRc = ReadRc_EOD|seq;
				} 
				else 
				{
			    		write_size = first_blk_bytes
			    			     + (bcnt-1) * BSIZE;
					m->MsgHdr.FnRc = ReadRc_More|seq;
					/* increment sequence number */
					seq += ReadRc_SeqInc;
				}
				if ( write_size == 16 * BSIZE )
				{
					/* complete the message informations */
					m->Data = write_offset;
					m->MsgHdr.DataSize = write_size - 4;
					m->Control = (word *) (write_offset + write_size - 4);
					m->MsgHdr.ContSize = 1;
				}
				else
				{
					/* complete the message informations */
					m->Data = write_offset;
					m->MsgHdr.DataSize = write_size;
					m->MsgHdr.ContSize = 0;
				}
				/* send the message to client */
				e=DebPutMsg(m);
				m->Control = (word *)rw;

DEBdebug ("	read_req :	brelse(0x%p, TAIL);", bp->b_tbp);

				/* release the packet written to client */
			    	brelse (bp->b_tbp, TAIL);
				/* break if DebPutMsg couldn't succeed */	
				if ( e < Err_Null ) 
				{
Error (FSErr [PutMsgFld], vol->vol_name, e);
					break;
				}
				sent += write_size;
				/* reset counter of contigious blocks */
			    	bcnt = 0;
			}
		}
	}

	/* update inode */
	ip->ii_i.i_atime = GetDate();
	if ( !volume[ip->ii_dev].writeprotected )
		ip->ii_changed = TRUE;

#if TIMING
	end = clock();
	Report (FSErr [TimeRead], byte_size, (end-start));
#endif
	Signal (&ip->ii_sem);
	rw->Size = sent;
	return (TRUE);
}
			    
			   
			  		     	       
/* ==================================================================== */

word
write_file (m, ip)

MCB *m;
struct incore_i *ip;

/*
*  Writes data to a file starting from the given byte-offset
*  up to the given byte-size, in order to handle the WriteFile 
*  Stream Operation. Allocates new data blocks if necessary.
*  Return : FALSE if any error occurred during write_file, TRUE if not
*/

{
	word    first_bytes,
	        last_bytes,
	        read_size,
	        got=0,
	        req_size,
	        acnt,
	        bcnt,
	        aacnt,
	        i,
	        first_blk_bytes,
	        save;
	caddr_t read_offset;

#if TIMING
	clock_t start,
	        end;     
#endif	
	struct buf *bp;
	daddr_t lgbnr,bnr,ibnr,lbnr;
	pkt_t pkt;	
	ReadWrite *rw = (ReadWrite *)m->Control;
	Port request = m->MsgHdr.Dest;	
	Port reply = m->MsgHdr.Reply;
	word msgdata = m->MsgHdr.DataSize;
	
	uword byte_offset = rw->Pos; 
	uword byte_size = rw->Size;
	word e = Err_Null;
	word error;
	int panic = 0;
	VD *vol;
	
	IncPC (20);
	
	vol = &volume[ip->ii_dev];
	
	IncPC (61);
	Wait (&ip->ii_sem);
	DecPC (61);
#if TIMING
	start = clock();
#endif
	/* check if byte-offset is valid */
	if (byte_offset > ip->ii_i.i_size ||
	    byte_size == 0 || 
	    (byte_offset+byte_size) >= ULONG_MAX /* (uword)(MAXDIND*BSIZE) */
	   ) 
	{
		ErrorMsg(m,EC_Error|EG_Parameter|1);
		Signal(&ip->ii_sem);

DEBdebug (" write_file() : byte-offset invalid !");
		DecPC (20);
		return (TRUE);
	}
	
	/* initialise the reply message */			
	InitMCB(m,MsgHdr_Flags_preserve,reply,NullPort,WriteRc_Sizes);
	
	/* calculate the first-bytes to the block limit */
	first_bytes = BSIZE - (byte_offset % BSIZE);
	/* if received data will be written to a single data block */
	if (first_bytes >= byte_size) 
	{
		/* if there isn't any data in the request message */
		if (msgdata == 0) 
		{
			/* provide and send a reply message */
			MarshalWord(m,byte_size);
			MarshalWord(m,0);
			if ( (e = DebPutMsg(m)) < Err_Null) 
			{
Error (FSErr [PutMsgFld], vol->vol_name, e);
				goto finish;
			}
		}

		/* find out if there is a corresponding data block on disc */
		lgbnr = -bmap(ip,byte_offset, &error);

		/* if no corresponding block allocated */
		if ( error ) 
		{
			ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
			Signal (&ip->ii_sem);

DEBdebug (" write_file() : failed to allocate block (1) !");

			DecPC (20);
			return (FALSE);
		}
		if (lgbnr >= 0)
		{
			if (vol->vol_full)
			{
				ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
				panic = 1;
				goto finish;
			}

DEBdebug ("	write_req : 	pkt = alloc(0x%p, %d, 1);", ip, lgbnr);

			/* allocate a data block */
			pkt = alloc(ip,lgbnr,1);
			if (!pkt.bcnt)
			{
				ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
				panic = 1;
				goto finish;
			}

DEBdebug ("	write_req : 	bp = getblk(%d, %d, 1, NOSAVE);", ip->ii_dev, pkt.bnr);

			/* get any buffer for this block and clear its contents*/
			bp = getblk(ip->ii_dev,pkt.bnr,1,NOSAVE);
			if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
				Signal(&ip->ii_sem);
				ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
				DecPC (20);
				return (FALSE);
			}			
			if (byte_size!=BSIZE) 
				clr_buf (bp);
		} 
		/* if there is a corresponding block on disc */
		else 
		{
			bnr = -lgbnr;
			/* if the data in this block will be overwritten */
			if ( (byte_size == BSIZE)
			   ||( ((byte_offset % BSIZE)==0) 
			     &&(byte_size >= (ip->ii_i.i_size % BSIZE))
			     ) 
			   ) 
			{

DEBdebug ("	write_req : 	bp = getblk(%d, %d, 1, NOSAVE);", ip->ii_dev, bnr);		

				/* get any buffer for this block */
				bp = getblk(ip->ii_dev,bnr,1,NOSAVE);
				if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
					Signal(&ip->ii_sem);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
					DecPC (20);
					return (FALSE);
				}			
			} 
			else 
			{

DEBdebug ("	write_req : 	bp = bread(%d, %d, 1, SAVEB);", ip->ii_dev, bnr);		

				/* read this block from disc */
				bp = bread(ip->ii_dev,bnr,1,SAVEB);
				if (bp == NULL) {
Error (FSErr [BlkReadFld], vol->vol_name);
					Signal(&ip->ii_sem);
					DebPutMsg(&vol->unload_mcb);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
					DecPC (20);
					return (FALSE);
				}
			}
		}

		IncPC (32);
	
		/* adjust pointer in the buffer and message size */
		read_offset = bp->b_un.b_addr + (byte_offset % BSIZE);
		read_size = byte_size;
		
		/* get message with data,
		   if it wasn't included in the request message */
		if (msgdata == 0) 
		{
			m->MsgHdr.Dest = request;
			m->Timeout = WriteTimeout;
			m->Data = read_offset;
			e = DebGetMsg(m);
		}
		/* else copy the data from the request message */
		else 
		{
			memcpy(read_offset,m->Data,read_size);
		}

		IncPC (33);

DEBdebug (" 	write_req : 	bdwrite (0x%p);", bp->b_tbp);

		/* write block to disc */
		if ( vol->syncwrite )
		{
			if (!bwrite ( bp->b_tbp )) {
Error (FSErr [BlkWriteFld], vol->vol_name);
				Signal(&ip->ii_sem);
				DebPutMsg(&vol->unload_mcb);
				ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
				DecPC (20);
				return (FALSE);
			}
		}
		else
			bdwrite (bp->b_tbp);

		IncPC (34);
		
		/* report any exceptions */
		if ( msgdata == 0 && e < Err_Null ) 
		{
Error (FSErr [GetMsgFld], vol->vol_name, e);
			goto finish;
		}
		/* mark the written size */
		got += read_size;
	} 
	
	/* if there is data in the request message,
	   but it overwrites more than one data block */
	elif (msgdata>0) 
	{

		/* the data in the request message
		   can overwrite maximal two consecutive blocks */
		for (i=0 ; i<2; i++) 
		{
			/* check if there's a corresponding block on disc */
			lgbnr = -bmap(ip,(byte_offset+got), &error);
			/* if no corresponding block allocated */
			if ( error ) {
				ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
				Signal (&ip->ii_sem);

DEBdebug (" write_file() : failed to allocate block (2) !");

				DecPC (20);
				return (FALSE);
			}
			if (lgbnr >= 0) 
			{
				if (vol->vol_full)
				{
					ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
					panic = 1;
					goto finish;
				}

DEBdebug ("	write_req : 	pkt = alloc(0x%p, %d, 1);", ip, lgbnr);

				/* allocate the block */
				pkt = alloc(ip,lgbnr,1);
				if (!pkt.bcnt)
				{
					ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
					panic = 1;
					goto finish;
				}

DEBdebug ("	write_req : 	bp = getblk(%d, %d, 1, NOSAVE);", ip->ii_dev, pkt.bnr);		

				/* get any buffer and clear its contents */
				bp = getblk(ip->ii_dev,pkt.bnr,1,NOSAVE);
				if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
					Signal(&ip->ii_sem);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
					DecPC (20);
					return (FALSE);
				}			
				if (byte_size!=BSIZE) 
					clr_buf (bp);
			} 
			/* if there is a corresponding block on disc */
			else 
			{
				/* mark its address */
				bnr = -lgbnr;
				/* if the data in this block will be overwritten */
				if ( (byte_size == BSIZE)
			   	   ||( (((byte_offset+got) % BSIZE)==0) 
			     	     &&(byte_size >= (ip->ii_i.i_size % BSIZE))
			     	     ) 
			   	   ) 
			   	{

DEBdebug ("	write_req : 	bp = getblk(%d, %d, 1, NOSAVE);", ip->ii_dev, bnr);		

					/* get any buffer for this block */
					bp = getblk(ip->ii_dev,bnr,1,NOSAVE);
					if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
						Signal(&ip->ii_sem);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}			
				} 
				else 
				{

DEBdebug ("	write_req : 	bp = bread(%d, %d, 1, SAVEB);", ip->ii_dev, bnr);		

					/* read this block into a buffer */
					bp = bread(ip->ii_dev,bnr,1,SAVEB);
					if (bp == NULL) {
Error (FSErr [BlkReadFld], vol->vol_name);
						Signal(&ip->ii_sem);
						DebPutMsg(&vol->unload_mcb);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}
				}
			}		
			/* adjust the pointer in the buffer */
			read_offset = bp->b_un.b_addr + 
					((byte_offset+got) % BSIZE);
			/* calculate the size of data which 
			   will be read from the request message */
			read_size = (i?(byte_size-first_bytes):first_bytes);
			/* copy the data from the request message to buffer */
			memcpy(read_offset,&m->Data[got],read_size);

DEBdebug (" 	write_req : 	bdwrite (0x%p);", bp->b_tbp);

			/* write the block to disc */
			if ( vol->syncwrite ) {
				if (!bwrite (bp->b_tbp)) {
Error (FSErr [BlkWriteFld], vol->vol_name);
					Signal(&ip->ii_sem);
					DebPutMsg(&vol->unload_mcb);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
					DecPC (20);
					return (FALSE);
				}
			}
			else
				bdwrite (bp->b_tbp);
			/* notice already written data */
			got += read_size;
		}		
	
	/* if there's no data in the request message 
	   AND more than one data block will be overwritten */
	} 
	else 
	{
		word dsize;		


		/* form and send a reply message to the client,
		   in order to inform him how the data will be partitioned
		   in consecutive messages to FileServer */
		MarshalWord(m,first_bytes);
		MarshalWord(m,BSIZE);
		if ( (e = DebPutMsg(m)) < Err_Null) 
		{
Error (FSErr [PutMsgFld], vol->vol_name, e);
			goto finish;
		}

		/* initialise message header for GetMsg */
		m->MsgHdr.Dest = request;
		m->Timeout = WriteTimeout;

		/* calc the number of data blocks to be received */
		req_size = ((byte_size - first_bytes) - 1) / BSIZE + 2;
		/* calc the data-size of the last message */
		last_bytes = (byte_offset + byte_size - 1) % BSIZE + 1;

		for (i=0,acnt=0,bcnt=0,lgbnr=0;i<req_size;i++) 
		{
			/* find out the address if block already alloced */
			ibnr = bmap(ip,byte_offset + i * BSIZE, &error);
			if ( error ) {
				ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
				Signal (&ip->ii_sem);

DEBdebug (" write_file() : failed to allocate block (3) !");

				DecPC (20);
				return (FALSE);
			}
			/* if alloced blocks no more contigious
			   OR next block not yet alloced */
			if ((bcnt)&&( (ibnr!=lbnr+1) ||
				      ((lbnr>0)&&(ibnr<=0)) )) 
			{
				/* decrement loop variables */
				ibnr = lbnr;
				i--;
				/* get a packet with alloced blocks */
				goto b_toggled;
			}	      	
			/* if next block already alloced */
			if ((acnt)&&((lbnr<=0)&&(ibnr>0))) 
			{
				/* decrement loop variables */
				ibnr = lbnr;
				i--;
				/* alloc a packet of free blocks */
				goto a_toggled;
			}	      	
			/* if it's a block to be alloced */
			if (ibnr<=0) 
			{
				/* notice the first block */
				if (!acnt)
					lgbnr = -ibnr;
				/* increment the block counter */
				acnt++;
			} 
			else 
			{
				/* notice the first block */
				if (!bcnt)
					bnr = ibnr;
				/* increment the block counter */
				bcnt++;
			}				
/* $$$

DEBdebug ("	write_req :	| %d | %d | %d |", i, acnt, bcnt);

*/
			/* notice the address of the last block */
			lbnr = ibnr;

			/* if huge packet-size 
			   OR requested size of blocks gathered */
			if ( (bcnt==phuge) || ((bcnt)&&(i==req_size-1)) ) 
			{
b_toggled:			
				/* adjust swapping mode */
				save = NOSAVE;
			    	if ((bcnt==i+1)&&(byte_offset%BSIZE))
			    		save += SAVEF;
			    	if ( ((byte_offset+byte_size)<ip->ii_i.i_size)
			    	    &&(i==req_size-1)
			    	    &&((byte_size-first_bytes)%BSIZE) )
			    	    	save += SAVEL;

				/* if there is data in the packet
				   which won't be overwritten */
			    	if (save) 
			    	{

DEBdebug ("	write_req : 	bp = bread(%d, %d, %d, %d);", ip->ii_dev, bnr, bcnt, save);		

					/* read packet into Buffer Cache */
			    		bp = bread (ip->ii_dev,bnr,bcnt,save);
					if (bp == NULL) {
Error (FSErr [BlkReadFld], vol->vol_name);
						Signal(&ip->ii_sem);
						DebPutMsg(&vol->unload_mcb);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}
			    	} 
			    	else 
			    	{

DEBdebug ("	write_req : 	bp = getblk(%d, %d, %d, %d);", ip->ii_dev, bnr, bcnt, save);		

					/* reserve an appropriate free packet */
			    		bp = getblk(ip->ii_dev,bnr,bcnt,save);
					if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
						Signal(&ip->ii_sem);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}			
				}
				/* adjust offset in the packet
				   and the byte-size of the first block */
			        if (bcnt==i+1) 
			        {
			    		read_offset = bp->b_un.b_addr +
			    			       (byte_offset % BSIZE);
			    		first_blk_bytes = first_bytes;
				} 
				else 
				{
			    		read_offset = bp->b_un.b_addr;
			    		first_blk_bytes = BSIZE;
			    	}
				/* calc data size to be written
				   to this packet */
			    	if (i==req_size-1)
			    		read_size = first_blk_bytes
			    			    + (bcnt-2) * BSIZE
			    			    + last_bytes;
			    	else
			    		read_size = first_blk_bytes
			    			    + (bcnt-1) * BSIZE;

				/* get several messages 
				   in order to fill this packet */
				for (dsize=0; dsize<read_size; 
				     dsize+=m->MsgHdr.DataSize) 
				{
				     	m->Data = read_offset + dsize;
					if ( (e = DebGetMsg(m)) < Err_Null )
						break;
				}

DEBdebug (" 	write_req : 	bdwrite (0x%p);\n",bp->b_tbp);

				/* write the packet out */
				if ( vol->syncwrite ) {
					if (!bwrite (bp->b_tbp)) {
Error (FSErr [BlkWriteFld], vol->vol_name);
						Signal(&ip->ii_sem);
						DebPutMsg(&vol->unload_mcb);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}
				}
				else
					bdwrite(bp->b_tbp);
			    	/* report exception if necessary */
				if ( e < Err_Null ) 
				{
Error (FSErr [GetMsgFld], vol->vol_name, e);
					break;
				}
				/* update already received data size */
			    	got += read_size;
				/* reset block counter */
			    	bcnt = 0;
			}

			/* if huge packet-size 
			   OR requested size of blocks gathered */
			if ( (acnt == phuge) || ((acnt)&&(i==req_size-1)) ) 
			{
a_toggled:
				/* notice block counter */
				aacnt = acnt;
				if (vol->vol_full)
				{
					ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
					panic = 1;
					goto finish;
				}

DEBdebug ("	write_req : 	pkt = alloc(0x%p, %d, %d);", ip, lgbnr, acnt);

				/* try to alloc requested number 
				   of contigious data blocks */
			    	pkt = alloc (ip,lgbnr,acnt);
				if (!pkt.bcnt)
				{
					ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
					panic = 1;
					goto finish;
				}
aa_loop:


DEBdebug ("	write_req : 	bp = getblk(%d, %d, %d, NOSAVE);", ip->ii_dev, pkt.bnr, pkt.bcnt);		

				/* get suitable free packet */
			    	bp=getblk(ip->ii_dev,pkt.bnr,pkt.bcnt,NOSAVE);
				if (!bp) {
Error (FSErr [BlkGetFld], vol->vol_name);
					Signal(&ip->ii_sem);
					ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
					DecPC (20);
					return (FALSE);
				}			
				/* adjust offset in the packet
				   and the byte-size of the first block */
				if ((acnt==i+1)&&(aacnt==acnt)) 
				{
			    		read_offset = bp->b_un.b_addr +
			    			       (byte_offset % BSIZE);
			    		first_blk_bytes = first_bytes;
					/* clear contents of the first buffer */
					if (byte_offset%BSIZE)
						clr_buf (bp);
				} 
				else 
				{
			    		read_offset = bp->b_un.b_addr;
			    		first_blk_bytes = BSIZE;
			    	}
				/* update block counter */
				aacnt -= pkt.bcnt;
				/* calc data size to be written
				   to this packet */
			    	if ((i==req_size-1)&&(!aacnt)) 
			    	{
			    		read_size = first_blk_bytes
			    			    + (pkt.bcnt-2) * BSIZE
			    			    + last_bytes;
					/* clear contents of the last buffer */
					if ((byte_size-first_bytes)%BSIZE)
						clr_buf (bp+pkt.bcnt-1);
				} 
				else 
				{
			    		read_size = first_blk_bytes
			    			    + (pkt.bcnt-1) * BSIZE;
				}
				/* get several messages 
				   in order to fill this packet */
				for (dsize=0; dsize<read_size; 
				     dsize+=m->MsgHdr.DataSize) 
				{
				     	m->Data = read_offset + dsize;
					if ( (e = DebGetMsg(m)) < Err_Null )
						break;
				}

DEBdebug (" 	write_req : 	bdwrite (0x%p);", bp->b_tbp);

				/* write the packet out */
				if ( vol->syncwrite ) {
					if (!bwrite (bp->b_tbp)) {
Error (FSErr [BlkWriteFld], vol->vol_name);
						Signal(&ip->ii_sem);
						DebPutMsg(&vol->unload_mcb);
						ErrorMsg(m, EC_Error+EG_Broken+EO_Medium);
						DecPC (20);
						return (FALSE);
					}
				}
				else
					bdwrite(bp->b_tbp);
				/* report exception if necessary */
				if ( e < Err_Null ) 
				{
Error (FSErr [GetMsgFld], vol->vol_name, e);
					break;
				}
				/* update the size of already received data */
			    	got += read_size;
				/* if not all the requested blocks 
				   have been allocated */
			    	if (aacnt) 
			    	{
					/* update block number */
					lgbnr += pkt.bcnt;
					if (vol->vol_full)
					{
						ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
						panic = 1;
						goto finish;
					}

DEBdebug ("	write_req : 	pkt = alloc(0x%p, %d, %d)", ip, lgbnr, aacnt);

					/* try to alloc remaining blocks */
				    	pkt = alloc (ip,lgbnr,aacnt);
					if (!pkt.bcnt)
					{
						ErrorMsg(m, EC_Error+EG_NoMemory+EO_Medium);
						panic = 1;
						goto finish;
					}
					/* go to fill these blocks */
				    	goto aa_loop;
				}
				/* reset the block counter */
			    	acnt = 0;
			}	
		}
	}

	
finish:
	/* update inode */
	if (got) 
	{
		ip->ii_i.i_size = (ip->ii_i.i_size > (byte_offset+got))?(ip->ii_i.i_size):(byte_offset+got);
		ip->ii_i.i_mtime = GetDate();
	}	
	ip->ii_i.i_atime = GetDate();
	ip->ii_changed = TRUE;
#if TIMING
	end = clock();
	Report (FSErr [TimeWrite], got, (end-start));
#endif

	Signal (&ip->ii_sem);

	/* send a last reply message
	   in order to inform the client 
	   of data size received in several messages */
	InitMCB(m,0,reply,NullPort,e<0?e:WriteRc_Done);
	MarshalWord(m,got);
	DebPutMsg(m);

	rw->Size = got;
	if (panic)
	{

DEBdebug (" write_file() : returned because of panic == TRUE !");

		DecPC (20);
		return (FALSE);		/* return error */
	}
	else
	{
		DecPC (20);
		return (TRUE);	
	}
	DecPC (20);
}


/* ==================================================================== */

/* end of fsyscall.c */
