/* pwd.h : Posix password database header			*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: pwd.h,v 1.1 90/09/05 11:07:02 nick Exp $ */

#ifndef _pwd_h
#define _pwd_h

#ifndef _types_h
#include <sys/types.h>
#endif

struct passwd {
	char		*pw_name;
	char		*pw_passwd;	/* deprecated */
	uid_t		pw_uid;
	uid_t		pw_gid;
	char		*pw_gecos;	/* BSD's name */
	char		*pw_dir;
	char		*pw_shell;
};

extern struct passwd *getpwuid(uid_t uid);
extern struct passwd *getpwnam(char *name);

#ifndef _POSIX_SOURCE
/* POSIX 1003.1-1988 has removed getpwent,setpwent, and endpwent,  */
/* we have done the same. These routines now return errors. Do not */
/* use them.							   */
extern struct passwd *getpwent(void);
extern void setpwent(void);
extern int endpwent(void);
#endif

#endif

/* end of pwd.h */
