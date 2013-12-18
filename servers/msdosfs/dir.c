/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- dir.c							--
--                                                              --
--	The directory handling routines      			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/dir.c,v 1.3 1991/03/28 18:45:30 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*------------------------------------------------------------------*/
/*                        read_dir()                                */
/*------------------------------------------------------------------*/

/*** read the contents of a directory, ignoring deleted files etc
**** returns a DirStream structure, allocated locally
**** The structure contains :
****       the number of entries in dir + an offset +
**** info for each entry in a DirEntry structure
**** (struct DirEntry defined in syslib.h)
****
**** Assumes access lock has been claimed !!
***/

DirStream *read_dir(STRING localname)
{
  DirStream *stream = Null(DirStream); /* info about each entry */
  word	number; 	/* no of valid entries */
  int i;
  DirEntry *entry;
  int num_entries;	/* # of entries in parent dir */
  int chain[MAX_DIR];   /* chain of sector for parent dir */
  DosDirEntry *e;	/* ptr to an entry of parent dir */


#ifdef debug
    IOdebug("msdosfs : read_dir(%s)",localname);
#endif

		/* get chain of sectors for this dir */
  num_entries = get_dir_chain(subdir(localname), &chain[0]);
  if( num_entries < 0 )
     return(Null(DirStream));

		/* alloc space for each entry in dir sectors */
		/* rq : could be more than effectives entries */

  
  stream=(DirStream *) Malloc(sizeof(DirStream)+(num_entries*sizeof(DirEntry)));
  if( stream == Null(DirStream))
    { 
#ifdef debugE
	  IOdebug("msdosfs ERROR : readdir : can't Malloc");
#endif
          return(Null(DirStream));
    }
  
		/* examine and copy each entry in the directory */

  entry = (DirEntry *) &(stream->entries[0]);
  for(i=0, number=0; i<num_entries; i++)
  {
	e = search(i, &chain[0]);
	if( e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	  IOdebug("msdosfs ERROR: read_dir : search failed");
#endif
	  Free(stream);
          return(Null(DirStream));
	}
	
	if( e->Name[0] == 0 )	 /* if last entry in dir, break */
		 break;

	if( e->Name[0] == 0xE5 || (e->Attrib & 0x08) )    /* erased or label */
		 continue;

		/* get & copy unix style name of object */
	unixname((char *)e->Name, (char *)e->Ext, (char *)entry->Name);
	
        entry->Type = (e->Attrib & 0x10) ? Type_Directory : Type_File;
        entry->Flags = 0; 
	entry->Matrix = (e->Attrib & 0x10) ? DefDirMatrix : DefFileMatrix;
#ifdef debug
	IOdebug("msdosfs : read_dir : get %s",entry->Name);
#endif
	number++;
	entry++;
   }

  stream->number = number;
  stream->offset = 0;
#ifdef debug
   IOdebug("msdosfs :$ read_dir");
#endif
  return(stream);
}

/*----------------------------------------------------------------------*/
/*				subdir()				*/
/*----------------------------------------------------------------------*/

/** subdir() find the cluster for a sub-directory
*** Returns : 1st cluster of the subdir , or -1 if error

*** Keep a static copy of last request.
**/

int subdir( string path )
{
	static char tmpb[256];
	char *s, *name, *tmp = (char *)tmpb;
  	int chain[MAX_DIR];
	int num_entries;
	static int first;

#ifdef debug
	  IOdebug("msdosfs : subdir(%s)",path);
#endif

        if( path == NULL )    /* if arg==NULL disable optimisation & return */
	 {
	 tmpb[0] = NULL;
	 return 0;
	 }       

	if( !strcmp(path, tmpb) )     /* if same subdir return last result */
        {
#ifdef debug
	  IOdebug("msdosfs : subdir : return old cluster");
#endif
	  return(first);
	}

 	strcpy(tmp,path);

	if(*tmp == '/' ) 	/* skip first '/' if present */
		tmp++;
				/* if root return root chain */
	if( *tmp == NULL )
	{  
#ifdef debug
	    IOdebug("msdosfs : subdir : return FIRST_ROOT");
#endif
	    first = FIRST_ROOT;
	    return FIRST_ROOT; 
	}

	num_entries = reset_chain(&chain[0]);
	for (s = tmp; *s; s++)
	{
		if (*s == '/') 
		{
			name = tmp;
			*s = NULL;
			first = get_dir_entry(name, &chain[0], num_entries);
			*s = '/';

			if( first < 0 )
				return -1;

			if((num_entries=get_dir_chain(first, &chain[0])) == -1)
				return -1;
			tmp = s+1;
		}
	}

	first = get_dir_entry(tmp, &chain[0], num_entries);

	if( first == 0 )	 /* if subdir == NULL, return old value */
		first = chain[0];

#ifdef debug
	IOdebug("msdosfs : subdir : return %d", first);
#endif
	return(first);
}

/*----------------------------------------------------------------------*/
/*				get_dir_entry()				*/
/*----------------------------------------------------------------------*/

/** look for an entry in a subdir
*** Arguments : subdir_name
		chain	    : directory sector chain
		num_entries : number of directory entries

*** Return : -1 on error, 0 if subdir_name = NULL,
***          else start sector number
**/


int get_dir_entry(string subdir_name, int *chain, int num_entries)
{
	int entry, start;
        char Name[9], Ext[4];
	DosDirEntry *dir;

#ifdef debug
	char newname[15];
#endif
					/* nothing required */
	if ( *subdir_name == NULL)
        {
#ifdef debug
	  IOdebug("msdosfs : get_dir : subdir_name = NULL");
#endif
	  return(0);
        }

#ifdef debug
	  IOdebug("msdosfs : get_dir(%s)",subdir_name);
#endif

	for (entry=0; entry<num_entries; entry++) 
        {
		if((dir = search(entry, chain)) == Null(DosDirEntry))
		     return -1;
					/* if end of dir */
		if (dir->Name[0] == 0)
			break;
					/* if erased or not dir */
		if ((dir->Name[0] == 0xe5) || !(dir->Attrib & 0x10))
			continue;

		fixname(subdir_name, Name, Ext);
#ifdef debug
		unixname((char *)dir->Name, (char *)dir->Ext, newname);
		IOdebug("msdosfs : get_dir : parse %s",newname);
#endif
		if (!strncmp((char *)dir->Name, Name, 8)
		   && !strncmp((char *)dir->Ext, Ext, 3)) 
		{
		   start = dir->Cluster[1]*0x100 + dir->Cluster[0];
#ifdef debug
		   IOdebug("msdosfs : get_dir : find, first=%d",start);
#endif
		   return(start);
		}
	}
#ifdef debugE
	IOdebug("msdosfs ERROR : get_dir : %s is not a dir", subdir_name);
#endif
	return(-1);
}

/*----------------------------------------------------------------------*/
/*				reset_chain()				*/
/*----------------------------------------------------------------------*/

/* 
 * Reset the global variable dir_chain to the root directory.
 * Return the number of entries in root dir.
 */

int reset_chain(int *chain)
{
	int i;

#ifdef debug
	IOdebug("msdosfs : reset_chain");
#endif

	for (i=0; i<Dir_size; i++)
		chain[i] = Dir_start + i;
	return(Nb_entries);
}

/*----------------------------------------------------------------------*/
/*				search()				*/
/*----------------------------------------------------------------------*/

/*
 * Search and extract a directory structure.  The argument is the
 * relative directory entry number (no sanity checking) and the chain 
 * of sectors.
 * It returns a pointer to the directory structure at that location.
 * Attempts to optimize by trying to determine if the buffer needs to be 
 * re-read. The buffer contains a cluster.
 * A call to writedir() will scribble on the real buffer, so watch out!

 * returns : Null(DosDirEntry) if error, else ptr to the entry.
 */

DosDirEntry *search(int num, int *chain)
{
	int skip;
	static int last; /* last cluster read */
	static DosDirEntry dirs[64]; /* large enough for a cluster */
	
	if (num == 0)		/* first call disables optimization */
		last = 0;

	skip = chain[num / Sector_entries];	 /* which sector */

	if( skip != last ) 	/* don't read it if same sector */
        {
#ifdef debug
	  IOdebug("msdosfs : search -> read sector %d",skip);
#endif

	  if(flop_read(skip, Sector_size, (char *)&dirs[0]) < Sector_size )
	  {
		last = 0;
                return Null(DosDirEntry); 
          }
	  last = skip;
	}
	return(&dirs[num % Sector_entries]);   /* which entry in sector */
}
  



/*----------------------------------------------------------------------*/
/*				get_dir_chain()				*/
/*----------------------------------------------------------------------*/

/** follow the sector chain, given the starting cluster
*** Returns : max number of dir entries in chain
*** The calling code must pre-allocate space for the chain
**/

int get_dir_chain(int first, int *chain)
{
  int i,j,next;
  int num = first;
#ifdef debug
  char s[80], *z = &s[0];
  s[0]=NULL;
#endif

  if(first == -1)		/* some previous error */
	return -1;

  if( first == FIRST_ROOT )
  {
#ifdef debug
		IOdebug("msdosfs : get_dir_chain : first = FIRST_ROOT");
#endif
	return(reset_chain(chain));
  }

  for(i=0;;i++)
  {
    chain[i] = (num - 2)*Cluster_size + Dir_start + Dir_size;
#ifdef debug
	if(strlen(&s[0])<75)
	{
	  z += sprintf(z," %d",chain[i]);
        }
#endif
    if( Cluster_size > 1 ) 
      for(j = 1; j < Cluster_size; j++, i++)
	chain[i+1] = chain[i] + 1;

    if( i > MAX_DIR - Cluster_size )
    {
#ifdef debugE
	   IOdebug("msdosfs ERROR : get_chain : dir too large");
#endif
	return(Sector_entries*i);
    }

    if((next = get_fat_entry(num)) == -1)
    {
#ifdef debugE
	IOdebug("msdosfs ERROR : get_chain : fat error, %d cluster bad", next);
#endif
	return(-1);
    }
    num = next;

    if( num >= 0xff8 )
        break;  /* fin de chainage */
  } 

#ifdef debug
  if(z != s)
     IOdebug("msdosfs : get_dir_chain : chain is %d large, %s", i+1, s);
#endif

  return(Sector_entries*(i+1));
}

/*----------------------------------------------------------------------*/
/*				mk_entry()				*/	
/*----------------------------------------------------------------------*/
/** 
*** Create a dos directory entry
***
**/


void mk_entry(string filename, char attrib, int cluster, WORD size,
		 DosDirEntry *ndir)
{
	long clock;
	int i;
	struct tm *now;
	unsigned char hour, min_hi, min_low, sec;
	unsigned char year, month_hi, month_low, day;

	time((time_t*)&clock);
	now = localtime((time_t*)&clock);
	fixname(filename, (char *)ndir->Name, (char *)ndir->Ext);

	ndir->Attrib = attrib; 
	for (i=0; i<10; i++)
		ndir->Reserved[i] = NULL;
	hour = now->tm_hour << 3;
	min_hi = now->tm_min >> 3;
	min_low = now->tm_min << 5;
	sec = now->tm_sec / 2;
	ndir->Time[1] = hour + min_hi;
	ndir->Time[0] = min_low + sec;
	year = (now->tm_year - 80) << 1;
	month_hi = (now->tm_mon+1) >> 3;
	month_low = (now->tm_mon+1) << 5;
	day = now->tm_mday;
	ndir->Date[1] = year + month_hi;
	ndir->Date[0] = month_low + day;
	ndir->Cluster[1] = cluster / 0x100;
	ndir->Cluster[0] = cluster % 0x100;
	ndir->Size[3] = 0;		/* can't be THAT large */
	ndir->Size[2] = size / 0x10000;
	ndir->Size[1] = (size % 0x10000) / 0x100;
	ndir->Size[0] = (size % 0x10000) % 0x100;

	exists_obj(NULL);	/* new entry */
#ifdef debug
	IOdebug("msdosfs : mk_entry: name %s attrib %x", filename,ndir->Attrib);
#endif
}

/*----------------------------------------------------------------------*/
/*				grow()					*/	
/*----------------------------------------------------------------------*/

/**
***  Make a subdirectory or file grow in length by adjunction of a new cluster.
***  Root dir can't grow.
***  Args : cluster = 1st cluster of object chain
		      if 0 allocate a new cluster
	    zero : 1 if new cluster to be zero-filled
	    fat_change : if != NULL gives list of fat sectors modified
***  Returns : -1 on error (disk full) or number of new cluster
**/

int grow(int cluster, int zero, int *fat_change)
{
	int next, last, sector;
/* @@@ should the 2048 below be Cluster_byte long ? */
	static char tbuf[2048]; /* have to be large enough for a cluster */


	last = get_free_cluster();
#ifdef debug
	 IOdebug("msdosfs : grow : cluster=%d, newclus=%d",cluster,last);
#endif
	if (last == -1)
		return(-1);

  	if( cluster )
	{
		while (1) 
		{
			next = get_fat_entry(cluster);
			if (next == -1)
			        return -1;
			if (next >= 0xff8)
				break;
					/* end of cluster chain */
			cluster = next;
		}
					/* mark the end of the chain */
		put_fat_entry(cluster, last,  fat_change);
		put_fat_entry(last,    0xfff, fat_change);
	}

		/* zero the cluster and write it if asked */
        if( zero )
	{ 
          memset((void *)tbuf, (int)NULL, (u_int) Cluster_byte);
	  sector = (last - 2) * Cluster_size + Dir_start + Dir_size;
	  if(flop_write(sector*Sector_size, Cluster_byte, tbuf)!=Cluster_byte) 
		return -1;
	}
	return(last);
}

/*----------------------------------------------------------------------*/
/*			write_dir_entry()			 	*/	
/*----------------------------------------------------------------------*/
/*
 * Write a directory entry.  The first argument is the directory entry
 * number to write to.  The second is a pointer to the directory itself.
 */

int write_dir_entry(int num, DosDirEntry *dir, int *dir_chain)
{
	int skip;
/* @@@ check this - should we malloc ? */
	static DosDirEntry dirs[32]; 

#ifdef debug
	IOdebug("msdosfs : write_dir_entry : num_entry=%d",num);
#endif
					/* which sector */
	skip = dir_chain[num / Sector_entries];

					/* read the sector */
	if(flop_read(skip, Sector_size, (char *) &dirs[0]) < Sector_size)
            return -1;
					/* copy the structure */
	dirs[num % 16] = *dir;
					/* write the sector */
	return ( (flop_write(skip*Sector_size,Sector_size, (char *) &dirs[0])
                      != Sector_size) ? -1 : 0 );
}

/*----------------------------------------------------------------------*/
/*				reduce_dir()				*/
/*----------------------------------------------------------------------*/

/** reduce directory chain by a cluster
*** Args : first = 1st cluster of directory
	   fat_change = modified FAT sectors
*** Returns : -1 if error (invalid entry in FAT)
**/

int reduce_dir(int first, int *fat_change)
{
  int next1, next2; 		/*  num -> next1 -> next2 */
  int num = first;
  
  next1 = get_fat_entry(num);
  if( next1 == -1 )  /* if entry non-valid */
  {
#ifdef debugE
	IOdebug("msdosfs : reduce_dir : fat error, %d cluster bad", next1);
#endif
	return(-1);
  }

  if( next1 > 0xff8 )
  {
#ifdef debugE
	IOdebug("msdosfs : reduce_dir : chain is only one long");
#endif
    return -1;
  }

  while(1)
  {
    next2 = get_fat_entry(next1);
    if( next2 == -1 )  /* if entry non-valid */
    {
#ifdef debugE
	IOdebug("msdosfs : reduce_dir : fat error, %d cluster bad", next2);
#endif
	return(-1);
    }

    if( next2 >= 0xff8 )  /* if end of chain */
    {
	put_fat_entry(next1, 0,     fat_change);
	put_fat_entry(num,   0xfff, fat_change);
#ifdef debug
    	IOdebug("msdosfs : reduce_dir : cluster %d erased",next1);
#endif
        return 0;
    }

    num = next1;
    next1 = next2;

  }
}
