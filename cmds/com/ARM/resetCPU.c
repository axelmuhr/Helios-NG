/*> resetCPU.c <*/
/* This is a very simple program that attempts to reset the Helios system.
 * The "ResetCPU()" function is provided by ABClib.
 */

#include <stdio.h>

int main(void)
{
 ResetCPU() ;
 /* the above call should never return */
 printf("resetCPU: Failed to reset the system\n") ;
 return(0) ;
}

/*> EOF resetCPU.c <*/
