/*> sysstrip/c <*/
/*----------------------------------------------------------------------*/
/*									*/
/*				sysstrip				*/
/*				--------				*/
/* Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom.	*/
/*									*/
/* The command "sysbuild" constructs a nucleus image from multiple	*/
/* image files. It constructs a relative pointer table to each of the	*/
/* objects with their image headers removed.				*/
/* This command simply performs the action of removing an image header	*/
/* from an object. It is used in systems where the nucleus image is	*/
/* constructed at run-time.						*/
/*									*/
/* It can also be used to remove "T_ResRef" structures from the start	*/
/* of objects we wish to treat as pure binaries.			*/
/*									*/
/* NOTE: At the moment it assumes little-endian architecture.		*/
/*----------------------------------------------------------------------*/

#include <stdio.h>

/*----------------------------------------------------------------------*/

typedef unsigned char byte ;
typedef unsigned int  word ;

#define TRUE	(-1)			/* non-zero TRUE */
#define FALSE	(0)			/* zero FALSE */

#define whoami	"sysstrip"		/* the program name */
#define version	"0.03"			/* and version number */

#define maxstring	(256)		/* maximum string size */

/* These magic numbers should be kept in-step with the true definitions */
#define ImMagic		0x12345678	/* image magic number */
#define T_ResRef	0x60F260F2	/* T_ResRef magic number */

/*----------------------------------------------------------------------*/

void syntax(void)
{
 printf("%s v%s (%s %s)\n",whoami,version,__DATE__,__TIME__) ;
 printf("Syntax: %s [-r] <infile> -o<outfile>\n",whoami) ;
 printf("[-r]        : remove T_ResRef structures from the image head\n") ;
 printf("<infile>    : standard Helios image file\n") ;
 printf("-o<outfile> : destination object (image header removed)\n") ;
 exit(1) ;
}

/*----------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 int   index ;			/* used in argument processing */
 int   loop ;			/* used in argument processing */
 char *cp ;			/* used in argument processing */
 int   remove = FALSE ;		/* remove T_ResRef structures */
 char  imfile[maxstring] ;	/* output image filename */
 char  infile[maxstring] ;	/* input image filename */
 FILE *infd ;			/* input file descriptor */
 FILE *outfd ;			/* output file descriptor */
 word  header[3] ;		/* standard Helios image header */
 char *buffer ;			/* image data buffer */
 char *outbuff ;		/* output image data buffer address */
 word  size ;			/* size of the image in bytes */

 imfile[0] = '\0' ;	/* no image file */
 infile[0] = '\0' ;	/* no input file */

 for (index = 1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch (argv[index][1])
      {
       case 'o' : /* output (image) file */
		  if (argv[index][2] == '\0')
		   cp = argv[++index] ;
		  else
		   cp = &argv[index][2] ;

                  for (loop=0; (*cp && (*cp != ' ')); cp++)
		   imfile[loop++] = *cp ;
		  imfile[loop] = '\0' ;
		  break ;

       case 'r' : /* remove T_ResRef header structures */
	          remove = TRUE ;
	          break ;

       default  : printf("%s: unknown option '%c'\n",whoami,argv[index][1]) ;
       case 'h' : /* help requested */
	          syntax() ;
      }
    }
   else
    {
     /* default intput (image) file */
     if (infile[0] == '\0')
      {
       cp = argv[index] ;
       for (loop=0; (*cp && (*cp != ' ')); cp++)
        infile[loop++] = *cp ;
       infile[loop] = '\0' ;
      }
     else
      {
       fprintf(stderr,"%s: unrecognised arg \"%s\"\n",whoami,argv[index]) ;
       exit(1) ;
      }
    }
  }

 if ((infd = fopen(infile,"rb")) == NULL)
  {
   fprintf(stderr,"%s: failed to open \"%s\"\n",whoami,infile) ;
   exit(2) ;
  }

 if (fread((char *)&header[0],12,1,infd) != 1)
  {
   fclose(infd) ;
   fprintf(stderr,"%s: failed to read header from \"%s\"\n",whoami,infile) ;
   exit(3) ;
  }

 if (header[0] != ImMagic)
  {
   fclose(infd) ;
   fprintf(stderr,"%s: invalid magic number in \"%s\"\n",whoami,infile) ;
   exit(4) ;
  }

 /* Get the image size and word-align it */
 size = header[2] ;
 size = ((size + (sizeof(word) - 1)) & ~(sizeof(word) - 1)) ;

 if ((buffer = (char *)malloc(size)) == NULL)
  {
   fclose(infd) ;
   fprintf(stderr,"%s: unable to allocate buffer\n",whoami) ;
   exit(5) ;
  }

 if (fread(buffer,size,1,infd) != 1)
  {
   free(buffer) ;
   fclose(infd) ;
   fprintf(stderr,"%s: failed to read data from \"%s\"\n",whoami,infile) ;
   exit(6) ;
  }

 fclose(infd) ;

 outbuff = buffer ;
 if (remove)
  {
   /* remove the 4 NULL bytes placed at the end of the image by the linker */
   size -= 4 ;
   /* remove the T_ResRef structures from the front of the image */
   for (;;)	/* loop until all T_ResRef structures removed */
    {
     word *header = (word *)outbuff ;
     /* check the magic number */
     if (header[0] == T_ResRef)
      {
       /* modify the output image depending on the length word */
       size -= header[1] ;
       outbuff += header[1] ;
      }
     else
      break ; /* no more T_ResRef structures */
    }
  }


 if ((outfd = fopen(imfile,"wb")) == NULL)
  {
   free(buffer) ;
   fprintf(stderr,"%s: failed to open \"%s\"\n",whoami,imfile) ;
   exit(2) ;
  }

 if (fwrite(outbuff,size,1,outfd) != 1)
  {
   free(buffer) ;
   fclose(outfd) ;
   fprintf(stderr,"%s: failed to write data to \"%s\"\n",whoami,imfile) ;
   exit(7) ;
  }

 free(buffer) ;
 fclose(outfd) ;
 return(0) ;
}

/*----------------------------------------------------------------------*/
/*> EOF sysstrip/c <*/
