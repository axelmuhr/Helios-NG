/*> mwr.c <*/
/*---------------------------------------------------------------------------*/
/* HEX ASCII memory modify/examine utility
 * JGSmith 1993.
 */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define whoami	"mwr"
#define version "0.01"

#define	byte unsigned char
#define	word unsigned long

/*---------------------------------------------------------------------------*/

void usage(void)
{
 fprintf(stderr,"%s (Memory modify utility) v%s (%s %s)\n",whoami,version,__TIME__,__DATE__) ;
 fprintf(stderr,"usage: %s <addr> [<val>|r] [w]\n",whoami) ;
 fprintf(stderr,"<addr> : address\n") ;
 fprintf(stderr,"<val>  : value\n") ;
 fprintf(stderr,"r      : read address, rather than writing value\n") ;
 fprintf(stderr,"w      : word value (default is byte)\n") ;
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

int main(int argc,char **argv)
{
 int    index ;		    /* index variable */
 int    gotaddr = 0 ;	    /* have start address */
 int    gotval = 0 ;	    /* have end address */
 int    doread = 0 ;        /* TRUE if we should read from address */
 int    doword = 0 ;        /* TRUE if we should be processing word values */
 word   addr = 0x00000000 ; /* address in memory */
 word   val = 0x00000000 ;  /* data value value */

 for (index=1; (index < argc); index++)
  {
   if (!gotaddr)
    {
     addr = readhex(argv[index]) ;
     gotaddr = -1 ;
    }
   else
    {
     if (!gotval)
      {
       if ((argv[index][0] == 'r') || (argv[index][0] == 'R'))
	doread = -1 ;
       else
        val = readhex(argv[index]) ;
       gotval = -1 ;
      }
     else
      {
       if (!doword)
	{
         if ((argv[index][0] == 'w') || (argv[index][0] == 'W'))
  	  doword = -1 ;
        }
       else
	{
         fprintf(stderr,"%s: unrecognised option \"%s\"\n",whoami,argv[index]);
	 usage() ;
         exit(1) ;
        }
      }
    }
  }

 if (!gotaddr)
  {
   usage() ;
   exit(0) ;
  }

 if (doread)
  {
   if (doword)
    printf("Word at %08lX = %08lX\n",addr,*((word *)addr)) ;
   else
    printf("Byte at %08lX = %02X\n",addr,*((byte *)addr)) ;
  }
 else
  {
   if (doword)
    *((word *)addr) = (word)val ;
   else
    *((byte *)addr) = (byte)val ;
  }

 return(0) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF mwr.c <*/
