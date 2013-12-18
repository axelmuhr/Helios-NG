/*------------------------------------------------------------------------
--                                                                      --
--			       ROMSPLIT.C				--
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1989, Active Book Company Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--                                                                      --
-- This utility will split a pure binary file into seperate image	--
-- files. The original image must be a multiple of 4bytes long.		--
-- If a padding value is given, then the image files are padded with	--
-- 0xFF characters to make image files totalling the size given.	--
--                                                                      --
--	Author: JGS 891017						--
--									--
-- Extensions:	JGS 900720						--
--		Generate 16bit and 8bit wide ROM images if requested.	--
--									--
--	"romsplit -p 20000 -s 32 image"					--
--				name	length	info			--
--				---------------------			--
--		creates:	image0	8000	byte0			--
--				image1	8000	byte1			--
--				image2	8000	byte2			--
--				image3	8000	byte3			--
--									--
--	"romsplit -p 20000 -s 16 image"					--
--				name	length	info			--
--				--------------------			--
--		creates:	image4	10000	byte0;byte1		--
--				image5	10000	byte2;byte3		--
--									--
--	"romsplit -p 20000 -s 8 image"					--
--				name	length	info			--
--				--------------------			--
--		creates:	image6	20000	00000..07FFF byte0	--
--						08000..0FFFF byte1	--
--					        10000..17FFF byte2	--
--						18000..1FFFF byte3	--
--									--
------------------------------------------------------------------------*/

static char *SccsId = " %W% %G% Copyright (C) 1989, Active Book Company Ltd.\n";

#include <stdio.h>
#include <a.out.h>
#include <stab.h>

#define	FALSE	0
#define TRUE	1

#define whoami	"romsplit"	/* The name of this utility */

#define defaultsplit	(32)	/* Either 8, 16 or 32 */

#define SEEK_SET	(0)
#define SEEK_INCR	(1)

/*--------------------------------------------------------------------------*/

void usage()
{
 fprintf(stderr,"Usage: %s [-p<size>] [-s[8|16|32]] binaryfile\n",whoami) ;
 fprintf(stderr,"-p : pad the total size of all image files to this\n") ;
 fprintf(stderr,"-s : specify the split style\n") ;
 fprintf(stderr,"       32 %s : \"binaryfile0..3\" created\n",((defaultsplit == 32) ? "(default)" : "         ")) ;
 fprintf(stderr,"       16 %s : \"binaryfile4..5\" created\n",((defaultsplit == 16) ? "(default)" : "         ")) ;
 fprintf(stderr,"        8 %s : \"binaryfile6\" created\n",((defaultsplit == 8) ? "(default)" : "         ")) ;
 fprintf(stderr,"NOTE: the \"binaryfile\" must have a word-aligned length\n") ;
 exit(1) ;
}

/*---------------------------------------------------------------------------*/

unsigned int readhex(char *cptr)
{
 unsigned int value = 0x00000000 ;
 int loop ;

 if ((cptr == NULL) || (*cptr == '\0'))
  {
   fprintf(stderr,"%s: NULL number given\n",whoami) ;
   exit(2) ;
  }

 for (loop = 0; (loop < 8); loop++)
  {
   if (*cptr == '\0')
    break ;

   *cptr = toupper(*cptr) ;

   if (((*cptr < '0') || (*cptr > '9')) && ((*cptr < 'A') || (*cptr > 'F')))
    {
     fprintf(stderr,"%s: invalid digit given\n",whoami) ;
     exit(2) ;
    }

   value = ((value << 4) | ((*cptr <= '9') ? (*cptr - '0') : ((*cptr - 'A') + 10))) ;
   cptr++ ;
  }

 if (*cptr != '\0')
  {
   fprintf(stderr,"%s: Too many digits given\n",whoami) ;
   exit(2) ;
  }

 return(value) ;
}

/*---------------------------------------------------------------------------*/

int readword(FILE *inf,unsigned int *value)
{
 return((fread((char *)value,sizeof(unsigned int),1,inf) == 1) ? 0 : -1) ;
}

/*--------------------------------------------------------------------------*/

int main(argc, argv)
int argc ;
char **argv ;
{
 FILE	      *inf = stdin ;
 FILE	      *outf[defaultsplit / 8] ;
 int	       loop ;
 int           padlen = 0x00000000 ;
 unsigned int  imagelen = 0x00000000 ;
 char	      *basename = NULL ;
 char	       namebuff[256] ;
 unsigned int  value ;
 int	       imagetype = defaultsplit ;
 int	       numimages = (defaultsplit / 8) ;

 for (loop=1; (loop < argc) ; loop++)
  {
   if (argv[loop][0] == '-')
    {
     switch(argv[loop][1])
      {
       case 'p' : /* pad to given size */
                  if (argv[loop][2] == '\0')
                   padlen = readhex(argv[++loop]) ;
                  else
                   padlen = readhex(&argv[loop][2]) ;
                  if ((padlen % 4) != 0)
                   {
                    fprintf(stderr,"%s: pad length must be word-aligned\n",whoami) ;
                    exit(2) ;
                   }
                  break ;

       case 's' : /* specify type of images created */
                  if (argv[loop][2] == '\0')
		   imagetype = readhex(argv[++loop]) ;
		  else
		   imagetype = readhex(&argv[loop][2]) ;
		  switch (imagetype)
		   {
		    case 0x08 : imagetype = 8 ;
		                break ;
		    case 0x16 : imagetype = 16 ;
		                break ;
		    case 0x32 : imagetype = 32 ;
		                break ;
		    default   : fprintf(stderr,"%s: split type must be 8, 16 or 32\n",whoami) ;
		                exit(2) ;
		   }
		  break ;

       default  : usage() ;
                  break ;
      }
    }
   else
    {
     if ((inf=fopen(argv[loop],"r")) == NULL)
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

 numimages = (imagetype / 8) ;

 /* numbering scheme base */
 value = ((imagetype == 32) ? 0 : ((imagetype == 16) ? 4 : 6)) ;
 /* create the image files */
 for (loop=0; (loop < numimages); loop++)
  {
   sprintf(namebuff,"%s%d\0",basename,(loop + value)) ;
#ifdef __STDC__ /* ANSI */
   if ((outf[loop] = fopen(namebuff,"wb")) == NULL)
#else
   if ((outf[loop] = fopen(namebuff,"w")) == NULL)
#endif
    {
     fprintf(stderr,"%s: unable to create output file %s%d\n",whoami,basename,(loop + value)) ;
     exit(4) ;
    }
  }

 /* now copy the data across */
 switch (imagetype)
  {
   case  8 : /* If no "pad" size if specified then we need to get the
	      * word-aligned length of the file and divide it by 4. This will
	      * give us the offset between the bytes.
	      */
             if (padlen == 0x00000000)
	      {
	       /* At the moment we will just complain */
	       fprintf(stderr,"%s: -p<size> must be specified with -s8\n",whoami) ;
	       exit(6) ;
	      }

	     /* generate file of desired size */
	     {
	      char cval = 0xFF ;
	      for (loop = 0; (loop < padlen); loop++)
	       if (fwrite(&cval,sizeof(char),1,outf[0]) != 1)
	        {
		 fprintf(stderr,"%s: unable to write padding byte\n",whoami) ;
		 exit(4) ;
		}
	      rewind(outf[0]) ;
	     }

	     padlen >>= 2 ; /* divide by 4 */
             /* "padlen" is the distance between the bytes. If we finish
	      * reading bytes from the file before we have reached "padlen" we
	      * should pad each offset with 0xFF bytes.
	      */

             while (readword(inf,&value) == 0)
	      {
	       for (loop = 0; (loop < 4); loop++)
	        {
		 char cval = ((value >> (loop * 8)) & 0xFF) ;
		 if (fwrite(&cval,sizeof(char),1,outf[0]) != 1)
		  {
		   fprintf(stderr,"%s: unable to write character\n",whoami) ;
		   exit(4) ;
		  }
		 if (loop != 3)
		  {
	           if (fseek(outf[0],(padlen - 1),SEEK_INCR) != 0)
	            {
                     fprintf(stderr,"%s: unable to seek within file\n",whoami) ;
                     exit(7) ;
	  	    }
		  }
		}
	       if (fseek(outf[0],(-3 * padlen),SEEK_INCR) != 0)
                {
                 fprintf(stderr,"%s: unable to seek within file\n",whoami) ;
                 exit(7) ;
	  	}
	       imagelen += 4 ;
	      }
             break ;

   case 16 : while (readword(inf,&value) == 0)
	      {
               for (loop=0; (loop < numimages); loop++)
	        {
                 int dval = ((value >> (loop * 16)) & 0xFFFF) ;
		 if (fwrite(&dval,(2 * sizeof(char)),1,outf[loop]) != 1)
		  {
		   fprintf(stderr,"%s: unable to write short to file %d\n",whoami,loop) ;
		   exit(4) ;
		  }
		}
	       imagelen += 4 ;
	      }

             /* and pad the image with 0xFFs if specified */
             if (padlen != 0x00000000)
	      {
	       if (imagelen > padlen)
	        {
		 fprintf(stderr,"%s: image file greater then specified pad length\n",whoami) ;
		 exit(5) ;
		}
               while (imagelen < padlen)
	        {
		 for (loop=0; (loop < numimages); loop++)
		  {
		   int dval = 0xFFFF ;
		   if (fwrite(&dval,(2 * sizeof(char)),1,outf[loop]) != 1)
		    {
		     fprintf(stderr,"%s: unable to write short to file %d\n",whoami,loop) ;
		     exit(4) ;
		    }
		  }
                 imagelen += 4 ;
		}
	      }

             break ;

   case 32 : {
              while (readword(inf,&value) == 0)
               {
                for (loop=0; (loop < numimages); loop++)
                 {
                  char cval = ((value >> (loop * 8)) & 0xFF) ;
                  if (fwrite(&cval,sizeof(char),1,outf[loop]) != 1)
                   {
                    fprintf(stderr,"%s: unable to write character to file %d\n",whoami,loop) ;
                    exit(4) ;
                   }
                 }
                imagelen += 4 ;
               }

              /* and pad the image with 0xFFs if specified */
              if (padlen != 0x00000000)
               {
                if (imagelen > padlen)
                 {
                  fprintf(stderr,"%s: image file greater than specified pad length\n",whoami) ;
                  exit(5) ;
                 }
                while (imagelen < padlen)
                 {
                  for (loop=0; (loop < numimages); loop++)
                   {
                    char cval = 0xFF ;
                    if (fwrite(&cval,sizeof(char),1,outf[loop]) != 1)
                     {
                      fprintf(stderr,"%s: unable to write character to file %d\n",whoami,loop) ;
                      exit(4) ;
                     }
                   }
                  imagelen += 4 ;
                 }
               }
	     }
	     break ;
  }
  
 /* and exit cleanly */
 for (loop=0; (loop < numimages); loop++)
  fclose(outf[loop]) ;
  
 fclose(inf) ;
 exit(0) ;
 SccsId = SccsId ; /* remove warning */
}

/*--------------------------------------------------------------------------*/
/*> EOF romsplit.c <*/
