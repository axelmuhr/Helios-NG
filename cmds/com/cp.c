/***************************************************************************
 ***************************************************************************
 **	CP COMMAND for HELIOS                                             **
 **	John Fitch - University of Bath and Fitch-Norman                  **
 **	23 February 1988                                                  **
 ***************************************************************************
 ***************************************************************************/

/* Revision history:
   Started 23 February 1988
   Added recursive case 28 February 1988
   Conversion to Helios 19 April 1988
   Revised to use Objects rather than keep strings 1988 May 2
   Tested and updated PAB 25/5/88
   Big buffer copy PAB 22/11/88
   Recursive calls do locate before create JMP 30/03/89
*/

#ifdef __TRAN
static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/com/RCS/cp.c,v 1.8 1994/12/08 18:11:10 nickc Exp $";
#endif

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <syslib.h>
#include <gsp.h>
#include <string.h>
#include <helios.h>
#include <stdarg.h>

#define USAGE "Usage: %s <src> <dest>; or cp [-i -r] <src1 src2... srcn> <dir>\n"

#define TIMEOUT -1	/* infinity */  /* 10*OneSec	10 seconds */

#define	MAXPATH	200	/* maximum path length */

bool		interactive = FALSE;
bool		recursive   = FALSE;
bool		direct      = FALSE;
bool		textcopy    = FALSE;
char *		ProgName    = "<Unknown>";
DirEntry 	Dirbuf;
char		dname[ MAXPATH ];

static void
warn( char * pWarnMessage, ... )
{
  va_list	vArgs;


  va_start( vArgs, pWarnMessage );

  fprintf( stderr, "%s: Warning: ", ProgName );
  vfprintf( stderr, pWarnMessage, vArgs );
  fputc( '\n', stderr );
  
  va_end( vArgs );

  return;
  
} /* warn */
  
static void
fail( char * pErrMessage, ... )
{
  va_list	vArgs;


  va_start( vArgs, pErrMessage );

  fprintf( stderr, "%s: ", ProgName );
  vfprintf( stderr, pErrMessage, vArgs );
  fputc( '\n', stderr );

  va_end( vArgs );

  exit (1);
  
} /* fail */
  

/*
 * Copy srcpath object into dstdir object with the name dst.
 */

#define BUFFERSIZE	(16 * 1024)

static char 	aFileBuffer[ BUFFERSIZE ];

static void
copyfile(
	 Object *	srcpath,
	 Object *	dstdir,
	 char *		dst )
{
  Stream *	inf;
  char		fulldst[ MAXPATH ];
  

  /*  fprintf(stderr,"recursive=%d, Type=%8x\n",recursive,o->Type);*/
  
  if (srcpath->Type & Type_Directory)
    {
      /* if src is a directory */

      if (recursive)
	{
	  int	i;
	  int	len;

	  
	  /* Must work recusively; first create destination directory */
	  /* and then work through directory */
	  
	  unless (Locate( dstdir, dst ))
	    {
	      unless (Create( dstdir, dst, Type_Directory, 0, 0 ))
		{
		  fail( "Cannot create %s : %lx", dst, Result2( dstdir ) );
		}
	    }
	  
	  dstdir = Locate( dstdir, dst );
	  inf    = Open( srcpath, NULL, O_ReadOnly );
	  len    = (int) GetFileSize( inf ) / sizeof (DirEntry);
	  
	  for (i = 0; i < len; i++)
	    {
	      word	size;
	      Object *	srcobj;

	      
	      size = Read( inf, (byte*)&Dirbuf, sizeof (DirEntry), TIMEOUT );
	      
	      if (size != sizeof (DirEntry))
		{
		  fail( "Read failure on directory '%s': fault = %lx",
		       srcpath->Name, Result2( inf ) );
		}
	      
	      if (Dirbuf.Name[ 0 ]=='.')
		{
		  if (Dirbuf.Name[ 1 ]=='\0')
		    continue;
		  elif ((Dirbuf.Name[ 1 ] == '.')
			&& (Dirbuf.Name[ 2 ] == '\0'))
		    continue;
		}
	      
	      /* recursive entry */
	      /*** JMP bug fix 09/08/89 locate object close in recursive calls ***/
	      
	      srcobj = Locate( srcpath, Dirbuf.Name );
	      
	      copyfile( srcobj, dstdir, Dirbuf.Name );
	      
	      Close( srcobj );
	    }
	  
	  Close( inf );

	  return;
	}
      else
	{
	  warn( "directory %s not copied (use -r option)", srcpath->Name );
	  
	  return;
	}
    }
  
  /* construct full dst path to check for copy to same name */
  if (*dst != '/')
   {
	  strcpy( fulldst, dstdir->Name );
	  strcat( fulldst, "/" );
	  strcat( fulldst, dst );
   }
  else
  {
	  strcpy( fulldst, dst );
  }
  
  if (strcmp( fulldst, srcpath->Name ) == 0)
    {
      fail( "Cannot copy file to itself" );
    }
  
  if (interactive)
    {
      int	c;

      
      if (direct) /* if last arg was a dir. */
	fprintf( stderr, "Copy %s to %s ? (y/n)[n] ", srcpath->Name, fulldst );
      else	
	fprintf( stderr, "Copy %s to %s ? (y/n)[n] ", srcpath->Name, dst );
      
      fflush( stderr );

      c = getchar();

      if (c != EOF)
	{
	  /* eat rest of line typed in by user */
	  
	  while (getchar() != '\n')
	    ;
	}

      if (c == EOF || tolower( c ) != 'y')
	{
	  fprintf( stderr, "Not copied\n" );
	  
	  return;
	}
    }

  /* Copy file 'srcpath' to 'dstdir'/'dst' */

  if (textcopy)
    {
      FILE *	in;
      FILE *	out;
      int	read;
      
      
      if ((in = fopen( srcpath->Name, "r" )) == NULL)
	{
	  warn( "File '%s' not found", srcpath );
	  return;
	}
      
      if ((out = fopen( fulldst, "w" )) == NULL)
	{
	  warn( "Failed to open %s for writing", fulldst );
	  return;
	}

      do
	{
	  int	written;

	  
	  if ((read = fread( aFileBuffer, 1, BUFFERSIZE, in )) < 1)
	    {
	      break;
	    }
	  
	  if ((written = fwrite( aFileBuffer, 1, read, out )) < 1)
	    {
	      fail( "Write failed" );
	    }
	}
      while (read == BUFFERSIZE);

      fclose( in  );
      fclose( out );
    }
  else
    {
      Stream *	outf;
      word	buffsize;

      
      if ((inf = Open( srcpath, NULL, O_ReadOnly )) == NULL)
	{
	  warn( "Cannot open '%s': fault = %lx\n", srcpath->Name, Result2( srcpath ) );
	  
	  return;
	}
      
      /***  14-03-89 JMP did not truncate file on opening ***/
      
      if ((outf = Open( dstdir, dst, O_Create | O_Truncate | O_WriteOnly)) == NULL)
	{
	  warn( "Unable to create '%s': fault = %lx", dst, Result2( dstdir ) );
	  
	  return;
	}
      
      if ((buffsize = GetFileSize( inf )) == 0) /* zero sized file has been copied ok !*/
	{
	  Close( inf );
	  Close( outf );
	  
	  return;
	}

      /*** JMP fix to get around Write bug 04/08/89 ***/
#if 0      
      while (buffsize > 16000)
	{
	  buffsize /= 2;
	  buffsize++;             /* stop one byte transfers on odd files */
	}
#else
      /* XXX - new version by NC 17/11/93 */

      if (buffsize > 16000)
	buffsize = 16000;
#endif
      
      forever
	{
	  int	n;
	  
	  
	  outf->Result2 = 0;        /* BLV : clear the error flag */
	  
	  n = (int) Read( inf, aFileBuffer, buffsize, TIMEOUT );
	  
	  if (n <= 0)
	    {
	      /* n == 0 means timeout with no chars read */
	      /* n == -1 means no more data to read      */

	      if (n != -1)
		warn( "Error whilst reading '%s': fault = %lx",
		     srcpath->Name, Result2( (Object *)inf ) );
	      
	      Close( inf  );
	      Close( outf );
	      
	      return;
	    }
	  
	  /**
	   *** BLV : bug in the 1.1 release for Write(). it returns n even if the
	   *** server gave an error message such as disk full. Result2 is filled in.
	   **/
	  
	  elif ((Write( outf, aFileBuffer, n, TIMEOUT ) != n) ||
		(outf->Result2 != 0))
	    {
	      warn( "Error whilst writing '%s': fault = %lx",
		   dstdir->Name, Result2( (Object *)outf) );
	      
	      Close( inf  );
	      Close( outf );
	      
	      return;
	    }
	}
    }

  return;
  
} /* copyfile */

static void
usage( char * message, ... )
{
  va_list	args;

  
  va_start( args, message );
  
  if (message != NULL)
    {
      fprintf( stderr, "%s: ", ProgName );
      vfprintf( stderr, message, args );
      fputc( '\n', stderr );
    }
  
  fail( USAGE );
}


int
main(
     int 	argc,
     char **	argv )
{
  Object *	TargetObj;
  Object *	SrcObj;
  char *	dest;
  int		i;
  int		l;


  
  ProgName = strrchr( argv[ 0 ], '/' );

  if (ProgName == NULL)
    ProgName = argv[ 0 ];
  else
    ProgName++;

  if (!strcmp( ProgName, "tcp" ))
    textcopy = TRUE;
  
  if (argc < 3)
    {
      usage( "Insufficient arguments" );
    }
  
  while (*(++argv)[0] == '-')
    {
      if (strcmp(*argv,"-i") == 0)
	interactive = TRUE;
      elif (strcmp(*argv,"-r") == 0)
	recursive = TRUE;
      else
	{
	  usage( "Unknown option %s",*argv );
	}
      
      argc--;
    }
  
  argc--;
  
  if (argc < 2)
    {
      /* Have we still got enough arguments? */
      
      usage( "Too few arguments" );
    }
  
  /* Check if destination is a directory */
  
  if ((TargetObj = Locate( CurrentDir, argv[ argc - 1 ])) != NULL
      && TargetObj->Type & Type_Directory)
    direct = TRUE;
  
  if (argc!=2 && direct == FALSE)
    {
      usage( "Destination must be a directory when using more than two arguments" );
    }
  
  if (direct)
    {
      /* file1 ... filen -> dir copy */
      
      for (i = 0; i < argc - 1; i++)
	{
	  if ((SrcObj = Locate( CurrentDir, argv[ i ] )) == NULL)
	    {
	      fail( "File %s does not exist", argv[i] );
	    }
	  
	  /* check if src dir contains target dir! */
	  if (recursive && (l = strlen(SrcObj->Name)) <= strlen(TargetObj->Name)
	      && strncmp(SrcObj->Name, TargetObj->Name, l) == 0
	      && (TargetObj->Name[l] == '/' || TargetObj->Name[l] == '\0'))
	    /*
	     * last test is to make sure src dir is not a
	     * partial name of target i.e. fred in freddy
	     */
	    {
	      fail( "Cannot recursively copy directory into itself" );
	    }
	  else
	    {
	      /* get  last part of name */
	      
	      if ((dest = strrchr(SrcObj->Name,'/')) == NULL)
		dest = SrcObj->Name;  /* no '/' so use entire name */
	      else
		++dest;	/* inc past last '/' */
	      
	      copyfile( SrcObj, TargetObj, dest );
	    }
	}
    }
  else
    {
      /* file -> file copy */
      
      if ((SrcObj = Locate(CurrentDir, *argv)) == NULL)
	{
	  fail( "File %s does not exist", *argv );
	}
      
      /* Modification to deal with recursive copy to new directory */
      
      if ( (! (recursive)) && (SrcObj->Type & Type_Directory) ) 
	{
	  /* PRH 4/7/90 */
	  
	  fail( "Cannot copy a directory to a file" );
	}
      
      copyfile( SrcObj, CurrentDir, argv[ 1 ] );
    }

  return 0;
  
} /* main */
