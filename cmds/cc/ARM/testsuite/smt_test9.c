/*> smt_test9.c <*/
/* Split Module Table test program 9 */

#include <stdio.h>

extern unsigned int (*extfunc)() ;

int main(void)
{
 printf("Hello Split Module Table world (9)\n") ;
 extfunc() ;
 return(0) ;
}

/*> EOF smt_test9.c <*/
