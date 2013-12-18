/* grp.h : Posix groups database header				*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: grp.h,v 1.1 90/09/05 11:06:34 nick Exp $ */

#ifndef _grp_h
#define _grp_h

#ifndef _types_h
#include <sys/types.h>
#endif

struct group
{
	char		*gr_name;
	gid_t		gr_gid;
	char		**gr_mem;
};

/* HELIOS does not support groups in the Unix sense, hence any	*/
/* attempts to access the group database will result in failure	*/

#define getgrgid(gid) (NULL)
#define getgrnam(name) (NULL)

#endif

/* end of grp.h */

