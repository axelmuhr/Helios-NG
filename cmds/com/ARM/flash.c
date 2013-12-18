/*> flash.c <*/
/*----------------------------------------------------------------------*/
/*				flash.c					*/
/*				-------					*/
/*  Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom  */
/*									*/
/* Write a file to the FlashEPROM. The binary file contents are written */
/* to the ABFP FlashEPROM. The only error generated should be a		*/
/* "File too large" error.						*/
/*									*/
/*	JGSmith	901029							*/
/*----------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

/*----------------------------------------------------------------------*/

#define whoami	"flashwrite"	/* program title */
#define version	"0.01"		/* program version */

#define FALSE	0
#define TRUE	1

/*----------------------------------------------------------------------*/

#include <abcARM/ABClib.h>	/* FlashEPROM interface */

/*----------------------------------------------------------------------*/

static void usage(void)
{
 fprintf(stderr,"%s: v%s (%s %s)\n",whoami,version,__TIME__,__DATE__) ;
 fprintf(stderr,"%s [-h] [-e] [-v] <file>\n",whoami) ;
 fprintf(stderr,"-h     : this help message\n") ;
 fprintf(stderr,"-e     : erase FlashEPROM contents\n") ;
 fprintf(stderr,"-v     : verify FlashEPROM contents against <file>\n") ;
 fprintf(stderr,"<file> : file to be written to FlashEPROM\n") ;
 exit(1) ;
}

/*----------------------------------------------------------------------*/
/* checkimage:
 * returns:	TRUE	image exists		(size updated)
 *		FALSE	image does NOT exist	(size preserved)
 */
static int checkimage(char *imfile,word *size)
{
 FILE *tf ;
 
 /* check if we can open the given file */
 /* updating "size" if we can */
 
 if ((tf = fopen(imfile,"r")) == NULL)
  return(FALSE) ;

 fflush(tf) ;
 if (fseek(tf,0,2/* should be SEEK_END */) != 0)
  {
   fprintf(stderr,"%s: Unable to seek to end of file \"%s\"\n",whoami,imfile) ;
   exit(5) ;
  }
 *size = (word)ftell(tf) ;
 fclose(tf) ;

 return(TRUE) ;
}

/*----------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 int   index ;		/* used in argument processing */
 int   erase = FALSE ;	/* perform erase on FlashEPROM */
 int   verify = FALSE ; /* verify file against FlashEPROM (do not program) */
 char *infile = NULL ;	/* input filename */
 word  filesize ;	/* input file size in bytes */
 char *buffer ;		/* image file buffer */
 FILE *ihand ;		/* input FILE handle */

 /* verify FlashEPROM presence */
 if (!FlashCheck())
  {
   fprintf(stderr,"%s: FlashEPROM not found\n",whoami) ;
   exit(9) ;
  }

 for (index=1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch (argv[index][1])
      {
       case 'h' : /* help */
                  usage() ;

       case 'e' : /* erase */
                  erase = TRUE ;
                  break ;

       case 'v' : /* verify */
	          verify = TRUE ;
                  break ;
      }
    }
   else
    {
     /* default input file */
     if (infile == NULL)
      {
       infile = argv[index] ;
       if (!checkimage(infile,&filesize))
        {
	 fprintf(stderr,"%s: \"%s\" not found\n",whoami,infile) ;
	 exit(2) ;
	}
       if (filesize > flash_size)
        {
	 fprintf(stderr,"%s: specified file bigger than &%08X bytes\n",whoami,flash_size) ;
	 exit(3) ;
	}
      }
     else
      {
       fprintf(stderr,"%s: unrecognised arg \"%s\"\n",whoami,argv[index]) ;
       exit(4) ;
      }
    }
  }

 if (erase && verify)
  {
   fprintf(stderr,"%s: -e and -v are mutually exclusive\n",whoami) ;
   exit(5) ; 
  }

 if ((infile == NULL) && !erase)
  {
   fprintf(stderr,"%s: Image file not specified\n",whoami) ;
   exit(5) ;
  }

 /* Erase the FlashEPROM is the user desires */
 if (erase)
  {
   if ((index = FlashErase()) != -1)
    {
     fprintf(stderr,"%s: FlashEPROM erase failed at index &%08X\n",whoami,index) ;
     exit(10) ;
    }
  }

 if (infile != NULL)
  {
   if ((buffer = (char *)malloc(filesize)) == NULL)
    {
     fprintf(stderr,"%s: unable to allocate image buffer memory\n",whoami) ;
     exit(6) ;
    }

   if ((ihand = fopen(infile,"r")) == NULL)
    {
     fprintf(stderr,"%s: unable to open file \"%s\"\n",whoami,infile) ;
     exit(7) ;
    }

   if (fread(buffer,filesize,1,ihand) != 1)
    {
     fprintf(stderr,"%s: unable to read data from file \"%s\"\n",whoami,infile);
     exit(8) ;
    }
   fclose(ihand) ;

   if (verify)
    {
     if ((index = FlashVerify(buffer,filesize)) != -1)
      {
       fprintf(stderr,"%s: verify failed at index &%08X\n",whoami,index) ;
       exit(10) ;
      }
    }
   else
    {
     if ((index = FlashWrite(buffer,filesize)) != -1)
      {
       fprintf(stderr,"%s: write failed at index &%08X\n",whoami,index) ;
       exit(10) ;
      }
    }
  }
 return(0) ;
}

/*----------------------------------------------------------------------*/
/*> EOF flash.c <*/

