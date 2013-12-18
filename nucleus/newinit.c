/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- init.c								--
--                                                                      --
--	System initialiser, started by ProcMan if config flags say so.	--
--                                                                      --
--	Author:  NHG 15/3/89						--
--									--
------------------------------------------------------------------------*/
/* SccsId: %W% %G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: newinit.c,v 1.13 1992/10/05 09:05:03 nickc Exp $ */

#include <syslib.h>
#include <string.h>
#include <servlib.h>
#include <task.h>
#ifdef __ABC
# include <abcARM/ABClib.h>
#endif
#undef Malloc

#define FDBBUFMAX 	512
#define LINEMAX		256
#define ARGVMAX		32

#define INITRC "/helios/etc/initrc"

#ifdef __ABC
/* In rom based systems, the init program runs through a script that loads */
/* the basic servers. These servers can then be used to load the user */
/* defined initrc. Separating the servers from the nucleus allows them to */
/* be patched in Patch FlashEPROM systems. The /helios server will configure */
/* itself to point at the relevant fs server that is defined in the system */
/* microcontroller EEPROM */
# define PREINITRC "/rom/sys/helios/etc/preinitrc"
# define CANNEDINITRC "/rom/sys/helios/etc/initrc.shell" /* special default helios shell startup */
#endif

typedef struct FDB {
	Stream	*stream;
	int	pos;
	int	upb;
	char	buf[FDBBUFMAX];
} FDB;

int run(Object *w, string program, string *argv, bool wait);

static FDB *fdbopen(string name);
static void fdbclose(FDB *fdb);
static void rdline(FDB *fdb, char *buf);
static int rdch(FDB *fdb);
static void parse(FDB *fdb);
static void splitline(char *p, char **argv);

static char **dorun(char **argv, word dummy);
static char **autoserver(char **argv, word dummy);
static char **waitfor(char **argv, word dummy);
static char **ifabsent(char **argv, word dummy);
static char **setconsole(char **argv, word dummy);

struct keyentry { char *key; char **(*fn)(); word arg1; } keytab[] =
{
	{ "run",	dorun,		0	},
	{ "ifabsent",	ifabsent,	0	},
	{ "ifpresent",	ifabsent,	1	},	
	{ "waitfor",	waitfor,	0	},
	{ "console",	setconsole,	0	},
	{ "auto",	autoserver,	0	},
	{ 0, 		0, 		0 	}
};

Object *Console = NULL;
Object *CServer = (Object *)MinInt;
Object *Logger = NULL;
Object *Root = NULL;

void _stack_error()
{
	IOdebug("stack error in init");
}

#ifdef __ARM
#pragma no_check_stack
#else
#pragma -s0
#endif

#ifdef __ABC
int main(void)
{
	FDB *fdb = fdbopen(PREINITRC);
	
	Root = Locate(NULL,"/");

	if( fdb == NULL ) {
		IOdebug("init: warning cannot locate %s",PREINITRC);
	}
	else
	{
		parse(fdb);
		fdbclose(fdb);
	}

	/* the /helios server should now point at the required fs server */
	if (ReadEEPROM(EEPROM_ServerID) == EEPROM_ServerIndexShell || (ResetKeyState() & (1 << ShellBootKey)))
	{
		/* Special escape if case 'boot' key was held down during */
		/* reset, boot to Helios shell */
		fdb = fdbopen(CANNEDINITRC);

		if( fdb == NULL ) { IOdebug("init: cannot locate %s",CANNEDINITRC); Exit(0); }
	}
	else
	{
		fdb = fdbopen(INITRC);

		if( fdb == NULL ) { IOdebug("Init: cannot locate %s",INITRC); Exit(0); }
	}

	Logger = Console = Locate(NULL,"/logger");

	parse(fdb);
	fdbclose(fdb);

	Exit(0);
}
#else
int main(void)
{
	FDB *fdb = fdbopen(INITRC);
	
	if( fdb == NULL ) { IOdebug("init: cannot locate %s",INITRC); Exit(0); }

	Root = Locate(NULL,"/");

	Logger = Console = Locate(NULL,"/logger");
	
	parse(fdb);
	
	fdbclose(fdb);
	
	Exit(0);
}
#endif

static void parse(FDB *fdb)
{
	char line[LINEMAX];
	char *argvec[ARGVMAX];
	
	forever
	{
		char **argv = argvec;
		struct keyentry *k;

		rdline(fdb,line);
		
		if( line[0] == '#' || line[0] == '\n' ) continue;
		
		if( line[0] == 0x1a || line[0] == 0 ) break;
		
		splitline(line,argv);

		if( argv[0] == NULL ) continue;
		
		while( argv != NULL )
		{
			for( k = keytab; k->key != 0; k++ )
			{

				if( strcmp(k->key,argv[0]) == 0 )
				{
					char **(*f)() = k->fn;
					argv = f(argv,k->arg1);
					break;
				}
			}
		}
	}
}

static char **dorun(char **argv, word dummy)
{
	bool wait = FALSE;
	bool res;
	bool sendenv = FALSE;

	argv++;
	
	while( **argv == '-' )
	{
		char *arg = *argv++;
		arg++;
		switch( *arg++ )
		{
		case 'w': wait = TRUE; 	  break;
		case 'e': sendenv = TRUE; break;
		}
	}

	/* console must have been set if we are to send environment */
	if( sendenv && Console == NULL )
	{
		IOdebug("init: Console not set");
		return NULL;
	}

	/* now run either as a server (no env) or as a normal prog */
	if( !sendenv ) res = run(NULL,*argv,NULL,wait);
	else res = run(Console,*argv,argv+1,wait);

	if( !res ) IOdebug("init: Failed to run %s",argv[0]);

	return NULL;
}

static char **ifabsent(char **argv, bool invert)
{
	Object *o = Locate(NULL,argv[1]);
	
	if( o != NULL )
	{
		Close(o);
		return invert?argv+2:NULL;
	}

	return invert?NULL:argv+2;
}

static char **autoserver(char **argv, word dummy)
{
	NameInfo info;

	info.Port = NullPort;
	info.Flags = Flags_StripName;
	info.Matrix = DefDirMatrix;
	info.LoadData = NULL;

	Create(Root,argv[1]+1,Type_Name,sizeof(NameInfo),(byte *)&info);	
	
	return NULL;
}

static char **waitfor(char **argv, word dummy)
{
	Object *o = NULL;
	
	do
	{
		o = Locate(NULL,argv[1]);
		if( o == NULL ) Delay(OneSec);
	} while( o == NULL );

	dummy = dummy;
	
	Close( o );
	
	return NULL;
}

static char **setconsole(char **argv, word dummy)
{
	Object *server = Locate(NULL,argv[1]);
	
	if( server == NULL )
	{
		IOdebug("init: cannot locate %s",argv[1]);
		return 0;
	}

		/* Cope with console /logger */
	if (argv[2] == NULL)
	 { if (CServer != Null(Object)) Close(CServer);
	   CServer = (Object *) MinInt;
	   if (Console != Null(Object) && Console != Logger) Close(Console);
	   Console = server;
	   return(NULL);
	 }
	 
	CServer = server;

	if( Console != NULL && Console != Logger ) Close(Console);
		
	Console = Create(server,argv[2],Type_File,0,0);
	
	if( Console == NULL ) IOdebug("init: Failed to create console %s",argv[2]);
	
	dummy = dummy;
	
	return NULL;
}

static FDB *fdbopen(string name)
{
	Object *o = NULL;
	Stream *s = NULL;
	FDB *fdb = New(FDB);

	if( fdb == NULL ) return NULL;

	fdb->pos = 0;	
	fdb->upb = 0;	
	
	o = Locate(NULL,name);
	
	if( o == NULL ) goto fail;
	
	s = Open(o,NULL,O_ReadOnly);

	if( s == NULL ) goto fail;
	
	fdb->stream = s;
	Close(o);
	
	return fdb;
fail:
	if( o != NULL ) Close(o);
	if( s != NULL ) Close(s);
	if( fdb != NULL ) Free(fdb);
	return NULL;
}

static void fdbclose(FDB *fdb)
{
	Close(fdb->stream);
	Free(fdb);
}

static void splitline(char *p, char **argv)
{
	int i = 0;
	
	forever
	{
		while( *p == ' ' || *p == '\t' ) p++;
		if( *p == '\n' || *p == 0 ) break;
		argv[i++] = p;
		until( *p == ' ' || *p == '\t' || *p == '\n' || *p == 0) p++;
		if( *p == 0 ) break;
		*p++ = 0;
	}
	
	argv[i] = NULL;
}

static void rdline(FDB *fdb, char *buf)
{
	forever
	{
		int c = rdch(fdb);
		if( c == -1 )
		{
			break;
		}
		if( c == '\r' ) continue;
		*buf++ = c;
		if( c == '\n' ) break;	
	}
	*buf = 0;
}

static int rdch(FDB *fdb)
{
	if( fdb->upb == -1 ) return -1;
	if( fdb->pos == fdb->upb )
	{
		int size = (int)Read(fdb->stream,fdb->buf,FDBBUFMAX,-1);
		fdb->upb = size;
		if( size == -1 ) 
		{
			return -1;
		}
		fdb->pos = 0;
	}
	return fdb->buf[fdb->pos++];
}


int run(Object *w, string program, string *argv, bool wait)
{	
	Object *code, *source;
	Object *prog, *objv[OV_End+1];
	Stream *s = NULL, *strv[5];
        char   *envv[3];
	word e;
	Environ env;
	
	if ((source = Locate(NULL,program)) == NULL) {
		IOdebug("init: Cannot locate %s",program);
		return false;
	}

	if ((code = Load(NULL,source)) == NULL) {
		IOdebug("init: Cannot load %s", program);
		return false;
	}

	if ((prog = Execute(NULL,code)) == NULL) {
		IOdebug("init: Cannot Execute %s", program);
		return false;
	}

	if( w != NULL || argv != NULL )
	{
		s = Open(prog,NULL,O_ReadWrite);

		if( s == Null(Stream) ) {
			IOdebug("init: Prog Stream not opened");
			return false;
		}

		objv[0] = Locate(NULL,"/helios");
		objv[1] = prog;
		objv[2] = code;
		objv[3] = source;
		objv[4] = (Object *)MinInt;
		objv[5] = objv[0];
		objv[6] = w;
		objv[7] = CServer;
		objv[8] = (Object *)MinInt;
		objv[9] = (Object *)MinInt;
		objv[10] = (Object *)MinInt;
		objv[11] = Null(Object);

		if( w != NULL )
		{
			strv[0] = Open(w,NULL,O_ReadOnly);
			strv[1] = Open(w,NULL,O_WriteOnly);
			strv[2] = strv[1];
			strv[3] = Open(Logger,NULL,O_WriteOnly);
			strv[4] = Null(Stream);
		}
		else strv[0] = NULL;

		envv[0]	= "_UID=00000000";
		envv[1] = "_GID=00000000";
		envv[2] = NULL;

		env.Argv = argv;
		env.Envv = &envv[0]; 
		env.Objv = &objv[0];
		env.Strv = &strv[0];
	
		e = SendEnv(s->Server,&env);
	}

	if( wait )
	{	if (s == NULL)
			s = Open(prog, NULL, O_ReadWrite);
	   	if (s == NULL) {
			IOdebug("init: Prog Stream not opened");
			return false;
		}

		if (InitProgramInfo(s, PS_Terminate) >= Err_Null)
		 (void) GetProgramInfo(s, NULL, -1);
	}

	Close(source);
	Close(code);
	Close(prog);
	if (s != NULL) Close(s);
	Close(objv[0]);
	Close(strv[0]);
	Close(strv[1]);
	Close(strv[3]);

	return true;
}
