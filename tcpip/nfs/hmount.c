
#include <rpc/rpc.h>
#include "mount.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	char *host;
	CLIENT *mountcl;

	if( argc != 2 ) 
	{
		printf("usage: hmount <host>\n");
		exit(1);
	}
	
	host = argv[1];	

	mountcl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");
	if( mountcl == NULL )
	{
		clnt_pcreateerror(host);
		return 0;
	}
	mountcl->cl_auth = authunix_create_default();
	
	mountproc_umntall_1( NULL, mountcl);	
}
