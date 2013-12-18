/* A simple test program to verify that the executive error handler is ok */

#include <stdio.h>
#include <syslib.h>

int main(int argc,char **argv)
{
	int *fred = (int *)0xffffffff;

	printf("About to cause Address error\n");
	Delay(OneSec/2);
	
	*fred = 1;

	printf("Returned after Address error\n");

}
	

