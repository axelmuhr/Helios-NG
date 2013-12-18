
#include <stdio.h>
#include <string.h>
#include <fault.h>
#include <codes.h>

int main(int argc, char **argv)
{
	int val;
	char msg[128];
	
	if( argc < 2 ) exit(1);
	
	sscanf(argv[1],"%x",&val);
	
	Fault(val,msg,80);
	
	printf("%08x: %s\n",val,msg);
	
	return 0;
}

