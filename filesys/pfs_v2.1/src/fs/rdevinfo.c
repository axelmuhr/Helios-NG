/* $Header: /hsrc/filesys/pfs_v2.1/src/fs/RCS/rdevinfo.c,v 1.1 1992/07/13 16:17:41 craig Exp $ */

/* $Log: rdevinfo.c,v $
 * Revision 1.1  1992/07/13  16:17:41  craig
 * Initial revision
 *
 * Revision 2.1  90/08/31  11:16:05  guenter
 * first multivolume/multipartition HFS with tape
 * 
 * Revision 1.6  90/08/31  10:52:44  guenter
 * first multivolume/multipartition HFS with tape
 * 
 * Revision 1.5  90/08/08  07:57:43  guenter
 * minor bugs fixed
 * 
 * Revision 1.4  90/08/03  09:37:36  guenter
 * multipartition/multivolume
 * 
 * Revision 1.3  90/05/30  15:44:59  chris
 * nothing
 * 
 * Revision 1.2  90/02/01  17:35:50  chris
 * Tape support amongst other things
 * 
 * Revision 1.1  90/01/02  19:02:59  chris
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
   |  rdevinfo.c (to be INCLUDED by deal_fs.c)                               |
   |                                                                         |
   |    Reading and analysing devinfo file.                                  |
   |                                                                         |
   |-------------------------------------------------------------------------|
   |                                                                         |
   |  History:                                                               |
   |    4 - O.Imbusch - 30 April   1991 - Error handling centralized         |
   |    3 - O.Imbusch - 26 March   1991 - Parameter added to load_devinfo () |
   |    2 - G.Lauven  -  3 August  1990 - Multivolume/-partition             |
   |    1 - Chris     -  2 January 1990 - Basic version                      |
   |                                                                         |
  []-------------------------------------------------------------------------[]
                                                                                */
/*
#define	 PushDEBUG    DEBUG
#define	 PushGEPDEBUG GEPDEBUG	
#define	 PushFLDEBUG  FLDEBUG

#undef	 DEBUG
#undef	 GEPDEBUG
#undef	 FLDEBUG

#define	 DEBUG    0
#define	 GEPDEBUG 0
#define	 FLDEBUG  1
#undef   __ERROR_H
#include "error.h"
*/

#include <limits.h>
#include <module.h>
#include <string.h>

#include "misc.h"
#include "fserr.h"

void *load_devinfo (char *PathToDevInfo)
{
  Stream    *s       = NULL;
  Object    *o       = NULL;
  void      *devinfo = NULL;
  int        size;
  ImageHdr   hdr;
  char      *ETCdir = "/helios/etc/",
             PathTaken [PATH_MAX];

#define NotFound(S) ((o = Locate (NULL, strcpy (PathTaken, S))) == NULL)

  if (
       (
         (PathToDevInfo == NULL) ||
         (
              NotFound (PathToDevInfo)
           && NotFound (strcat (ETCdir, PathToDevInfo))
         )
       )
       && NotFound ("/rom/devinfo")
       && NotFound ("/loader/DevInfo")
       && NotFound ("/helios/etc/devinfo")
     )
	  return NULL;

#undef NotFound

/*
  if (PathToDevInfo != NULL)
  {
    o = Locate (NULL, strcpy (PathTaken, PathToDevInfo));
    if (o == NULL)
      o = Locate (NULL, strcpy (PathTaken, strcat (ETCdir, PathToDevInfo)));
  }      
  if (o == NULL)
    o = Locate (NULL, strcpy (PathTaken, "/rom/devinfo"));
  if (o == NULL)
    o = Locate (NULL, strcpy (PathTaken, "/loader/Devinfo"));
  if (o == NULL)
    o = Locate (NULL, strcpy (PathTaken, "/helios/etc/devinfo"));
  if (o == NULL)
    return NULL;  
*/

  Report (FSErr [DISrc], PathTaken);

  s = Open(o,NULL,O_ReadOnly);
  if( s == NULL ) {
	  Close(o);
	  return NULL;
  }

  if(Read(s,(byte *)&hdr,sizeof(hdr),-1) != sizeof(hdr) )
	  goto done;
  if( hdr.Magic != Image_Magic )
	  goto done;

  size = hdr.Size;
  devinfo = Malloc(size);
  if( devinfo == NULL )
	  goto done;

  if(Read(s,devinfo,size,-1) != size ) {
	  Free(devinfo);
	  devinfo = NULL;
  }

done:
  Close(s);
  Close(o);

  return devinfo;

}

InfoNode *find_info(void *devinfo, word type, char *name)
{
	InfoNode *info = (InfoNode *)((Module *)devinfo + 1);

	forever
	{
		if( strcmp(name,RTOA(info->Name))==0 &&
		    info->Type == type ) return info;
		    
		if( info->Next == 0 ) break;
		info = (InfoNode *)RTOA(info->Next);
	}
	return NULL;
}

word init_size (FileSysInfo *fsi, DiscDevInfo *ddi)
{
	
/*	
*   this routine reads the blocksize of the devicedriver ( addressing )
*   and checks if it works with the fileserver blocksize, and then
*   calculates the conversion constant.
*/	

	word	size,addressing;

/*  getsize does not work here because no device is initialized up to now */
/*
	getsize_dev(0,&size,&addressing);
	if (size == 0) {
Error (FSErr [PartNotInDI], 0);
		return (FALSE);		
	}
*/
	addressing = ddi->Addressing;	
FLdebug ("addressing (0x%x)", addressing);
	if ( fsi->BlockSize < addressing )
	{
Error (FSErr [AddrGTBlk]);
		return (FALSE);
	}
	if ( (fsi->BlockSize % addressing ) != 0 ) {
Error (FSErr [AddrNFBlk]);
		return (FALSE);
	}
	block_to_addr_shifts = 0;
	size = (fsi->BlockSize / addressing);
	while ( (size = size >> 1) != 0 ) 
		block_to_addr_shifts++; 

GEPdebug (" init_size() : block_to_addr_shifts = %d",block_to_addr_shifts);

FLdebug ("addressing (0x%x)", addressing);
	return (TRUE);
}


word count_volumes (VolumeInfo *vvi)
{
	
/*
*   this routine counts the volumes in 'devinfo' 	
*/
	
	word		num_of_vols = 1;

	while (vvi->Next != -1) {
		num_of_vols++;
		vvi = (VolumeInfo *)RTOA(vvi->Next);
	}
	return (num_of_vols);
}


word count_partitions (VolumeInfo *vvi)
{
			
/* 
*   this routine counts the partitions in volume vvi 	
*/

		word		num_of_parts = 1;
		Partition	*ppltemp;
		
		ppltemp = (Partition *)RTOA(vvi->Partitions);
		while (ppltemp->Next != -1) {
			num_of_parts++;
			ppltemp = (Partition *)RTOA(ppltemp->Next);
		}
		return (num_of_parts);
}		

word calc_size (word blocks, word curvol, word *cgs, word *bpcg)
{
	
/*	
* this routine checks the parameter cgs and bpcg and acts according to
* the following list :
*
*       known      |     unknown      |   action
* ---------------------------------------------------------------------
*    cgs, bpcg     |     -------      | try to set up configuration
* ---------------------------------------------------------------------
*       bpcg       |       cgs        | calculate maximum cgs
* ---------------------------------------------------------------------
*       cgs        |       bpcg       | calculate maximum bpcg
* ---------------------------------------------------------------------
*      ------      |    cgs, bpcg     | calculate optimal configuration
* ---------------------------------------------------------------------
*/

    	int 	waste;		/* number of unusable blocks	*/
    	int	lim;

	if ( blocks < MIN_BPG ) {
Error (FSErr [NoSpace4Any],  blocks);
		return (FALSE);		
	}
	
	if ( (*cgs > 0) && (*bpcg > 0) ) {
		if ( *bpcg < MIN_BPG ) {	
Error (FSErr [BlkPerCG], *bpcg, MIN_BPG);
        	       return (FALSE);
		}				
		if ( (*cgs) * (*bpcg) > blocks ) {
Error (FSErr [NoSpace4Vol], curvol, blocks, (*cgs) * (*bpcg));
			return (FALSE);
		}
		return (TRUE);
	}

	if ( *bpcg > 0 ) {
		*cgs = blocks / (*bpcg);
		if ( *cgs == 0 ) {
Error (FSErr [TooFewBlk1CG], blocks, *bpcg);
			return (FALSE);
		}
		if ( *cgs > LIMIT_NCG )
			*cgs = LIMIT_NCG;
		return (TRUE);				
	}
	
	if ( *cgs > 0 ) {
		*bpcg = blocks / (*cgs);
		if ( *bpcg < MIN_BPG ) {	
Error (FSErr [TooFewBlkCG], blocks, MIN_BPG, *cgs);
			return (FALSE);
		}		
		if ( *bpcg > LIMIT_BPG )
			*bpcg = LIMIT_BPG;
		return (TRUE);
	}


/*
*
* - The cylinder group parameters are initialised 
*   according to the disk size.
* - The initialisation algorithm uses a linear relation between
*   number of cylinder groups and cylinder group size,
*   then it tries to minimise waste within a 20 % area.
*/


    	*cgs	= MIN_NCG;		/* number of cylinder groups	*/
    	*bpcg	= MIN_BPG;	/* cylinder group size (blocks)	*/

    	if ( blocks > LIMIT_NCG * LIMIT_BPG )	/* Check whether the device	*/
    	{					/* exceeds the largest possible	*/
		*cgs = LIMIT_NCG;		/* size.			*/
		*bpcg = LIMIT_BPG;
		return (TRUE);
   	}

    	while ( (*cgs) * (*bpcg) < blocks )	/* First, use a linear relation	*/
   	{					/* between cylinder group size	*/
		(*cgs)++;			/* and number of cyl. groups.	*/
		(*bpcg) += 16;
    	}
    	(*bpcg) = blocks / --(*cgs);

    	if ( (*bpcg) > LIMIT_BPG )		/* Check for bpcg overflow	*/
    	{
		(*bpcg) = LIMIT_BPG;		/* and adjust if necessary.	*/
		(*cgs) = blocks / LIMIT_BPG;
    	}

    	if ( blocks - (*cgs) * (*bpcg) > 0 )	/* Some blocks will be lost:	*/
    	{					/* Find the best relation	*/
		int bcgs	= *cgs;		/* between *cgs and *bpcg.	*/
		int bbpcg	= *bpcg;
		int bwaste	= blocks - (*cgs) * (*bpcg) + (*cgs);
		int tcgs;
		int tbpcg;
					/* Search for the best relation	*/
					/* within a range of 20 % from	*/
					/* the first cgs value.		*/

		tbpcg = *bpcg;		/* First, search in direction	*/
		tcgs = *cgs;		/* of lower cgsize.		*/
		lim = ( (*bpcg) * 4 ) / 5;
		do
		{
	    		tcgs++;
	    		tbpcg = blocks / tcgs;
	    		waste = blocks - ( tcgs * tbpcg ) + tcgs + ( tcgs - (*cgs) );
	    		if ( waste < bwaste )
	    		{
				bcgs = tcgs;
				bbpcg = tbpcg;
				bwaste = waste;
	   	 	}
		}				/* Check for limit and LIMIT_NCG */
		while ( tbpcg > lim && tcgs <= LIMIT_NCG );

		tbpcg = *bpcg;			/* Now, search in direction of	*/
		tcgs = *cgs;			/* lower cgs.			*/
		lim = (*cgs) * 4 / 5;
		do
		{
	    		tcgs--;
	    		tbpcg = blocks / tcgs;
	    		waste = blocks - ( tcgs * tbpcg ) + tcgs + ( (*cgs) - tcgs );
	    		if ( waste < bwaste && tbpcg <= LIMIT_BPG )
	    		{
				bcgs = tcgs;
				bbpcg = tbpcg;
				bwaste = waste;
	    		}
		}			/* Check for limit and LIMIT_BPG */
		while ( tcgs > lim && tbpcg <= LIMIT_BPG );

		*cgs = bcgs;		/* Now cgs and bpcg should be	*/
		*bpcg = bbpcg;		/* valid and give low waste.	*/
    	}
	return (TRUE);
}

word init_info (FileSysInfo *fsi, DiscDevInfo *ddi)

/* 
 *   This routine initializes the quick global volume info pointed to
 *   by *volume.
 *   Undefined fields in fsi are filled in.   
 */

{	
	VolumeInfo 	*vvi = (VolumeInfo *)RTOA(fsi->Volumes);
#if 0
	PartitionInfo 	*pii = (PartitionInfo *) RTOA(ddi->Partitions);
#endif	
	int 		cacheblocks;
	VolumeInfo 	*vvitemp;
	Partition 	*ppltemp;
	word 		num_of_vols, curvol;
	word 		num_of_parts, curpart;

		
/*
*   handle buffer cache parameters
*/

	/* reduce huge pkt size in small cache */
	if( fsi->HugePkt == -1 ) 
	{
		if( fsi->CacheSize <= 256 ) fsi->HugePkt = 12;
		else fsi->HugePkt = 16;
	}
	fsi->MaxPktSize = fsi->HugePkt;
	
	if( fsi->MediumPkt == -1 ) fsi->MediumPkt = fsi->HugePkt/2;
	
	cacheblocks = fsi->CacheSize/(BSIZE/1024);

	/* Huge pkts get up to half the cache */
	if( fsi->HugeCount == -1 ) 
		fsi->HugeCount = (cacheblocks/2)/fsi->HugePkt;

	cacheblocks -= fsi->HugeCount*fsi->HugePkt;	

	/* Medium pkts get half of the remainder */
	if( fsi->MediumCount == -1 ) 
		fsi->MediumCount = (cacheblocks/2)/fsi->MediumPkt;

	cacheblocks -= fsi->MediumCount*fsi->MediumPkt;

	/* Small packets get the rest */
	if( fsi->SmallCount == -1 ) fsi->SmallCount = cacheblocks;

/* OI 28 Oct 1991 */
/*	fsi->MaxInodes = IHSZ;*/

/*
*   handle device parameters
*/

	/* check and init device to fileserver block conversion */
	if ( init_size(fsi,ddi) == 0)
		return (FALSE);	
	

	/* count volumes */
	num_of_vols = count_volumes(vvi);

	/* allocate and initialize volumetable */
	volume = Malloc(num_of_vols * sizeof(VolDescriptor));
	if (volume == NULL) {
		Error (FSErr [VDNotAlloc]);
		return(FALSE);
	}

	vvitemp = vvi;
	/* step through single volumes and handle partitions */
	for (curvol = 0; curvol < num_of_vols; curvol++) {
		volume[curvol].volnum = curvol;
		volume[curvol].num_of_vols = num_of_vols;
		volume[curvol].raw = FALSE;
		volume[curvol].loaded = FALSE;
		volume[curvol].loadable = FALSE;
		volume[curvol].sync_allowed = FALSE;
		strcpy ( volume[curvol].vol_name,RTOA(vvitemp->Name));		
		
		if ( vvitemp->Type == vvt_raw ) {
			volume[curvol].raw = TRUE;
		}
		else {  /* following parameters only for structured devices */
			if ( vvitemp->CgCount > LIMIT_NCG ) {
Error (FSErr [NCG2Big], curvol, vvitemp->CgCount, LIMIT_NCG);
				Free (volume);
				return (FALSE);			
			}
			volume[curvol].found_cgs = vvitemp->CgCount;
		
			if ( vvitemp->CgSize > LIMIT_BPG ) {
Error (FSErr [CGS2Big], curvol,vvitemp->CgSize, LIMIT_BPG);
				Free (volume);
				return (FALSE);			
			}
			volume[curvol].found_bpcg = vvitemp->CgSize;
		
			if ( vvitemp->CgOffset == -1 )
				vvitemp->CgOffset = CG_OFFSET;
			volume[curvol].cgoffs = vvitemp->CgOffset;
		
			volume[curvol].minfree = vvitemp->MinFree;
		}	
		
		num_of_parts = count_partitions(vvitemp);

		/* allocate and initialize partitiontable */
		volume[curvol].logic_partition = Malloc(num_of_parts * sizeof(PartDescriptor));
		if (volume[curvol].logic_partition == NULL) {
Error (FSErr [PDNotAlloc], curvol);
			Free (volume);
			return (FALSE);
		}
		volume[curvol].num_of_parts = num_of_parts;
		
		if ( volume[curvol].raw && volume[curvol].num_of_parts != 1 ) {
Error (FSErr [TooMuchPart], curvol, volume [curvol].num_of_parts);
			Free(volume);
			return (FALSE);			
		}
		
		ppltemp = (Partition *)RTOA(vvitemp->Partitions);
		/* step through single partitions and initialize partitiontables */
		for (curpart = 0; curpart < num_of_parts; curpart++) {
			volume[curvol].logic_partition[curpart].partnum = ppltemp->Partition;
			ppltemp = (Partition *)RTOA(ppltemp->Next);
		}
		vvitemp = (VolumeInfo *)RTOA(vvitemp->Next);		
	}	
	return (TRUE);
}


/*
 *  init_volume_info starts here
 */

word
init_volume_info (VolDescriptor *curvol)

/*
 *   Tries to init size parameter of volume *curvol
 */
 
{
	word		phys_part,i,num_of_parts;			
	word		size,addressing;
	word		border,num_of_blocks;

	border = 0;	
	num_of_parts = curvol->num_of_parts;
	for (i = 0; i < num_of_parts; i++) {
		phys_part = curvol->logic_partition[i].partnum;
		if ( curvol->raw ) {
			curvol->logic_partition[i].border = 0;
			curvol->logic_partition[i].size = 0;
		}
		else {  /* for structured devices */
			getsize_dev(phys_part,&size,&addressing);
			if (size == 0) {
Error (FSErr [PartNotInDI], phys_part);
				return (FALSE);		
			}
			num_of_blocks = addr_to_block(size);

			if (num_of_blocks == 0) {
Error (FSErr [Part2Small], phys_part);
				return (FALSE);
			}
			border += num_of_blocks;
			curvol->logic_partition[i].border = border;
			curvol->logic_partition[i].size = num_of_blocks;

GEPdebug (" logic partition %d, border %d, physical partition %d",i,curvol->logic_partition[i].border,phys_part);

		}
	}	
	/* calculate and set up blocks per cylindergroup and 	*/
	/* number of cylindergroups for structured devices    	*/	
		
	if ( !curvol->raw ) { /* for structured devices */
		if ( calc_size(border, curvol->volnum, &curvol->cgs, &curvol->bpcg) == 0 ) {
			return (FALSE);
		}
	}
	else {  /* for raw devices set to zero */
		curvol->cgs = 0;
		curvol->bpcg = 0;
	}

GEPdebug (" init_info() : Set up volume %d with %d cylindergroups, %d blocks\n"
          "               per cylindergroup, offering %d Kbytes raw space",
                          curvol->volnum,
                          curvol->cgs,curvol->bpcg,
                          (curvol->cgs * curvol->bpcg * BSIZE)/1024);

	curvol->size_known = TRUE;
	return (TRUE);
}

/*
#undef	 DEBUG
#undef	 GEPDEBUG
#undef	 FLDEBUG

#define	 DEBUG    PushDEBUG
#define	 GEPDEBUG PushGEPDEBUG	
#define	 FLDEBUG  PushFLDEBUG
*/
/*******************************************************************************
**
**  rdevinfo.c
**
*******************************************************************************/
