/* -> fpio/h
 * Title:               Floating Point operations
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef fpio_h
#define fpio_h

/*---------------------------------------------------------------------------*/

#include "constant.h"

typedef enum {Single,Double} Size ;

typedef enum {OK,ReadOverflow,NoNumberFound} ReadResponse ;


/* takes characters from string^[index] onwards and reads an IEEE floating
 * point number, in result1 and (if necessary) in result2. Returns with index
 * updated to point to the next character after the number.
 */
ReadResponse hRead(char *string,CARDINAL *index,Size size,CARDINAL *result1,CARDINAL *result2) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF fpio/h */
