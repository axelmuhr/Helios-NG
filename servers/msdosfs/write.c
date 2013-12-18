/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- write.c							--
--                                                              --
--	The floppy write routines            			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /giga/HeliosRoot/Helios/servers/msdosfs/RCS/write.c,v 1.1 91/03/07 11:35:40 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*				write_to_file()				*/	
/*----------------------------------------------------------------------*/

/** write to open file (struct FileInfo initialised by file_open)
    Arguments : info : struct FileInfo = info on open file
 		pos  : position in file
		size : no of bytes to write
		buf  : ptr to buffer.

    Retour     : no of bytes correctly written
		 otherwise, return 0.

*** Rq : don't save the FAT after writing
***      On error, call undo_fat to free the newly allocated clusters
**/

WORD write_to_file(WORD pos, WORD size, char *buf, FileInfo *info)
{
  word reste = size;
  int total_clus;	/* total clusters in file */
  int to_write;		/* no of bytes to write in current cluster */
  int begin;		/* pos, in the cluster, to start writing */
  int sector;		/* 1st sectour of cluster */
  int num_clus;		/* num of cluster (relative to disk) */
  int first_clus;	/* if file is 0 long, first cluster of written file */
  int update_entry=0;	/* -  -    -   -  , rewrite dir entry with first_clus */
  int size_grow = 0;	/* how many clusters to add to file */
  DosDirEntry dir;
  int chain[MAX_DIR];
  int fat_change[MAX_FAT];
  int i, next, last;

#ifdef debug
   int n=0;
#endif

   for(i=0; i<MAX_FAT-1; i++)
   	fat_change[i] = 0;
   fat_change[MAX_FAT -1] = -1;

   Wait(&Var_access);

#ifdef debug 
    IOdebug("msdosfs  file_write(%s) pos=%d size=%d",info->Name,
		pos,size);
#endif

   if((last = last_clus(info->Cluster)) < 0 )
   {
       Signal(&Var_access);
       return 0;
   }

   if( pos > info->Size )
   {
#ifdef debugE
	  IOdebug("msdosfs ERROR : file_write : pos is after eof");
#endif
	Signal(&Var_access);
 	return 0;
   }

	/* get sectors chain for parent directory */

   if( get_dir_chain(info->DirCluster, &chain[0]) < 0  )
   {
	Signal(&Var_access);
	return -1;
   }

   begin = pos % Cluster_byte;
   to_write = Min(Cluster_byte - begin, reste);

		/* get (disk number of) first cluster to write */
		/* if cluster already exists */

   if( begin!=0 || pos<info->Size )
   {
     total_clus = pos/Cluster_byte;
     num_clus = info->Cluster;
   
     for(i=0; i<total_clus; i++)
     {
	num_clus = get_fat_entry(num_clus);
	if(num_clus < 0) 
    	{
	  Signal(&Var_access);
	  return -1;
    	}

	if( num_clus >= 0xff8 )
	{
#ifdef debugE
	   IOdebug("msdosfs ERROR : write_to_file : eof unexpected");
#endif
	  Signal(&Var_access);
	  return -1;
	}
     }
   }
	/* if file is 0 long get new cluster & update dir entry */
  elif( info->Size == 0 )
  {
    num_clus = first_clus = grow(0,0,NULL);
    if( first_clus < 0 ) 
    {
	  Signal(&Var_access);
	  return -1;
    }
    put_fat_entry(first_clus, 0xfff, fat_change);
    update_entry = 1;
  } 
  else num_clus = last ;

   while( reste > 0 )
   {
		/* if begin. of cluster & eof  -> alloc new cluster */

	if( !begin && pos==info->Size && info->Size!=0 )
	{
#ifdef debug
	  IOdebug("msdosfs : file_write : alloc %d cluster ",++n);
#endif
	  if((num_clus = grow(num_clus, 0, fat_change)) < 0 )
    	  {
	    Signal(&Var_access);
	    return -1;
    	  }
			/* which sector is to be writen */
	  sector = (num_clus-2)*Cluster_size + Dir_start + Dir_size;
	}
	else /* get next sector in chain of file sectors */
	{
          if( num_clus > 0xff8 )
          {
#ifdef debugE
	    IOdebug("msdosfs ERROR : file_write : end of file !!");
#endif
	    goto error;
	  }

	  sector = (num_clus-2)*Cluster_size + Dir_start + Dir_size;
	  if( sector > Nb_sector ) 
	  {
#ifdef debugE
	   IOdebug("msdosfs ERROR : write_to_file : sector is out of disk");
#endif
 	   goto error;
	  }

	  next = get_fat_entry(num_clus);
	  if( num_clus < 0 )
		goto error;
	  if( next < 0xff8 ) 
		num_clus = next;
	}
	
		/* write the cluster */
#ifdef debug
	  IOdebug("msdosfs : file_write : write %d to %d",pos,
		   pos+to_write);
#endif

	if(flop_write( sector*Sector_size + begin, to_write, buf) < to_write )
		goto error;

		/* update file size if necessary */

	pos += to_write;

	if( pos > info->Size )
	{
	  info->Size = pos;
	  size_grow = 1;
	}

	reste -= to_write;
	buf += to_write;
 	begin = pos % Cluster_byte;
 	to_write = Min(Cluster_byte - begin, reste);
   }
		/* end of write, save FAT and dir */

  if( size_grow || update_entry )
  {
    mk_entry(info->Name, info->Attrib, 
	     (update_entry) ? first_clus : info->Cluster, 
	     info->Size, &dir);

	/* rewrite the parent directory */

    if( write_dir_entry(info->NumEntry, &dir, chain) < 0 )
    {
      undo_fat(last);
      Signal(&Var_access);
      return(0);
    }

    if( update_entry )
       info->Cluster = first_clus;
  }

  if( write_fat(fat_change) >= 0 )
  	{
  	Signal(&Var_access);
  	return(size);
	}

error :
#ifdef debugE
  IOdebug("msdosfs ERROR : file_write failed : restore FAT");
#endif
		/* restore the fat changes (in memory only) */
  undo_fat(last);
  Signal(&Var_access);
  return 0;
}

/*----------------------------------------------------------------------*/
/*			flop_write()					*/
/*----------------------------------------------------------------------*/

/** write size bytes at pos in open file

*** if size is not a full-sector, read old sector from disk into memory
*** if sector in mem, modify it
*** write sector to disk

*** Returns number of bytes written
**/
  
WORD flop_write(WORD pos, WORD size, char *buf)
{
  int first_sector, nb_sector, last_sector;
  int sector, offset;
  int begin, start, startbuf, to_change;
  int j;

  if( pos!=0 && ( pos+size>Nb_sector*Sector_size || pos<0 ) )
  {
    IOdebug("msdosfs ERROR : flop_write : OUT OF DISK (pos=%d)",pos);
    return 0;
  }

  first_sector = pos/Sector_size;
  begin = pos % Sector_size;
  nb_sector = (size+begin-1)/Sector_size + 1;
  last_sector = first_sector + nb_sector -1;

   for(sector = first_sector; sector <= last_sector; sector++)
   {
  	offset = sector - first_sector;
  	to_change = Sector_size;
  	if( sector == last_sector )
    		to_change = (size + begin)%Sector_size;

  	if( sector == first_sector )
  	{ to_change = Min(Sector_size - begin, size);
    	  start = begin;
    	  startbuf = 0;
  	}
  	else 
  	{ start = 0;
    	  startbuf = Sector_size*(offset) - begin;
  	}

	for(j = 0 ; j < MAX_BUF ; j++)
		if(Sector_buf[j].Sector == sector)
			break;

	if(j == MAX_BUF && to_change != Sector_size && to_change != 0)
			/* not in mem and not full block write */
		{
#ifdef debugE
		IOdebug("msdosfs : flop_write -> read old sector %d", sector);
#endif
		if((j = read_sector(sector)) == -1)
			return 0;
		}

	if(j != MAX_BUF)
		{
#ifdef debug
	 IOdebug("msdosfs : flop_write : modify %d in-mem buf",j);
#endif
		memcpy(Sector_buf[j].Buf + start, buf+startbuf, to_change);
		time((time_t *)&Sector_buf[j].LastUse);

		}

	if(!direct_write(sector, Sector_size,
		     (j != MAX_BUF) ? Sector_buf[j].Buf : buf + startbuf))
		return 0;
     }

    return size;
}

WORD direct_write(WORD pos, WORD size, char *buf)
{
  DiscReq req;
  int retry;

  	req.DevReq.Request = FG_Write;
  	req.DevReq.Action = Action;
	req.DevReq.SubDevice = DEFAULTPART;
  	req.Pos = pos;
  	req.Size = size/Sector_size;
	if(size % Sector_size)
		req.Size++;
  	req.Buf = buf; 
#ifdef debugE
	IOdebug("msdosfs : direct_write %d %d", req.Pos, req.Size);
#endif
		
  	for( retry = 0; retry<MAX_RETRY; retry++ )
  	{ 
	   Operate(dcb, &req);
    	   if( req.DevReq.Result == 0 ) 
	   {
#ifdef debugE
	     IOdebug("msdosfs : wrote %d bytes", req.Actual);
#endif
	     return size;
	   }
#ifdef debugE
		 
		IOdebug("msdosfs WARN : flop_write :can't write, retry");
#endif
	}
#ifdef debugE
		 
		IOdebug("msdosfs ERROR : flop_write : can't write");
#endif
	return 0;
}
