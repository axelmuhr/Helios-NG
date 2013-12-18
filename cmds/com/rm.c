char *rcsid = "$Header: /hsrc/cmds/com/RCS/rm.c,v 1.4 1993/07/12 12:22:50 nickc Exp $";

#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>

extern void exit(int);

/* will hold anything from objectinfo */
typedef union {
	DirEntry	DirEntry;
	ObjInfo		Oinfo;
	Link_Info	Linfo;
	byte		buff[IOCDataMax];
} Dinfo;

int     errors = 0;
int     fflag = 0;
int     iflag = 0;
int     rflag = 0;

void usage () {
    fprintf(stderr,"Usage: rm [-f] [-i] [-r] <file list>\n");
    exit (1);
}

void
del( char * name )
{
  if (strcmp ("..", name) && strcmp (".", name))
    {
      word e;

      e = Delete(CurrentDir,name);

      if ( e < 0 && !fflag ) 
	{
	  fprintf(stderr,"rm: Cannot remove %s: %lx\n",name,e);
	  errors++;
	}
    }
} /* del */

int confirm () {
    char    c,
            t;
    c = getchar();
    t = c;
    while (t != '\n' && t != -1)
	t = getchar();
    return (c == 'y' || c == 'Y');
}

void
rm( char * name )
{
  DirEntry *	b;
  DirEntry *	m;
  word		no_entries;
  word		i;
  word		size;
  word		dirsize;
  Stream *	s;
  char		rname[ 128 ];
  word		isdir;
  word		islink;
  Dinfo		obinfo;  
  Object *	o = Locate( CurrentDir, name );

  
  if ( ObjectInfo(CurrentDir,name,obinfo.buff) != 0)
    {
      if (!fflag)
	fprintf(stderr,"Cannot find %s : %lx\n",name,
		Result2(CurrentDir));
      errors++;
      return;
    }
  
  islink = obinfo.DirEntry.Type & Type_Link;
  isdir  = o->Type & Type_Directory;
  
  if (iflag) 
    {
      fprintf(stderr,"rm: delete %s %s? ", 
	      (isdir ? "directory" : "file"), name);

      fflush(stderr);
      if (!confirm ())
	return;
    }
  
  /* If a simple file or directory, delete it */
  
  if ( islink || !(isdir && rflag))
    {
      del(name); 
      return;
    }
  
  /* More complicated case - scan a directory */
  
  s = Open(o,NULL,O_ReadOnly);
  
  if ( s == Null(Stream) )
    {
      fprintf(stderr,"Cannot open %s : %lx\n",name,Result2(o));
      errors++;
      return;
    }
  
  dirsize = GetFileSize(s);
  
  no_entries = dirsize/sizeof(DirEntry);
  
  if (no_entries > 0)
    {
      m = b = (DirEntry *) Malloc(dirsize);
      
      if (b == Null(DirEntry))
	{
	  fprintf(stderr,"rm: Out of memory\n");
	  exit(1);
	}
      
      size = Read(s,(char *)b,dirsize,-1);
      
      if ( size != dirsize)
	{
	  fprintf(stderr,"Read failure %lx\n",Result2(s));
	  errors++;
	  return;
	}
      
      for ( i = 0 ; i < no_entries; i++ )
	{ 
	  if (strcmp ("..", m->Name) && strcmp (".", m->Name))
	    {
	      strcpy (rname, name);
	      strcat (rname, "/");
	      strcat (rname, m->Name);
	      rm (rname);
	    }
	  m++;
	}
      
      Free(b);
    }
  
  Close(s);
  Close(o);

  del (name);

  return;
  
} /* rm */

int
main(
     int     	argc,
     char *  	argv[] )
{
  int 		exstatus;
  char *	opt;
  

  if (argc < 2)
    usage ();

  ++argv;
  --argc;

  while (**argv == '-')
    {
      opt = *argv;

      while (*++opt != '\0')
	switch (*opt)
	  {
	  case 'f': 
	    fflag++;
	    break;
	  case 'i': 
	    iflag++;
	    break;
	  case 'r': 
	    rflag++;
	    break;
	  default: 
	    fprintf(stderr,"rm: unknown option %c\n",*opt);
	    usage ();
	    break;
	  }
      argc--;
      ++argv;
    }

  if (argc < 1)
    usage ();

  while (argc--)
    rm (*argv++);

  exstatus = (errors == 0 ? 0 : 1);

  if (fflag)
    exstatus = 0;

  return (exstatus);

} /* main */
