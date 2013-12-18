/*> testdivzero.c <*/

#include <stdio.h>
#include <syslib.h>

int main(void)
{
 int a = 100 ;
 int b = 0 ;

 printf("About to divide by zero!\n"), fflush(stdout) ;
 Delay(OneSec/2);

 a = a / b ;
 printf("%d\n",a) ;
 return(0) ;
}

/*> EOF testdivzero.c <*/
