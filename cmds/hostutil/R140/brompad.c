/* $Header: /giga/Helios/cmds/hostutil/R140/RCS/brompad.c,v 1.1 91/02/14 21:56:58 paul Exp $ */
/* $Source: /giga/Helios/cmds/hostutil/R140/RCS/brompad.c,v $ */

/*------------------------------------------------------------------------*/
/*                                                             brompad.c  */
/*------------------------------------------------------------------------*/

/* This is the sourc file for a utility that pads a file by adding as     */
/*   many 0xFF bytes as necessary to increase the size of the file to     */
/*   a given size. Default is 2Meg                                        */

/*------------------------------------------------------------------------*/
/*                                                         Include Files  */
/*------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#define buffsize (16 * 1024)

/*------------------------------------------------------------------------*/
/*                                                             main(...)  */
/*------------------------------------------------------------------------*/

int main(int argc,char **argv)
{
 unsigned long  sz ;
 unsigned long  as ;
 unsigned long  i ;
 int            arg ;
 FILE          *fh ;
 static char    buf[buffsize] ;

 if (argc<2 || argc>3)
  goto GiveHelp ;
   
 if (argc == 2)
  {
   sz  = 2*1024*1024 ;
   arg = 1 ;
  }
 else
  {
   char          *ptr ;
   char          *ep ;
   unsigned long  term ;

   sz = 0 ;
   ptr = argv[1] ;
   do
    {
     term = strtoul(ptr,&ep,0) ;
     if (ep == ptr)
      term = 1 ;
     else
      ptr = ep ;

     switch (*ptr)
      {
       case 'k' :
       case 'K' : term *= 1024 ;
                  ptr++ ;
                  break ;
       case 'm' :
       case 'M' : term *= 1024*1024 ;
                  ptr++ ;
                  break ;
      }

     sz += term ;
     if (*ptr=='+')
      {
       ptr++ ;
       continue ;
      }
     break ;
    } while(1) ;

   if (*ptr)
    goto GiveHelp ;
   arg = 2 ;
  }

 printf("Padding file \"%s\" to %lu (=0x%lX) bytes (all 0xFF bytes)\n",argv[arg],sz,sz) ;
   
 fh = fopen(argv[arg],"ab+") ;
 if (fh == NULL)
  {
   perror("Cannot open file ") ;
   return(1) ;
  }
   
 fseek(fh,0L,SEEK_END) ;
 as = ftell(fh) ;
 if (as > sz)
  {
   printf("File is already %lud (=0x%luX) bytes long : Not truncating\n",as,as) ;
   fclose(fh) ;
   return(1) ;
  }

 for (i = 0; (i < buffsize); i++)
  buf[i] = 0xFF ;

 for (; (as < sz); as += i) 
  {
   i = buffsize ;
   if ((as + i) > sz)
    i = sz - as ;
   fwrite((void*)buf,sizeof(char),(int)i,fh) ;
  }
 fclose(fh) ;
 return(0) ;
   
GiveHelp:
 printf("Format : brompad [<size>] <file-name>\n") ;
 printf("Pads the given file to a size using 0xFF bytes\n") ;
 printf("<size> by default is 2 MegaBytes\n") ;
 printf("<size> can be specified in C-format integer constants\n") ;
 printf("(Hence you can use hex, octal or decimal)\n") ;
 printf("A constant may be suffixed by k or K for kilo-bytes\n") ;
 printf("A constant may be suffixed by m or M for mega-bytes\n") ;
 printf("You can add any number of such costants together using '+'\n") ;
 return(1) ;
}

/*---------------------------------------------------------------------------*/
/*> EOF brompad/c <*/
