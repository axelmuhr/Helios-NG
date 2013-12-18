/*> c.disass <*/
/*---------------------------------------------------------------------------*/
/* Simple ARM disassembler                                                   */
/*---------------------------------------------------------------------------*/
/* RcsId: $Id: armdisas.c,v 1.10 1994/03/08 14:04:36 nickc Exp $ */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(SUN4) || defined(RS6000) /* FIXME : we need a native Helios-ARM version of clxlib.a */
#include "disass.h" /* currently held in "armltd.binaries/include" */
#else
#include "ARMshape.h"
#endif

#ifdef RS6000
typedef unsigned int uint;
#endif

#define whoami	"disass"

#ifndef TRUE
#define TRUE (1 == 1)
#define FALSE (1 == 0)
#endif

/*---------------------------------------------------------------------------*/

int pcsregs = TRUE;		/* TRUE == display PCS register aliases */

#if defined(SUN4) || defined(RS6000)
char *ARM_regnames[] = {"a1","a2","a3","a4","v1","v2","v3","v4","v5","dp","sl", "fp", "ip", "sp", "lr", "pc"} ;
char *ARM_rawnames[] = {"r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","r11","r12","r13","r14","pc"} ;

static char *local_decode(dis_cb_type type,int32 offset,unsigned32 address,int width,void *arg,char *buffer)
{
 /* We provide no more decoding at the moment */
 return(buffer) ;
}

static void disassemble(unsigned long instruction,unsigned long pc,int flag)
{
 static char buffer[256] ; /* disassembly line */

 disass_32or26((unsigned32)instruction,(unsigned32)pc,buffer,NULL,local_decode,TRUE) ;
 printf(buffer) ;
 return ;
}
#else
extern void disassemble(unsigned long instruction,unsigned long pc,int flag);
#endif

#ifdef HOSTISBIGENDIAN
int swap(int *x) {
		char	*from = (char *)x;
		char	to[4];

		to[0] = from[3];
		to[1] = from[2];
		to[2] = from[1];
		to[3] = from[0];

		return(*((int *)to));
}
#endif

unsigned int readhex(char *cptr)
{
 unsigned int value = 0x00000000 ;
 int loop ;

#ifdef DEBUG
 printf("HEX string \"%s\"\n",cptr) ;
#endif
 
 if ((cptr == NULL) || (*cptr == '\0'))
  {
   fprintf(stderr,"%s: NULL hex number given\n",whoami) ;
   exit(3) ;
  }

 for (loop = 0; (loop < 8); loop++)
  {
   if (*cptr == '\0')
    break ;

   *cptr = toupper(*cptr) ;

#ifdef DEBUG
   printf("HEX character '%c'\n",*cptr) ;
#endif

   if (((*cptr < '0') || (*cptr > '9')) && ((*cptr < 'A') || (*cptr > 'F')))
    {
     fprintf(stderr,"%s: invalid HEX digit given\n",whoami) ;
     exit(3) ;
    }

   value = ((value << 4) | ((*cptr <= '9') ? (*cptr - '0') : ((*cptr - 'A') + 10))) ;
   cptr++ ;
  }

 if (*cptr != '\0')
  {
   fprintf(stderr,"%s: Too many HEX digits given\n",whoami) ;
   exit(3) ;
  }

#ifdef DEBUG
 printf("HEX value &%08X\n",value) ;
#endif

 return(value) ;
}

/*---------------------------------------------------------------------------*/
/* return a printable character for the specified byte in the number */

static char byte(int bnum,unsigned int val)
{
 char retval = ((val & (0xFF << (8 * bnum))) >> (8 * bnum)) ;
 /* Only return 7bit printable characters */
 return(((retval < ' ') || (retval >= 0x7F)) ? '.' : retval) ;
}

/*---------------------------------------------------------------------------*/

int readword(FILE *inf,unsigned int *value)
{
	return((fread((char *)value,sizeof(unsigned int),1,inf) == 1) ? 0 : -1);
}

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 FILE		*inf = stdin ;
 int		loop ;
 char		*cptr ;
 unsigned int	value ;
 unsigned int	baseaddress = 0x00000000 ;
 int		baseaddressgiven = FALSE ;
 unsigned int	endaddress = 0x00000000 ;
 unsigned int	foffset = 0x00000000 ;

 for (loop=1; (loop < argc); loop++)
  {
   if (argv[loop][0] == '-')
    {
     switch (tolower(argv[loop][1]))
      {
       case 'b' : if (argv[loop][2] == '\0')
                   cptr = argv[++loop] ;
                  else
                   cptr = &argv[loop][2] ;

                  baseaddress = readhex(cptr) ;
                  baseaddressgiven = TRUE ;
                  break ;
                  
       case 's' : if (argv[loop][2] == '\0')
                   cptr = argv[++loop] ;
                  else
                   cptr = &argv[loop][2] ;

                  foffset = readhex(cptr) ;
                  break ;
                  
	case 'p':
		pcsregs = TRUE;
		break;

	case 'r':
		pcsregs = FALSE;
		break;

       case 'h' : printf("%s: ARM binary disassembler v1.1\n",whoami) ;
                  printf("options:\n") ;
		  printf("-h           : this help message\n");
		  printf("-p           : use PCS register aliases (default)\n");
          printf("-r           : use raw ARM register names\n");
                  printf("-b <address> : Specify the base address to use.\n") ;
                  printf("-s <offset>  : Specify a start offset within the image file\n") ;
                  printf("<imagefile>  : An ARM binary image file.\n") ;
                  printf("\nNOTE: If the \"-b\" flag is used the disassembly address will always start\n") ;
                  printf("      at this value, otherwise it will start at the \"-s\" offset.\n") ;
                  printf("\n") ;

       default  : printf("Usage: %s [-h] [-p] [-r] [-b<address>] [-s<offset>] <imagefile>\n",whoami) ;
                  exit(1) ;
      }
    }
   else
    {
     /* open specified input file */
     if ((inf = fopen(argv[loop],"r")) == NULL)
      {
       fprintf(stderr,"%s: cannot open input file\n",whoami) ;
       exit(2) ;
      }
    }
  }
 /* end of argument processing */

 if (foffset != 0)
  {
   if (fseek(inf,foffset,1) != 0)
    {
     fprintf(stderr,"%s: failed to seek to starting offset\n",whoami) ;
     exit(4) ;
    }
   /* It is probably nicer having the "baseaddress" as specified */
   if (!baseaddressgiven)
    baseaddress += foffset ;
  }

#if defined(SUN4) || defined(RS6000)
 if (pcsregs)
  disass_setregnames(ARM_regnames,NULL) ;
 else
  disass_setregnames(ARM_rawnames,NULL) ;
#endif

 /* disassemble from the current point in the file */
 while (readword(inf,&value) == 0)
  {
#ifdef HOSTISBIGENDIAN
    value = swap((int *)&value);
#endif
   printf("%08X : ",baseaddress) ;
   printf("%c%c%c%c ",byte(0,value),byte(1,value),byte(2,value),byte(3,value)) ;
   printf(": %08X : ",value) ;

   disassemble(value,baseaddress,TRUE) ;
   putchar('\n') ;
   baseaddress += 4 ;

   /* when we are looking at the "text" segment of a file, we must
    * terminate at the correct point
    */
   if ((endaddress != 0x00000000) && (endaddress <= baseaddress))
    break ;
  }
  
 fclose(inf) ;
 exit(0) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF c.disass <*/
