#include <helios.h>
#include <stdlib.h>

#include "cats_c40.h"
/*
  **** Second public release version 22/11/1993 ****  

  This release does not need the include file "CATS-PUB.H"
  
*/

#define COM_PORT	1

int
main( void )
{
  int		nCommPort = COM_PORT;
  size_t	nWords    = 1;
  word *	pBuffer;

  
  pBuffer = (word *) malloc( nWords * sizeof (word) );

  if (pBuffer == NULL)
    return -1;

  if (!AllocateLink( nCommPort ))
    return -2;

#if 1
  /*
   * To read on a COM port use the following code ...
   */

  C40SimpleRead( nCommPort, pBuffer, nWords );
#else
  /*
   * To write on a COM port use this code ...
   */

  C40SimpleWrite( nCommPort, pBuffer, nWords );
#endif

  if (!DeAllocateLink( nCommPort ))
    return -3;
  
  return 0;

} /* main */
