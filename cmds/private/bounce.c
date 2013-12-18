
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/bounce.c,v 1.4 1994/03/08 12:03:14 nickc Exp $";
#endif

#include <stdio.h>
#include <syslib.h>
#include <nonansi.h>
#include <codes.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

void bounce(Port, Port);
void echo(Port);

char mcname[100];
unsigned int bounces;
unsigned int msgsize;

int main(int argc, char **argv)
{
	Object *code, *pm;
	Object *prog, *objv[2];
	Stream *s, *strv[4];
        char   *dummy = Null(char);
	word e;
#if 0
	MCB m;
#endif
	Environ env;

	if( argc < 2 ) 
	{
		printf("usage: bounce machine [msgsize [bounces]]\n");
		return 20;
	}

	msgsize = argc<3?1024:atoi(argv[2]);
	bounces = argc<4?5000:atoi(argv[3]);
	
	if( strcmp("remote",argv[0]) != 0 )
	{
		int i;

		MachineName(mcname);
		i = strlen(mcname);
		while(mcname[--i] != c_dirchar);
		mcname[i+1] = '\0';
		strcat(mcname,argv[1]);
		strcat(mcname,"/tasks");

		pm = Locate(CurrentDir,mcname);

		code = Locate(CurrentDir,"/loader/bounce");

		prog = Execute(pm,code);

		s = Open(prog,NULL,O_WriteOnly);

		objv[0] = CurrentDir;
		objv[1] = Null(Object);

		strv[0] = Heliosno(stdin);
		strv[1] = Heliosno(stdout);
		strv[2] = Heliosno(stderr);
		strv[3] = Null(Stream);

		argv[0] = "remote";
		env.Argv = argv;
		env.Envv = &dummy; 
		env.Objv = &objv[0];
		env.Strv = &strv[0];
	
		e = SendEnv(s->Server,&env);

		echo(prog->Reply);

#if 0
		InitMCB(&m,0,prog->Reply,NullPort,0);
		m.Timeout = MaxInt;

		e = GetMsg(&m);
#else
		InitProgramInfo(s,PS_Terminate);
		e = GetProgramInfo(s,NULL,-1);
#endif
		Close(code);
		Close(prog);
		Close(s);
		Close(pm);
	}
	else {
		bounce(MyTask->Parent,MyTask->Port);	
	}	

	return 0;
}

void bounce(Port txport, Port rxport)
{
	int i;
	MCB m;
	int start , end;
	unsigned int total;
	word e;
	byte *buf = (byte *)Malloc(msgsize);

	m.Data = buf;
	start = clock();
	for( i = 1; i <= bounces ; i++ )
	{
		InitMCB(&m,MsgHdr_Flags_preserve,txport,rxport,i);
		m.Timeout = OneSec*5;
		*(int *)&m |= msgsize;
		e = PutMsg(&m);
		m.MsgHdr.Dest = rxport;
		e=GetMsg(&m);
	}
	end = clock();

	total=end-start;

	printf("%u microseconds for %u bounces of a %u byte message\n",
		total*10000,bounces,msgsize);

	printf("%u microseconds per message\n",(total*10000)/(bounces*2));

	printf("%u bytes per second\n",	(msgsize*2*bounces*100)/total);

	Free(buf);
}


void echo(Port rxport)
{
	MCB m;
	word n;
	byte *buf = (byte *)Malloc(msgsize);

	m.Data = buf;

	do
	{
		m.MsgHdr.Dest = rxport;
		m.Timeout = OneSec*5;
		n = GetMsg(&m);
		InitMCB(&m,0,m.MsgHdr.Reply,NullPort,0);
		*(int *)&m |= msgsize;
		PutMsg(&m);
	} while( n < bounces );

	Free(buf);
}
