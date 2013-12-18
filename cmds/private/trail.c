#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/trail.c,v 1.3 1994/03/08 12:46:08 nickc Exp $";
#endif

#include <stdio.h>
#include <syslib.h>
#include <message.h>
#include <root.h>
#include <link.h>
#include <codes.h>
#include <stdlib.h>

PUBLIC void Follow(Port port);

int
main(int argc, char **argv)
{
	if( argc < 2 )
	{
		printf("usage: trail port...\n");
		exit(1);
	}
	

	argv++;
	
	for( ;*argv; argv++ )
	{
		Port port;
		
		sscanf(*argv,"%x",&port);
		
		Follow(port);		
	}
}


PUBLIC void Follow(Port port)
{
	MCB mcb;
	word ctrl[3];
	LinkInfo *link = GetRoot()->Links[0];
	
	InitMCB(&mcb,0,link->LocalIOCPort,NullPort,FC_Private|FG_FollowTrail);

	mcb.Control = ctrl;
	
	MarshalWord(&mcb,port);
	
	PutMsg(&mcb);
}

