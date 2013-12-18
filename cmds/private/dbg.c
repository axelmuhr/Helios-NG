#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/dbg.c,v 1.3 1994/03/08 12:11:56 nickc Exp $";
#endif

#include <syslib.h>
#include <codes.h>
#include <task.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct { char *key; int code; } keytab[] =
{
	"ioc1",		1,
	"ioc2",		2,
	"ioc3",		4,
	"search",	8,
	"link",		16,
	"swork",	32,
	"null",		64,
	"tasks",	128,
	"info",		0x01000000,
		
	"ioc", 		Task_Flags_ioc<<8,
	"stream", 	Task_Flags_stream<<8,
	"memory", 	Task_Flags_memory<<8,
	"error", 	Task_Flags_error<<8,
	"process", 	Task_Flags_process<<8,
	"pipe", 	Task_Flags_pipe<<8,
	"taskinfo", 	Task_Flags_info<<8,
	"meminfo", 	Task_Flags_meminfo<<8,
	/* cannot do fixmem as 'Task_Flags_fixmem << 8' exceeds 32 bits! */
	NULL, 		0
};

int main(int argc, char **argv)
{
	Object *pm;
	MCB m;
	word control[10];
	byte data[100];
	char pname[100];
	int mask = 0;

	if( argc < 2 )
	{
		int i;
		printf("usage: dbg processor|. [number|keyword] ...\n");
		printf("keywords:");
		for( i = 0; keytab[i].key != NULL ; i++ )
		{
/*			printf("%10s %08x\n",keytab[i].key,keytab[i].code);*/
			printf(" %s",keytab[i].key);
		}
		putchar('\n');
		return 0;
	}
	
	if( strcmp(argv[1],".") == 0 ) 
	{
		strcpy(pname,"/tasks");
	}
	else
	{
		strcpy(pname,"/");
		strcat(pname,argv[1]);
		strcat(pname,"/tasks");
	}


	for( argv+=2; *argv != NULL ; argv++ )
	{
		char *arg = *argv;
		int i;
		
		if( '0' <= *arg && *arg <= '9' ) mask |= atoi(arg);
		else for( i = 0; keytab[i].key != NULL; i++ )
		{
			if( strcmp(keytab[i].key,arg) == 0) 
				mask |= keytab[i].code;
		}
	}

	pm = Locate(CurrentDir,pname);

	if( pm == NULL )
	{
		printf("Cannot locate %s\n",pname);
		exit(1);
	}

	InitMCB(&m,MsgHdr_Flags_preserve,NullPort,NullPort,FC_GSP|FG_Debug);
	m.Control = control;
	m.Data = data;

	MarshalCommon(&m,pm,NULL);
	MarshalWord(&m,mask);

	SendIOC(&m);
	
	return 0;
}
