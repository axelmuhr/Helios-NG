/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- misc.c							--
--                                                              --
--	Miscellaneous routines               			--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/misc.c,v 1.4 1991/08/19 10:04:48 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*			open_file()					*/
/*----------------------------------------------------------------------*/

 int open_file(STRING localname, FileInfo *file_info)
{ 
  int r;

  Wait(&Var_access);

#ifdef debug
  IOdebug("msdosfs : open_file(%s)",localname);
#endif

  r = get_object_info(localname, file_info);

  if( r < 0 || r != Type_File )
  {
#ifdef debugE
    if(r > 0)
	IOdebug("msdosfs WARN : open_file(%s) is a dir",localname);
    else
    	IOdebug("msdosfs ERROR : open_file(%s) failed",localname);
#endif
    Signal(&Var_access);
    return -1;
  }

  Signal(&Var_access);
  return 0;
}

/*----------------------------------------------------------------------*/
/*			close_file()					*/
/*----------------------------------------------------------------------*/

void close_file(FileInfo *info)
{
#ifdef debug
	IOdebug("msdosfs : close_file(%s)",info->Name);
#endif
	info = info;
}


/*----------------------------------------------------------------------*/
/*			object_info()					*/
/*----------------------------------------------------------------------*/

int object_info(string localname, FileInfo *info)
{
  int res;

  Wait(&Var_access);
  res = get_object_info(localname, info);
  Signal(&Var_access);
  return res;
}

/*----------------------------------------------------------------------*/
/*			get_object_info()				*/
/*----------------------------------------------------------------------*/

/** get information about an object
*** FileInfo is filled with the info

*** Returns : -1 if error. Type_Nothing if object non-existent
*** else type of object
**/

int get_object_info(string localname, FileInfo *info)
{
  char *filename, *pathname;
  int start, entry;
  int num_entries; 
  int chain[MAX_DIR];
  DosDirEntry *e;
  char Name[9], Ext[4];
  struct tm tm;
  char *freeloc;
#ifdef debug
  char newname[15];
#endif

#ifdef debug
    IOdebug("msdosfs : get_object_info(%s)",localname);
#endif

  if( !strcmp(localname, "/") )
  {
	strcpy(info->Name, "/");
 	time((time_t *)&info->Time);
 	return Type_Directory;
  }
   		
  freeloc = get_name_and_path(localname, &filename, &pathname);
  if( freeloc == NULL || filename[0]==NULL)
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : get_object_info : no name");
#endif
	try_and_free(freeloc);
	return -1;
  }

  if ( (num_entries = get_dir_chain(start = subdir(pathname), &chain[0])) < 0 )
	{
	Free(freeloc);
	return -1;
	}
  
		/* examine each entry in the directory */
  for(entry=0; entry < num_entries; entry++)
  {
	e = search(entry, &chain[0]);
	if( e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	    
	    IOdebug("msdosfs ERROR : get.o_info : search failed");
#endif
	  Free(freeloc);
          return -1;
	 }
#ifdef debug
	 strncpy(newname,(char *)e->Name,8); newname[8]='\0';
	 IOdebug("msdosfs : get_object_info : look %s",newname);
#endif
	
		/* if last entry in dir, break */
	if( e->Name[0] == 0 ) break;

		/* skip if erased or label or dir entry */
	if(( e->Name[0] == 0xE5 ) || ( e->Attrib & 0x08 ))
	        continue;

	fixname(filename, Name, Ext);

			/* if file found */
	if (!strncmp((char *)e->Name, Name, 8) && !strncmp((char *)e->Ext, Ext, 3)) 
	{
		unixname((char *)e->Name, (char *)e->Ext, info->Name);
		if( !(e->Attrib & 0x10) )
		  info->Size 	 = e->Size[0] + ((word)e->Size[1])*0x100 + 
					((word)e->Size[2])*0x10000;	
		else info->Size  = 0;

		info->Cluster 	 = e->Cluster[1]*0x100 + e->Cluster[0];
		info->DirCluster = start;
		info->NumEntry 	 = entry;
		info->Attrib 	 = (int)e->Attrib;
		info->Use	 = 1;
		info->Pos 	 = 0;
			/* dos time to unix time */
   		tm.tm_sec        = (((u_int)e->Time[0])&0x1f) * 2;
   		tm.tm_min        = (((u_int)e->Time[0])>>5) + 
			           (((u_int)e->Time[1])&7)*8;
   		tm.tm_hour       = ((u_int)e->Time[1])>>3;
   		tm.tm_mday       = ((u_int)e->Date[0])&0x1f;
   		tm.tm_mon        = (((u_int)e->Date[0])>>5) + 
				   ((((u_int)e->Date[1])&1)*8) - 1;
   		tm.tm_year       = 80 + (((u_int)e->Date[1])>>1); 
   		tm.tm_isdst      = -1;

   		info->Time       = mktime(&tm);
     		Free(freeloc);
		return( (e->Attrib&0x10) ? Type_Directory :Type_File );
        }
     }
#ifdef debugE
	IOdebug("msdosfs ERROR : get_object_info : %s is not a file",
				filename);
#endif
     Free(freeloc);
     return Type_Nothing;
}

/*----------------------------------------------------------------------*/
/*				create_obj()				*/	
/*----------------------------------------------------------------------*/
/** create a file or directory; create entry in parent directory

*** Rq : errors cna present problems
 	 example : if all is OK but FAT write fails, what shall we do ?
**/

int create_obj(string name, int type)
{
   int 	parent_clus, 	/* number of first cluster of parent dir */
        num_entries,	/* max entries in parent dir (#sector/sector_entry) */
       	entry, 
	find = -1,
       	i, 
 	obj_clus;	      	/* new cluster for new dir */
   DosDirEntry *e, 	      	/* ptr to each entry of parent dir */
   	  	new_entry;      /* new entry for new object */
   char *pathname, *objname;
   int chain[MAX_DIR];          /* chain of sectors for parent dir */
   int fat_change[MAX_FAT];     /* secteurs de la fat modifie */
   char attrib;
   u_int buflen, sector;
   char Name[9], Ext[4];
   char tbuf[2048];             /* have to be large enough for a cluster */
			        /* used to initialise a new dir */
   char *freeloc;

   for(i=0; i<MAX_FAT-1; i++)
   	fat_change[i] = 0;
   fat_change[MAX_FAT -1] = -1;

   Wait(&Var_access);

#ifdef debug
    IOdebug("msdosfs : create_obj(%s, type=%d)",name,type);
#endif
  
   /* one special case - create "/" is not allowed */

   if(name[0] == '/' && name[1] == 0)
   {
	Signal(&Var_access);
	return -1;
   }

   freeloc = get_name_and_path(name, &objname, &pathname);
   if( freeloc == NULL || objname[0] == NULL )
   {
#ifdef debugE
	IOdebug("msdosfs ERROR : create_obj : name = NULL");
#endif
	try_and_free(freeloc);
	Signal(&Var_access);
	return -1;
   }


		/* get chain of parent dir */
   
   num_entries = get_dir_chain(parent_clus = subdir(pathname), &chain[0]);
   if( num_entries < 0 ) 
   {
	Free(freeloc);
	Signal(&Var_access);
	return -1;
   }
		/* search a free entry */

   for(entry=0; entry<num_entries; entry++)
   {
	e = search(entry, &chain[0]);
	if( e == Null(DosDirEntry) )  
   	{
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
   	}
	
		/* if label entry, skip */
	if( e->Attrib & 0x08 )
	    continue;

		/* compare objname */

	fixname(objname, Name, Ext);

			/* if it matches - don't create !! */

	if (!strncmp((char *)e->Name, Name, 8) && !strncmp((char *)e->Ext, Ext, 3)) 
   	{
#ifdef debugE
	  IOdebug("msdosfs ERROR : create_obj : object exists");
#endif
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
   	}

		/* if last or erased entry in dir */

	if( find == -1 && (e->Name[0] == 0 || e->Name[0] == 0xE5 )) 
        {
	   find = entry;		/* remember free position for later */
#ifdef debug
	   IOdebug("msdosfs : create_obj : new entry is %d",entry);
#endif
 	}
   }

    		/* if no space, get a new cluster for dir */
    if(find == -1)
    {
#ifdef debug
	  IOdebug("msdosfs :    create_obj : get new cluster for dir ");
#endif

	if( pathname[1] == '\0' )  /* if root, can't expand */
	{
 	  IOdebug("msdosfs ERROR : create_obj : ROOT DIR is FULL");
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
        }
		/* expand the chain of clusters */
	if( grow(parent_clus, 1, fat_change) < 0)
	{ 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
	}

		/* update the dir chain */
	if( get_dir_chain(parent_clus, &chain[0]) < 0 ) 
	{ 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
	}
    }

		/* if new object = dir, get & init a cluster */

    if( type == Type_Directory )
    {
#ifdef debug
	IOdebug("msdosfs :    creat_obj : get new clus for new dir");
#endif
      	obj_clus = grow(0,0,NULL);
      	if( obj_clus < 0 ) 
	{ 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
	}
		/* initialise . and .. */
#ifdef debug
		IOdebug("msdosfs :     create_obj : initialise . & ..");
#endif
	buflen = Cluster_byte;
	mk_entry(".", 0x10, obj_clus, 0, (DosDirEntry *)&tbuf[0]);
		/* mk entry for .. */
	mk_entry("..", 0x10, parent_clus, 0, (DosDirEntry *)&tbuf[32]);
	memset((void *)(tbuf+64), NULL, buflen-64); 

			/* write the cluster */
#ifdef debug
	IOdebug("msdosfs : creat_obj : write new cluster for new dir");
#endif
	sector = (obj_clus - 2) * Cluster_size + Dir_start + Dir_size;
	if( flop_write(sector*Sector_size, buflen, tbuf) != buflen ) 
	{ 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
	}

		/* first clust of new object = end of chain */
    	put_fat_entry(obj_clus, 0xfff, fat_change);
    }
		/* else first cluster of new file = 0 */
    else obj_clus = 0;


		/* what the dos attrib for the new object */
    attrib = (char)( (type==Type_Directory) ? 0x10 : 0x20 );

		/* fill-in the new_entry structure */
    mk_entry(objname, attrib, obj_clus, 0, &new_entry);

    if( write_dir_entry((find == -1) ? entry : find, &new_entry, &chain[0]) ) 
    { 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
    }

#ifdef debug
    IOdebug("msdosfs :    create_obj : save the FAT");
#endif
    if( write_fat(&fat_change[0]) ) 
    { 
	  Free(freeloc);
  	  Signal(&Var_access);
	  return -1;
    }

    Free(freeloc);
    Signal(&Var_access);
    return 0;
}

/*----------------------------------------------------------------------*/
/*			delete_object()					*/
/*----------------------------------------------------------------------*/

/** Remove file or directory
*** 
*** Args : object name
*** Returns : 0  ok, 1 if object nonexistent, -1 if disk error
***
**/

int delete_object(string localname)
{
  char *objname , *pathname;
  int entry;
  int num_entries; 
  int chain[MAX_DIR];
  DosDirEntry *e;
  char Name[9], Ext[4];
  int fat_change[MAX_FAT];
  int cluster, i;
  char *freeloc;

#ifdef debug
  char newname[15];
#endif

	/* initialize fat_change */
  for(i=0; i<MAX_FAT-1; i++)
       fat_change[i] = 0;
  fat_change[MAX_FAT -1] = -1;

  Wait(&Var_access);

#ifdef debug
    IOdebug("msdosfs : delete_obj(%s)",localname);
#endif

  freeloc = get_name_and_path(localname, &objname, &pathname);
  if( freeloc == NULL || objname[0]==NULL )
  {
#ifdef debugE
		
	IOdebug("msdosfs ERROR : delete_object : no name");
#endif
	try_and_free(freeloc);
	Signal(&Var_access);
	return -1;
  }

  num_entries = get_dir_chain(subdir(pathname), &chain[0]);
  if( num_entries < 0 ) 
  {
	Free(freeloc);
	Signal(&Var_access);
	return -1;
  }
  
		/* examine each entry in the directory */
  for(entry=0; entry<num_entries; entry++)
  {
	e = search(entry, &chain[0]);
	if( e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	    
	  IOdebug("msdosfs ERROR : delete_object : search failed");
#endif
	  Free(freeloc);
	  Signal(&Var_access);
          return -1;
	}
	
		/* if last entry in dir, break */
	if( e->Name[0] == 0 )
	   break;

		/* skip if erased or label */
	if(( e->Name[0] == 0xe5 ) || ( e->Attrib & 0x08 ))
	   continue;
	 
#ifdef debug
	 strncpy(newname,(char *)e->Name,8); newname[8]='\0';
	 IOdebug("msdosfs : delete_object : look %s (%d)",
		  newname, e->Cluster[0] + e->Cluster[1]*0x100);
#endif

	fixname(objname, Name, Ext);

		/* if found */
	if (!strncmp((char *)e->Name, Name, 8) && !strncmp((char *)e->Ext, Ext, 3)) 
	{
		if(e->Attrib & 0x10)	/*directory*/
			{
			DirStream *stream = read_dir(localname);
			if (stream == Null(DirStream))
				{
	  			Free(freeloc);
	  			Signal(&Var_access);
				return( Type_Directory );
				}

			if (stream->number > 2)	/*directory not empty*/
				{
	  			Free(freeloc);
	  			Signal(&Var_access);
				free(stream);
				return( Type_Directory );
				}
			free(stream);
			}

		e->Name[0] = 0xe5;

    		if( write_dir_entry(entry, e, &chain[0]) ) 
		{
	  		Free(freeloc);
	  		Signal(&Var_access);
			return( (e->Attrib&0x10) ? Type_Directory : Type_File );
		}

			/* delete chain of object */
		cluster = e->Cluster[0] + e->Cluster[1]*0x100;
		if( zapit(cluster, &fat_change[0]) ) 
		{
	  		Free(freeloc);
	  		Signal(&Var_access);
			return( (e->Attrib&0x10) ? Type_Directory : Type_File );
		}
#ifdef debug
		IOdebug("msdosfs : del_obj : rewrite FAT ...");
#endif
		if( write_fat((int *)fat_change) )
		{
	  		Free(freeloc);
			Signal(&Var_access);
			return( (e->Attrib&0x10) ? Type_Directory : Type_File );
		}

	  	Free(freeloc);
		Signal(&Var_access);
		return(0);
        }
  }

#ifdef debugE
  IOdebug("msdosfs ERROR : delete_object : %s does not exist", objname);
#endif

  Free(freeloc);
  Signal(&Var_access);
  return Type_Nothing;
}

/*----------------------------------------------------------------------*/
/*			rename_object()					*/
/*----------------------------------------------------------------------*/

/** Rename file or directory
***
*** several choices here :
***
***	1) rename file to file same directory (simple case)
***	2) rename dir to dir same directory (simple case)
***	3) rename file to file different directory
***	4) rename dir to dir different directory
***
*** The later two cases are more difficult, in each case, retrieve old
*** directory information, remove old entry and make a new one. For
*** the directory move, make sure that we don't move a directory further
*** down (away from root) the same branch of the directory tree.
*** 
*** Args : oldname, newname
*** Returns : type of object if ok, Type_Nothing if non-existent
	     -1 if error
**/

int rename_object(string oldname, string newname)
{
  char *old_objname , *old_pathname;
  char *new_objname , *new_pathname;
  int simplecase = FALSE, badmove = FALSE;
  int old_start, new_start;
  int entry, entries; 
  int old_chain[MAX_DIR];
  char Name[9], Ext[4];
  DosDirEntry *old_e, *new_e;
  char *old_freeloc, *new_freeloc;

#ifdef debug
  char name[15];
#endif

  Wait( &Var_access );
    
#ifdef debug
  IOdebug("msdosfs : rename_obj : %s->%s",oldname,newname);
#endif

  old_freeloc = get_name_and_path(oldname, &old_objname, &old_pathname);
  new_freeloc = get_name_and_path(newname, &new_objname, &new_pathname);

  if( old_freeloc == NULL || old_objname[0]==NULL ||
	 new_freeloc == NULL || new_objname[0]==NULL )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : rename_object : no name");
#endif
	try_and_free(old_freeloc);
	try_and_free(new_freeloc);
	Signal(&Var_access);
	return -1;
  }

  if((old_start = subdir(old_pathname)) < 0 ||
  	(new_start = subdir(new_pathname)) < 0 )
  {
	Free(old_freeloc);
	Free(new_freeloc);
	Signal(&Var_access);
	return -1;
  }

  if(old_start == new_start)		/* same directory - simple case */
	simplecase = TRUE;
  else					/* bad directory move ? */
	if(substr(old_pathname, new_pathname))
		badmove = TRUE;

  if( (entries = get_dir_chain(old_start, &old_chain[0])) < 0)
  {
	Free(old_freeloc);
	Free(new_freeloc);
	Signal(&Var_access);
	return -1;
  }
  
		/* for each entry */
  for(entry=0; entry < entries; entry++)
  {
	old_e = search(entry, &old_chain[0]);
	if( old_e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	    IOdebug("msdosfs ERROR : rename_object : search failed");
#endif
	  Free(old_freeloc);
	  Free(new_freeloc);
	  Signal(&Var_access);
          return -1;
	}
	
		/* if last entry in dir, break */
	if( old_e->Name[0] == 0 ) break;

		/* skip if erased or label */
	if(( old_e->Name[0] == 0xe5 ) || ( old_e->Attrib & 0x08 ))
	      continue;
	 
#ifdef debug
	strncpy(name,(char *)old_e->Name,8); name[8]='\0';
	IOdebug("msdosfs : rename_object : look %s/%s", old_pathname, name);
#endif

	fixname(old_objname, Name, Ext);

			/* if found */
	if (!strncmp((char *)old_e->Name, Name, 8) &&
		 !strncmp((char *)old_e->Ext, Ext, 3)) 
	{
		if(simplecase)
		  {
		  fixname(new_objname, (char *)old_e->Name, (char *)old_e->Ext);
    		  if( write_dir_entry(entry, old_e, &old_chain[0]) ) 
	  	  {
	  		Free(old_freeloc);
	  		Free(new_freeloc);
  		  	Signal(&Var_access);
			return -1;
		  }
	  	  Free(old_freeloc);
	  	  Free(new_freeloc);
  		  Signal(&Var_access);
		  return( (old_e->Attrib&0x10) ?
			 Type_Directory : Type_File );
		  }
		else
		{
		  int save_entry = entry;	/* remember old pos */
		  int new_chain[MAX_DIR];
  		  DosDirEntry old_dirent;

		  if(old_e -> Attrib & Type_Directory && badmove)
			{
#ifdef debugE
			IOdebug("msdosfs : rename_obj - recursive dir");
#endif
	  		Free(old_freeloc);
	  		Free(new_freeloc);
  		  	Signal(&Var_access);
			return -1;
		  }
			
		  memcpy(&old_dirent, old_e, sizeof(DosDirEntry));

		  entries = get_dir_chain(new_start, &new_chain[0]);
		  if(entries < 0)
	  	  {
	  		Free(old_freeloc);
	  		Free(new_freeloc);
  		  	Signal(&Var_access);
			return -1;
		  }

		  for(entry = 0; entry < entries ; entry++)
		  {
			new_e = search(entry, &new_chain[0]);
			if( new_e == Null(DosDirEntry) ) 
		  	{
#ifdef debugE
		  IOdebug("msdosfs ERROR : rename_object : new_search failed");
#endif
	  			Free(old_freeloc);
	  			Free(new_freeloc);
	  			Signal(&Var_access);
          			return -1;
			}
	
					/* if free entry in dir */
			if( new_e->Name[0] == 0  || 
				new_e->Name[0] == 0xe5)
			{
			  new_e = &old_dirent;
		  	  fixname(new_objname, (char *)new_e->Name,
					       (char *)new_e->Ext);

    			  if( write_dir_entry(entry, new_e, &new_chain[0])) 
	  		  {
	  		    Free(old_freeloc);
	  		    Free(new_freeloc);
  	  		    Signal(&Var_access);
			    return -1;
			  }

			  old_dirent.Name[0] = 0xe5;
    			  if(write_dir_entry(save_entry, new_e, &old_chain[0])) 
	  		  {
	  		    Free(old_freeloc);
	  		    Free(new_freeloc);
  	  		    Signal(&Var_access);
			    return -1;
			  }

	  		  Free(old_freeloc);
	  		  Free(new_freeloc);
  			  Signal(&Var_access);

			  return( (old_e->Attrib&0x10) ?
			 	Type_Directory : Type_File );
			}
		  }
		}
        }
  }

#ifdef debugE
  IOdebug("msdosfs ERROR : rename_object : %s does not exist", objname);
#endif
  Free(old_freeloc);
  Free(new_freeloc);
  Signal(&Var_access);
  return Type_Nothing;
}

/*----------------------------------------------------------------------*/
/*			exists_obj()					*/ 
/*----------------------------------------------------------------------*/ 
/** parse a pathname to see if object exist on disk
***
*** Args : localname
***
*** Returns : type of object (Type_File or Type_Directory).
	     or -1 if error
*** RQ : if localname = NULL -> disable optimisation.
**/

int exists_obj(string localname)
{
  char *objname , *pathname;
  int entry;
  int num_entries; 
  int chain[MAX_DIR];
  char Name[9], Ext[4];
  DosDirEntry *e;
  static type;   /* type of last object searched */
  static char lastname[Name_Max];
  char *freeloc;

#ifdef debug
  char name[15];
#endif


		/* if ask for disable optimisation */
  if( localname == NULL ) 
  {
    lastname[0]=NULL;
    return 0;
  }
		/* if root, return Type_Directory */
  if( !strcmp(localname,"/") )
    return Type_Directory;

  Wait(&Var_access);

#ifdef debug
  IOdebug("msdosfs : exists_obj : %s",localname);
#endif

 
  if( !strcmp(localname, (char *)lastname) )
  { 
#ifdef debug
    IOdebug("msdosfs : exists_obj : same object, type=%d",type);
#endif
    Signal(&Var_access);
    return type;
  }


  freeloc = get_name_and_path(localname, &objname, &pathname);
  if( freeloc == NULL || objname[0]==NULL )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : exists_obj : no name");
#endif
	try_and_free(freeloc);
    	Signal(&Var_access);
	return EG_Unknown + EO_Name;
  }

  num_entries = get_dir_chain(subdir(pathname), &chain[0]);
  if( num_entries < 0 ) 
  {
    Free(freeloc);
    Signal(&Var_access);
    return EG_Broken + EO_Medium;
  }
  
  for(entry=0; entry<num_entries; entry++)
  {
	e = search(entry, &chain[0]);
	if( e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	  IOdebug("msdosfs ERROR : exists_object : search failed");
#endif
	  Free(freeloc);
    	  Signal(&Var_access);
          return EG_Unknown + EO_Name;
	}
	
		/* if last entry in dir, break */
	if( e->Name[0] == 0 ) break;

		/* skip if erased or label */
	if(( e->Name[0] == 0xe5 ) || ( e->Attrib & 0x08 ))
		 continue;
#ifdef debug
	strncpy(name,(char *)e->Name,8); name[8]='\0';
	IOdebug("msdosfs : exists_object : look %s",name);
#endif

	fixname(objname, Name, Ext);

			/* if found */
	if (!strncmp((char *)e->Name, Name, 8) && !strncmp((char *)e->Ext, Ext, 3)) 
	{
	    type = (e->Attrib & 0x10) ? Type_Directory : Type_File;
	    strcpy(&lastname[0], localname);
	    Free(freeloc);
    	    Signal(&Var_access);
	    return type;
 	}
  }

#ifdef debugE
  IOdebug("msdosfs ERROR : exists_object : %s does not exist", objname);
#endif

  strcpy(&lastname[0], localname);
  type = Type_Nothing;
  Free(freeloc);
  Signal(&Var_access);
  return type;
}

/*----------------------------------------------------------------------*/
/*			change_date()					*/
/*----------------------------------------------------------------------*/

/** change date for a file or directory
*** 
*** Args : localname
*** Returns : Type of object : Type_File, Type_Directory, Type_Nothing
		-1 on error
    RQ : doesn't change date of directories
**/

int change_date(string localname)
{
  char *objname , *pathname;
  int entry;
  int num_entries; 
  int chain[MAX_DIR];
  char Name[9], Ext[4];
  DosDirEntry *e;
  long clock;
  struct tm *now;
  unsigned char hour, min_hi, min_low, sec;
  unsigned char year, month_hi, month_low, day;
  char *freeloc;

#ifdef debug
  char name[15];
#endif

  Wait(&Var_access);

#ifdef debug
  IOdebug("msdosfs : change_date : %s",localname);
#endif

  freeloc = get_name_and_path(localname, &objname, &pathname);
  if( freeloc == NULL || objname[0]==NULL )
  {
#ifdef debugE
	IOdebug("msdosfs ERROR : change_date : no name");
#endif
	try_and_free(freeloc);
	Signal(&Var_access);
	return -1;
  }

  num_entries = get_dir_chain(subdir(pathname), &chain[0]);
  if( num_entries < 0 )
  {
	 Free(freeloc);
	 Signal(&Var_access);
	 return -1;
  }
  
  for(entry=0; entry<num_entries; entry++)
  {
	e = search(entry, &chain[0]);
	if( e == Null(DosDirEntry) ) 
	{
#ifdef debugE
	    IOdebug("msdosfs ERROR : change_date : search failed");
#endif
	  Free(freeloc);
	  Signal(&Var_access);
          return -1;
	}
	
		/* if last entry in dir, break */
	if( e->Name[0] == 0 ) break;

		/* skip if erased or label */
	if(( e->Name[0] == 0xe5 ) || ( e->Attrib & 0x08 ))
	      continue;
	 
#ifdef debug
	strncpy(name,(char *)e->Name,8); name[8]='\0';
	IOdebug("msdosfs : change_date : look %s",name);
#endif

	fixname(objname, Name, Ext);

			/* if found */
	if (!strncmp((char *)e->Name, Name, 8) && !strncmp((char *)e->Ext, Ext, 3)) 
	{
          if( e->Attrib & 0x10 ) 
          {
	    Free(freeloc);
	    Signal(&Var_access);
	    return Type_Directory;
 	  }

  	  time((time_t *)&clock);

  	  now        = localtime((time_t *)&clock);
	  hour       = now->tm_hour << 3;
	  min_hi     = now->tm_min >> 3;
	  min_low    = now->tm_min << 5;
	  sec        = now->tm_sec / 2;
	  e->Time[1] = hour + min_hi;
	  e->Time[0] = min_low + sec;
	  year       = (now->tm_year - 80) << 1;
	  month_hi   = (now->tm_mon+1) >> 3;
	  month_low  = (now->tm_mon+1) << 5;
	  day        = now->tm_mday;
	  e->Date[1] = year + month_hi;
	  e->Date[0] = month_low + day;

    	  if( write_dir_entry(entry, e, &chain[0]) ) 
  	  {
	    Free(freeloc);
	    Signal(&Var_access);
   	    return -1;
	  }

	  Free(freeloc);
  	  Signal(&Var_access);
	  return Type_File;
        }
  }

#ifdef debugE
  IOdebug("msdosfs ERROR : change_date : %s does not exist", objname);
#endif

  Free(freeloc);
  Signal(&Var_access);
  return Type_Nothing;
}

/*----------------------------------------------------------------------*/
/*			drive_statistics()				*/
/*----------------------------------------------------------------------*/

/** return statistics of drive (for df etc)
**/

int drive_statistics(WORD *sizeptr, WORD *availptr)
{
  int i;
  word size=0;

  Wait(&Var_access);

  *sizeptr = Nb_sector * Sector_size;

  for(i = 2; i < ( (Nb_sector - Dir_start - Dir_size) / Cluster_size) + 2; i++)
     if( get_fat_entry(i) == 0 )
	size++;

  *availptr = size * Cluster_byte;
  
#ifdef debug
  IOdebug("msdosfs : drive_statistics");
#endif

  Signal( &Var_access );
  return 0;
}

/*----------------------------------------------------------------------*/
/*			format_disc() 					*/ 
/*----------------------------------------------------------------------*/ 
/** format disc
***
*** Args : mode (high or low density)
***
*** Returns : 0 if OK else error code
**/

int format_disc(word mode)
{
    FormatReq req;
    char *bootblock;
    int reret, sector;
    int fat_change[1];
    static unsigned int default_boot_lowden[] = 
	{ 
	0x48903ceb, 0x4f494c45, 0x00323153, 0x00010202,
	0xa0007002, 0x0003f905, 0x00020009, 0x00000000
	};

    static unsigned int default_boot_highden[] = 
	{ 
	0x48903ceb, 0x4f494c45, 0x00323153, 0x00010102,
	0x4000e002, 0x0009f00b, 0x00020012, 0x00000000
	};

#ifdef debug
    IOdebug("msdosfs : format_disc, mode = %x", mode);
#endif

    Wait( &Var_access );

    req.DevReq.Request = FG_Format;
    req.DevReq.Action = Action;
    req.DevReq.SubDevice = DEFAULTPART;
    req.StartCyl = 0;
    req.EndCyl = NCYLS -1;
    req.Interleave = 0;     /* @@@ what should this be ? */
    req.TrackSkew = 0;      /* @@@ what should this be ? */
    req.CylSkew = 0;        /* @@@ what should this be ? */

    Operate(dcb, &req);		/* do format */
    if( req.DevReq.Result != 0 )	/* failure */
	{
#ifdef debug
	IOdebug("msdosfs : format_disc - format failed");
#endif
        Signal( &Var_access );
	return req.DevReq.Result;
	}
#ifdef debug
    IOdebug("msdosfs : format_disc - O.K.");
#endif

	/* format OK - now write boot block, FATs and root directory */

    /* set up default boot block */

    if((bootblock = Malloc(Sector_size)) == NULL)
	{
#ifdef debug
        IOdebug("msdosfs : format_disc - can't malloc");
#endif
        Signal( &Var_access );
	return EG_NoMemory+ EO_Server ;
	}

    memcpy(bootblock, (mode & DT_HIGHDEN) ?
            (char *) default_boot_highden : (char *) default_boot_lowden,
		sizeof(struct Boot));

    if(direct_write(0, Sector_size, bootblock) != Sector_size)
	{
#ifdef debug
        IOdebug("msdosfs : format_disc - write boot failed");
#endif
	Free(bootblock);
	Signal( &Var_access );
	return EG_Broken+ EO_Medium ;
	}

    Free(bootblock);
	
    if((reret = reload()) != 0)
	{
#ifdef debug
	IOdebug("msdosfs : format_disc - reload failure");
#endif
	Signal( &Var_access );
	return reret;
	}

    memset(Fat, 0, Fat_size * Sector_size);	/* zero FAT */

    /* as we have a clean buffer, use it to zero root directory */

    for(sector = Dir_start; sector < Dir_start+Dir_size; sector++)
        if(direct_write(sector, Sector_size, Fat) != Sector_size)
	    {
#ifdef debug
            IOdebug("msdosfs : format_disc - write root dir failed");
#endif
	    Signal( &Var_access );
	    return EG_Broken+ EO_Medium ;
	    }

    put_fat_entry(0, (mode & DT_HIGHDEN) ? 0xff0 : 0xff9, NULL);
    put_fat_entry(1, 0xfff, NULL);

    fat_change[0] = WRITE_ALL_FAT;       /* try to rewrite FATs */

    if( write_fat(fat_change) < 0)
	{
        Signal( &Var_access );
	return EG_Broken+ EO_Medium ;	 /* No good */
	}

    Signal( &Var_access );
    return 0;
}

/*----------------------------------------------------------------------*/
/*			check_disc() 					*/ 
/*----------------------------------------------------------------------*/ 
/** check disc for corrupt FAT, lost chains etc.
***
*** Returns : 0 if OK else error code
**/

int check_disc(word mode, word *ret_code)
{
    int error;

#ifdef debug
    IOdebug("msdosfs : check_disc, mode = %x", mode);
#endif

    *ret_code = 0;				/* All OK */

    Wait( &Var_access );

    if((error = reload()) != 0)
	{
	Signal( &Var_access );
	return error;
	}

    if(Good_fat)				/* FAT 0 broken */
	{
	*ret_code |= BAD_FAT;
	if(mode & FSCK_WRITE)
		{
		int fat_change[1];
		int savegood = Good_fat;

		fat_change[0] = WRITE_ALL_FAT;       /* try to rewrite FAT 0 */
		Good_fat = 0;
		if( write_fat(fat_change) < 0)
			{
			Good_fat = savegood;
			Signal( &Var_access );
			return EG_Broken+ EO_Medium ;	/* No good */
			}
		*ret_code |= BAD_FAT_FIXED;
		}
	}

    /* FATs OK - start to analyse disc */

    error = recursive_search();	/* examine all files on disk */
    if(error)
	*ret_code |= error;

    Signal( &Var_access );
    return 0;
}


void Action(DCB *dcb, DiscReq *req)
{
	dcb = dcb ; req = req;
}

int substr(char *fromdir, char *todir)
{
	while(*fromdir)
		if(*fromdir++ != *todir++)
			return 0;
        return 1;
}

#define push_stack(x)	dir_stack[dir_stkp++] = x
#define pop_stack()	dir_stack[--dir_stkp]

int recursive_search(void)
{
	int i, cluster_num, cluster_count;
	int chain[MAX_DIR];
	int num_entries;
	char name[14];
	DosDirEntry *e;
	int directory_cluster;
	int *dir_stack;
	int dir_stkp = 0;

	dir_stack = (int *) Malloc((Nb_sector/Cluster_size)*sizeof(int));
	if(dir_stack == NULL)
		return(BAD_CHKDSK);

        push_stack(FIRST_ROOT);

	while(dir_stkp)
	    {
            if((num_entries = get_dir_chain(pop_stack(), &chain[0])) < 0)
                return BAD_DIR;

            for (i = 0 ; i < num_entries ; i++)
                {
                e = search(i, &chain[0]);
                if(e == Null(DosDirEntry))
                    return BAD_DIR;
                if(e -> Name[0] == 0)        /* end of dir */
                    break;
                if(e -> Name[0] == 0xE5 || (e -> Attrib & 0x08)) /* erased or */
                    continue;					 /* label     */
                if(e -> Name[1] == '.')		 /* .. */
                    continue;
                if(e -> Name[0] == '.')		 /* . */
                    continue;
                cluster_num = e -> Cluster[1]*0x100 + e -> Cluster[0];
                if(e -> Attrib & 0x10)        /* directory */
		    {
                    push_stack(cluster_num);
		    continue;
		    }
		cluster_count = 1;
		while((cluster_num = get_fat_entry(cluster_num)) < 0xff8)
			cluster_count++;
                strncpy(name,(char *)e->Name,8);name[8] = '.';name[9]='\0';
                strncat(name,(char *)e->Ext,3);name[12]=0;
                IOdebug("%s (%d)\n", name, cluster_count);
                }
	    }

	Free(dir_stack);
}
