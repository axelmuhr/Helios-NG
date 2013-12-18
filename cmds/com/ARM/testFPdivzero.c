/*> testFPdivzero.c <*/

#include <stdio.h>
#include <math.h>
#include <syslib.h>

int main(void)
{
 float a = _huge_val ;
 float b = 0.0 ;

 printf("About to divide by zero!\n"), fflush(stdout) ;
 Delay(OneSec/2);

 a = a / b ;
 printf("%f\n",a) ;
 return(0) ;
}

/*> EOF testFPdivzero.c <*/
