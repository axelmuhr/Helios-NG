/*
static char *rcsid = "$Header: /hsrc/cmds/private/RCS/dbt.c,v 1.3 1992/06/17 13:05:48 nickc Exp $";
*/
#include <stdio.h>
#include <syslib.h>
#include <codes.h>
#include <task.h>
#include <string.h>
#include <stdlib.h>

struct { char *key; int code; } keytab[] =
{
	"ioc", 		Task_Flags_ioc,
	"stream", 	Task_Flags_stream,
	"memory", 	Task_Flags_memory,
	"error", 	Task_Flags_error,
	"process", 	Task_Flags_process,
	"pipe", 	Task_Flags_pipe,
	"info", 	Task_Flags_info,
	"meminfo", 	Task_Flags_meminfo,
	"fixmem", 	Task_Flags_fixmem
};

int main(int argc, char **argv)
{
	Object *pm;
	MCB m;
	word control[10];
	byte data[100];
	char pname[100];
	char *task;
	int mask = 0;

	if( argc < 2 )
	{
		int i;
		printf("usage: dbt task [number|keyword] ...\n");
		printf("keywords:");
		for( i = 0; keytab[i].key != NULL ; i++ )
		{
/*			printf("%10s %08x\n",keytab[i].key,keytab[i].code);*/
			printf(" %s",keytab[i].key);
		}
		putchar('\n');
		return 0;
	}
	
	if( *argv[1] != '/' ) 
	{
		strcpy(pname,"/tasks");
		task = argv[1];
	}
	else
	{
		task = argv[1]+strlen(argv[1]);
		while(*task != '/') task--;
		*(task++) = '\0';
		strcpy(pname,argv[1]);
	}

	for( argv+=2; *argv != NULL ; argv++ )
	{
		char *arg = *argv;
		int i;
		
		if( '0' <= *arg && *arg <= '9' ) mask |= atoi(arg);
		else 
		{
		  for ( i = sizeof (keytab) / sizeof (keytab[ 0 ]); i--; )
		    if ( strcmp( keytab[ i ].key,arg) == 0)
		      {
			mask |= keytab[ i ].code;
			break;
		      }

		  if (i < 0)
		    {
		      fprintf( stderr, "unknown debugging option %s - ignored\n", arg );		      
		    }		  
		}
	}

	if (mask)
	  {
	    pm = Locate(CurrentDir,pname);

	    if( pm == NULL )
	      {
		printf("Cannot locate %s",pname);
		exit(1);
	      }

	    InitMCB(&m,MsgHdr_Flags_preserve,NullPort,NullPort,FC_GSP|FG_SetFlags);
	    m.Control = control;
	    m.Data = data;

	    MarshalCommon(&m,pm,task);
	    MarshalWord(&m,mask);

	    SendIOC(&m);
	  }	
	
	return 0;
}
