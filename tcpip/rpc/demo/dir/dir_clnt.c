#include <rpc/rpc.h>
#include <sys/time.h>
#include "dir.h"

static struct timeval TIMEOUT = { 25, 0 };

readdir_res *
readdir_1(argp, clnt)
	nametype *argp;
	CLIENT *clnt;
{
	static readdir_res res;

	bzero(&res, sizeof(res));
	if (clnt_call(clnt, READDIR, xdr_nametype, argp, xdr_readdir_res, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

