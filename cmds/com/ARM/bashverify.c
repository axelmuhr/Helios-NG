/* verify.c: provides an executable that calls the 'VerifyFS'
   routine as used by the main basher program. 
*/

#include <stdio.h>

#include "bash.h"


/*----------------------------------------------------------------------*/
/*                                                                 main */
/*----------------------------------------------------------------------*/

int main(int argc, char *argv[]) 
{ 

	do {
		VerifyFS();
		printf("\nHit return to re-verify (q = quit)\n");
	} while(getchar() != 'q');

	return 0; 
}
