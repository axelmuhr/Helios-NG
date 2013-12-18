/*{{{  Header*/

/* Author: Jamie Smith 1990 */
/* Rcs Id: $Id: newmotd.c,v 1.11 1994/06/08 09:04:27 vlsi Exp $ */

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*}}}*/
/*{{{  Variables */

char *months[] = {
	          "January",
		  "February",
		  "March",
		  "April",
		  "May",
		  "June",
		  "July",
		  "August",
		  "September",
		  "October",
		  "November",
		  "December"
	         } ;

char *days[] = {
		"Sunday",
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday"
               } ;

/*}}}*/

/*{{{  Code */

char *
do_date(int mday )
{
  char *dbuff = malloc(5) ;

  switch (mday)
    {
    case 1  :
    case 21 :
    case 31 : sprintf(dbuff,"%dst",mday) ; break ;
    case 2  :
    case 22 : sprintf(dbuff,"%dnd",mday) ; break ;
    case 3  :
    case 23 : sprintf(dbuff,"%drd",mday) ; break ;
    default : sprintf(dbuff,"%dth",mday) ; break ;
    }
  return(dbuff) ;
}

/*---------------------------------------------------------------------------*/

#ifdef __STDC__
int main(void)
#else
int main()
#endif
{
  struct tm * ltime ;
  time_t      ntime ;
  char	      lbuff[256] ;
  int	      nspaces ;

  
  time(&ntime) ;
  
  ltime = localtime(&ntime) ;

  /* FIXME: This information should really be derived from the same
            source as the Kernel build */
            
  sprintf(lbuff,"Helios Operating System Version 1.31/\"Sk\xF6ll\"") ;

  fputc(0x0D,stdout), fputc(0x0A,stdout) ;
  for (nspaces = ((80 - strlen(lbuff)) / 2); (nspaces > 0); nspaces--)
    fputc(0x20,stdout) ;
  fputc(0x1B,stdout), fputc(0x5B,stdout), fputc(0x37,stdout), fputc(0x6D,stdout) ;
  fprintf(stdout,lbuff) ;
  fputc(0x1B,stdout), fputc(0x5B,stdout), fputc(0x30,stdout), fputc(0x6D,stdout) ;
  fputc(0x0D,stdout), fputc(0x0A,stdout) ;

  sprintf( lbuff,"Copyright (C) 1987-1994, Perihelion Distributed Software") ;  
  for (nspaces = ((80 - strlen(lbuff)) / 2); (nspaces > 0); nspaces--)
    fputc(0x20,stdout) ;
  fprintf(stdout,lbuff) ;
  fputc(0x0D,stdout), fputc(0x0A,stdout) ;

  sprintf( lbuff,"GPL'ed 2013, by the Helios-NG Team") ;  
  for (nspaces = ((80 - strlen(lbuff)) / 2); (nspaces > 0); nspaces--)
    fputc(0x20,stdout) ;
  fprintf(stdout,lbuff) ;
  fputc(0x0D,stdout), fputc(0x0A,stdout) ;

  /* AxM: Fixed this part to be y2k compliant */
  sprintf( lbuff, "Build date: %s %s %s %d",
	  days[    ltime->tm_wday ],
	  months[  ltime->tm_mon ],
	  do_date( ltime->tm_mday ),
	  1900 + ltime->tm_year ) ;
          
  for (nspaces = ((80 - strlen(lbuff)) / 2); (nspaces > 0); nspaces--)
    fputc(0x20,stdout) ;
  fprintf(stdout,lbuff) ;
  fputc(0x0D,stdout), fputc(0x0A,stdout) ;
  fputc(0x0D,stdout), fputc(0x0A,stdout) ;
  
  return(0) ;
}

/*}}}*/

/* end of newmotd.c */

