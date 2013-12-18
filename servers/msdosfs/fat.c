/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- fat.c							--
--                                                              --
--	The FAT processing routines          			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/fat.c,v 1.3 1991/08/19 10:04:48 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*				read_fat()				*/
/*----------------------------------------------------------------------*/

/** read FAT number num
*** argument : num of FAT to read (calling code reads 2nd copy if first
*** is bad
**/
int read_fat(int num)
{
  WORD e;

#ifdef debug
    IOdebug("msdosfs : read_fat %d", num);
#endif

  e = direct_read(Sector_size*(Fat_size*num+1), Fat_size*Sector_size, Fat);

  if( e < Fat_size * Sector_size )
  {
#ifdef debugE
	  IOdebug("msdosfs ERROR : read_fat failed");
#endif
	return -1;
  }
  return(0);
}

/*----------------------------------------------------------------------*/
/*				write_fat()				*/
/*----------------------------------------------------------------------*/

/** write FAT to disk
*** fat_change contains list of sectors to re-write
*** or WRITE_ALL_FAT to re-write the whole FAT

*** the global Good_fat contains 0 normally, but can contain the number
*** of the (known) good FAT
**/

int write_fat(int *fat_change)
{  
  int i,j;

#ifdef debug
	char s[80];
	char *z = (char *)s;
#endif

#ifdef debug
    IOdebug("msdosfs : write_fat");
#endif

  for(i=Good_fat; i<Nb_fat; i++)
  {
		/* if all the FAT is to be rewritten */
   if( fat_change[0] == WRITE_ALL_FAT )
   {
#ifdef debug
        IOdebug("msdosfs : write_fat : write all FAT");
#endif
     if(flop_write((1+i*Fat_size)*Sector_size, Fat_size*Sector_size, Fat)
           < Fat_size*Sector_size )
     {
#ifdef debugE
	  IOdebug("msdosfs ERROR : write_fat failed");
#endif
	return -1;
     }
     continue;
   }
	 	/* else rewrite changed sectors */
#ifdef debug
	if( i == 0 )
	{
          for(j=0; fat_change[j]>0 && strlen(s) < 75; j++)
            z += sprintf(z,"%d ",fat_change[j]);
	  if(z != s)
            IOdebug("msdosfs : write_fat : write sector(s) %s",s);
        }
#endif

    for(j=0; fat_change[j] > 0; j++ )
    {
    	if(flop_write((fat_change[j]+(i*Fat_size))*Sector_size, Sector_size, 
			Fat+(fat_change[j]-1)*Sector_size) < Sector_size )
        {
#ifdef debugE
	   IOdebug("msdosfs ERROR : write_fat failed");
#endif
	 return -1;
	}
    }
 }

 return(0);
}

/*----------------------------------------------------------------------*/
/*				reload()				*/
/*----------------------------------------------------------------------*/

/** reload() reread the fat. This function is called by GSPWorker
*** if the time since the last access is > MAX_ACCESS.
*** Assumes that Var_access lock has been claimed  !!

*** Return : 0 if ok, else error codes
**/

int reload()
{
  Boot boot;
  static word fat_size = 0; /* to avoid reallocation if same size */
  static old_sector_size = 0;
  int i;

#ifdef debug
    IOdebug("msdosfs : reload");
#endif

  if(read_boot(&boot))
    return EO_Medium + EG_Boot;

		/* Analyse boot block */
  
		/* if debug flag list the contains of boot block */
#ifdef debug
	IOdebug("msdosfs : Jump : %d %d",(int)boot.Saut[0],(int)boot.Saut[1]);
	IOdebug("msdosfs : OEM : %s",boot.Oem);
	IOdebug("msdosfs : Sector Size : %d",(int)stoS(boot.SectorSize));
	IOdebug("msdosfs : Sectors per Cluster : %d",(int)boot.ClusterSize);
	IOdebug("msdosfs : Reserved Sectors : %d",(int)stoS(boot.BootSize));
	IOdebug("msdosfs : No. of FATs : %d",(int)boot.NbFat);
	IOdebug("msdosfs : No. of root-directory entries : %d", (int)stoS(boot.NbEntry));
	IOdebug("msdosfs : Total sectors : %d secteurs", (int)stoS(boot.NbSector));
	IOdebug("msdosfs : Media descriptor byte : %x",(int)boot.DiskType);
	IOdebug("msdosfs : Sectors per FAT : %d",(int)stoS(boot.FatSize));
	IOdebug("msdosfs : Sectors per track : %d", (int)stoS(boot.SectorTrack));
	IOdebug("msdosfs : No. of hidden sectors : %d",(int)stoS(boot.HideSector));
#endif

	/* verify some values */

  if( stoS(boot.SectorSize) != driveinfo.SectorSize )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : reload : SectorSize bad : %d",
			stoS(boot.SectorSize));
#endif
        return EO_Medium + EG_WrongSize;
  }
  Sector_size = driveinfo.SectorSize;

  Cluster_size = boot.ClusterSize;
  if( Cluster_size > 4  ||  Cluster_size < 1)
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : reload : boot bad, Cluster_size = %d",
			Cluster_size);
#endif
    return EO_Medium + EG_WrongSize;
  }

  Nb_fat = boot.NbFat;
  if( Nb_fat > 2  ||  Nb_fat < 1 )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : reload : boot bad, Nb_fat = %d",
			Nb_fat);
#endif
    return EO_Medium + EG_WrongSize;
  }

  Fat_size = stoS(boot.FatSize);
  if(Fat_size > 20  ||  Fat_size < 3 )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : reload : boot bad, Fat_size = %d",
			Fat_size);
#endif
    return EO_Medium + EG_WrongSize;
  }

  Nb_entries = stoS(boot.NbEntry);
  Nb_sector = stoS(boot.NbSector);
  Dir_start = Fat_size * Nb_fat + (int)stoS(boot.BootSize);
  Sector_entries = Sector_size/sizeof(DosDirEntry);
#ifdef debug
  IOdebug("msdosfs : sector_entry = %d",Sector_entries);
#endif
  Dir_size  = (Nb_entries+Sector_entries-1)/Sector_entries;
  Cluster_byte = Cluster_size * Sector_size;

		/* if sector size changed, alloc in-mem buffer */
  if( old_sector_size != Sector_size )
  {
    for( i=0; i<MAX_BUF; i++ )
    {
 	if( Sector_buf[i].Buf != NULL )
		Free( Sector_buf[i].Buf );
	Sector_buf[i].Buf = (char *) Malloc(Sector_size);
	if( Sector_buf[i].Buf == NULL )
	{
#ifdef debugE
	  IOdebug("msdosfs ERROR : can't Malloc %d",Sector_size);
#endif
	  return EO_Server + EG_NoMemory;
	}
    }
    old_sector_size = Sector_size;
  }

 		/* disable optimisation for subdir */
  subdir(NULL);
  exists_obj(NULL);
  for( i=0; i<MAX_BUF; i++ )
  {
 	Sector_buf[i].Sector = -1;
 	Sector_buf[i].LastUse = 0;
  }

		/* Fat Malloc and read */
#ifdef debug
  IOdebug("msdosfs : reload : (Free and ) Malloc Fat");
#endif

		/* if new FAT is greater than previous one, alloc */
  if( Fat_size > fat_size )
  {
    if( Fat != NULL) 
    { 
#ifdef debug
	  IOdebug("msdosfs : reload : Free the fat buf");
#endif
	Free(Fat);
	Fat = NULL;
    }
      
    Fat = (char *)Malloc(Fat_size * Sector_size);
    if( Fat == NULL)
    {
#ifdef debugE
	IOdebug("msdosfs ERROR : reload can't Malloc Fat");
#endif
	fat_size = 0;
	return EO_Server + EG_NoMemory;
    }
    fat_size = Fat_size;
  }
		/* try to read first FAT */
  if( read_fat(0) )
  {
#ifdef debugE
	  IOdebug("msdosfs ERROR: reload : can't read FAT 1 try FAT 2 ...");
#endif
  	  if( Nb_fat<2 || read_fat(1) )
  	  {
#ifdef debugE
	     IOdebug("msdosfs ERROR: reload : can't read FAT 2, ABORTED");
#endif
  	    return EO_Medium + EG_Broken;
          }
	  Good_fat = 1;
  }
  else Good_fat = 0;
#ifdef debugE
  IOdebug("msdosfs : read FAT %d OK", Good_fat);
#endif

  return 0;
}

/*----------------------------------------------------------------------*/
/*				get_fat_entry()				*/
/*----------------------------------------------------------------------*/

/*
 * Get and decode a FAT (file allocation table) entry.  The FAT entries
 * are 1.5 bytes long and switch nibbles (.5 byte) according to whether
 * or not the entry starts on a byte boundary.  Returns the cluster 
 * number on success or -1 on failure.
 *
 * Only 12 bit FATs are currently catered for
 */

int get_fat_entry(int num)
{
	unsigned int fat_hi, fat_low, byte_1, byte_2;
	int start;

			/* which bytes contain the entry */
	start = (num * 3) / 2;
	if (start < 0 || start+1 > (Fat_size * Sector_size))
        {
#ifdef debugE
		IOdebug("msdosfs ERROR : get_fat : %d out of FAT", num);
#endif
		return(-1);
	}

	byte_1 = *(Fat + start);
	byte_2 = *(Fat + start + 1);
			/* (odd) not on byte boundary */
	if (num % 2) {
		fat_hi   = (byte_2 & 0xff) << 4;
		fat_low  = (byte_1 & 0xf0) >> 4;
	}
			/* (even) on byte boundary */
	else {
		fat_hi   = (byte_2 & 0xf) << 8;
		fat_low  = byte_1 & 0xff;
	}
	return ((fat_hi + fat_low) & 0xfff);
}


/*----------------------------------------------------------------------*/
/*			put_fat_entry()					*/
/*----------------------------------------------------------------------*/
/*
 * Puts a code into the FAT table.  Is the opposite of get_fat_entry().  No
 * sanity checking is done on the code.  
 * Args : num = number of cluster to modifier
	  code = new value for the FAT entry
	  fat_change : if != NULL on met a jour la liste des secteurs de 
			 FAT modifies.
 * Return -1 on error
 */

int put_fat_entry(int num, int code, int *fat_change)
{
/*
 *	|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|0|1|2|3|4|5|6|7|
 *	| | | | | | | | | | | | | | | | | | | | | | | | |
 *	|  n.0  |  n.5  | n+1.0 | n+1.5 | n+2.0 | n+2.5 |
 *	    \_____  \____   \______/________/_____   /
 *	      ____\______\________/   _____/  ____\_/
 *	     /     \      \          /       /     \
 *	| n+1.5 |  n.0  |  n.5  | n+2.0 | n+2.5 | n+1.0 |
 *	|      FAT entry k      |    FAT entry k+1      |
 */
	int start;
	int x, sectors[2];
	int sector, nb=1;
        int i, j;
					/* which bytes contain the entry */
	start = (num * 3) / 2;
	if (start < 0 || start+1 > (Fat_size * Sector_size))
	{
#ifdef debug
		IOdebug("msdosfs ERROR : put_fat : out of fat");
#endif
	  return(-1);
	}
					/* (odd) not on byte boundary */
	if (num % 2)
	{
		*(Fat+start) = (*(Fat+start) & 0x0f) + ((code << 4) & 0xf0);
		*(Fat+start+1) = (code >> 4) & 0xff;
	}
					/* (even) on byte boundary */
	else
	{
		*(Fat+start) = code & 0xff;
		*(Fat+start+1) = (*(Fat+start+1) & 0xf0) + ((code >> 8) & 0x0f);
	}

  if( fat_change != NULL )
  {
    sectors[0] = start/Sector_size + 1;

		/* if the cluster is on the boundary of 2 sectors */
		/* do both */
    x = ((num+1)*3) /2;
    if( (x/Sector_size + 1) != sectors[0] )
    {
      sectors[1] = sectors[0]+1;
#ifdef debug
	  IOdebug("msdosfs : put_fat : fat entry behind 2 sectors");
#endif
      nb=2;
    }

    for(j=0; j<nb; j++)
    {
      sector = sectors[j];
      for(i = 0; ; i++)
      {
		/* if already repported, break */
	if( fat_change[i] == sector )
	      break;

		/* if end of chain, write new sector */
	if( fat_change[i] == 0 )
        {
	  fat_change[i] = sector; 
#ifdef debug
	   IOdebug("msdosfs : put_fat : new fat sector = %d",sector);
#endif
	  break;
	}
		/* if end of fat_change chain */
	if( fat_change[i] == -1 ) 
	{
#ifdef debugE
	  IOdebug("msdosfs : get_free_c : fat_change chain to small");
#endif
	  fat_change[0] = WRITE_ALL_FAT; /* force all fat rewriting */
	  return 0;
        }
      }
    }
  }
  return(0);
}

/*----------------------------------------------------------------------*/
/*				get_free_cluster()			*/	
/*----------------------------------------------------------------------*/

/** returns new cluster number of -1 if disk full
**/


int get_free_cluster()
{
/* @@@ if we really need this, it should be restored when a reload is done */
  static int free=2;
  int max;
  int begin;

  max = (Nb_sector - Dir_start - Dir_size) / Cluster_size;

  for( begin = free; free < max + 2; free++ )
  {
	if( get_fat_entry(free) == 0 )
		return(free);
  }

		/* if not found parse from beginning of FAT */
  if( begin > 2 )
    for( free=2; free<begin; free++)
	if( get_fat_entry(free) == 0 )
		return(free);

  IOdebug("msdosfs : DISK FULL");
  return -1;

}

/*----------------------------------------------------------------------*/
/*				zapit()					*/	
/*----------------------------------------------------------------------*/

/*
 * Remove a string of FAT entries (delete the file).  The argument is
 * the beginning of the string.  Does not consider the file length, so
 * if FAT is corrupted, watch out!  
 */

int zapit(int fat, int *fat_change)
{
	int next;

#ifdef debug
	 IOdebug("msdosfs : zapit : cluster=%d",fat);
#endif

        if( fat == 0 )
               return 0;

	while (1) 
	{
					/* get next cluster number */
	  next = get_fat_entry(fat);
					/* mark current cluster as empty */
#ifdef debug
	  IOdebug("msdosfs : zapit : next cluster = %d",next);
#endif
	  if( next == -1 || put_fat_entry(fat, 0, fat_change) ) 
	     return -1;
	  if (next >= 0xff8)
	     break;
	  fat = next;
	}
	return 0;
}

/*----------------------------------------------------------------------*/
/*				undo_fat()				*/	
/*----------------------------------------------------------------------*/

/*
** Restore the FAT entries by deleting changes. Giving old last cluster
** of file, reput that cluster as eof and free next cluster of chain.

** RQ : Does not consider the file length, so if FAT is corrupted, watch out!
** 	Modifications are done in memory only.
**/ 

int undo_fat(int oldlast)
{
  int fat = oldlast;
  int next;

  while(1) 
  {
			/* get next cluster number */
	next = get_fat_entry(fat);
			/* old end of file is restored */
	if( fat == oldlast )
	{
	  if( put_fat_entry(fat, 0xfff, NULL) )
              return -1;
	}
 	else
 	{
			/* mark current cluster as empty */
 	  if (put_fat_entry(fat, 0, NULL) || next == -1) 
		return -1;
        }
	if( next >= 0xff8 )
		break;
	fat = next;
  }
  return 0;
}

/*----------------------------------------------------------------------*/
/*				last_clus()				*/	
/*----------------------------------------------------------------------*/
/** find the last cluster in chain
*** Arg : no of 1st cluster of file
**/

int last_clus(int first)
{
  int next;

	/* if file is just created, size=0, first cluster=0 */
  if( first == 0 )
      return 0;

  while(1)
  {
    next = get_fat_entry(first);
    if( next < 0 )
	return -1;
    if( next >= 0xff8 )
	break;
    first = next;
  }
  
  return first;
}
