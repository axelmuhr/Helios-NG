/* sys/wait.h: Posix library wait defines				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: wait.h,v 1.4 90/11/15 15:24:06 nick Exp $ */

#ifndef _wait_h
#define _wait_h

#define WNOHANG		1	/* return immediately if no children	*/
#define WUNTRACED	2	/* return status for stopped children	*/

extern pid_t	wait(int *statloc);
extern pid_t	waitpid(pid_t pid, int *stat_loc, int options);

#define WIFEXITED(s)	((((int)(s))&0xc0)==0x00)
#define WEXITSTATUS(s)	(((int)(s))>>8)
#define WIFSIGNALED(s)	((((int)(s))&0xc0)==0x80)
#define WTERMSIG(s)	(((int)(s))&0x3f)
#define WIFSTOPPED(s)	((((int)(s))&0xc0)==0x40)
#define WSTOPSIG(s)	(((int)(s))&0x3f)

#ifndef _POSIX_SOURCE
#ifdef _BSD

#define WCOREDUMP(s)	(0)	/* No core dumps in Helios	*/

extern int wait2(int *stat_loc, int options);
extern int wait3(int *stat_loc, int options, struct rusage *rusage);

#ifndef fork
#define fork() vfork()
#endif

union wait
{
	int	w_status;
	struct
	{
		unsigned int	w_Termsig:6;
		unsigned int	w_Status:2;
		unsigned int	w_Retcode:8;
	} w_s;
};
#define w_termsig	w_s.w_Termsig
#define w_retcode	w_s.w_Retcode
#endif
#endif

#endif
