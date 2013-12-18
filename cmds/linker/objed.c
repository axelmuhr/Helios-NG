/****************************************************************/
/* Helios Linker 						*/
/*								*/
/* File: objed.c                                                */
/*                                                              */
/*                                                              */
/* Author: NHG 16-May-88                                        */
/* 					                        */
/****************************************************************/
/* RcsId: $Id: objed.c,v 1.10 1992/10/01 10:01:17 nickc Exp $ */

/* do NOT include link.h */

#ifdef __STDC__
# include <string.h>
#else
# define void int
# include <strings.h>
#endif

#include <stdio.h>
#include <module.h>
#include <unistd.h>	/* for SEEK_SET */

extern FILE *verfd;
extern WORD verbose;

#ifndef __STDC__
#include <sys/file.h>
#define SEEK_SET L_SET
#endif

#ifdef __STDC__
extern void error(char *, ...);
extern void report(char *, ...);
extern void warn(char *, ...);
#else
extern void error();
extern void report();
extern void warn();
#endif

extern int errno;

#if (defined(HOSTISBIGENDIAN) && !defined(__BIGENDIAN)) || (defined(__BIGENDIAN) && !defined(HOSTISBIGENDIAN))
long swap(x)
long x;
{
   long r;
   char *a = (char *)&x;
   char *b = (char *)&r;

   b[0] = a[3];
   b[1] = a[2];
   b[2] = a[1];
   b[3] = a[0];

   return r;
}
#else
# define swap(x) (x)
#endif

#ifdef __STDC__
void
objed( int file, char * name, long stacksize, long heapsize)
#else
void
objed( file, name, stacksize, heapsize )
  int		file;
  char *	name;
  long		stacksize;
  long		heapsize;
#endif
{
  Program prog;
  ImageHdr hdr;
  WORD info = verbose;
  int size;
  long progtype;
  char *type;


  lseek( file, 0, SEEK_SET );   /* ensure we are at start of file */

  errno = 0;

  size = read( file, (char *)&hdr, sizeof (ImageHdr) );
  
  if ( size != sizeof(ImageHdr) )
    error("Read failure, reading ImageHdr structure: %#x, errno = %d", size, errno );
  
  /*printf("header magic: %x swapped  %x \n",hdr.Magic,swap(hdr.Magic));*/
  
  if( swap(hdr.Magic) != Image_Magic )
    error("(objed) Corrupt output image file detected");
  
  size = read(file, (char *)&prog, sizeof(Program));
  
  if( size != sizeof(Program) )
    error("Read failure, reading Program structure: %#x",size);
  
  progtype = swap(prog.Module.Type);
  
  if   ( progtype == T_Program ) type = "Program";
  elif ( progtype == T_Module  ) type = "Module";
  elif ( progtype == T_ResRef  ) type = "ResRef"; 
  else type = "<UNKNOWN>";
  
  report("Type %s Size %ld bytes",type,swap(hdr.Size));
  
  report(                  "                  Old        New");
  
  if ( info )
    fprintf(verfd,"Name       %10s",prog.Module.Name);
  
  if ( name != 0 )
    {
      int i;

      
      for (i = 0; i < 32; i++ )
	prog.Module.Name[i] = 0;
      
      (void)strcpy( prog.Module.Name, name );
      
      if ( info )
	fprintf(verfd," %10s",prog.Module.Name);
    }
  
  if ( info )
    putc('\n',verfd);
  
  if ( swap(prog.Module.Type) == T_Program )
    {
      if ( info )
	fprintf(verfd,"Stacksize      %6ld",swap(prog.Stacksize));
      
      if ( stacksize != -1 )
	{
	  prog.Stacksize = swap(stacksize);
	  
	  if ( info )
	    fprintf(verfd,"     %6ld",stacksize);
	}
      
      if ( info )
	fprintf(verfd,"\nHeapsize       %6ld",swap(prog.Heapsize));
      
      if ( heapsize != -1 )
	{
	  prog.Heapsize = swap(heapsize);
	  
	  if ( info )
	    fprintf(verfd,"     %6ld",heapsize);
	}
      
      if ( info )
	putc('\n',verfd);
    }
  
  lseek( file, 0, SEEK_SET);   /* ensure we are at start of file */
  
  size = write(file, (char *)&hdr, sizeof(ImageHdr));
  
  if( size != sizeof(ImageHdr) )
    error("Failed to write back image header: %d %d",size,errno);
  
  size = write( file, (char *)&prog, sizeof (Program) );
  
  if ( size != sizeof (Program) )
    error("Failed to write back program header: %d %d",size,errno);

  return;
  
} /* objed */
