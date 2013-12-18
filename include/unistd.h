/* unistd.h: POSIX symbolic constants				*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: unistd.h,v 1.3 1993/07/06 14:11:00 nickc Exp $ */

#ifndef __unistd_h
#define __unistd_h

#ifndef __types_h
#include <sys/types.h>
#endif

/* constants for access() */
#define R_OK	1
#define W_OK	2
#define X_OK	4
#define F_OK	8

/* constants for lseek */
#ifndef SEEK_SET
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2
#endif 

#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2

/* level of standard */

#define _POSIX_VERSION	198808L

#ifdef _POSIX_JOB_CONTROL
#undef _POSIX_JOB_CONTROL
#endif

#ifdef _POSIX_SAVED_IDS
#undef _POSIX_SAVED_IDS
#endif

/* Variable names for sysconf()				*/

#define _SC_ARG_MAX		1
#define _SC_CHILD_MAX		2
#define _SC_CLK_TCK		3
#define _SC_NGROUPS_MAX		4
#define _SC_OPEN_MAX		5
#define _SC_JOB_CONTROL		6
#define _SC_SAVED_IDS		7
#define _SC_VERSION		8

/* Variable names for pathconf()			*/

#define	_PC_LINK_MAX		1
#define	_PC_MAX_CANON		2
#define	_PC_MAX_INPUT		3
#define	_PC_NAME_MAX		4
#define	_PC_PATH_MAX		5
#define	_PC_PIPE_BUF		6
#define	_PC_CHOWN_RESTRICTED	7
#define	_PC_NO_TRUNC		8
#define	_PC_VDISABLE		9

extern int 	errno;

extern char 	**environ;

#ifndef _POSIX_SOURCE
extern char 	**_posix_init(void);
extern int	oserr;
#endif

extern int 	link(char *path1, char *path2);
extern int 	unlink(char *path);
extern int 	rmdir(char *path);
extern int 	rename(const char *old_name, const char *new_name);
extern int 	access(char *path, int amode);
extern int 	chown(char *path, uid_t owner, uid_t group);
extern int 	pathconf(char *path, int name);
extern int 	fpathconf(int fd, int name);
extern int 	pipe(int fildes[2]);
extern int 	dup(int fd);
extern int 	dup2(int fd,int fd2);
extern int 	close(int fd);
extern int 	read(int fd, char *buf, unsigned nbyte);
extern int 	write(int fd, char *buf, int nbyte);
extern off_t 	lseek(int fd, off_t offset, int whence);

extern int 	chdir(char *path);
extern char 	*getcwd(char *buf, int size);

extern int 	getpid(void);
extern int 	getppid(void);
extern uid_t 	getuid(void);
extern uid_t 	geteuid(void);
extern uid_t 	getgid(void);
extern uid_t 	getegid(void);
extern int 	setuid(uid_t uid);
extern int 	setgid(uid_t uid);
extern int 	getgroups(int setsize, uid_t *list);
extern char	*getlogin(void);
extern char	*cuserid(char *s);
extern pid_t 	getpgrp(void);
extern pid_t 	setsid(void);
extern int 	setpgid(pid_t pid, pid_t pgid);
extern time_t 	time(time_t *tloc);
extern char 	*getenv(const char *name);
extern char 	*ctermid(char *s);
extern char	*ttyname(int fd);
extern int 	isatty(int fd);
extern int 	sysconf(int name);

extern int 	vfork(void);
extern int 	execl(char *path,...);
extern int 	execv(char *path,char **argv);
extern int 	execle(char *path,...);
extern int 	execlp(char *file,...);
extern int 	execvp(char *file, char **argv);
extern int 	execve(char *name, char **argv, char **envv);
extern void 	_exit(int code);

#ifndef _POSIX_SOURCE

/* define fork() as vfork() to avoid changing sources		*/
#ifndef fork
#define fork() vfork()
#endif

#ifdef _BSD
extern int 	readv(int fd, struct iovec *iov, int iovlen);
extern int 	writev(int fd, struct iovec *iov, int iovlen);
#endif

/* The following are Helios extensions to map from Posix to Helios objects */

#ifdef __syslib_h

extern Stream 	*fdstream(int fd);
extern Object 	*cdobj(void);
extern int 	sopen(Stream *s);
extern Environ  *getenviron(void);

#endif

/* POSIX configuration flags */

#define		PE_BLOCK	1
#define		PE_UNBLOCK	2
#define		PE_SETMASK	3

#define		PE_RemExecute	1	/* All Execute requests may be sent */
					/* To the TFM if this bit is set in */
					/* pflagword			    */

extern int _posixflags(int how, int set );

#endif

#endif

/* end of unistd.h */
