
/***********************************************************************/
/*                                                                     */
/*      L I B R A R Y     F O R    T H E    C    L A N G U A G E       */
/*                                                                     */
/*              A. C. Norman  and  A. Mycroft.   March 1986            */
/*                                                                     */
/*                                                                     */
/*                       Copyright 1986 ACN/AM                         */
/*         Do not use or redistribute without written permission       */
/*                                                                     */
/***********************************************************************/

/* version 0.02 */

/* AnsiLib.c:  compile *together* all the sources for the standard
   header functions and variables.  Note that normal practice is to
   compile all the files mentioned below separately (with relevant
   pre-defines) to make a library.
*/


/* conditional compilation flags examined herein:
   NO_FLOATING_POINT: #define'ing this ensures that the library does
                     not need the FPE at some loss of functionality.
*/

/* Now include all the separate parts of the library */

#include "hostsys.h"

#ifdef ARM
#  include "armsys.c"
#endif
#ifdef ibm370
#  include "ebcdic.c"
#  include "s370sys.c"
#endif
/* exhaustivity checked by hostsys.h */
#include "signal.c"
#include "stdio.c"
#include "ctype.c"
#include "string.c"
#include "math.c"
#include "fpprintf.c"
#include "printf.c"
#include "scanf.c"
#include "stdlib.c"
#include "sort.c"
#include "alloc.c"
#include "time.c"
#include "error.c"
#include "locale.c"

/* End of library */
