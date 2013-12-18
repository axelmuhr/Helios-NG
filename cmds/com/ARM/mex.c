/*> mex.c <*/
/*---------------------------------------------------------------------------*/
/* HEX ASCII memory examine utility
 * JGSmith 1990.
 *
 * usage:	mex [<start> [<end>]] [[-a] -w<width>|-f]
 */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define whoami	"mex"
#define version "0.02"

#define	byte unsigned char
#define	word unsigned long

/*---------------------------------------------------------------------------*/

#define	MIN_WIDTH	1
#define	MAX_WIDTH	16
#define MAX_AWIDTH	64

#define	SPACING		3

/*---------------------------------------------------------------------------*/

void usage(void)
{
 fprintf(stderr,"%s (Memory examine utility) v%s (%s %s)\n",whoami,version,__TIME__,__DATE__) ;
 fprintf(stderr,"usage: %s [<start> [<end>]] [[-a] -w<width>|-f]\n",whoami) ;
 fprintf(stderr,"<start>   : start address (default &00000000)\n") ;
 fprintf(stderr,"<end>     : end address (default &03FFFFFF)\n") ;
 fprintf(stderr,"-f        : full word and binary information\n") ;
 fprintf(stderr,"-a        : display ASCII data only (default width 64)\n") ;
 fprintf(stderr,"-w<width> : bytes per display line (default 16)\n") ;
 return ;
}

/*--------------------------------------------------------------------------*/

word readhex(char *cptr)
{
 word value = 0x00000000 ;
 int loop ;

 if ((cptr == NULL) || (*cptr == '\0'))
  {
   fprintf(stderr,"%s: NULL hex number given\n",whoami) ;
   exit(2) ;
  }

 for (loop = 0; (loop < 8); loop++)
  {
   if (*cptr == '\0')
    break ;

   *cptr = toupper(*cptr) ;

   if (((*cptr < '0') || (*cptr > '9')) && ((*cptr < 'A') || (*cptr > 'F')))
    {
     fprintf(stderr,"%s: invalid HEX digit given\n",whoami) ;
     exit(2) ;
    }

   value = ((value << 4) | ((*cptr <= '9') ? (word)(*cptr - '0') : ((word)(*cptr - 'A') + 10L))) ;
   cptr++ ;
  }

 if (*cptr != '\0')
  {
   fprintf(stderr,"%s: Too many HEX digits given\n",whoami) ;
   exit(2) ;
  }

 return(value) ;
}

/*---------------------------------------------------------------------------*/

byte rd_byte(byte *address)
{
 /* This may need to be updated to get access to protected memory. */
 /* At the moment it just assumes we have access. */
 return(*address) ;
}

/*---------------------------------------------------------------------------*/

word rd_word(word *address)
{
 /* This may need to be updated to get access to protected memory. */
 /* At the moment it just assumes we have access. */
 return(*address) ;
}

/*---------------------------------------------------------------------------*/

void display_char(word start,word endaddr,word width)
{
 byte r ;			/* character value */

 for (; (start < endaddr); start++)
  {
   /* get next byte */
   r = rd_byte((byte *)start) ;

   /* write address if we're starting new line */
   if ((start % width) == 0)
    printf("\n%06lX: ",start) ;

   printf("%c",((r >= 0x20) && (r <= 0x7E)) ? r : '.') ;
  }
 printf("\n") ;
 return ;
}

/*---------------------------------------------------------------------------*/

void display_byte(word start,word endaddr,word width)
{
 int  index ;			/* general index */
 byte r ;			/* character value */
 char pc[MAX_WIDTH] ;		/* array for character interpretation */

 for (;;)
  {
   /* stop if specified length reached */
   if (start < endaddr)
    {
     /* get next byte */
     r = rd_byte((byte *)start) ;

     /* write address if we're starting new line */
     if ((start % width) == 0)
      printf("%06lX: ",start) ;

     /* save byte for char interpretation */
     pc[start % width] = r ;
     start++ ;
     printf("%02X%c",r,' ') ;

     /* if we're at end of line do char interpretation */
     if ((start % width) == 0)
      {
       printf(" ") ;
       for (index = 0;(index < width);index++)
        {
	 r = pc[index] ;
	 printf("%c",((r >= 0x20) && (r <= 0x7E)) ? r : '.') ;
	}
       printf("\n") ;
      }
    }
   else
    {
     /* check if partial line to print */
     if (start % width)
      {
       /* pad out to character field */
       for (index = 0;(index < (SPACING * (width - start % width)));index++)
	printf(" ") ;

       /* and print characters */
       printf(" ") ;
       for (index = 0;(index < (start % width));index++)
        {
	 r = pc[index] ;
         printf("%c",((r >= 0x20) && (r <= 0x7E)) ? r :'.') ;
        }
       printf("\n") ;
      }
     goto endbyte ;
    }
  }
endbyte:
 return ;
}

/*---------------------------------------------------------------------------*/

void display_full(word start,word endaddr)
{
 word v ;
 word  loop ;

 for (; (start < endaddr); start += sizeof(word))
  {
   printf("%06lX : ",start) ;
   v = rd_word((word *)start) ;
   printf("%08lX : ",v) ;
   for (loop=0; (loop < sizeof(int)); loop++)
    {
     word r = ((v & (0xFFL << (loop * 8))) >> (loop * 8)) ;
     printf("%c",((r >= 0x20) && (r <= 0x7E)) ? (char) r : '.') ;
    }
   printf(" : ") ;
   for (loop=0; (loop < (sizeof(int) * 8)); loop++)
    printf("%c",(((v & (1L << ((sizeof(int) * 8L) - (loop+1)))) == 0)?'0':'1')) ;
   printf("\n") ;
  }
 return ;
}

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 int    index ;			/* index variable */
 int    gotstart = 0 ;		/* have start address */
 int    gotend = 0 ;		/* have end address */
 int    gotwidth = 0 ;		/* have width value */
 char  *cp ;			/* temporary text pointer */
 int    full = 0 ;		/* full word and binary display */
 int    ascii = 0 ;		/* do ASCII only display */
 word   width = 16 ;		/* display line width */
 word   start = 0x00000000 ;	/* start address in memory */
 word   endaddr = 0x03FFFFFF ;	/* end address in memory */

 for (index=1; (index < argc); index++)
  {
   if (argv[index][0] == '-')
    {
     switch (argv[index][1])
      {
       case 'f' : /* format selection */
	          if (gotwidth || ascii)
		   {
		    fprintf(stderr,"%s: -%c specified (cannot use -f)\n",whoami,(gotwidth ? 'w' : 'a')) ;
		    exit(1) ;
		   }

	          full = -1 ; 
                  break ;

       case 'w' :
	          if (full)
		   {
		    fprintf(stderr,"%s: -f flag specified (cannot use -w)\n",whoami) ;
		    exit(1) ;
		   }

		  if (argv[index][2] == '\0')
		   cp = argv[++index] ;
		  else
		   cp = &argv[index][2] ;
		  if ((cp == NULL) || (!cp[0]))
		   {
		    fprintf(stderr,"%s: NULL length number given\n",whoami) ;
                    exit(1) ;
		   }
		  {
		   int loop ;
		   for (loop = 0; cp[loop]; loop++)
                    if ((cp[loop] < '0') || (cp[loop] > '9'))
		     {
		      fprintf(stderr,"%s: bad number %s\n",whoami,argv[index]);
                      exit(1) ;
                     }
		  }
                  width = atol(cp) ;
		  if (ascii)
		   {
		    if (width > MAX_AWIDTH)
		     width = MAX_AWIDTH ;
		   }
		  else
		   {
		    if (width > MAX_WIDTH)
		     width = MAX_WIDTH ;
		   }
		  if (width == 0)
		   width = MIN_WIDTH ;
		  gotwidth = -1 ;
		  break ;

       case 'a' :
	          if (full)
		   {
		    fprintf(stderr,"%s: -f flag specified (cannot use -a)\n",whoami) ;
		    exit(1) ;
		   }
	          
                  if (!gotwidth)
                   width = MAX_AWIDTH ;

		  ascii = -1 ;
                  break ;

       case 'h' : usage() ;
                  exit(1) ;

       default  :
	          fprintf(stderr,"%s: unrecognised option \"%s\"\n",whoami,argv[index]) ;
		  exit(1) ;
      }
    }
   else
    {
     if (!gotstart)
      {
       start = readhex(argv[index]) ;
       gotstart = -1 ;
      }
     else
      {
       if (!gotend)
        {
         endaddr = readhex(argv[index]) ;
         gotend = -1 ;
        }
       else
        {
         fprintf(stderr,"%s: unrecognised option \"%s\"\n",whoami,argv[index]);
         exit(1) ;
	}
      }
    }
  }

 if (!full)
  {
   start = (start & ~(width - 1)) ;	/* "width" align the start address */
   if (ascii)
    display_char(start,endaddr,width) ;
   else
    display_byte(start,endaddr,width) ;
  }
 else
  {
   start = start & ~(sizeof(word) - 1) ; /* word align the start address */
   display_full(start,endaddr) ;
  }

 return(0) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF mex.c <*/
