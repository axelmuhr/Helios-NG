/*> smt_test5.c <*/
/* Split Module Table test program 5 */

#include <stdio.h>

static unsigned int variable = 0xFFFFFFFF ;

static void internal(void)
{
 printf("internal function\n") ;
}

void external(void)
{
 printf("external function\n") ;
}

int main(void)
{
 printf("Hello Split Module Table world (5)\n") ;
 internal() ;
 external() ;
 return(0) ;
}

/*> EOF smt_test5.c <*/

