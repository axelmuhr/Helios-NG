#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/private/RCS/mtime.c,v 1.3 1993/08/12 11:15:29 nickc Exp $";
#endif

#include <stdio.h>
#include <syslib.h>
#include <message.h>
#include <stdlib.h>
#include <helios.h>
#include <process.h>
#include <nonansi.h>

char *buf;
int bsize = 0;

word mtime;
word ttime;
Port port;

void foo()
{
	MCB m;
	
	m.MsgHdr.Dest = port;
	m.Timeout = -1;
	m.Data = buf;
	
	for(;;)
	{	
		GetMsg(&m);
	
		mtime = _ldtimer(0) - mtime - ttime;

		printf("%ld\n",mtime);
	
		PutMsg(&m);
	}
}


int main(int argc, char **argv)
{
	MCB m;
	
	if( argc == 2 )
	{
		bsize = atoi(argv[1]);
		buf = (char *)malloc(bsize);
	}
	
	port = NewPort();
		
	Fork(3000,foo,0);
	
	InitMCB(&m,0,port,NullPort,0);
	m.MsgHdr.DataSize = bsize;
	m.Data = buf;
	
	for(;;)
	{
		/* ensure that we explicitly test the time to a waiting */
		/* receiver.						*/
		while( PutReady(port) != 0 ) Delay(20000);
	
		/* calculate overhead of doing timing.			*/
		ttime = _ldtimer(0);
		ttime = _ldtimer(0) - ttime;
		
		mtime = _ldtimer(0);
		
		PutMsg(&m);
	
		GetMsg(&m);
		
		Delay(1000000);
	}
}
