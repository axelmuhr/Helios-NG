
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/remote.c,v 1.2 1990/08/23 10:30:28 james Exp $";

#include <stdarg.h>
#include <stdio.h>
#include <syslib.h>
#include <gsp.h>
#include <codes.h>
#include <nonansi.h>
#include <signal.h>

extern char **environ; /* HELIOSARM fxi */


int remote(char *machine,char **argv, bool waitp);
void error(char *f, ... );
void remotesignal(int sig);

static Stream *progstream;

int main(int argc, char **argv)
{
	int wait = true;
	
	if( argc < 2 ) 
	{
		error("Usage: remote [-d] mc cmd args...");
		return 20;
	}

	argv++;
	
	if( **argv == '-' )
	{
		char *arg = *argv++;
		while( *(++arg) )
		{
			switch( *arg )
			{
			case 'd': wait = false; break;
			}
		}
	}
	exit(remote(*argv,&argv[1],wait));

}

int remote(char *machine,char **argv, bool waitp)
{
	Object *code, *pm;
	Object *prog, *objv[2];
	Stream *s, *strv[4];
        char   *dummy = Null(char);
	word e;
	MCB m;
	Environ env;
	Object *comdir;
	static char pmname[100];
	static char progname[100];

	strcpy(pmname,"/");
	strcat(pmname,machine);
	strcat(pmname,"/tasks");
	
	pm = Locate(CurrentDir,pmname);

	if( pm == NULL )
	{
		error("Cannot locate %s: %x",pmname,Result2(CurrentDir));
		return 20;
	}
	
	find_file(progname,argv[0]);

	code = Locate(CurrentDir,progname);

	if( code == NULL )
	{
		error("Cannot locate %s : %x",progname,Result2(comdir));
		return 20;
	}

	prog = Execute(pm,code);

	if( prog == Null(Object))
	{
		error("Cannot execute %s : %x",argv[0],pm==NULL?Result2(code):Result2(pm));
		return 20;
	}

	s = Open(prog,NULL,O_WriteOnly);

	if( s == Null(Stream) )
	{
 		error("Cannot open %s : %x",&prog->Name,Result2(prog));
		return 20;
	}

	objv[0] = CurrentDir;
	objv[1] = Null(Object);

	strv[0] = fdstream(0);
	strv[1] = fdstream(1);
	strv[2] = fdstream(2);
	strv[3] = Null(Stream);

	env.Argv = argv;
	env.Envv = environ;
	env.Objv = &objv[0];
	env.Strv = &strv[0];
	
	e = SendEnv(s->Server,&env);

	if( waitp )
	{

		progstream = s;
		signal(SIGINT, remotesignal);
		InitProgramInfo(s,PS_Terminate);
		e = GetProgramInfo(s,NULL,-1);
	}
	
	Close( code );
	Close( prog );
	Close( s );
	Close( pm );
	
	return e;
}

void remotesignal(int sig)
{
	Stream *s;
	
	s = NewStream(progstream->Name,&progstream->Access,O_WriteOnly);
	if( s != NULL )
	{
		SendSignal(s,SIGINT);
		Close(s);
	}
}

void error(char *f, ... )
{
	va_list a;
	
	va_start(a,f);
	
	vfprintf(stdout,f,a);	
	
	putchar('\n');
}
