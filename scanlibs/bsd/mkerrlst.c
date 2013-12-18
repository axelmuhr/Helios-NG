/* $Id: mkerrlst.c,v 1.1 1990/09/05 13:38:14 nick Exp $ */

#include <stdio.h>
#include <errno.h>
#include <fault.h>

#define MSIZE	100
char msg[MSIZE];

int main(void)
{
	FDB *fdb;
	int i;
	
	fdb = fdbopen("/helios/etc/faults");
	if( fdb == NULL ) exit(10);

	printf("char * sys_errlist[] =\n{\n");
		
	for(i = 0; i <= MAX_PERROR; i++ )
	{
		msg[0] = 0;
		fdbfind(fdb,"Posix",i,msg,MSIZE);
		printf("/* %3d */\t\"%s\",\n",i,msg);
		fdbrewind(fdb);
	}
	printf("\t\t0\n};\nint sys_nerr = %d;\n",MAX_PERROR+1);
	

	printf("\nchar * sys_siglist[] =\n{\n");
		
	for(i = 0; i < 32; i++ )
	{
		msg[0] = 0;
		fdbfind(fdb,"Signal",i,msg,MSIZE);
		printf("/* %3d */\t\"%s\",\n",i,msg);
		fdbrewind(fdb);
	}

	printf("\t\t0\n};\n");
	
	fdbclose(fdb);
}
