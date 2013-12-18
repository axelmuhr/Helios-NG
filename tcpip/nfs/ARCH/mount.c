
#include <rpc/rpc.h>
#include "mount.h"

#include <stdio.h>

CLIENT *mountcl;

fhandle nfsroot;

extern int Mount(char *host, char *root)
{
	fhstatus *status;
	
	mountcl = clnt_create(host, MOUNTPROG, MOUNTVERS, "udp");

	if( mountcl == NULL )
	{
		clnt_pcreateerror(host);
		return 0;
	}
	mountcl->cl_auth = authunix_create_default();

	for(;;)
	{	
		status = mountproc_mnt_1((dirpath *)&root, mountcl);

		if( status->fhs_status == 0 )
		{
			memcpy(nfsroot,status->fhstatus_u.fhs_fhandle,sizeof(nfsroot));
			return 1;
		}
		else IOdebug("mount error %d",status->fhs_status);
	}
	
	return 0;
}

extern void DisMount(char *root)
{
	mountproc_umnt_1((dirpath *)&root, mountcl);
}
