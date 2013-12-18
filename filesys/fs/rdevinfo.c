/* $Header: /giga/HeliosRoot/Helios/filesys/fs/RCS/rdevinfo.c,v 1.2 1991/03/25 15:45:33 nick Exp $ */

/* $Log: rdevinfo.c,v $
 * Revision 1.2  1991/03/25  15:45:33  nick
 * default value for MaxInodes set to 1 per cache block.
 *
 * Revision 1.1  90/10/05  16:28:06  nick
 * Initial revision
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

#include <module.h>

void *load_devinfo(void)
{
/*
	Stream *s = Open(CurrentDir,"/helios/etc/devinfo",O_ReadOnly);
*/
	Stream *s = NULL;
	Object *o;
	void *devinfo = NULL;
	int size;
	ImageHdr hdr;
/*
	if( s == NULL ) return NULL;
*/	
	o = Locate(NULL,"/rom/devinfo");

	if( o == NULL ) o = Locate(NULL,"/loader/DevInfo");

	if( o == NULL ) o = Locate(NULL,"/helios/etc/devinfo");

	if( o == NULL ) return NULL;

	s = Open(o,NULL,O_ReadOnly);

	if( s == NULL ) { 
		Close(o);
		return NULL;
	}

	if(Read(s,(byte *)&hdr,sizeof(hdr),-1) != sizeof(hdr) )
		goto done;
		
	if( hdr.Magic != Image_Magic ) goto done;
	
	size = hdr.Size;
	
	devinfo = Malloc(size);
	
	if( devinfo == NULL ) goto done;
	
	if(Read(s,devinfo,size,-1) != size ) 
	{ 
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

/* This routine fills in any undefined fields in the devinfo 	*/
/* for each volume.						*/
/* At present it assumes 1 partition per volume			*/
/* 1 partition per drive.					*/

void init_info(FileSysInfo *fsi, DiscDevInfo *ddi)
{
	VolumeInfo *vvi;
	word vvioff;
	DriveInfo *dvi;
	PartitionInfo *pii;
	Partition *pi;
	int spb;		/* Sectors per block */
	int cacheblocks;

	vvi = (VolumeInfo *)RTOA(fsi->Volumes);

/* Don't play with anything if this is the tape */
	if( vvi->Type == vvt_raw ) return;

	forever
	{
/* Find the PartitionInfo for this volume (assuming 1 partition/volume) */
		{	int partnum;
			pi = (Partition *)RTOA(vvi->Partitions);
			partnum = pi->Partition;
			pii  = (PartitionInfo *) RTOA(ddi->Partitions);
			while(partnum--) pii = (PartitionInfo *) RTOA(pii->Next);
		}

/* Then the DriveInfo for this partition */
		{	int drivenum = pii->Drive;
			dvi  = (DriveInfo *) RTOA(ddi->Drives);
			while(drivenum--) dvi = (DriveInfo *) RTOA(dvi->Next);
		}

		spb = BSIZE/dvi->SectorSize;
/* partition extends to end of disc by default*/
		if( pii->EndCyl == -1 ) pii->EndCyl = dvi->Cylinders-1;

	/* set up offset of first logical sector in partition */
		pii->StartSector = pii->StartCyl * dvi->TracksPerCyl * dvi->SectorsPerTrack;

	/* find the best cylinder group size */	
		if( vvi->CgSize == -1 )
		{
			int cyls = pii->EndCyl-pii->StartCyl+1;
			int cgsize, best;
			int cylinc = 1;
			int spc = dvi->SectorsPerTrack * dvi->TracksPerCyl;
	
			while( ((spc*cylinc)/spb)*spb != spc*cylinc ) cylinc++;
	
			for(cgsize=best=((32/cylinc)*cylinc); cgsize < 64; cgsize+=cylinc)
			{
				if( cyls % cgsize < cyls % best ) best = cgsize;
				if( cyls % best == 0 ) break;
			}
	
			vvi->CgSize = best * dvi->TracksPerCyl * dvi->SectorsPerTrack
						/ spb;
		}
	
/* calculate number of cylinder groups */
		if( vvi->CgCount == -1 )
		{
			int blocks = (dvi->Cylinders * dvi->TracksPerCyl * dvi->SectorsPerTrack)
						/ spb;
			vvi->CgCount = 	blocks/vvi->CgSize;
		}
		
/* CgOffset moves CG onto next track */
		if( vvi->CgOffset == -1 )
			vvi->CgOffset = (dvi->SectorsPerTrack+spb-1)/spb;
		
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
		
/* Inode cache has 1 inode per cache block */
		if( fsi->MaxInodes == -1 ) fsi->MaxInodes = fsi->CacheSize/4;
		
		if( vvi->Next == -1 ) break;
		vvi = (VolumeInfo *)RTOA(vvi->Next);
	}
}
