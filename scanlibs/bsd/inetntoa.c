/* $Id: inetntoa.c,v 1.2 1993/04/20 08:39:02 nickc Exp $ */
/*LINTLIBRARY*/

#include <stdio.h>
#include <stdlib.h>


typedef union inaddr {
   unsigned char typea[4];
   unsigned long typec;
} INADDR;


char *
inet_ntoa(unsigned long inal )
{   INADDR ina;
/*    char *str;*/
    static char str[16];

    ina.typec = inal;
/*    str = malloc(16);*/
    sprintf(str, "%d.%d.%d.%d", 
        ina.typea[0] & 255, ina.typea[1] & 255, 
        ina.typea[2] & 255, ina.typea[3] & 255);
    return str;
}

