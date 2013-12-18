/*> dump.c <*/
/*---------------------------------------------------------------------------*/
/* HEX ASCII dump utility	(for anti od'ers)
 * Taken from the program by Neil Robinson, August 1989.
 * JGSmith 1990.
 *
 * usage:	dump [-l<length>] [-o<offset>] [-w<width>] <filename>
 */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/file.h>		/* for lseek() */

#define whoami	"dump"

#define	byte unsigned char
#define	word unsigned long

/*---------------------------------------------------------------------------*/

#define	MIN_WIDTH	1
#define	MAX_WIDTH	16
#define	MAX_PER_LINE	14

#define	SPACING		3
#define	BAD_NUM		-1

/* externs for getopt() */
extern char *optarg ;
extern int   optind ;

/* function declarations */
byte rd_byte(FILE *) ;
word atoh(char *) ;
word get_num(char *) ;

FILE *fp ;			/* FILE pointer			  */
char *fn ;			/* file name pointer		  */
long  adr,len,off ;		/* address, length, offset	  */
char  pc[MAX_WIDTH] ;		/* array for char. interpretation */

/* option flags */
int lflg,oflg,wflg,eflg,sflg,nflg ;

/*---------------------------------------------------------------------------*/

void usage(void)
{
 fprintf(stderr,"usage: %s [-l<length>] [-o<offset>] [-w<width>] <filename>\n",whoami) ;
 fprintf(stderr,"-l<length> : number of bytes to display\n") ;
 fprintf(stderr,"-o<offset> : starting offset into the file\n") ;
 fprintf(stderr,"-w<width>  : number of bytes per display line\n") ;
 return ;
}

/*---------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 byte r ;
 int  i ;
 int  c ;
 int  width = 16 ;

 if (argc == 1)
  eflg++ ;

 while ((c = getopt(argc,argv,"l:o:w:h")) != EOF)
  {
   switch (c)
    {
     case 'l' :
		lflg++ ;
		if ((len = get_num(optarg)) == BAD_NUM)
		 nflg++ ;
		break ;

     case 'o' :
		oflg++ ;
		if ((adr = off = get_num(optarg)) == BAD_NUM)
		 nflg++ ;
		break ;

     case 'w' :
		wflg++ ;
		if ((width = get_num(optarg)) == BAD_NUM)
		 nflg++ ;
		if (width > MAX_WIDTH)
		 width = MAX_WIDTH ;
		if (width <= 0)
		 width = MIN_WIDTH ;
		break ;

     case 'h' :
                usage() ;
                exit(1) ;

     default  :
		eflg++ ;
		break ;
    }
  }

 if (nflg)
  {
   fprintf(stderr,"%s: invalid number specified (use 0x for hex)\n",whoami) ;
   eflg++ ;
  }
 if (eflg)
  {
   usage() ;
   exit(1) ;
  }

 /* adjust argv so options are transparent */
 argv += optind - 1 ;

 /* get filename */
 fn = *++argv ;
 if ((fp = fopen(fn,"rb")) == NULL)
  {
   fprintf(stderr,"can't open file: %s\n",fn) ;
   exit(1) ;
  }

 /* check if offset specified */
 if (oflg)
  {
   if (lseek(fileno(fp),off,0) == -1)
    {
     fprintf(stderr,"error: can't lseek file %s\n",fn) ;
     exit(1) ;
    }
  }

 for (;;)
  {
   /* get next byte from file */
   r = rd_byte(fp) ;
   /* stop if eof or specified length reached */
   if (!feof(fp) && (!lflg || len))
    {
     /* write address if we're starting new line */
     if ((adr - off) % width == 0)
      printf("%06lX: ",adr) ;

     /* save byte for char interpretation */
     pc[(adr - off) % width] = r ;
     adr++ ; len-- ;
     printf("%02X%c",r,' ') ;

     /* if we're at end of line do char interpretation */
     if ((adr - off) % width == 0)
      {
       printf(" ") ;
       for (i = 0;i < width;i++)
        {
	 r = pc[i] ;
	 printf("%c",((r >= 0x20) && (r <= 0x7e)) ? r : '.') ;
	}
       printf("\n") ;
      }
    }
   else
    {
     /* check if partial line to print */
     if ((adr - off) % width)
      {
       /* pad out to character field */
       for (i = 0;i < (SPACING * (width - (adr - off) % width));i++)
	printf(" ") ;
       /* and print characters */
       printf(" ") ;
       for (i = 0;i < ((adr - off) % width);i++)
        {
	 r = pc[i] ;
         printf("%c",((r >= 0x20) && (r <= 0x7e)) ? r :'.') ;
        }
       printf("\n") ;
      }
     goto end ;
    }
  }
end:
 return(0) ;
}

/*---------------------------------------------------------------------------*/

byte rd_byte(FILE *fp)
{
 char buf[1] ;
 if (fread(buf,sizeof(char),1,fp) < 1 && !feof(fp))
  {
   fprintf(stderr,"fread error on %s\n",fn) ;
   exit(1) ;
  }
 return(*buf) ;
}

/*---------------------------------------------------------------------------*/

word atoh(char *cp)
{
 word n = 0 ;
 int  i = 2 ;
 while (cp[i])
  {
   n <<= 4 ;
   if (cp[i] >= '0' && cp[i] <= '9')
    n |= cp[i] - '0' ;
   else
    if ((cp[i] & 0x5f) >= 'A' && (cp[i] & 0x5f) <= 'F')
     n |= (cp[i] & 0x5f) - 'A' + 10 ;
    else
     {
      n = BAD_NUM ;
      goto error ;
     }
   i++ ;
  }
error: return(n) ;
}

/*---------------------------------------------------------------------------*/

word get_num(char *cp)
{
 int  i = 0 ;
 word n ;

 if (cp[0] == '0' && (cp[1] ==  'x' || cp[1] == 'X'))
  n = atoh(cp) ;
 else
  {
   /* check for silly number */
   while (cp[i])
    {
     if (cp[i] < '0' || cp[i] > '9')
      return(BAD_NUM) ;
     i++ ;
    }
   n = atol(cp) ;
  }
 return(n) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF dump.c <*/
