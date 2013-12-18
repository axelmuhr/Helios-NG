
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/enable.c,v 1.3 1994/03/08 12:47:42 nickc Exp $";
#endif

#include <stdio.h>
#include <syslib.h>
#include <message.h>
#include <stdlib.h>
#include <link.h>

int main(int argc, char **argv)
{
	int link;
	word e;
	
	if( argc < 2 ) 
	{
		printf("Usage: enable linkno\n");
		exit(20);
	}
	
	link = atoi(argv[1]);

	e = EnableLink(link);
	
	if( e != 0 )
	{
		printf("EnableLink failed: %lx",e);
		exit(20);
	}
	return 0;
}
