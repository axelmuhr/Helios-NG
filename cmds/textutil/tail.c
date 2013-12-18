/************************************************
*						*
*	tail command				*
*						*
* Author: C.G. Selwyn				*
*		Perihelion Software		*
*		7-dec-89			*
*						*
************************************************/

/*
	This program is (unnecessarily) complicated
	because of CR stripping on a PC filing system.
	The program will still work on a 'proper'
	filing system!

	It is also supposed to work properly on a FIFO
	
	It also conforms to the POSIX spec. (roughly)
*/
#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/textutil/RCS/tail.c,v 1.5 1994/03/14 14:55:05 nickc Exp $";
#endif

#include <stdio.h>
#include <syslib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <posix.h>
#include <nonansi.h>	/* for fileno() */

#define DEF_COUNT 10
#define LINES 1
#define CHARS 2
#define WAIT  4

#define BEGIN 1
#define END 2

#define BUF_SIZE 4096

void tail(FILE *f, int mode, int count);
void do_output( int p, int psize , int fd);
void fromend( int fd, int count, long int l, int mode);
void fromstart( int fd, int count, int mode);
long int waitforlengthchange(int fd, long int l);
void fifotail(FILE *in, int goal, int mode );
void error(char *s,...);

int count = 0;
int mode = 0;
int whichend = 0;

void usage()
{
	fprintf(stderr,"usage: tail [-c | -l] [-f] [-n number] [file]\n       tail [-|+ [number][c|l][f]] [file]\n");
	exit(0);
}

void posixargs( int *argcp, char **argvp[] )
{
	int argc = *argcp;
	char **argv = *argvp;
	char *s;

	while( argc )
	{	s = *argv;
		if( s[0] != '-' ) break;
		if(s[2] != '\0') usage();
		switch( s[1] )
		{	case 'l': mode |= LINES; break;
			case 'c': mode |= CHARS; break;
			case 'f': mode |= WAIT; break;
			case 'n':
				if( argc > 1 )
				{	argv++,argc--;
					s = *argv;
					count = (int) strtol(s,&s,10);
					if( *s != '\0' ) usage();
					unless(count)
						exit(0);
					if( count < 0 )
					{	count = -count;
						whichend = END;
					}
					else
						whichend = BEGIN;
					break;
				}
				else usage();
			default: usage();
		}
		argc--,argv++;
	}
	*argcp = argc;
	*argvp = argv;
}

int main(int argc, char *argv[])
{
	argc--, argv++;

	if( argc )
	{
		char *s = *argv;
		switch( *s )
		{
		case '-':
		{	int posixcase = 0;
			switch( s[1] )
			{ 
			case 'l': case 'c': case 'f': case 'n': 
				posixargs( &argc, &argv );
				posixcase = 1;
				break;
			}
			if( posixcase ) break;
			/* otherwise fall though for deprecated case */
			whichend = END;
		}
		case '+':
			argc--,argv++;
			if( *s != '\0' )
			{	if ((count = (int) strtol( s, &s, 10)) == 0)
					exit(0);
				if( count < 0 )
				{	count = -count;
					whichend = END;
				}
				else
					whichend = BEGIN;
			}				
			if( *s == '\0' ) break;
			mode = 0;
			switch( *s )
			{
			case 'l': mode = LINES; s++; break;
			case 'c': mode = CHARS; s++; break;
			}
			if( *s == NULL ) break;
			if( *s == 'f' ) 
			{	mode |= WAIT;
				break;
			}
			usage(); /* Doesn't return! */
		default:
			break;	/* must be file name */
		}
	}

	if( (mode & (LINES|CHARS)) == (LINES|CHARS) ) usage();
	if( (mode & (LINES|CHARS)) == 0 ) mode |= LINES;
	if( count == 0 ) count = DEF_COUNT;
	if( whichend == 0 ) whichend = END;

#if 0
	fprintf(stderr,"whichend = %d\n",whichend);
	fprintf(stderr,"mode = %d\n",mode);
	fprintf(stderr,"count = %d\n",count);
#endif
	if( argc ) 
	{
		do
		{	FILE *f = fopen(*argv,"r");
			if( f == NULL )
				error("can't open file %s",*argv);
			tail( f, mode, count );
			fclose(f);
		} while( argv++,--argc );
	}
	else
		tail(stdin, mode, count);
	exit(0);
}

int isfifo(FILE *f)
{	struct stat fs;
	fstat(fileno(f),&fs);
	if( S_ISFIFO(fs.st_mode) || (f == stdin)) return 1;
	else return 0;
}

char cbuf[BUF_SIZE];

void tail(FILE *f, int mode, int count)
{	int isf = isfifo(f);
	if( isf && whichend == END )
		fifotail(f,count,mode);
	else
	{	int fd;
		fd = fileno(f);
		while(1)
		{	long int new_l;
			long int l = 0;
			int nread;
			if( whichend == END )
			{	if( (l = lseek(fd,0,SEEK_END)) == -1 )
					error("can't find length of file");
				fromend(fd,count,l,mode);
			}
			else
				fromstart(fd, count, mode);
	
			if( mode & WAIT )
			{	while(1)
			
/* perpetually write out any additional text that is appended to the file */

				{	new_l = waitforlengthchange(fd,l);
					nread = (int) lseek(fd,(off_t)l,SEEK_SET); /* reset disc pointer after wait */
					nread = read(fd, cbuf, sizeof(cbuf));
					do_output( 0, nread , fd);
					l = new_l;
				}
			}
			else return;
		}
	}
}

long int waitforlengthchange(int fd, long int l)
{
	while(1)
	{	long int newl;
		Delay(OneSec);
		newl = lseek(fd,0,SEEK_END);
		if( newl != l ) return newl;
	}
}

void do_output( int p, int psize, int fd )
{	int nread;

	
	for( ; p < psize; p++ ) /* output buffer contents... */
	{	char ch;
		if((ch = cbuf[p])!='\r') putchar(ch);
	}

				/* if section to write exceeds buffer size
				   then refill buffer and write until done */
				   
	while( (nread = read(fd,cbuf,sizeof(cbuf))) != 0)
	{	char ch;
		for( p = 0; p<nread; p++)
			if((ch = cbuf[p])!='\r') putchar(ch);
	 }
}

void fromend( int fd, int count, long int l, int mode)
{	int n;
	int lastbufsiz;
	int i;
	
	if( mode & LINES ) count++;

	n = 0;
	while( 1 )
	{	long int pos;
		int done = 0;
		n++;
		pos = l - (long) n * sizeof(cbuf);
		if( pos < 0 ) pos = 0;
		i = (int) lseek(fd,(off_t) pos,SEEK_SET);
		lastbufsiz = read(fd,cbuf,sizeof(cbuf));
		for(i=lastbufsiz-1;i>=0;i--)
		{	int ch = cbuf[i];
			if( (ch == '\r') ) continue;
			if( (ch == '\n')||(mode&CHARS) )
			{
				if( --count == 0 ) 
				{ done = 1; break; }
			}
		}
		if(done || (pos==0)) break;
	}
	do_output( i+((mode&LINES)!=0), lastbufsiz, fd);
}

void fromstart( int fd, int count, int mode)
{	int i;
	int done = 0;
	int bufsiz;

	while(1)
	{	bufsiz = read(fd,cbuf,sizeof(cbuf));
		if( bufsiz == 0 ) return;

		for( i=0; i < bufsiz; i++ )
		{	char ch = cbuf[i];
			if( ch=='\r' ) continue;
			if( (ch == '\n')||(mode&CHARS) )
			{	if( --count == 0 ) { done = 1; break; }
			}
		}
		if( done ) break;
	}

	do_output(i + 1, bufsiz, fd );
}

void error(char *s,...)
{
	va_list ap;
	va_start(ap,s);
	fprintf(stderr,"tail: ");
	vfprintf(stderr,s,ap);
	fputc('\n',stderr);
	va_end(ap);
	exit(1);
}

#define INCR(p)  if (p >= end) p=cbuf ; else p++

void fifotail(FILE *in, int goal, int mode )
{
	int c, ocount=0 ;
	char *start, *finish, *end ;

        start = finish = cbuf ;
	end = &cbuf[ sizeof(cbuf) - 1 ] ;

	while ((c=getc(in)) != EOF ) 
	{
		*finish = c ;
		INCR(finish);
		if (start == finish )
		{       INCR(start); }

		if (!(mode & LINES) || c == '\n' )
		{
			ocount++ ;
			if (ocount > goal ) 
			{
				ocount = goal ;
				if ( mode & LINES )
					while (*start != '\n' )
					{ INCR(start); }
				INCR(start);
			}
		}
	} /* end while */
		
	while (start != finish ) 
	{
		putc(*start, stdout);
		INCR(start);
	}

} /* end tail */

