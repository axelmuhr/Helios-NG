/*
B7)
The following program illustrates the problem with the initialisation
of static pointers:
*/

#include <stdio.h>

struct fred {
	     unsigned int word ;
            } arrayfred[256] ;

static struct fred *fredptr = arrayfred - 1  ;

int display(void)
{
 printf("fred = &%08X\n",(int)arrayfred) ;
 printf("fredptr = &%08X\n",(int)fredptr) ;
 printf(" sizeof arrayfred = %d, sizeof arrayfred[0] = %d\n",sizeof(arrayfred), sizeof(arrayfred[0])) ;
 printf(" sizeof fred = %d\n", sizeof( struct fred)) ;
 return 1;
}

int main(void)
{
 return(display()) ;
}


