/*> smt_test11.c <*/
/* Split Module Table test program 11 */

#include <stdio.h>

extern void function(void) ;

void (*intfunc)() = function ;

void function(void)
{
 printf("void function\n") ;
}

int main(void)
{
 printf("Hello Split Module Table world (11)\n") ;
 intfunc() ;
 return(0) ;
}

/*> EOF smt_test11.c <*/
