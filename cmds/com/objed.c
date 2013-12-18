#if 0
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/objed.c,v 1.15 1993/08/27 14:11:05 nickc Exp $";
#endif

#include <stdio.h>
#include <module.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

Program 	prog;
ImageHdr	hdr;
FILE *		fd;


#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#define _ARG_ if( *arg == 0 ) arg = *(++argv);

#ifdef HOSTISBIGENDIAN
int	swapopt = TRUE;	/* default to swapping */
#else
int	swapopt = FALSE;
#endif

word
swapword( word it )
{
  if (!swapopt)	return (it);
    {
      byte	from[4];
      byte	to[4];

      *((word *)from) = it;

      to[0] = from[3];
      to[1] = from[2];
      to[2] = from[1];
      to[3] = from[0];
      
      return(*((word *)to));
    }
}

void
putmodules( void )
{
  word 		isize = swapword( hdr.Size );
  word *       	v;
  Module *	m;
  int		rsize;
  word		mtype;

  
  v  = (word *)malloc( (int)isize );
  
  if ( v == NULL )
    return;
  
  fseek( fd, sizeof (ImageHdr), SEEK_SET );
  
  rsize = fread( v, 1, (int)isize, fd );
  
  if ( rsize != isize )
   { fprintf(stderr, "Image file smaller than header size\n" );
     exit(1);
   }
  
  m = (Module *)v;
  
  while ((mtype = swapword( m->Type )) != 0 )
    {
      word	msize    = swapword( m->Size    );
      word	mslot    = swapword( m->Id      );
      word	mdsize   = swapword( m->MaxData );
      word	mversion = swapword( m->Version );
      char *	mname    = m->Name;

      
      switch ( mtype )
	{		
	case T_Program:
	  printf( "Program : %10s slot %3ld version %4ld size %5ld datasize %4ld\n",
		 mname, mslot, mversion, msize, mdsize );
	  break;
	  
	case T_Module:
	  printf( "Module  : %10s slot %3ld version %4ld size %5ld datasize %4ld\n",
		 mname, mslot, mversion, msize, mdsize );
	  break;
	  
	case T_ResRef:
	  printf( "ResRef  : %10s slot %3ld version %4ld\n",
		 mname, mslot, mversion );
	  break;
	  
	default:
	  fprintf( stderr, "Unknown module type: %lx\n", mtype );
	  exit(1);
	}
      
      m = (Module *)((char *)m + msize);
    }
  
  free( v );

  return;
  
} /* putmodules */

typedef struct Image_Magics
  {
    unsigned long	Magic;
    char *		Type;	  
  }
Image_Magics;


int
main(
     int argc,
     char **argv )
{
  long 		info      = false;
  int		size;
  long		changed   = false;
  long		progtype;
  char *	file      = NULL;
  char *	type 	  = NULL;
  char *	name	  = 0;
  long		stacksize = -1;
  long		heapsize  = -1;
  long		modules	  = false;

  
  argv++;
  
  while ( *argv != 0 )
    {
      char *arg = *argv;
      
      if ( *arg++ == '-' )
	{
	  switch ( *arg++ )
	    {
	    case 'n':
	      _ARG_;
	      name = arg;
	      break;
	      
	    case 's':
	      _ARG_;
	      stacksize = (long)atol( arg );
	      break;
	      
	    case 'h':
	      _ARG_;
	      heapsize = (long)atol( arg );
	      break;			
	      
	    case 'i':
	      info = true;
	      break;	
	      
	    case 'm':
	      modules = true;
	      break;
	      
	    }
	}
      else
	{
	  file = *argv;
	}
      
      argv++;
    }

  if (file == NULL)
    {
      fprintf( stderr, "objed: must have name of file to examine\n" );
      exit(1);
    }
  
  if (name == 0 && stacksize == -1 && heapsize == -1) /* nowt to write */
    {
      fd = fopen(file,"rb");
    }
  else
    {
#if defined(__TRAN) || defined(__ARM) || defined(R140) || defined(__C40) || defined (__HELIOS)
      fd = fopen( file, "r+b" );
#else
      fd = fopen( file, "rwb" );
#endif
    }
  
  if ( fd == 0 ) 
    { fprintf(stderr, "Cannot open %s\n",file );
      exit(1);
    }
  
  size = fread( &hdr, 1, sizeof (ImageHdr), fd );
  
  if ( size != sizeof(ImageHdr) )
   { fprintf( stderr, "Read failure: %d\n", size );
     exit(1);
   }
  
    {
      static Image_Magics	Values[] =
	{
	  /*
	   * XXX - the following have been extracted from /hsrc/include/module.h
	   *       make sure that they are kept up to date
	   */

	  { 0x12345678L,	"Transputer Helios Executable\n" },
	  { 0xc4045601L,	"Helios-C40 Executable\n" },
	  { 0x0a245601L,	"Helios-ARM Executable\n" },
	  { 0x86045601L,	"Helios-I860 Executable\n" },
#ifdef HOSTISBIGENDIAN
	  { 0x01560468L,	"Helios-M68K Executable\n" },
#else
	  { 0x68045601L,	"Helios-M68K Executable\n" },
#endif
	  { TaskForce_Magic,	"Task Force Binary\n" },
	  { RmLib_Magic,	"Resource Management Library Binary\n" }
	};
      unsigned long	value = swapword( hdr.Magic );
      int		i;


      for (i = sizeof (Values) / sizeof (Values[0]); i--;)
	{
	  if (value == Values[ i ].Magic)
	    {
	      printf( Values[ i ].Type );
	      break;	      
	    }
	}
      
      if (i < 0)
	{
	  fprintf( stderr,  "File not object image\n" );
	  exit(1);
	}

      if (Values[i].Magic == 0x01560468)
	{
	  swapopt = FALSE;
	}      
    }
  
  if ( info )
    printf( "Image size = %ld bytes\n", swapword( hdr.Size ) );

  size = fread( &prog, 1, sizeof (Program), fd );
  
  if ( size != sizeof (Program) )
   { fprintf(stderr, "Read failure: %d\n", size );
     exit(1);
   }
   
  progtype = swapword( prog.Module.Type );
  
  if   ( progtype == T_Program ) type = "Program";
  elif ( progtype == T_Module  ) type = "Module";
  elif ( progtype == T_ResRef  ) type = "ResRef"; 
  
  if ( type == NULL )
   { fprintf(stderr, "Invalid Module type: %lx\n",progtype );
     exit(1);
   }
  
  if ( info )
    {
      printf( "Object type is %s\n",type );
  
      printf( "Name is '%s' ",prog.Module.Name );
    }
  
  if ( name != 0 )
    {
      int i;

      
      changed = true;
      
      for (i = 0; i < 32; i++ )
	prog.Module.Name[ i ] = 0;
      
      strncpy( prog.Module.Name, name, 31 );
      
      if ( info )
	printf( "New = '%s'", prog.Module.Name );
    }
  
  if ( info )
    putchar('\n');
  
  if ( swapword( prog.Module.Type ) == T_Program )
    {
      if ( info )
	printf( "Stacksize = %ld ", swapword( prog.Stacksize ) );
      
      if ( stacksize != -1 )
	{
	  changed = true;
	  
	  prog.Stacksize = swapword( stacksize );
	  
	  if ( info )
	    printf( "New = %ld", stacksize );
	}
      
      if ( info )
	printf( "\nHeapsize = %ld ", swapword( prog.Heapsize ) );
      
      if ( heapsize != -1 )
	{
	  changed = true;
	  
	  prog.Heapsize = swapword( heapsize );
	  
	  if ( info )
	    printf( "New = %ld", heapsize );
	}
      
      if ( info )
	putchar('\n');
    }
  else if (stacksize != -1 || heapsize != -1)
    {
      printf( "Cannot set stacksize/heapsize - image does not include a program stucture\n" );
    }
  
  if ( modules )
    putmodules();
  
  if ( changed )
    {
      
      fseek( fd, 0L, SEEK_SET );
      
      size = fwrite( &hdr, 1, sizeof (ImageHdr), fd );
      
      if ( size != sizeof (ImageHdr) )
	{ fprintf(stderr,  "Write failure writing header: wrote %d bytes, errno = %d\n",
	      size, errno);
	  exit(1);
	}
      
      size = fwrite( &prog, 1, sizeof (Program), fd );
      
      if ( size != sizeof (Program) )
	{ fprintf( stderr, "Write failure writing program: wrote %d bytes, errno = %d\n",
	      size, errno);
	  exit(1);
	}	      
    }
  
  fclose( fd );
  
  return 0;

} /* main */
