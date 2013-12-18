/* quick test file */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void Handler(int dummy)
{
	printf("Handler (^c) entered - exiting\n");
	exit(1);
}

int main(int argc, char **argv)
{
	char line[255];
	int loop;
	
	printf("Hello World\n");
	printf("press return to install handler\n");
	gets(line);

	signal(SIGINT, Handler);
		
	for (loop =0; loop < 3000000; loop++);
	
	printf("Automatically raising SIGINT\n");
	raise(SIGINT);
	for (loop =0; loop < 3000000; loop++);
	printf("Quiting\n");
	return 0;
}
