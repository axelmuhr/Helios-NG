/*> smt_test13.c <*/
/* Split Module Table test program 13 */

#include <stdio.h>

extern void function(void) ;

void (*intfunc)() = function ;

#if 0
void function(void)
{
 printf("void function\n") ;
}
#endif

int main(void)
{
 printf("Hello Split Module Table world (11)\n") ;
 intfunc() ;
 return(0) ;
}

/*> EOF smt_test11.c <*/
