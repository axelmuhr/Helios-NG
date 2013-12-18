/* $Id: inetaddr.c,v 1.2 1992/12/07 11:15:16 nickc Exp $ */
/*LINTLIBRARY*/

#include <stdlib.h>
#include <sys/types.h>

typedef union inaddr {
   unsigned char typea[4];
   unsigned long typec;
} INADDR;


u_long
strtoinaddr(
	    char *str,
	    char **strp )
{   INADDR ina;
    int    i;
    long   x;

    for (i = 0; i < 4; i++)
    {
        if ((x = strtol(str, &str, 10)) > 255) return -1;
        ina.typea[i] = (int)x;
        if ((i < 3) && (*str++ != '.')) return -1;
    }
    if (strp) *strp = str;
    return (ina.typec);
}

u_long
inet_addr(
  char *str )
{
    return strtoinaddr(str, 0);
}

