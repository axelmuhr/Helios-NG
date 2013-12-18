#include <rpc/rpc.h>
#include <sys/time.h>
#include "nfs_prot.h"
#include <string.h>

#ifdef hpux

#ifndef NULL
#define NULL  0
#endif  /* NULL */

#endif /* hpux */ 

static struct timeval TIMEOUT = { 25, 0 };

void *
nfsproc_null_2(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_NULL, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


attrstat *
nfsproc_getattr_2(argp, clnt)
	nfs_fh *argp;
	CLIENT *clnt;
{
	static attrstat res;


	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_GETATTR, xdr_nfs_fh, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


attrstat *
nfsproc_setattr_2(argp, clnt)
	sattrargs *argp;
	CLIENT *clnt;
{
	static attrstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_SETATTR, xdr_sattrargs, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


void *
nfsproc_root_2(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_ROOT, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


diropres *
nfsproc_lookup_2(argp, clnt)
	diropargs *argp;
	CLIENT *clnt;
{
	static diropres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_LOOKUP, xdr_diropargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readlinkres *
nfsproc_readlink_2(argp, clnt)
	nfs_fh *argp;
	CLIENT *clnt;
{
	static readlinkres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_READLINK, xdr_nfs_fh, argp, xdr_readlinkres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readres *
nfsproc_read_2(argp, clnt)
	readargs *argp;
	CLIENT *clnt;
{
	static readres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_READ, xdr_readargs, argp, xdr_readres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


void *
nfsproc_writecache_2(argp, clnt)
	void *argp;
	CLIENT *clnt;
{
	static char res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_WRITECACHE, xdr_void, argp, xdr_void, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&res);
}


attrstat *
nfsproc_write_2(argp, clnt)
	writeargs *argp;
	CLIENT *clnt;
{
	static attrstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_WRITE, xdr_writeargs, argp, xdr_attrstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


diropres *
nfsproc_create_2(argp, clnt)
	createargs *argp;
	CLIENT *clnt;
{
	static diropres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_CREATE, xdr_createargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_remove_2(argp, clnt)
	diropargs *argp;
	CLIENT *clnt;
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_REMOVE, xdr_diropargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_rename_2(argp, clnt)
	renameargs *argp;
	CLIENT *clnt;
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_RENAME, xdr_renameargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_link_2(argp, clnt)
	linkargs *argp;
	CLIENT *clnt;
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_LINK, xdr_linkargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_symlink_2(argp, clnt)
	symlinkargs *argp;
	CLIENT *clnt;
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_SYMLINK, xdr_symlinkargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


diropres *
nfsproc_mkdir_2(argp, clnt)
	createargs *argp;
	CLIENT *clnt;
{
	static diropres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_MKDIR, xdr_createargs, argp, xdr_diropres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


nfsstat *
nfsproc_rmdir_2(argp, clnt)
	diropargs *argp;
	CLIENT *clnt;
{
	static nfsstat res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_RMDIR, xdr_diropargs, argp, xdr_nfsstat, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


readdirres *
nfsproc_readdir_2(argp, clnt)
	readdirargs *argp;
	CLIENT *clnt;
{
	static readdirres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_READDIR, xdr_readdirargs, argp, xdr_readdirres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}


statfsres *
nfsproc_statfs_2(argp, clnt)
	nfs_fh *argp;
	CLIENT *clnt;
{
	static statfsres res;

	memset(&res, 0, sizeof(res));

	if (clnt_call(clnt, NFSPROC_STATFS, xdr_nfs_fh, argp, xdr_statfsres, &res, TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return (&res);
}

