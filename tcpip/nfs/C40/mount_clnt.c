#include <rpc/rpc.h>
#include <sys/time.h>
#include "mount.h"

#ifdef hpux

#ifndef NULL
#define NULL  0
#endif  /* NULL */

#endif /* hpux */

static struct timeval TIMEOUT = { 25, 0 };

void *
mountproc_null_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_NULL, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


fhstatus *
mountproc_mnt_1(argp, clnt)
	dirpath *argp;
	CLIENT *clnt;
{
	static fhstatus res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_MNT, xdr_dirpath, argp, xdr_fhstatus, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


mountlist *
mountproc_dump_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static mountlist res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_DUMP, xdr_void, argp, xdr_mountlist, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


void *
mountproc_umnt_1(argp, clnt)
	dirpath *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_UMNT, xdr_dirpath, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


void *
mountproc_umntall_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_UMNTALL, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


exports *
mountproc_export_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static exports res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_EXPORT, xdr_void, argp, xdr_exports, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


exports *
mountproc_exportall_1(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static exports res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, MOUNTPROC_EXPORTALL, xdr_void, argp, xdr_exports, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

