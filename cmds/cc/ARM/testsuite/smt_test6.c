/*> smt_test6.c <*/
/* Split Module Table test program 6 */

#include <stdio.h>

static unsigned int variable = 0xFFFFFFFF ;

static void internal(void)
{
 static unsigned int intfuncvar ;
 printf("internal function\n") ;
}

void external(void)
{
 static unsigned int extfuncvar ;
 printf("external function\n") ;
}

int main(void)
{
 printf("Hello Split Module Table world (6)\n") ;
 internal() ;
 external() ;
 return(0) ;
}

/*> EOF smt_test6.c <*/

