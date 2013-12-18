/*> testFPoverflow.c <*/

#include <stdio.h>
#include <math.h>
#include <syslib.h>

int main(void)
{
 float a = _huge_val ;
 float b = _huge_val ;

 printf("About to overflow!\n"), fflush(stdout) ;
 Delay(OneSec/2);

 a = a * b ;
 printf("%f\n",a) ;
 return(0) ;
}

/*> EOF testFPoverflow.c <*/

