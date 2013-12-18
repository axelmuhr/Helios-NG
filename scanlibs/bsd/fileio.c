/* $Id: fileio.c,v 1.7 1993/07/09 12:58:37 nickc Exp $ */
#include <unistd.h>
#include <stdio.h>
#include <syslib.h>
#include <fcntl.h>
#include <nonansi.h>
#include <errno.h>
#include <signal.h>
#include <strings.h>
#include <syslib.h>
#include <attrib.h>

extern void setlinebuf(FILE *f)
{
	setvbuf(f,NULL,_IOLBF,BUFSIZ);
}

extern int ftruncate(int fd, unsigned long length)
{
	Stream *s = fdstream(fd);
	if( s == NULL ) return -1;
	SetFileSize(s,length);
	return 0;
}

extern int truncate(char *path, unsigned long length)
{
	int e;
	int fd = open(path,O_WRONLY);
	if( fd < 0 ) return -1;
	e = ftruncate(fd,length);
	close(fd);
	return e;
}

extern int putw(int w, FILE *stream)
{
	(void)fwrite(&w,sizeof(int),1,stream);
	return w;
}

extern int getw(FILE *stream)
{
	int w;
	size_t size;
	size = fread(&w,sizeof(w),1,stream);
	if( size != 1 ) return EOF;
	return w;
}

extern char *getpass(char *prompt)
{
	static char pwd[9];
	int i;
	int c;
	Attributes oattr, nattr;
	Stream *s = fdstream(0);
	
	GetAttributes(s,&oattr);
	nattr = oattr;
	RemoveAttribute(&nattr,ConsoleEcho);
	SetAttributes(s,&nattr);
	while( *prompt ) putchar(*prompt++);
	fflush(stdout);
	for(i=0;;)
	{
		c = getc(stdin);
		if( c == '\n' || c == EOF ) break;
		if( i < 8 ) pwd[i++] = c;
	} 
	putchar('\n');
	pwd[i] = 0;
	SetAttributes(s,&oattr);	
	return pwd;
}

extern void bsd_perror(const char *s)
{
  if (s == NULL)
    s = "";
  
  if (errno < 0 || errno >= sys_nerr) 
    fprintf( stderr,"%s: posix error %d\n", s, errno );
  else
    fprintf( stderr,"%s: %s\n", s, sys_errlist[ errno ] );
}

extern void psignal(int sig, const char *s)
{
	if( sig < 0 || sig >= NSIG ) 
		fprintf(stderr,"%s:signal %d",s,sig);
	else    fprintf(stderr,"%s:%s",s,sys_siglist[sig]);
}

static int unique = -1;

char *mktemp(char *Template)
{
	char *p = Template+strlen(Template)-1;
	char *p1 = p+1;
	char *fmtt = "%s%0Xd";
	char fmt[8];
	char ustr[2];
	int len;
	word nonce = GetDate()^_ldtimer(0)^(int)p;
	
	if( nonce < 0 ) nonce = -nonce;
	
	strcpy(fmt,fmtt);
	ustr[0] = 0;
	
	while( (p != Template) && (*(p-1) == 'X') ) p--;
	
	len = p1-p;
	
	switch( len )
	{
	case 0:	return "";
	case 1: fmt[4] = '1'; nonce %= 9; break;
	case 2: fmt[4] = '2'; nonce %= 99; break;
	case 3: fmt[4] = '3'; nonce %= 999; break;
	case 4: fmt[4] = '4'; nonce %= 9999; break;
	case 5: fmt[4] = '5'; nonce %= 99999; break;
	default:
		unique++;
		if( unique >= 26 ) unique = 0;
		ustr[0] = 'A' + unique;
		ustr[1] = 0;
		fmt[4] = '5'; nonce %= 99999; 
		p = p1-6;
		break;
	}
	sprintf(p,fmt,ustr,nonce);
	return Template;
}
