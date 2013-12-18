
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/du.c,v 1.9 1993/07/12 12:20:17 nickc Exp $";

/*
-- crf: 13/08/92 - Bug 832
-- There is some discrepancy regarding what du should really do. 
-- According to X/Open (Vol. 1, p. 99): "The du utility gives an estimate,
-- in 512-byte units, of the file space contained in all the specified
-- files ...". 
-- Under Unix (in this case, SunOs 4.1), the disk usage is reported in 1 Kbyte
-- units and it appears that du bases its calculations on the actual number
-- of blocks used by each file.
-- This information is not available under Helios; the best we can do is
-- report the size of the files, rounded up to Kbytes. Note: the fact that
-- we are use 1 Kbyte as opposed to 512-byte units is a deviation from
-- X/Open.
-- This implementation has a problem with the total disk usage that is 
-- returned. The sizes of individual entries are reported in 1 Kbyte units;
-- however, the total is calculated by summing all the individual file sizes
-- and rounding. Consequently, if you have a directory containing 100 files,
-- all of size 1 byte, du reports that each file is of size 1 Kbyte and that
-- the total disk usage of the directory is also 1 Kbyte. So, this bug fix
-- simply entails changing the way the disk usage totals are derived (i.e.
-- now uses the rounded values in the summation).
*/

#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <ctype.h>
#include <nonansi.h>
#include <setjmp.h>
#include <stdarg.h>
#define eq ==
#define ne !=

void usage(void);
WORD WalkDir(char *, WordFnPtr, ...);
WORD DiskUsage(char *dir, DirEntry *entry, WORD start);

int      findall = 0;
jmp_buf  exit_buf;
int	 all     = FALSE;
int	 summary = FALSE;
char* path;

int main(int argc, char **argv)
{ int total; 
  Object *dir;
  int    no_args = 0;
/*  char* path_b = (char*) malloc ( 255 * sizeof (char) ); */

  path = (char*) malloc ( 255 * sizeof (char) );
    
  /* get any command line parameters */
  argv++;
  while (*argv ne NULL && **argv eq '-' )
  {
  	char *x = *argv;
  	char c  = *x++;
  	while ( c ne 0 )
  	{
  		switch( c )
  		{
  			case '-': break;
  			case 's': summary = TRUE; break;
  			case 'a': all = TRUE; break;
  			default : usage();
  		}
  		c = *x++;
  	}
  	argv++;
  	argc--;
  }
  if ( argc eq 1 )
  	no_args = 1;
  else
  	no_args = argc - 1;
  		 
  for(; no_args > 0 ; no_args--)
  {
  
  	if (argc eq 1)
    	{
    		strcpy ( path , "./" );
    		dir = Locate(CurrentDir,".");
    	}
  	else
   	{ 
   		strcpy ( path , *argv );
   		if ( path [strlen(path)-1] != '/' )
   			strcat ( path , "/" );
   		/* strcpy ( path_b , path ); */

		if (strcmp(*argv,"./") == 0)
		{
		  (*argv)[1] = 0;		/* ACC kludge kludge kludge */	
		}

   		dir = Locate(CurrentDir, *(argv++));
     		if (dir eq Null(Object))
      		{ 
      			fprintf(stderr, "Cannot find directory %s\n", *(--argv));
        		exit(1);
      		}
   	}
   
  	if ((dir->Type & Type_Flags) ne Type_Directory)
      	{ 
      		fprintf(stderr, "%s is not a directory\n", dir->Name);
        	exit(1);
      	}
   
  	if (!setjmp(exit_buf))
   	{
   		total = (int) WalkDir(dir->Name, DiskUsage , strlen(dir->Name) + 1);
     		path[strlen(path) - 1] = '\0'; /* Ommit trailling / */
#if 0
		printf("%-8d    %s\n", ( total + 1023 ) /1024 , path);
#else
/*
-- crf: 13/08/92 - Bug 832
-- Refer comments at top
*/
		printf("%-8d    %s\n", total , path);
#endif
   	} 
  }
  return(0);
}

void usage(void)
{ fprintf(stderr, "usage : du [ -s ] [ -a ] [ pathname ... ]\n");
  exit(0);
}

WORD DiskUsage(char *dir, DirEntry *entry, WORD offset)
{ 
  int sub_total = 0;
  char buf[512];
  strcpy(buf, dir);
  strcat(buf, "/");
  strcat(buf, entry->Name);

  if ( ((entry->Type & Type_Flags) eq Type_Directory) ||
       (entry->Type eq Type_Name))
     	sub_total += (int) WalkDir(buf, DiskUsage, offset);
  else if( entry->Type == Type_Link )
  {
  	return 0;
  }
  else
   { ObjInfo info;
     int size;
     word e = ObjectInfo(CurrentDir, buf, (BYTE *) &info);
     if( e >= 0 )
     {
     	if( info.Size >= 0 )
		size = (int) info.Size;	  	
     	else
     	  	size = 0;

#if 0
     	if (all)
      		printf("%-8d    %s%s\n", ( size + 1023 ) / 1024 , path, &buf[offset]);
#else
/*
-- crf: 13/08/92 - Bug 832
-- Refer comments at top
*/
	size = ( size + 1023 ) / 1024 ;
     	if (all)
      		printf("%-8d    %s%s\n", size , path, &buf[offset]);
#endif
     } else size = 0;
     return(size);
   }

  if( !summary )
#if 0
    printf("%-8d    %s%s\n", ( sub_total + 1023 ) / 1024 , path, &buf[offset]);
#else
/*
-- crf: 13/08/92 - Bug 832
-- Refer comments at top
*/
    printf("%-8d    %s%s\n", sub_total , path, &buf[offset]);
#endif    

  return(sub_total);
}
 
WORD WalkDir(char *name, WordFnPtr fn, ...)
{ Object *o  = Locate(CurrentDir, name);
  WORD   sum = 0, arg;
  va_list ap;
  
  va_start(ap, fn);
  arg = va_arg(ap, WORD);
  va_end(ap);

  if (o == Null(Object))
     return(0);
   
  if ((o->Type & Type_Flags) eq Type_Stream)
   return(0);
   
  { 
    Stream *s = Open(o, Null(char), O_ReadOnly);
    WORD size, i;
    DirEntry *entry, *cur;
    
    if (s eq Null(Stream))
     { 
       Close(o);
       return(0);
     }

    size = GetFileSize(s);
    if (size <= 0)
     return(0);
     
    entry = (DirEntry *) Malloc(size);
    if (entry == Null(DirEntry))
     { Close(s); Close(o); return(0); }
     
    if (Read(s, (BYTE *) entry, size, -1) ne size)
     { fprintf(stderr, "Error reading directory %s\n", name);
       Close(s); Close(o); Free(entry);
       return(0);
     }
    
    cur = entry;
    for (i = 0; i < size; cur++, i += sizeof(DirEntry) )
     { 
       if ( (!strcmp(cur->Name, ".")) || (!strcmp(cur->Name, "..")) )
         continue;
       sum += (*fn)(s->Name, cur, arg);
     }

    Free(entry);
    Close(s);
  }  
    
  Close(o);
  return(sum);
}


