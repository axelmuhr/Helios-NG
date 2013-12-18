
#include <rpc/rpc.h>
#include "mount.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
/*
-- crf: 17/03/93
-- If mount fails, retry up to MAX_RETRIES times, every DELAY seconds
*/
#define MAX_RETRIES	20
#define DELAY		5	/* seconds */

fhandle nfsroot;

extern int Mount(char *host, char *root)
{
	CLIENT *mountcl;
	fhstatus *status;

	
	mountcl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");

	if( mountcl == NULL )
	{
	  clnt_pcreateerror(host);
	  return 1;
	}

	mountcl->cl_auth = authunix_create_default();

	for(;;)
	{	
		status = mountproc_mnt_1((dirpath *)&root, mountcl);

		if( status->fhs_status == 0 )
		{
			memcpy(nfsroot,status->fhstatus_u.fhs_fhandle,sizeof(nfsroot));
			break;
		}
		else
		{
			static int count = 0 ;
			if (!count)
				printf("NFS: Mount %s:%s failed, still trying\n",host,root);
			if (count++ == MAX_RETRIES)
				break ;
			else
				sleep (DELAY) ;
		}
	}

	CLNT_DESTROY(mountcl);
	
	return status->fhs_status;
}

extern void DisMount(char *host, char *root)
{
	CLIENT *mountcl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");

	mountproc_umnt_1((dirpath *)&root, mountcl);

	CLNT_DESTROY(mountcl);
}
