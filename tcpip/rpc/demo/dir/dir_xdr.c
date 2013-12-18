#include <rpc/rpc.h>
#include "dir.h"


bool_t
xdr_nametype(xdrs, objp)
	XDR *xdrs;
	nametype *objp;
{
	if (!xdr_string(xdrs, objp, MAXNAMELEN)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_namelist(xdrs, objp)
	XDR *xdrs;
	namelist *objp;
{
	if (!xdr_pointer(xdrs, (char **)objp, sizeof(struct namenode), xdr_namenode)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_namenode(xdrs, objp)
	XDR *xdrs;
	namenode *objp;
{
	if (!xdr_nametype(xdrs, &objp->name)) {
		return (FALSE);
	}
	if (!xdr_namelist(xdrs, &objp->next)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_readdir_res(xdrs, objp)
	XDR *xdrs;
	readdir_res *objp;
{
	if (!xdr_int(xdrs, &objp->errno)) {
		return (FALSE);
	}
	switch (objp->errno) {
	case 0:
		if (!xdr_namelist(xdrs, &objp->readdir_res_u.list)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}


