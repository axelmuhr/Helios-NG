/*----------------------------------------------------------------*/
/*                                                constantXfer.c  */
/*----------------------------------------------------------------*/

/* $Header: constantXfer.c,v 1.1 90/02/07 00:03:42 charles Locked $ */
/* $Source: /server/usr/users/charles/world/drawp/RCS/source/constantXfer.c,v $ */

/* This file contains code to write an 'objasm' comatible file  */
/*   on the standard output constaining assignments to global   */
/*   variables of some of the manifest constants declared in    */
/*   'C' code. That way we get automatic tranfser from 'C' to   */
/*   assembly.                                                  */

/*----------------------------------------------------------------*/
/*                                                #include files  */
/*----------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "private.h"

/*----------------------------------------------------------------*/
/*                                            The 'main' routine  */
/*----------------------------------------------------------------*/

int main()
{  printf(" GBLA LgAng\n");
   printf("LgAng SETA %d\n",LogAngles);
   printf(" GBLA Bf\n");
   printf("Bf SETA %d\n",BinaryFigures);
   printf(" END\n");
   return 0;
}
