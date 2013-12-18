/* $Id: syslog.c,v 1.3 1993/07/09 12:58:52 nickc Exp $ */
#include <sys/types.h>
#include <syslog.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <syslib.h>
#include <errno.h>
#include <unistd.h>

static int logfd = -1;
static int logopt = 0;
static int logfac = 0;
static char *logident = NULL;
static int logmask = -1;

static char logbuf[100];
static char fbuf[100];

extern int openlog(char *ident, int opt, int facility)
{
	logopt |= opt;
	logfac |= facility;
	if( ident ) 
	{
		if( logident ) Free(logident);
		logident = (char *)Malloc((long)strlen(ident)+1);
		if( logident ) strcpy(logident,ident);
	}
	
	if( logopt & LOG_NDELAY )
	{
		if( fdstream(3) != NULL ) logfd = 3;
		else logfd = open("/logger",O_WRONLY);
	}

	return 0;
}

extern int syslog(int priority, char *message, ... )
{
	va_list a;
	int n = 0;
	int pos = 0;

	if( !(logmask & LOG_MASK(priority)) ) return 0;
	
	if( logfd == -1 ) openlog(NULL,LOG_NDELAY,0);
	
	va_start(a,message);

	if( logident ) pos += sprintf(logbuf,"SYSLOG: %s ",logident);
	else pos += sprintf(logbuf,"SYSLOG: ");

	for(;;)
	{
		char ch = *message++;
		if( ch == 0 || (ch == '%' && *message == 'm')) 
		{
			fbuf[n] = 0;
			pos += vsprintf(logbuf+pos,fbuf,a);
			if( ch == 0 ) break;
			message++;
			pos += vsprintf(logbuf+pos,sys_errlist[errno],a);
			n = 0;
		}
		else if( n < 99 ) fbuf[n++] = ch;
	}
	n = strlen(logbuf);
	if( logbuf[n-1] != '\n' ) logbuf[n++] = '\r',logbuf[n++] = '\n';
	write(logfd,logbuf,n);
	return 0;
}

extern int closelog()
{
	close(logfd);
	logfd = -1;
	logopt = logfac = 0;
	Free(logident);
	logident = NULL;

	return 0;
}

extern int setlogmask(int maskpri)
{
  int	oldpri = logmask;
  
  logmask = maskpri;

  return oldpri;  
}

