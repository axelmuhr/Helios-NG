/*----------------------------------------------------------------
--                                                              --
--        H E L I O S   M S D O S - F S   S O F T W A R E	--
--        -----------------------------------------------	--
--                                                              --
--          Copyright (C) 1991, Perihelion Software Ltd.        --
--          Copyright (C) 1990, Telmat Informatique.            --
--                     All Rights Reserved.                     --
--                                                              --
-- path.c							--
--                                                              --
--	The filename and pathname manipulation routines		--
--                                                              --
--	Author:  Olivier Reins, Martyn Tovey                    --
--                                                              --
----------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/servers/msdosfs/RCS/path.c,v 1.2 1991/03/28 18:04:09 martyn Exp $";

#include "msdosfs.h"
#include "prototype.h"

/*----------------------------------------------------------------------*/
/*			GetFullName()					*/
/*----------------------------------------------------------------------*/

/**
*** A name conversion routine, to turn the GSP bits of the name into a
*** simple string.
***
*** The first bit of code goes through the names, starting at pointer rest which
*** may be in either the context or the name fields, and IOdebug all the data 
*** into IOname. I try to update the next field in the message in case the
*** message has to be passed from one server to another, i.e. if another name
*** conversion may be required. This bit of code is rather dubious. It may be
*** necessary to extract bits of the name from the name field as well, in case
*** the rest pointer was still somewhere inside context. Having got hold of
*** the whole name I put in a terminator.
***
*** Sadly IOname may still contain elements . and .. which have to be filtered
*** out. Hence I must go through my entire local name again looking for these
*** special cases, and this is done by routine flatten() which is also called
*** by Rename handlers. Only now am I finished, and I can put in another
*** terminator just in case and produce another debugging option. It is
*** possible for the name conversion to fail if there is an attempt to
*** backtrack to a server outside the Server, as I refuse to forward messages
*** to other bits of the network.
**/


string GetFullName(string DeviceName, MCB *mcb)
{ BYTE *data     = mcb->Data;
  IOCCommon *common = (IOCCommon *) mcb->Control;
  int  context   = common->Context;
  int  name      = common->Name;
  int  next      = common->Next;
  string NewName, tmp;
  string dest = (string) Malloc(Name_Max);

  if (dest == Null(char))
   {
#ifdef debugE
    IOdebug("msdosfs ERROR : GetFullName : can't Malloc");
#endif
    SendError(mcb, EC_Error + SS_FloppyDisk + EG_NoMemory + EO_Server);
    return(Null(char));
   }

  NewName = dest;

  for (tmp = &(DeviceName[0]); *tmp != '\0'; )
    *dest++ = *tmp++;

  *dest++ = '/';

  for ( ; data[next] != '/' && data[next] != '\0'; next++)
    *dest++ = data[next];

  if (data[next] == '/')
      for ( ; data[next] != '\0'; next++)
        *dest++ = data[next];

  if (name != -1)
  if(((next < name) && (context < name)) || ((next > name) && (context > name)))
   { 
     *dest++ = '/';
     for ( ; data[name] != '\0'; name++)
       *dest++ = data[name];
   }

  if (*(--dest) != '/') dest++;		/* Get rid of any trailing '/' */
  *dest = '\0';

	/* remove '.' and '..' */

  if (!flatten(NewName))
  { 
    Free(NewName); 
    return(Null(char)); 
  }

#ifdef debug
    IOdebug("msdosfs : GetFullName: next=%s, name=%s->%s",&data[common->Next],
		 &data[common->Name],NewName);
#endif

  return(NewName);
}

/*----------------------------------------------------------------------*/
/*			flatten()					*/
/*----------------------------------------------------------------------*/

/** remove . and .. from pathname
*** ex : /a/b/./.. -> /a 
***/
WORD flatten(string name)
{
  char *source = name, *dest = name;
  int  entries = 0;

  while(*source != '\0')
   {
	if (*source == '.')
       	{
	   source++;
           if (*source == '/')
	   {
	     source++;
	     continue;
	   }
           elif (*source == '\0')
           {
	     if (entries < 1)
	        return (WORD) FALSE;
             dest--;
	     break;
           }
           elif (*source == '.')
           {
	      source++;
              if (*source == '/' || *source == '\0')
               { 
                 if (entries <= 1)
		    return (WORD) FALSE;
                 dest -= 2;
		 while (*dest != '/')
		      dest--;
                 if (*source != '\0') 
                  {
		     dest++;
		     source++;
		  }
                 entries--; continue;
               }
              else
               {
		 *dest++ = '.';
		 *dest++ = '.';
	       }
            }
         else
	    *dest++ = '.';
	}

     	while (*source != '/' && *source != '\0')
	    *dest++ = *source++;
     	if (*source != '\0')
       	{
	    *dest++ = '/';
	    source++;
            while (*source == '/')
	        source++;
            entries++;
       	}
   }

  *dest = '\0';

  return (WORD) TRUE;
}


/*----------------------------------------------------------------------*/
/*			GetLocalName()					*/
/*----------------------------------------------------------------------*/

/**
*** Another name conversion routine, to turn a Helios name into a local name.
**/
/** Convert pathname Helios -> PC
**/

string GetLocalName(string HeliosName)
 {
   string localname = (char *) Malloc(Name_Max);
   char *tempptr    = (char *) HeliosName;
   int dot=0, wstart=1;
   char c, *p;

#ifdef debugE
   IOdebug("msdosfs : GetLocalName %s\n", HeliosName);
#endif

   if( localname == NULL )
   {
#ifdef debugE
	IOdebug("msdosfs ERROR : GetLocalName : can't Malloc");
#endif
     return NULL;
   }
		/* skip first bit = drive name */
    for(; (*tempptr != '/') && (*tempptr != '\0'); tempptr++);
  
    if( *tempptr == '\0' )
        strcpy(localname, "/");
    else
    {
	p = localname;
	while((c = *tempptr++) != 0)
		switch(c)
			{
			case '/':
				*p++ = c;
				dot = 0;
				wstart = 1;
				continue;
				
			case '.':
				if( wstart )
				    continue;
				if(++dot > 1)
				    {
				    free(localname);
				    return (char *)(-1);
				    }
				*p++ = c;
				continue;

			default:
				*p++ = toupper(c);
				wstart = 0;
				continue;
			}
	*p++ = 0;
     }
    
#ifdef debug
    IOdebug("msdosfs : GetLocalName : %s->%s",HeliosName,localname);
#endif
    return localname;
}

/*----------------------------------------------------------------------*/
/*			unixname()					*/
/*----------------------------------------------------------------------*/
/**
 * Get rid of spaces in a MSDOS 'raw' name (one that has come from the
 * directory structure) so that it can be used for regular expression
 * matching with a unix file name.  Also used to 'unfix' a name that has
 * been altered by fixname().  
 */

void unixname(string name, string ext, string ans)
{
	int i;
#ifdef debug
	char *tmp = ans;
#endif

	for(i = 0; *name != ' ' && i < 8 ; i++)
		*ans++ = tolower(*name++);

	if(*ext != ' ')
		{
		*ans++ = '.';
		for(i = 0; *ext != ' ' && i < 3 ; i++)
			*ans++ = tolower(*ext++);
		}
        *ans = 0;
#ifdef debug
	IOdebug( "msdosfs : unixname: %s", tmp);
#endif
}

/*----------------------------------------------------------------------*/
/*			get_name_and_path()				*/
/*----------------------------------------------------------------------*/
/*
 * Get name and path components of filename.  
 * Returns pointer to malloc'd space.  Could be NULL.
 */

string get_name_and_path(string name, char **filename, char **pathname)
{
	char *s, *freeloc;

	if (*name == NULL)
		return(NULL);

	freeloc = Malloc(strlen(name)+1);

	if( freeloc == NULL )
	{
#ifdef debugE
	  IOdebug("msdosfs ERROR : getname : can't Malloc %d", strlen(name));
#endif
	  return( NULL );
	}

	strcpy(freeloc, name);

	*pathname = *filename = freeloc;
				
	if ( (s = strrchr(freeloc, '/')) != NULL )
		{
		*s = NULL;
		*filename = s+1;
		}

	if (!strlen(freeloc))
		*pathname = "/";

	return(freeloc);
}

/*----------------------------------------------------------------------*/
/*				fixname()				*/	
/*----------------------------------------------------------------------*/

/** Convert a filename to Dos format : name:8.ext:3
**  Assumes that name has already been converted to dos conventions
**/

void fixname(char *srcname, char *name, char *ext)
{
  char *s = srcname;
  char *n = name;
  int i;

  memset(name, 0x20, 8);
  memset(ext,  0x20, 3);
  
  if( srcname == NULL || *srcname == '\0' )
	return;

  if( !(srcname[0] == '.' && (srcname[1] == '\0' || (srcname[1] == '.'
			&& srcname[2] == '\0'))))
	{

  	while(*s != '.' && *s != '\0')
		s++;

  	switch(*s)
		{
		case '.':
			for(s++, i=0 ; *s && i < 3 ; i++)
				ext[i] = toupper(*s++);

		case '\0':
			s = srcname;
			for(i=0 ; *s != '.' && *s && i < 8 ; i++)
				*n++ = toupper(*s++);
		}
	}
   else
	while(*s)
	   *n++ = *s++;
	

#if 0
#ifdef debug
	{
	char buffer[20];
        memcpy(buffer, name, 8);
	buffer[8] = '.';
	memcpy(& buffer[9], ext, 3);
	buffer[12] = 0;
	IOdebug("msdosfs : fixname : %s -> %s",srcname, buffer);
	}
#endif
#endif

}
