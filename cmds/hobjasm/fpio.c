/* -> fpio/c
 * Title:               Floating Point operations
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "constant.h"
#include "fpio.h"
#include "p1hand.h"
#include <stdio.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------*/
/* takes characters from string^[index] onwards and reads an IEEE floating
 * point number, in result1 and (if necessary) in result2. Returns with index
 * updated to point to the next character after the number.
 */
ReadResponse hRead(char *string,CARDINAL *index,Size size,CARDINAL *result1,CARDINAL *result2)
{
 union {
        float    f ;
        CARDINAL f1 ;
       } u1 ;
 union {
        double d ;
        struct {
                CARDINAL d1 ;
                CARDINAL d2 ;
               } s ;
       } u2 ;

 int i = (size == Single) ? sscanf((string + *index)," %f ",&u1.f) : sscanf((string + *index), " %lf ",&u2.d) ;
 if (i != 1)
  {
   printf("Scanf failed to read floating point number\n") ;
   return(NoNumberFound) ;
  } ;
 while (string[*index] == Space)
  (*index)++ ;
 while (!TermCheck(string[*index]))
  (*index)++ ;
 while (string[*index] == Space)
  (*index)++ ;
 if (size == Single)
  *result1 = u1.f1 ;
 else
  {
   *result1 = u2.s.d1 ;
   *result2 = u2.s.d2 ;
  } ;
 return(OK) ;
}

/*---------------------------------------------------------------------------*/
/* EOF fpio/c */
