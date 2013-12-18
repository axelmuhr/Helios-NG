/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/inode.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: inode.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:05:25  guenter
 * first multivolume/multipartition HFS with tape
 * 
 * Revision 1.4  90/06/15  14:10:06  adruni
 * nonimportant iget modification.
 * 
 * Revision 1.3  90/05/30  15:43:11  chris
 * iput bug
 * searchi no longer locks
 * pad names to NameMax
 * 
 * Revision 1.2  90/01/12  19:08:18  chris
 * Fix use of semaphores in make_obj
 * 
 * Revision 1.1  90/01/02  19:02:51  chris
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
   |  inode.c								     |
   |                                                                         |
   |    Routines handling the Inodes of the FileSystem.			     |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    1 - A.Ishan - 25 July 1988 - Basic version                           |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */

#define GETIPUT	   0
#define DEBUG	   0
#define GEPDEBUG   0
#define FLDEBUG	   0
#define MONITOR	   0
#define COUNTDEBUG 0
#define IN_NUCLEUS 1

#include "error.h"

#define  PROCCNT 1
#include "proccnt.h"

#include "fstring.h"
/*#include "inode.h"*/
#include "nfs.h"

#if 0 & COUNTDEBUG
	word icount = IHSZ;
#else
	word IHashSize;
#endif

/* ================================================================= */

/* Local procedures */

static struct incore_i *isearch_hash (dev_t, daddr_t, word);

/* ================================================================= */

daddr_t
get_daddr (dev, bnr, pos)

dev_t dev;
daddr_t bnr;
word pos;

/*
*  Adjusts the block-number in the indirect-block at the given position
*  Returns -1 if I/O-error occurres
*/

{
	struct buf *bp;

/*	
Report ("	get_daddr :	bp = bread (%d, %d, 1, SAVEA); ",dev,bnr);
*/
	/* read indirect block */
	bp = bread (dev, bnr, 1, SAVEA);
	if (bp == NULL ) {
Error (" get_daddr() : Volume /%s failed to read indirect block !",volume[dev].vol_name);
		PutMsg (&volume[dev].unload_mcb);	
		return (-1);
	}
	/* adjust blocknr of data block */	
	bnr = *(bp->b_un.b_daddr + pos);
/*
Report ("	get_daddr :	bp = brelse (0x%p, TAIL); ",bp->b_tbp);
*/
	/* release indirect block */
	brelse (bp->b_tbp, TAIL);

	return (bnr);
}


/* ================================================================= */

daddr_t
bmap (ip, byte_offset, error)

struct incore_i *ip;
uword byte_offset;
word *error;

/*
*  Calculates the logical block-number of the given byte-offset.
*  Tries to map the physical block-number with the help of inode.
*  Reads the indirect-blocks by 'get_daddr' if necessary.
*  Returns the block-number on disc if already allocated,
*  else the negative logical block-number
*  Returns 0 in error if no I/O error occurred, 1 if so.
*/

{
	daddr_t bnr,lgbnr;
	dev_t dev;
	word pos;
	
	IncPC (80);
	
	*error = 0;
	/* calc logical blocknr */
	lgbnr = byte_offset / BSIZE;

	/* if (direct data block) */
	if (lgbnr < MAXDIR) 
	{
		/* adjust data blocknr */
		bnr = ip->ii_i.i_db[lgbnr];
		/* if (block not allocated) */
		if (!bnr) 
			goto finish;
	} 
	else 
	{
		/* notice the device */
		dev = ip->ii_dev;
	
		/* if (single indirect data block) */
		if (lgbnr < MAXSIND) 
		{
			/* adjust indirect blocknr */	
			bnr = ip->ii_i.i_ib[0];
			/* if (ind block not allocated) */
			if (!bnr) 
				goto finish;
			/* position of blocknr in ind block */
			pos = lgbnr - MAXDIR;
			/* adjust data blocknr */
			bnr = get_daddr (dev, bnr, pos);
			if (bnr < 0) {
				*error = 1;
				DecPC (80); return (0);
			}
			/* if (data block not allocated) */
			if (!bnr) 
				goto finish;
		}		

		/* if (double ind data block) */
		elif (lgbnr < MAXDIND) 
		{
			/* adjust double indirect blocknr */	
			bnr = ip->ii_i.i_ib[1];			
			/* if (double ind not allocated) */
			if (!bnr) 
				goto finish;
			/* position of single ind block in double ind blk */
			pos = (lgbnr - MAXSIND) / SIADDR;
			/* adjust single indirect blocknr */
			bnr = get_daddr (dev, bnr, pos);
			if (bnr < 0) {
				*error = 1;
				DecPC (80); return (0);
			}
			/* if (single ind not allocated) */
			if (!bnr) 
				goto finish;
			/* position of blocknr in single ind block */
			pos = (lgbnr - MAXSIND) % SIADDR;
			/* adjust data blocknr */
			bnr = get_daddr (dev, bnr, pos);
			if (bnr < 0) {
				*error = 1;
				DecPC (80); return (0);
			} 
			/* if (data block not allocated) */
			if (!bnr) 
				goto finish;
		}

		/* if (triple indirect data block) */
		else
		{

Error ("	bmap : 		ERROR : TIND data blocks not implemented !");
			*error = 1;
			DecPC (80); return (0);
		} 

	}
	DecPC (80); return (bnr);

finish:
	/* return (negative logical blknr) */

	DecPC (80); return (-lgbnr);
}

/* ================================================================= */

struct incore_i *
iget (bp, offset, parbnr, parofs)

struct buf *bp;
word offset;
daddr_t parbnr;
word parofs;

/*
*  If inode of file already incore merely increment reference-counter, 
*  else copy inode on disc into an empty incore-inode
*  after removing it from free-list,
*  and insert it into according inode-hash queue.
*  Returns a pointer to the incore-inode.
*/

{
	struct incore_i *ip;

#if GEPDEBUG
	VD *vol = &volume[bp->b_dev];
#endif

FLdebug ("iget");

	/* Block inode-hash queue */
	Wait (&ihash_sem);
	
	/* if it's the root-dir then there isn't the need of a previous
	   'bread' because root-dir inode is kept incore all the time,
	   therefore there is no pointer to a buffer */

	ip = isearch_hash(bp->b_dev,bp->b_bnr,offset);
	
	/* if file's inode already incore increment reference-counter */
	if (ip)
		ip->ii_count++;

	else 
	{
		ip = iremove_free();
		if (!ip)
		{
			Signal (&ihash_sem);
			Error (" iget() : no free inodes in freelist");
			return ((struct incore_i *)NULL);
		}		
		/* copy inode on disc into incore-inode */	   
		ip->ii_dev = bp->b_dev;
		ip->ii_parbnr = parbnr;
		ip->ii_parofs = parofs;
		ip->ii_dirbnr = bp->b_bnr;
		ip->ii_dirofs = offset;
		ip->ii_count = 1;
		ip->ii_changed = FALSE;
		strcpy(ip->ii_name,(bp->b_un.b_dir + offset)->de_name);
		ip->ii_i = (bp->b_un.b_dir + offset)->de_inode;
		InitSemaphore (&ip->ii_sem,1);

		/* insert in the according inode-hash queue */
		iinsert_hash (ip);
	}
#if GETIPUT
Report ("iget: >%s.%d< ",ip->ii_name,ip->ii_count);
#endif
	/* signal inode-hash queues */
	Signal (&ihash_sem);
	
	return (ip);
}

/* ================================================================= */

word
iput (ip)

struct incore_i *ip;

/*
*  Decrement the reference-counter of the incore-inode.
*  If no more references to the incore-inode copy it to disc,
*  clear the incore-inode after removing it from inode-hash queue,
*  and insert it into free-list.
*  Return : FALSE if any I/O-error occurred, TRUE if not.
*/

{
	struct buf *bp;
	word ii_count = ip->ii_count;
	word removed = FALSE;
		
	VD *vol = &volume[ip->ii_dev];

	Wait (&ihash_sem);

	if ( (ip->ii_count == 1) && (ip->ii_parbnr == ip->ii_dirbnr) ) {

GEPdebug (" iput : Volume /%s, request to move incore-I of RootDir - ignored !",vol->vol_name);

	}
	else {
		/* decrement reference-counter */
		ip->ii_count--;
		ii_count = ip->ii_count;
	}
	Signal (&ihash_sem);
	
	/* if no more references */
	if (!ii_count) 
	{
		/* if incore-inode has been modified */
		if (ip->ii_changed) 
		{

			/* read the directory block of file */

			bp = bread (ip->ii_dev,ip->ii_dirbnr,1,SAVEB);
			if ( bp == NULL ) {
Error (" iput() : Volume /%s failed to read directory block !",vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
			/* copy incore-inode out to disc */
			strncpy ((bp->b_un.b_dir + ip->ii_dirofs)->de_name,
				ip->ii_name,NameMax);
			(bp->b_un.b_dir + ip->ii_dirofs)->de_inode = ip->ii_i;
			/* write out the directory block */
			if (!bwrite (bp->b_tbp)) {
Error (" iput() : Volume /%s failed to write directory block !",vol->vol_name);
				PutMsg (&vol->unload_mcb);
				return (FALSE);
			}
		}

		Wait (&ihash_sem);
		
		if (!ip->ii_count) {
			/* remove it from the inode-hash queue */
			iremove_hash (ip);
			removed = TRUE;			
			/* clear contents of incore-inode */
			ip->ii_dev = 0;
			ip->ii_parbnr = 0;
			ip->ii_parofs = 0;
			ip->ii_dirbnr = 0;
			ip->ii_dirofs = 0;
			ip->ii_changed = FALSE;
			memset(ip->ii_name, 0, 32);
			memset(&(ip->ii_i), 0, sizeof(struct inode));
			InitSemaphore (&ip->ii_sem,0);
		}
		Signal (&ihash_sem);			
		if (removed) {
			/* insert the cleared incore-inode into free-list */	
			iinsert_free(ip);
		}
	}
	return (TRUE);
}	

/* ================================================================= */

dir_t
searchi (iip, name, len)

struct incore_i *iip;
string name;
word len;
/* 
*  Read the blocks of the directory pointed by 'iip'.
*  If the parameter 'len'(gth) is positive, search for the given 'name',
*  else for an empty DirEntry for a NewEntry.
*  Returns the pointer to the directory block in buffer-cache 
*  and the offset of the Entry, OR NULL-pointer if not found.
*/

{
	dir_t idir;
	word req_size, bcnt, i, j, k, error;
	char obj[32];
	daddr_t bnr, ibnr, lbnr;
	struct buf *bp;
	struct packet *tbp;
			
	/* if no DirEntries OR not a directory file */
	if (!iip->ii_i.i_size || iip->ii_i.i_mode != DIR)
	{
FLdebug ("i_size (%x), i_mode (%x)", iip->ii_i.i_size, iip->ii_i.i_mode);
FLdebug ("goto finish");
		goto finish;
	}

	/* if search for a given name */
	if (len>0) 
	{
		/* form the name as included in DirEntry */
		strncpy(obj,name,len);
		obj[len] = '\0';
		for (i=len+1;i<32;i++) 
			obj[i] = 0;
	}	
			
	/* calc the requested size of directory blocks */	
#if 0
	req_size = (iip->ii_i.i_size - 1) / BSIZE + 1;
#else
	req_size = iip->ii_i.i_blocks;
#endif
	for (i=0,bcnt=0; i<req_size; i++) 
	{
		/* check the existence of corresponding 
		   directory blocks on disc */
		ibnr = bmap (iip, i*BSIZE, &error);
		if (ibnr<=0) 
		{

Error ("	searchi : 	ERROR : Can't read non allocated blocks !");
FLdebug ("goto finish");
			goto finish;
		}	
		/* if next block on disc not contigious */
		if ((bcnt)&&(ibnr!=lbnr+1)) 
		{
			/* decrement the loop-variables */
			ibnr = lbnr;
			i--;
			/* go to read the gathered contigious blocks */
			goto read_pkt;
		}
		/* notice the first block of the packet */
		if(!bcnt)
			bnr = ibnr;
		/* increment size of contigious blocks */
		bcnt++;
		/* notice the address of the last block */
		lbnr = ibnr;
		/* if huge packet-size OR requested size of blocks gathered */
		if ((bcnt==phuge)||(i==(req_size-1))) 
		{
read_pkt:			


DEBdebug ("	searchi :		bp = bread (%d, %d, %d, SAVEA); ",
iip->ii_dev, bnr, bcnt);

			/* read the packet of directory-blocks */
			bp = bread(iip->ii_dev,bnr,bcnt,SAVEA);
			if ( bp == NULL ) {
Error (" searchi() : Volume /%s failed to read directory block !",volume[iip->ii_dev].vol_name);
				PutMsg(&volume[iip->ii_dev].unload_mcb);
FLdebug ("goto finish");
				goto finish;
			}
			tbp = bp->b_tbp;

			/* outer loop for the whole packet */
			for (j=0; j<tbp->p_blk_cnt; j++, bp++)
			{
#if MONITOR
Report ("%s[%d]->|",iip->ii_name, j);
fflush(stdout);
#endif
				/* inner loop for each directory-block */
				for (k=0; k<MAXDPB; k++) 
				{
#if MONITOR
Report (" %d:%s |",k,(bp->b_un.b_dir+k)->de_name);
fflush(stdout);
#endif
					if( 
					     /* a NewEntry will be performed */
					    ((!len)
					     &&
					     /* AND an empty DirEntry is found */	
					     (!(bp->b_un.b_dir+k)->
					     		de_inode.i_mode))
				            || 
					     /* a certain DirEntry is searched */	
					    ((len>0)
					     &&	
					     /* AND entry is not FREE */
					     ((bp->b_un.b_dir+k)
					     		->de_inode.i_mode)
					     &&
					     /* AND its name is found */
				             (!strcmp(((bp->b_un.b_dir+k)
				             		->de_name),obj))) ) 
					{
						/* notice the directory block */
						idir.bp = bp;
						idir.ofs = k;
/* if it's a NewEntry at the end of the list of DirEntries  */
/* update the size of directory */
if ( iip->ii_i.i_size == (i*BSIZE + k*sizeof(struct dir_elem)) )
{
	int empty;
	
	iip->ii_i.i_size += sizeof (struct dir_elem);
	empty = BSIZE - ( iip->ii_i.i_size % BSIZE );
	if ( empty < sizeof (struct dir_elem) )
		iip->ii_i.i_size += empty;
}
						/* return the pointer */
#if MONITOR
Report (" ");
#endif
FLdebug ("return");
						return (idir);
					} /* end of if */	    
				} /* end of the inner-loop */
#if MONITOR
Report (" ");
#endif
			} /* end of the outer-loop */

DEBdebug ("	searchi :		brelse (0x%p, TAIL); ",tbp);

			/* release the packet */
			brelse (tbp, TAIL);
			/* reset the (contigious) block-counter */
			bcnt = 0;
		}
	}
finish:	
	/* return NULL pointer if DirEntry not found 
	   OR no place for a NewEntry */
	idir.bp = (struct buf *) NULL;
	idir.ofs = 0;
FLdebug ("return NULL");
	return (idir);		    	
}	

/* ================================================================= */

char *VolInPath (char *Path,
                 char *Vol)
{
FLdebug ("path (%s), vol (%s)", Path, Vol);
	
	while (*Path)
	{
		if (*Path == c_dirchar)
		{
			char	*vp = Vol;
			char	*pp = Path;
			while ( *vp && *pp && *pp == *vp )
				pp++, vp++;
			if ( ! *vp && ( ! *pp || *pp == c_dirchar ))
{
FLdebug ("return (%s)", Path);
				return Path;
}				
		}
		Path++;
	}
	return NULL;
}	                 

struct incore_i *
namei (path, vol)

string path;
VD *vol;

/*
*  Search for the components of the given 'pathname' in the FileSystem tree. 
*  Return pointer to the incore-inode of the last component, if 'pathname'
*  followed successfully, i.e. each component of it found in the FileSystem,
*  else a NULL-pointer.
*/

{
	struct incore_i *ip, *iip;
	char *s1, *s2;
	dir_t idir;
	word len;
	struct buf bp;
	
	char  pathname [IOCDataMax],
	     *Occur,
	      SlashedVol [PATH_MAX];

FLdebug ("namei");	

	/* build a pathname absolute to the FileSystem's Root */ 
/*
s = strstr(path,vol->vol_name) + strlen (vol->vol_name);
  :
  :
	if (*pathname == '\0') 
	{
		*pathname = '/';
		*(pathname+1) = '\0';
	}
*/

/*************************************************************************/

/*
        Occur = VolInPath (path, vol->vol_name);
*/

        strcpy (SlashedVol, "/");
        strcat (SlashedVol, vol->vol_name);
        strcat (SlashedVol, "/");
        
        if ((Occur = strstr (path, SlashedVol)) != NULL)
   	  strcpy (pathname, Occur + strlen (SlashedVol) - strlen ("/"));
   	else  
        {
          Occur = strstr (path, CutLast (SlashedVol, 1));
          if ((Occur != NULL) && (*(Occur + strlen (SlashedVol)) == '\0'))
            strcpy (pathname, Occur + strlen (SlashedVol));
          else  
            strcpy (pathname, "/");
        }

/*************************************************************************/

        if (*pathname == '\0')
          strcpy (pathname, "/");

	/* check the given pathname */
	if (strncmp(pathname,"/",1)!=0) 
	{

Error ("	namei : 	ERROR : Pathname is not absolute !");
FLdebug ("but '%s'", pathname);
		return ((struct incore_i *)NULL);
	}

	/* get the pointer to RootDir to start the search */
	bp.b_bnr = 1;
	bp.b_dev = vol->volnum;
	iip = iget (&bp,0,1,0);

	if (*(pathname+1)=='\0')
		return iip;
		
	/* for each component of the 'pathname' except for the last one */
	for (s1=strchr(pathname,'/'),s1++ ;
	     (s2=strchr(s1,'/'))!=NULL ; s1=s2+1) 
	{
		/* check the length of component */
		len = s2-s1;
		if (!len) 
		{

Error ("	namei :		ERROR : Illegal pathname : %s !",pathname);
			goto finish;
		}
get_file:
		/* search the component in the ParentDir 
		   pointed by the previous component */
		Wait(&iip->ii_sem);
		idir = searchi (iip, s1, len);
		if (!idir.bp) 
		{
			Signal(&iip->ii_sem);
			goto finish;
		}
		/* copy inode of the component into a incore-inode */
		ip = iget (idir.bp, idir.ofs, 
			   iip->ii_dirbnr, iip->ii_dirofs);

DEBdebug ("	namei :		brelse (0x%p, TAIL); ",idir.bp->b_tbp);

		/* release the packet of ParentDir blocks */
		brelse (idir.bp->b_tbp, TAIL) ;
		Signal (&iip->ii_sem);

		/* release the incore-inode of the previous component */
		if (!iput (iip))
			goto finish;

		/* if no more empty incore-inodes available
		   send an exception message */
		if (!ip) 
			return (ip);

		/* if last component of the 'pathname',
		   return the pointer of its incore-inode */
		if (s2==NULL)
			return (ip);	 
		
		/* mark the current component as the ParentDir 
		   of the next component and continue */
		iip = ip;
	}
	/* if last component go to get the requested file */
	len = strlen (s1);
	goto get_file;
	
	/* return NULL-pointer releasing the incore-inode of 
 	   previous component (ParentDir) if any component of
 	   'pathname' not found */
finish: if (vol->sync_allowed)
		iput (iip);
	return ((struct incore_i *)NULL);
}

/* ================================================================= */
/* ==================================================================== */

static struct incore_i *
isearch_hash (dev, bnr, offset)

dev_t dev;
daddr_t bnr;
word offset;
 
/*
* Search for a file of the directory-block 'bnr' at the dir_entry 'offset'
* on device 'dev' in the incore inode list.
* Return a pointer to the found file 
* or NULL if there isn't any incore copy of it.
*/
 
{	
	struct incore_i	*ip;
	struct i_hash	*hip;
	
	/* find out the right inode-hash list */	
	hip = IHASH (dev, bnr, offset);

	/* search through cyclic inode-hash queue */
	ip = hip->ii_next;
	while ( ip != (struct incore_i *) hip )
	{
		if ( ip->ii_dirbnr == bnr && ip->ii_dirofs == offset 
			&& ip->ii_dev == dev)
			return ip;
		ip = ip->ii_next;
	}
	
	return (struct incore_i *) NULL;
}

	
/* ================================================================= */


void 
iinsert_hash (ip)

struct incore_i	*ip;

/*
* Insert the incore inode of the file into the according inode hash 
* queue (inserted at the head).
* The hash function is computed from  
*	ip->ii_dev  and  ip->ii_dirbnr  and  ip->ii_dirofs.
*/

{	
	struct i_hash	*hip;

	/* find out the right inode-hash list */
	hip = IHASH (ip->ii_dev, ip->ii_dirbnr, ip->ii_dirofs);

	/* insert inode at HEAD of inode-hash list */	
 	ip->ii_next = hip->ii_next;
	ip->ii_prev = (struct incore_i *)hip;
	hip->ii_next->ii_prev = ip;
	hip->ii_next = ip;
}


/* ================================================================= */

void 
iremove_hash (ip)

struct incore_i	*ip;

/* 
* Remove inode from inode-hash queue 
*/

{
	/* remove inode from inode-hash list */
	ip->ii_prev->ii_next = ip->ii_next;
	ip->ii_next->ii_prev = ip->ii_prev;
	ip->ii_next = ip;
	ip->ii_prev = ip;
}


/* ================================================================= */

void
iinsert_free (ip)

struct incore_i	*ip;

/*
* Insert the empty incore-inode at the head of the free incore-list.
* Incore-inodes are either in the inode-hash queue
* or after being written out, cleared and inserted in the free-list.
*/

{	
	struct free_ilist	*fip;
	
	/* pointer to free-inodes list */
	fip = &free_ilist;
	
	/* wait for free-inodes list */
	Wait (&fip->fi_sem);
	
	/* insert free-inode at HEAD */
	fip->ii_next->ii_prev = ip;
	ip->ii_next = fip->ii_next;
	fip->ii_next = ip;
	ip->ii_prev = (struct incore_i *) fip;
#if COUNTDEBUG
	icount++;
#endif

	/* signal free-inodes list */	
	Signal (&fip->fi_sem);
}

/* ================================================================= */

struct incore_i * 
iremove_free (void)

/*
* Remove an empty incore-inode to copy an inode from disc into it.
* Returns an exception if there isn't any empty incore-inode available
* cause the number of them is limited by 'MAXNII' defined in 'inode.h'.
*/

{
	struct incore_i 	*ip;
	struct free_ilist	*fip;
	
	/* pointer to free-inodes list */
	fip = &free_ilist;
	
	/* wait for free-inodes list */
	Wait (&fip->fi_sem);
#if COUNTDEBUG
Report (" i_remove : %d",icount);
	icount--;
#endif

	/* if (no more free-inodes) */
	if (fip->ii_next==(struct incore_i *)fip) 
	{
		/* signal free-inodes list */
		Signal (&fip->fi_sem);

Error ("	irem_free : 	ERROR : Incore i-list is full !!!");
		return ( (struct incore_i *)NULL );
	} 
	
	else 
	{
		/* remove free-inode at HEAD */	
		ip = fip->ii_next;

		/* update the pointers */
		fip->ii_next = ip->ii_next;
		ip->ii_next->ii_prev = (struct incore_i *) fip;
	}

	/* signal free-inodes list */
	Signal (&fip->fi_sem);
	return (ip);
}

/* ================================================================= */

/* end of inode.c */
