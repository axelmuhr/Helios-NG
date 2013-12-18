#ifdef __HELIOS

static char *rcsid = "$Header: /hsrc/tcpip/cmds/sendmail/RCS/extras.c,v 1.2 1992/02/26 20:58:42 craig Exp $";

#include <nonansi.h> 
#include "sendmail.h"
#include <stdarg.h>

/* debugf */

void debugf (char *format, ...)
{
#ifdef DEBUG
	char buf [256] ;
	va_list args;
	va_start (args, format);
	vsprintf (buf, format, args);
	IOdebug ("%s", buf) ;
	va_end (args);
#endif	
}

/* Init_Env */

void Init_Env (ENVELOPE *e)
{
	debugf ("Initialize Envelope") ;
	e->e_to   [0] = (char) NULL ;
	e->e_from [0] = (char) NULL ;
	e->e_df   [0] = (char) NULL ;
	e->e_to   [0] = (char) NULL ;
	e->e_id   [0] = (char) NULL ;
	e->e_dfp      = (FILE *) NULL ;
	e->e_xfp      = (FILE *) NULL ;
	create_xf (e) ;
}

/* Init_Mailer */

void Init_Mailer (MAILER *m)
{
	debugf ("Initialize Mailer") ;
	m->m_name[0] = (char) NULL ;
	m->m_mailer[0] = (char) NULL ;
	strcpy (m->m_eol, M_EOL) ;
}

/*
-- crf: OK, this is the simplest, most primitive address parser you'll
-- ever have the misfortune to encounter. All I'm trying to do here is
-- ascertain whether or not the recipient is local. I'm catering *only*
-- for the following address formats :
-- user
-- user@host
-- user@host.net
-- user<@host.net>
-- host!user
-- Once I've identified the host part, I simply compare it with the local
-- machine name (if the name is just <user>, it is assumed to be local).
-- Notice that at this point I don't really care whether or not <user>
-- is a valid local user name ... so I'm not using getpwnam() or anything
-- like that.
*/

#define PLING	'!'
#define AT	'@'
#define DOT	'.'

char user_name [MAXNAME] ;
char host [MAXNAME] ;

bool local_name (char *e_addr)
{
	char buf [MAXNAME] ;
	bool local = FALSE ;
	char *delim_ptr ;
	int strip_angle_brackets (char *) ;
	int break_up_addr (char *, char, char *) ;
		
	user_name[0] = '\0' ;
	host[0]      = '\0' ;

	strcpy (buf, e_addr) ;
	makelower (buf) ;
	if (strip_angle_brackets (buf) < 0)
		syslog (LOG_WARNING, "Bad address format") ;

	if (((delim_ptr = strchr (buf, AT))    != NULL) ||
 	    ((delim_ptr = strchr (buf, PLING)) != NULL))
		break_up_addr (buf, *delim_ptr, delim_ptr) ;
	else
		strcpy (user_name, buf) ;
	debugf ("user_name = %s", user_name) ;
	debugf ("host = %s", host) ;

	if ((!*host) || (!strcmp (host, MyHostName)))
	{
		local = TRUE ;
		strcpy (e_addr, user_name) ;
	}	
	return local ;
}

int strip_angle_brackets (char *buf)
{
	char *buf_ptr = buf ;
	static char res [MAXNAME] ;
	char *res_ptr = res ;
	int angle_cnt = 0 ;

	memset (res, (char) NULL, sizeof (res)) ;
	while (*buf)
	{
		if (*buf == '<')
			angle_cnt ++ ;
		elif (*buf == '>')
			angle_cnt -- ;
		else
			*res_ptr ++ = *buf ;
		buf ++ ;			
	}
	if (strcmp (buf_ptr, res))
		strcpy (buf_ptr, res) ;
	return (angle_cnt == 0 ? 0 : -1) ;
}

int break_up_addr (char *name, char delim, char *delim_ptr)
{
	int result = 1 ;
	*delim_ptr ++ = NULL ;
	if (delim == PLING)
	{
		strcpy (host, name) ;
		strcpy (user_name, delim_ptr) ;		
	}
	elif (delim == AT)
	{
		char *dot_ptr ;
		if ((dot_ptr = strchr (delim_ptr, DOT)) != NULL)
		{
			*dot_ptr = '\0' ;
		}
		strcpy (user_name, name) ;
		strcpy (host, delim_ptr) ;
	}
	else
		result = -1 ;
	return result ;
}

/* create_x_file */

#include <sys/types.h>
#include <sys/stat.h>

void create_xf (ENVELOPE *e)
{
	char xf_name [MAXNAME] ;
	strcpy (xf_name, queuename (e, 'x')) ;
	debugf ("xf_name = %s", xf_name) ;
	if ((e->e_xfp = fopen (xf_name, "w")) == NULL)
	{
		syslog (LOG_WARNING, "Cannot create transcript file: %s: %m", 
			xf_name);
	}
	else
	{
		(void) chmod(xf_name, FileMode);
	}
}

/*
-- crf: moved intsig() from main.c
*/

/* new page */
/*
**  INTSIG -- clean up on interrupt
**
**	This just arranges to exit.  It pessimises in that it
**	may resend a message.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Unlocks the current job.
*/

#ifndef __HELIOS
static void
#else
void
#endif
intsig()
{
#ifndef __HELIOS
	FileName = NULL;
	unlockqueue(CurEnv);
#else
	unlink_temps (CurEnv) ;
#endif	
	syslog (LOG_INFO, "terminated") ;
	exit(EX_OK);
}

/*
-- crf: moved finis() from main.c
*/

/* new page */
/*
**  FINIS -- Clean up and exit.
**
**	Parameters:
**		none
**
**	Returns:
**		never
**
**	Side Effects:
**		exits sendmail
*/

void
finis()
{
#ifndef __HELIOS
	if (tTd(2, 1))
		printf("\n====finis: stat %d e_flags %o\n", ExitStat, CurEnv->e_flags);

	/* clean up temp files */
	CurEnv->e_to = NULL;
	dropenvelope(CurEnv);

	/* post statistics */
	poststats(StatFile);
#else
/*
-- crf: XXX
-- This is not quite right. Should only be unlinking temporary files when we
-- are *sure* there have been no errors.
*/
	unlink_temps (CurEnv) ;
#endif

	/* and exit */
# ifdef LOG
	if (LogLevel > 11)
		syslog(LOG_DEBUG, "finis, pid=%d", getpid());
# endif /* LOG */
	if (ExitStat == EX_TEMPFAIL)
		ExitStat = EX_OK;

#ifdef MEM_CHECK
	IOdebug ("exiting ... : Bytes free : %d  Heap size : %d", 
		Malloc(-1), Malloc(-3));
#endif

	exit(ExitStat);
}

/*
 * itoa - integer to string conversion
 */

#define STR_LEN	9

char *itoa(register int i)
{
	static char b[STR_LEN] = "########" ;
	register char *p;

	p = &b[STR_LEN - 1];
	do
		*p-- = i%10 + '0';
	while (i /= 10);
	return(++p);
}

/* valid_name */

#include <pwd.h>

bool valid_name (char *user_name)
{
	char temp_name [MAXNAME] ;
	bool is_valid = TRUE ;
        strcpy (temp_name, user_name) ;
	if (local_name (temp_name))
	{
		if (getpwnam (temp_name) == (struct passwd *) NULL)
			is_valid = FALSE ;
	}
	return is_valid ;
}

#endif
