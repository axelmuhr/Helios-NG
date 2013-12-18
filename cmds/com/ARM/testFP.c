/*> testFP.c <*/
/* A simple test program to verify that the FPE system is working */

#include <stdio.h>
#include <syslib.h>
#include <nonansi.h>
#include <math.h>

int main(int argc,char **argv)
{
 float		f1 ;
 float		f2 ;
 float		f3 ;
 int		i1 ;
 unsigned int	u1 ;

 f1 = 0.1 ;
 f2 = 0.1 ;
 f3 = f1 * f2 ;
 printf("f3 = %f\n",f3), fflush(stdout) ;
 
 u1 = 0x10000000 ;
 f2 = u1 * f3 ;
 printf("f2 = %f\n",f2), fflush(stdout) ;
 
 i1 = -1 ;
 f1 = i1 * f3 ;
 printf("f1 = %f\n",f1), fflush(stdout) ;

 i1 = (int)f2 ;
 printf("i1 = %d\n",i1), fflush(stdout) ;
 
 u1 = (unsigned int)f2 ;
 printf("u1 = &%08X\n",u1), fflush(stdout) ;

 f1 = 0.1 ;
 printf("sin(%f) = %f\n",f1,sin(f1)), fflush(stdout) ;
 f3 = asin(sin(f1)) ;
 printf("asin(%f) = %f\n",sin(f1),f3), fflush(stdout) ;

 return(0) ;
}

