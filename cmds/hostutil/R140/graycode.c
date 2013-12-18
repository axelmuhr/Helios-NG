/*------------------------------------------------------------------------
--									--
--				GRAYCODE.C				--
--			--------------------------			--
--									--
--		Copyright (C) 1990, Active Book Company Ltd.		--
--			   All Rights Reserved				--
--									--
-- This utility will take a pure binary and generate a seperate		--
-- gray-coded image. The original image must be a multiple of 4bytes	--
-- long. The overall image will be padded to a multiple of 512bytes	--
-- long (with 0xFF characters).						--
--									--
--	Author: JGS 900822						--
--									--
------------------------------------------------------------------------*/

static char *SccsId = " %W% %G% Copyright (C) 1990, Active Book Company Ltd.\n";

#include <stdio.h>

#define	FALSE	0
#define TRUE	1

typedef unsigned int  word ;
typedef unsigned char byte ;

#define whoami	"graycode"	/* The name of this utility */

/*--------------------------------------------------------------------------*/

void usage()
{
 fprintf(stderr,"Usage: %s binaryfile\n",whoami) ;
 fprintf(stderr,"An image file \"binaryfile.gc\" will be created\n") ;
}

/*---------------------------------------------------------------------------*/

int readword(FILE *inf,word *value)
{
 return((fread((char *)value,sizeof(word),1,inf) == 1) ? 0 : -1) ;
}

/*--------------------------------------------------------------------------*/

int readbuffer(FILE *inf,word *inbuff)
{
 int loop ;
 int state = 0 ;

 /* read 128 words from the input file (padding if necessary) */
 for (loop = 0; (loop < 128); loop++)
  {
   if (state) /* no more data */
    inbuff[loop] = 0xFFFFFFFF ;
   else
    state = readword(inf,&inbuff[loop]) ;
  }

 return(state) ;
}

/*--------------------------------------------------------------------------*/

void graycode(word *inbuff,word *outbuff)
{
 int loop ;
 for (loop = 0; (loop < 128); loop++)
  outbuff[(loop ^ (loop >> 1))] = inbuff[loop] ;

 return ;
}

/*--------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 FILE *inf = stdin ;
 FILE *outf = stdout ;
 int   loop ;
 char *basename = NULL ;
 char  namebuff[256] ;
 int   state ;
 word *inbuff ;
 word *outbuff ;

 for (loop=1; (loop < argc) ; loop++)
  {
   if (argv[loop][0] == '-')
    {
     switch(argv[loop][1])
      {
       case 'h' : /* help request */
       default  : /* unrecognised option */
                  usage() ;
                  exit(0) ;
      }
    }
   else
    {
     if ((inf = fopen(argv[loop],"r")) == NULL)
      {
       fprintf(stderr,"%s: cannot open input file\n",whoami) ;
       exit(3) ;
      }
     basename = argv[loop] ;
    }
  }
 /* end of arg processing */

 if (basename == NULL)
  {
   fprintf(stderr,"%s: image file not given\n",whoami) ;
   exit(3) ;
  }

 /* allocate the buffers */
 if (((inbuff = (word *)malloc(512)) == NULL) || ((outbuff = (word *)malloc(512)) == NULL))
  {
   fprintf(stderr,"%s: unable to allocate memory for file buffers\n") ;
   exit(3) ;
  }

 /* create the output file */
 sprintf(namebuff,"%s.gc\0",basename) ;	/* output filename */
#ifdef __STDC__ /* ANSI */
 if ((outf = fopen(namebuff,"wb")) == NULL)
#else
 if ((outf = fopen(namebuff,"w")) == NULL)
#endif
  {
   fprintf(stderr,"%s: unable to create output file %s\n",whoami,namebuff) ;
   exit(4) ;
  }
 
 do
  {
   /* read in blocks of 128words (512bytes) into "inbuff" */
   state = readbuffer(inf,inbuff) ;
   /* graycode "inbuff" to "outbuff" */
   graycode(inbuff,outbuff) ;
   /* write "outbuff" to the output file */
   if (fwrite(outbuff,512,1,outf) != 1)
    {
     fprintf(stderr,"%s: unable to write buffer to output file\n",whoami) ;
     exit(4) ;
    }
  } while (state == 0) ;

 fclose(inf) ;
 exit(0) ;
 SccsId = SccsId ; /* remove warning */
}

/*--------------------------------------------------------------------------*/
/*> EOF graycode.c <*/
