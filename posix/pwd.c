/* $Id: pwd.c,v 1.6 1993/07/12 10:24:51 nickc Exp $ */

#include "pposix.h"
#include <pwd.h>

#define PWFILE "/helios/etc/passwd"

struct DBINFO
{
	struct passwd p;	
};

extern struct DBINFO *dbinfo;
static struct DBINFO *mydbinfo;


extern struct passwd *getpwuid(uid_t uid)
{
	struct passwd *p;	
	struct DBINFO *tdbi;
	
	/* We must test here for a Session Manager and obtain this info */
	/* from there if it exists.					*/
	tdbi = mydbinfo, mydbinfo = dbinfo, dbinfo = tdbi;
	errno = opendb(PWFILE,0);
	if( errno ) return NULL;
	p = &dbinfo->p;
	p->pw_uid = uid;
	errno = scandb("%s:%s:!d:%d:%s:%s:%s",&p->pw_name,&p->pw_passwd,
		uid,&p->pw_gid,&p->pw_gecos,&p->pw_dir,&p->pw_shell);
	if( errno ) return NULL;
	closedb(1);
	tdbi = mydbinfo, mydbinfo = dbinfo, dbinfo = tdbi;
	return p;
}

extern struct passwd *getpwnam(char *name)
{
	struct passwd *p;
	struct DBINFO *tdbi;
	
	/* We must test here for a Session Manager and obtain this info */
	/* from there if it exists.					*/
	tdbi = mydbinfo, mydbinfo = dbinfo, dbinfo = tdbi;
	errno = opendb(PWFILE,0);
	if( errno ) return NULL;
	p = &dbinfo->p;
	p->pw_name = name;
	errno = scandb("!s:%s:%d:%d:%s:%s:%s",&p->pw_name,&p->pw_passwd,
		&p->pw_uid,&p->pw_gid,&p->pw_gecos,&p->pw_dir,&p->pw_shell);
	if( errno ) return NULL;
	closedb(1);
	tdbi = mydbinfo, mydbinfo = dbinfo, dbinfo = tdbi;
	return p;
}

extern struct passwd *getpwent(void) { return NULL; }
extern int endpwent(void) {return -1; }
extern void setpwent(void) { return; }

