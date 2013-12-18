/***************************************************************************
*
*   ls - list contents of directory.
*
*   Originally created: NHG
*   re-written as independent program: AE (16th Dec. 1987)
*   Updated: 3May88 TJK
*   Added new options -C1RFtcu PAB 28 June 88
*   speeded up multiple file args output if -ltuc opts not used
*   Multiple options, time sort on actual args and %/# added to -F
*   => to list type_name links
*   -m option added to print full matrix: NHG 14/2/89
*   remove unecessary fflush for extra speed : JMP 17/5/89
*   Added -d option : PRH 27/7/90
****************************************************************************/

static char *rcsid = "$Header: /hsrc/cmds/com/RCS/ls.c,v 1.8 1993/07/12 12:08:07 nickc Exp $";

#define USAGE "usage: ls -acCdFHlmrRtu1 [file1/dir1] ... [filen/dirn]\n"

#include <posix.h>
#include <stdio.h>
#include <stdlib.h>
#include <root.h>
#include <codes.h>
#include <syslib.h>
#include <servlib.h>
#include <gsp.h>
#include <limits.h>
#include <string.h>
#include <nonansi.h>

/* will hold anything from objectinfo */
typedef union
  {
    DirEntry	DirEntry;
    ObjInfo	Oinfo;
    Link_Info	Linfo;
    byte	buff[ IOCDataMax ];
  }
Dinfo;

typedef struct
  {
    Dinfo	Dinfo;
    char	realname[ PATH_MAX ];
  }
Tinfo;

#undef  FALSE
#undef  TRUE
#define FALSE 0
#define TRUE  1

static int longopt = FALSE;
static int allopt = FALSE;
static int reverse = FALSE;
static int perline = 0;
static int oneperline;
static int noprint = 0;
static int notfirstlist = 0;
static int filemode = 1;
static int recursive = FALSE;
static int showtype = FALSE;
static int Ctime = FALSE;
static int Atime = FALSE;
static int Mtime = FALSE;
static int fullmat = FALSE;
static int dirlist = TRUE;

char linebuf[100];
#define PATHMAX 200
word list(char *);
void printinfo(Tinfo *,AccMask,bool);
void putcap(Capability *cap, word type);
void usage(void);
void defaultInfo(ObjInfo *Oinfo);

word cmpa(const char **arg1,const char **arg2);
word cmpb(const Dinfo **arg1,const Dinfo **arg2);
word cmpt(const Dinfo **arg1,const Dinfo **arg2);
typedef int (*cmpfn)(const void *, const void *);


/* Comparison routine for sorting argument list */
word cmpa(const char **arg1, const char **arg2)
{ word res = (strcmp((char *)*arg1, (char *)*arg2));
  return (reverse ? -res : res);
}

/* Comparison routine for sorting files by name */
word cmpb(const Dinfo **arg1, const Dinfo **arg2)
{ word res = (strcmp((*arg1)->DirEntry.Name, (*arg2)->DirEntry.Name));
  return (reverse ? -res : res);
}

/* Comparison routine for sorting files by time stamp */
word cmpt(const Dinfo **arg1, const Dinfo **arg2)
{
word res;
Dinfo *a = (Dinfo *)*arg1, *b = (Dinfo *)*arg2;
Date t1, t2;

/* if link, get times for link parent */
if (a->DirEntry.Type == Type_Link)
	{
		byte buff[IOCDataMax];
 		word e;
		
		if ( (e = ObjectInfo(cdobj(),a->Linfo.Name, buff)) != 0)
		{
			fprintf(stderr,"ls: Warning, Link ObjectInfo Failure - %lx\n",e);
			return(0);
		}
		a = (Dinfo *) buff;
	}
if (b->DirEntry.Type == Type_Link)
	{
		byte buff[IOCDataMax];
		word e;
		
		if ( (e = ObjectInfo(cdobj(),b->Linfo.Name, buff)) != 0)
		{
			fprintf(stderr,"ls: Warning, Link ObjectInfo Failure - %lx\n",e);
			return(0);
		}
		b = (Dinfo *) buff;
	}

if (Mtime) /* Last Modified time */
	{	t1 = a->Oinfo.Dates.Modified;
		t2 = b->Oinfo.Dates.Modified;
	}
elif (Ctime) /* creation time */
	{ 	t1 = a->Oinfo.Dates.Creation;
		t2 = b->Oinfo.Dates.Creation;
	}
else /* Last accessed time */
	{ 	t1 = a->Oinfo.Dates.Access;
		t2 = b->Oinfo.Dates.Access;
	}

if (t1 == t2)
	return 0;

res = t1 > t2 ? -1 : 1;

return (reverse ? -res : res);
}

/* Function to list the contents of the specified pathname */
/* Returns 1 if any error				   */

word
list( char * name )
{
  Tinfo *	b;
  Tinfo **	sort_index = NULL;
  Object *	o = Locate( cdobj(), name );
  int		dirslots;
  word		e;
  int		i;
  Stream *	s;
  char		newname[  PATHMAX ];
  char		tempname[ PATHMAX ];

  
  if (recursive && (name[ 0 ] == '\0') && notfirstlist)
    return (0);
  
  if ( o == NULL)
    {
      fprintf( stderr, "ls: Cannot find %s - %lx\n", name, Result2( cdobj() ) );

      return 1;
    }
  
  unless ( ( o->Type & Type_Directory ) && dirlist ) 
    {
      Tinfo 	info;
      char *	sp;

      
      /* get last part of name */
      
      if ((sp = strrchr(o->Name, '/')) == NULL)
	{
	  /* only use last part of path */
	  
	  strcpy(newname, o->Name);
	}
      else
	{
	  strcpy(newname, ++sp);
	}
      
      if (notfirstlist)
	{
	  if (!filemode)
	    {
	      if (perline)
		putchar('\n');

	      putchar('\n');

	      perline  = 0;
	      filemode = 1;
	    }
	}
      else
	{
	  notfirstlist = 1;
	}
      
      if ( strcmp ( o->Name , (cdobj())->Name ) == 0 )
	{
	  strcpy ( newname , "." );
	}
      
      /* if (longopt || o->Type == Type_Name) */
      /* stop ls loading servers by doing excessive ObjectInfo's */
      
      if ( longopt )
	{
	  if (o->Type == Type_Name)
	    strcpy(info.realname, newname); /* objinfo of type name changes name */
	  
	  e = ObjectInfo(o,NULL,info.Dinfo.buff);
	  
	  if ( strcmp ( newname , "./" ) == 0 )
	    {
	      strcpy ( info.Dinfo.DirEntry.Name , newname );
	    }
	  
	  if ( e >= Err_Null )
	    {
	      if (o->Type == Type_Name)
		info.Dinfo.DirEntry.Type |= Type_Name;
	      
	      printinfo( &info, o->Access.Access, FALSE );
	    }
	  else
	    {
	      fprintf( stderr, "ls: Warning, ObjInfo of %s failed - %lx\n", o->Name, e );
	      
	      /* set up some defaults */
	      
	      defaultInfo( (ObjInfo *)&info );
	      
	      strcpy( info.Dinfo.DirEntry.Name, newname );
	      
	      info.Dinfo.DirEntry.Type = o->Type;
	      
	      printinfo( &info, o->Access.Access, FALSE );
	    }
	}
      else
	{
#if 0
	  s = Open(o,NULL,O_ReadOnly);
	  
	  Read(s,&info,sizeof(DirEntry),-1);
#else			
	  unless ((strcmp(info.Dinfo.DirEntry.Name,".") == 0) ||
		  (strcmp(info.Dinfo.DirEntry.Name,"..")== 0))
	    strcpy(info.Dinfo.DirEntry.Name, newname);
	  
	  info.Dinfo.DirEntry.Type = o->Type;
#endif
	  printinfo( &info, o->Access.Access, FALSE );
	}
      
      return 0;
    }
  
  s = Open( o, NULL, O_ReadOnly );
  
  /* listing of directories */
  
  filemode = 0;
  
  if (!name || !strcmp(name,"") || !strcmp(name,".") || !strcmp(name,"..") || noprint)
    {
      /* do not print name */
      
      noprint      = 0;
      notfirstlist = 1;		
    }
  else
    {
      if (perline != 0 )
	{
	  perline = 0;
	  putchar('\n');
	}
      
      if (notfirstlist)
	putchar('\n');
      else
	notfirstlist = 1;
      
      printf( "%s:\n", name );		
    }
  
  if ( s == Null(Stream) )
    {
      fprintf(stderr, "ls: Cannot open %s - %lx\n",name,Result2(o));
      
      return 1;
    }
  
  dirslots = (int)(GetFileSize(s) / sizeof (DirEntry));
  
  if ( longopt )
    printf("%d Entries\n",dirslots);
  
  /*
   * Read the directory in one go. We get sufficient space
   * for all the directory entries obinfos and orig names,
   * plus an array which we will use to point to them all
   */
  
  if ((b = (Tinfo *) malloc(dirslots * sizeof(Tinfo))) == NULL
      || (sort_index = (Tinfo **) malloc(dirslots * sizeof(Tinfo *))) == NULL)
    {
      fprintf(stderr, "ls: Out of memory\n");
      
      exit(1);
    }
  
  for ( i = 0 ; i < dirslots; i++ )
    {
      word	size;

      
      size = Read(s,(char *)&b[i],sizeof(DirEntry),-1);
      
      if ( size != sizeof(DirEntry) )
	{
	  fprintf(stderr,"ls: Directory read failure - %lx\n",Result2(s));
	  
	  return 1;
	}
      
      sort_index[i] = &b[i];
    }
  
  if (longopt || Mtime || Atime || Ctime )
    /* get time and other info into dir struct */
    {
      for( i = 0 ; i < dirslots; i++ )
	{
	  int Tname = FALSE;
	  
	  if (b[i].Dinfo.DirEntry.Type == Type_Name)
	    {
	      strcpy(b[i].realname, b[i].Dinfo.DirEntry.Name);
	      
	      Tname = TRUE; /* remember if this was a name type */
	    }
	  
	  if ( b[i].Dinfo.DirEntry.Name[0] == '.'
	      && ( b[i].Dinfo.DirEntry.Name[1] == '\0'
		  || ( b[i].Dinfo.DirEntry.Name[1] == '.'
		      && b[i].Dinfo.DirEntry.Name[2] == '\0')))
	    {
	      /* . or .. entries names must be preserved */
	      /* objectInfo overlays them with full current / parent names */
	      
	      char dots[3];
	      
	      if ( allopt ) /* only process if we are going to use them */
		{
		  strcpy (dots, b[i].Dinfo.DirEntry.Name);

		  if ((e = ObjectInfo(o, b[i].Dinfo.DirEntry.Name, b[i].Dinfo.buff)) != 0)
		    {
		      /* statement has been commented out to avoid excess feedback... 
		       *
		       * fprintf(stderr,"ls: Warning, ObjectInfo failure for %s - %x\n",
		       *		 b[i].Dinfo.DirEntry.Name, e);
		       *
		       */

		      defaultInfo(&b[i].Dinfo.Oinfo);
		    }

		  strcpy (b[i].Dinfo.DirEntry.Name, dots);	/* return dot names */
		}
	    }
	  else if ((e=ObjectInfo(o, b[i].Dinfo.DirEntry.Name, b[i].Dinfo.buff)) != 0)
	    {
	      /* ignore link that isn't connected */

	      if ((e & 0x0ffffff) == (EG_Invalid | EO_Route))
		fprintf(stderr,"ls: Warning, invalid route for %s - %lx\n",b[i].Dinfo.DirEntry.Name, e);

	      /* statement commented out to avoid excess feedback
		 else
		 fprintf(stderr,"ls: Warning, ObjectInfo failure for %s - %x\n",b[i].Dinfo.DirEntry.Name, e);
		 */
	      defaultInfo(&b[i].Dinfo.Oinfo);
	    }
	  
	  if (Tname) /* objinfo gets rid of name type - reinstate it if ness. */
	    b[i].Dinfo.DirEntry.Type |= Type_Name;
	}
    }
  else
    {
      /* must find out what Name table entries are actually dirs */
      /* by doing an obinfo on them */
      
      for ( i = 0 ; i < dirslots; i++ )
	{
	  if (      b[i].Dinfo.DirEntry.Type == Type_Name   /* dont bother with . & .. entries */
	      && ! (b[i].Dinfo.DirEntry.Name[0] == '.'
	      && (  b[i].Dinfo.DirEntry.Name[1] == '\0'
	       || ( b[i].Dinfo.DirEntry.Name[1] == '.'
	      &&    b[i].Dinfo.DirEntry.Name[2] == '\0'))))
	    {
	      /* remember original name before conversion by objinfo */
	      
	      strcpy(b[i].realname, b[i].Dinfo.DirEntry.Name);
	      
	      if ((e=ObjectInfo(o, b[i].Dinfo.DirEntry.Name,b[i].Dinfo.buff)) != 0)
		{
		  /* ignore link that isn't connected */
		  
		  if ((e & 0x0ffffff) == (EG_Invalid | EO_Route))
		    fprintf(stderr,"ls: Warning, invalid route for %s - %lx\n",b[i].Dinfo.DirEntry.Name, e);
		  
		  /* statement commented out ot avoid excess feedback
		     else
		     fprintf(stderr, "ls: Warning, ObjectInfo failure for %s - %x\n",
		     b[i].Dinfo.DirEntry.Name, e);
		     */
		  
		  defaultInfo(&b[i].Dinfo.Oinfo);
		}
	      
	      b[i].Dinfo.DirEntry.Type |= Type_Name;
	    }
	}
    }
  
  if (Mtime || Ctime || Atime)	/* time ordered sort */
    qsort((void *) sort_index, (size_t) dirslots, sizeof(Tinfo *) ,(cmpfn) &cmpt);
  else  /* Sort the array into alphab. order */
    qsort((void *) sort_index, (size_t) dirslots, sizeof(Tinfo *) ,(cmpfn) &cmpb);
  
  /* Now print the information we need */
  
  for ( i = 0 ; i < dirslots; i++ )
    {
      if ( !allopt && sort_index[i]->Dinfo.DirEntry.Name[0] == '.' )
	continue;

      printinfo(sort_index[i],o->Access.Access,TRUE);
    }
  
  Close(s);
  Close(o);
  
  if (recursive)
    {
      if (name == NULL)
	tempname[0] = '\0';
      else
	{
	  strcpy(tempname,name);
	  
	  if (tempname)			/* NULL for current dir */
	    {	
	      i = strlen(tempname) - 1;
	      
	      if (tempname[i] != '/')
		strcat(tempname, "/");
	    }
	}    
      
      for ( i = 0 ; i < dirslots; i++ )
	{
	  Tinfo *	d = sort_index[i];

	  
	  if (d->Dinfo.DirEntry.Type & Type_Directory)
	    {
	      if( !allopt && d->Dinfo.DirEntry.Name[0] == '.' )
		continue;
	      
	      if ( strcmp(d->Dinfo.DirEntry.Name, "..") == 0 || strcmp(d->Dinfo.DirEntry.Name, ".") == 0)
		continue;
	      
	      if (tempname[0] != '\0')
		strcpy(newname,tempname);
	      else
		newname[0] = '\0';
	      
	      if ((d->Dinfo.DirEntry.Type & Type_Name) == Type_Name)
		strcat(newname, d->realname);
	      else
		strcat(newname,d->Dinfo.DirEntry.Name);
	      
	      list(newname);
	    }
	}
    }
  
  free(sort_index);
  
  free(b);
  
  return 0;
  
} /* list */


void printdate(Date date)
{
	char *ctime(Date *);
	char *time = ctime(&date);
	time[strlen(time)-1] = 0;
	printf(" %s ",time);
}

void printinfo(Tinfo *info, AccMask dirmask, bool indir)
{
	DirEntry *b = &info->Dinfo.DirEntry;
	char typech;
	char mask[40];
	char c = ' ';

	if (showtype)
	{
 	  if (b->Type & Type_Directory) c = '/';
	  else if (b->Type == Type_Link) c = '@';
        }

	mask[8] = 0;
	if ( longopt )
	{
		string bitchars = getbitchars(b->Type);
		switch( b->Type )
		{
		default:
			typech = (b->Type & Type_Directory) ? 'd' : 
				 (b->Type & Type_Stream)    ? 'f' :
				 (b->Type & Type_Private)   ? 'v' :
				 (b->Type & Type_Name)      ? 'n' : '?';

			goto putinfo;

		case Type_Name:		typech = 'n';   goto putinfo;
		case Type_Directory:	typech = 'd';   goto putinfo;
		case Type_Task:		typech = 't';	goto putinfo;
		case Type_Module:	typech = 'm';	goto putinfo;
		case Type_Program:	typech = 'p';	goto putinfo;
		case Type_CacheName:	typech = 'c';	goto putinfo;
		case Type_Fifo:
		case Type_File:		typech = 'f';

		putinfo:
			if( fullmat ) DecodeMatrix(mask,b->Matrix,b->Type);
			else 
			{
				if( !indir ) DecodeMask(mask,dirmask,bitchars);
				else DecodeMask(mask,UpdMask(dirmask,b->Matrix),bitchars);
			}
			printf("%c %8s %3ld %6ld",
			       typech, &mask, info->Dinfo.Oinfo.Account, info->Dinfo.Oinfo.Size );
			printdate(info->Dinfo.Oinfo.Dates.Modified);
			
			if ((b->Type & Type_Name) == Type_Name
			     && strcmp(info->realname, b->Name) != 0)
				printf("%s%c => %s\n",info->realname, c, b->Name);
			else
				printf("%s%c\n",b->Name,c);
			break;

		case Type_Link:
			{
				/* must print info from target obj */
				ObjInfo Oi;	/* safe to use objinf as cannot have ln to ln */
				word e;

				if ( (e = ObjectInfo(cdobj(),info->Dinfo.Linfo.Name, (byte *) &Oi)) != 0)
				{
					fprintf(stderr,"ls: Link ObjectInfo Failure - %lx\n",e);
					return;
				}
				bitchars = getbitchars(Oi.DirEntry.Type);
				/* I am not quite sure which matrix should be */
				/* used here, the one in the link or the one in */
				/* the linked object. For now it is the link */
				if( fullmat ) DecodeMatrix(mask,b->Matrix,b->Type);
				else 
				{
					Link_Info *l = &info->Dinfo.Linfo;
					/* The correct access to a linked    */
					/* object is given in the stored     */
					/* capability.			     */
					DecodeMask(mask,l->Cap.Access,bitchars);
				}
				printf("l %8s %3ld %6ld",&mask,Oi.Account,Oi.Size);
				printdate(Oi.Dates.Modified);
				printf("%s%c -> %s\n",b->Name, c, info->Dinfo.Linfo.Name);
				break;
			}
		}
	}
	else
	{
		int i, s;


		if ((b->Type & Type_Name) == Type_Name
		     && strcmp(info->realname, b->Name) != 0) {
			if ( ( s = strlen(info->realname) ) == 0 )
				{
				if ( ( s = strlen(b->Name ) ) == 0 )
					return;
				else
					printf("%s%c",b->Name,c);
				}
			else
				printf("%s%c",info->realname, c);
		}
		else {
			if ( ( s = strlen(b->Name ) ) == 0 )
				return;
			else
				printf("%s%c",b->Name,c);
	
		}
		for(i = s+1; i < 19 ; i++) putchar(' ');
		perline++;
		if( oneperline || perline == 4 )
		{
			perline = 0;
			printf("\n");
		}
		fflush(stdout);
	}
}

void
usage(void)
{
	fprintf(stderr,USAGE);
}

#if 0
void putcap(Capability *cap, word type)
{
	int i;
	char mask[10];
	char *s = DecodeMask(mask,cap->Access,getbitchars(type));
	*s = 0;
	printf("%8s ",&mask);
	for( i = 0 ; i < 7 ; i++ ) printf("%02x",cap->Valid[i]);
}
#endif

/* invent some defaults for object infos */

void
defaultInfo( ObjInfo * Oinfo )
{
  Oinfo->Account        = 0;
  Oinfo->Size           = 0;
  Oinfo->Dates.Creation = 0;
  Oinfo->Dates.Access   = 0;
  Oinfo->Dates.Modified = 0;
}


int
main(
     int 	argc,
     char **	argv )
{
  word errcode = 0;

  
  argv++;

  setvbuf( stdout, linebuf, _IOLBF, 100 );

  oneperline = !isatty(fileno(stdout));
	
  /* Process arguments */

  while ( *argv != NULL && **argv == '-' )
    {
      char *	x = *argv;
      char	c = *x++;

      
      while ( c != 0 )
	{
	  switch ( c )
	    {
	    case '-': break;
	    case 'a': allopt     = TRUE; break;	      /* show all (.) files */
	    case 'l': longopt    = TRUE; break;	      /* long listing */
	    case 'r': reverse    = TRUE; break;	      /* reverse order of sorting */
	    case '1': oneperline = TRUE; break;	      /* one file output per line */
	    case 'C': oneperline = FALSE; break;      /* force columnar output */
	    case 'F': showtype   = TRUE; break;	      /* trail dirs with '/', ln '@'*/
	    case 'R': recursive  = TRUE; break;	      /* recursively list dirs */
	    case 't': Mtime      = TRUE; break;	      /* last modified time ordered */
	    case 'u': Atime      = TRUE; break;	      /* last accessed time ordered */
	    case 'c': Ctime      = TRUE; break;	      /* creation time ordered */
	    case 'm': fullmat    = TRUE; break;	      /* show full matrix, not current mask */
	    case 'd': dirlist    = FALSE; break;      /* directories treated as files */
	    case 'H':
	      usage();
	      exit(0);
	    default:
	      fprintf(stderr, "ls: Option %c ignored\n",c); break;
	    }
	  
	  c  = *x++;
	}
      
      argv++;
      argc--;
    }
  
  /*
   * Handle each argument passed. If it is a file, add it to
   *  the list of files to be processed. If it is a directory
   *  process it at once. If no arguments given list CurrentDir.
   */
  
  /* if only one directory or file then dont print directory name */
  
  if (argc < 3)
    noprint = 1;
  
  if (*argv==NULL)
    {
      errcode = list( NULL );
    }
  else
    {
      if (argc > 2 && (Mtime || Ctime || Atime)) /* time sorted args required */
	{
	  Tinfo *	buff;
	  Tinfo *	bp;
	  Tinfo **	sort_ind = NULL;
	  Tinfo **	sp = NULL;
	  char **	av;
	  int		ac;
	  word		e;

	  
	  if ((buff = bp = (Tinfo *) malloc((argc-1) * sizeof(Tinfo))) == NULL
	      || (sort_ind = sp = (Tinfo **) malloc((argc-1) * sizeof(Tinfo *))) == NULL)
	    {
	      fprintf(stderr, "ls: Out of memory\n");
	      exit(1);
	    }
	  
	  for (ac = argc, av = argv; ac > 1; ac--, av++, bp++, sp++)
	    {
	      *sp = bp;
	      
	      strcpy( bp->realname, *av ); /* remember full name */
	      
	      if ((e = ObjectInfo(cdobj(), *av, (byte *)bp)) != 0)
		{
		  /* Warning message has been commented out to reduce excess feedback
		     fprintf(stderr,"ls: Warning, ObjectInfo failure for %s - %x\n",*av, e);
		     */
		  
		  defaultInfo((ObjInfo *)bp);
		}
	    }
	  
	  qsort( (void *) sort_ind, (size_t) argc - 1, sizeof (Tinfo *), (cmpfn) &cmpt);
	  
	  /* List them all */
	  
	  for (; argc>1; argc--, sort_ind++)
	    if (list((*sort_ind)->realname))
	      errcode = 1;
	}
      else /* Sort the cmd line arguments alphabetically as default */
	{
	  if (argc > 2)
	    qsort((void *) argv, (size_t)(argc-1), sizeof(char *), (cmpfn) &cmpa);
	  
	  /* List them all */
	  
	  for (; argc>1; argc--)
	    if (list(*argv++)) errcode = 1;
	}
    }
  
  if ( perline != 0 )
    printf("\n");
  
  return (int)errcode;

} /* main */

/* end of ls.c */
