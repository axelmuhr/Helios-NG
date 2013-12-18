/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- read.c							--
--                                                              --
--	The floppy read routines               			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/read.c,v 1.3 1991/03/28 18:45:30 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*			read_from_file()				*/
/*----------------------------------------------------------------------*/

/** read size bytes from open file at pos
*** Returns number of bytes read
**/

 WORD read_from_file(WORD pos, WORD size, char *buf, FileInfo *info)
{ int to_read;
  int sector,	  /* no of sector to read */
      first_clus, /* 1st cluster to read */
      num, 	  /* no of cluster */
      begin, 	  /* pos of 1st byte in 1st cluster */
      i,
      nb_clus, 	  /* no total of cluster to read */
      reste,
      e;

  Wait( &Var_access );
    
#ifdef debug
    IOdebug("msdosfs : read_from_file(%s), %dbytes",info->Name,size);
#endif

  if( size == 0 ) 
  {
    Signal( &Var_access );
    return 0;
  }

  first_clus = pos / Cluster_byte;
  begin      = pos % Cluster_byte;
  num        = info->Cluster;

 		/* get cluster in chain */
  
  for(i = 0; i < first_clus; i++)
  {
	num = get_fat_entry(num);
	if(num < 0)
	   goto error;
	if( num >= 0xff8 )
	{
#ifdef debugE
	   IOdebug("msdosfs ERROR : read_from_file : eof unexpected");
#endif
	   goto error;
	}
  }

		/* read up to boundary of next cluster */

  to_read = (size > (Cluster_byte - begin)) ? Cluster_byte - begin : size;
  sector = (num - 2)*Cluster_size + Dir_start + Dir_size;
  e = flop_read(sector, Cluster_byte, buf);
  for(i = 0 ; i < to_read ; i++)
	buf[i] = buf[begin + i];
  if( e < to_read )
     goto error;
  buf += to_read;

		/* read full clusters */

  nb_clus = (size-to_read)/Cluster_byte;
  for(i=0; i<nb_clus; i++)
  {
	num = get_fat_entry(num);
	if( num<0 || num>=0xff8 ) goto error;
  	sector = (num - 2)*Cluster_size + Dir_start + Dir_size;
  	if(flop_read(sector, Cluster_byte, buf) < Cluster_byte ) 
	{
          Signal( &Var_access );
	  return (i*Cluster_byte + to_read);
        }
	buf += Cluster_byte;
  }

		/* read last cluster */

  reste = size - to_read - nb_clus*Cluster_byte;
  if(reste>0)
  {
        char *freeloc;
	char *tbuff=Malloc(Cluster_byte);
	if(tbuff == NULL)
		goto error;
	freeloc = tbuff;
	num = get_fat_entry(num);
	if( num<0 || num>=0xff8 )
		{
		Free(freeloc);
		goto error;
		}
  	sector = (num - 2)*Cluster_size + Dir_start + Dir_size;
  	e = flop_read(sector, Cluster_byte, tbuff);
  	for(i = 0 ; i < reste ; i++)
		*buf++ = *tbuff++;
	Free(freeloc);
  	if( e < reste ) 
	{
          Signal( &Var_access );
	  return (nb_clus*Cluster_byte + to_read);
	}
  }

  Signal( &Var_access );
  return(size);

 error :
  Signal( &Var_access );
  return(0);

}

/*----------------------------------------------------------------------*/
/*			flop_read()					*/
/*----------------------------------------------------------------------*/

/**
*** read size bytes from pos
*** if sectro in memory, copy appropriate part
*** else read sector into memory and copy as before

*** Returns no of bytes read
**/
  
WORD flop_read(WORD pos, WORD size, char *buf)
{
  int   j;
  int 	first_sector, 	/* first sector to read */
 	last_sector,
	nb_sector;	/* # sectors to get */
  int sector;

  if( pos!=0 && ( (pos*Sector_size)+size>Nb_sector*Sector_size || pos<0 ) )
  {
#ifdef debugE
      IOdebug("msdosfs ERROR : flop_read : OUT OF DISK (pos=%d)",pos);
#endif
    return 0;
  }

  first_sector = pos;
  nb_sector = size/Sector_size;
  last_sector = first_sector + nb_sector -1;

  for( sector=first_sector; sector<=last_sector; sector++ )
  {
	for(j = 0 ; j < MAX_BUF ; j++)
		if(Sector_buf[j].Sector == sector)
			break;

	if (j == MAX_BUF)	/* not in mem */
	{
		if((j = read_sector(sector)) == -1)
			return 0;
	}

#ifdef debug
	else
		IOdebug("msdosfs : read -> sector in mem %d", j);
#endif

	memcpy(buf, Sector_buf[j].Buf, Sector_size);
	buf += Sector_size;
	time((time_t *)&Sector_buf[j].LastUse);
	

    }/*for read every sector*/

    return size;
}

WORD direct_read(WORD pos, WORD size, char *buf)
{
  DiscReq req;
#ifdef debug
	IOdebug("msdosfs : direct_read - pos %d, size %d", pos, size);
#endif

  	req.DevReq.Request = FG_Read;
  	req.DevReq.Action = Action;
	req.DevReq.SubDevice = DEFAULTPART;
  	req.Pos = pos/Sector_size;
  	req.Size = size/Sector_size;
	if(size % Sector_size)
		req.Size++;
  	req.Buf = buf; 

#ifdef debugE
	IOdebug("msdosfs : direct_read %d %d", req.Pos, req.Size);
#endif
		
	Operate(dcb, &req);
    	if( req.DevReq.Result == 0)
	{

#ifdef debugE
	  IOdebug("msdosfs : read %d bytes", req.Actual);
#endif
	  return size;
	}
#ifdef debugE
		IOdebug("msdosfs ERROR : flop_read : can't read");
#endif
	return 0;
}


/*----------------------------------------------------------------------*/
/*				read_boot()				*/
/*----------------------------------------------------------------------*/

int read_boot( Boot *boot )
{
	int i;
	char *tbuff = Malloc(Sector_size);
#ifdef debug
	IOdebug("msdosfs : read_boot");
#endif
	if(tbuff == NULL)
		{
#ifdef debugE
	IOdebug( "msdosfs ERROR : read_boot can't malloc");
#endif
		return -1;
		}

  if( (i = direct_read(0,Sector_size,tbuff)) == Sector_size )
	{
        memcpy((char *)boot, tbuff, sizeof(Boot));
	Free(tbuff);
  	return 0;
	}

#ifdef debugE
     	IOdebug("msdosfs ERROR : read_boot failed (expected %d, got %d)", sizeof(Boot), i);
#endif
    Free(tbuff);
    return -1;
}

/*----------------------------------------------------------------------*/
/*				read_sector()				*/
/*----------------------------------------------------------------------*/

/* read a given sector */
/* return sector number or -1 on error */

int read_sector(int sector)
{
	long min = Sector_buf[0].LastUse;
	int numbuf, i;

		/* get a free sector buf */
	
	for(numbuf=0, i=0; i<MAX_BUF; i++ )
	  if( min > Sector_buf[i].LastUse )
	  {
		min = Sector_buf[i].LastUse;
		numbuf = i;
	  }

  	Sector_buf[numbuf].Sector = sector;

		/* read sector */
        if(!direct_read(sector*Sector_size,Sector_size,Sector_buf[numbuf].Buf))
		return -1;

	time((time_t *)&Sector_buf[numbuf].LastUse);

	return(numbuf);
}
