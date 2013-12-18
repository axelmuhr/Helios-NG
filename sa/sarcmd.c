

#include <stdio.h>
#include <stdarg.h>

extern int sarun(char *, char *);

int error(char *f,...)
{
	va_list a;
	
	va_start(a,f);
	
	vfprintf(stderr,f,a);
	
	putc( '\n', stderr );
	
	exit(1);
}

int main(int argc, char **argv)
{
	int e;
	if( argc < 3 ) error("usage: sarun linkstream bootfile");

	if( (e=sarun(argv[1],argv[2]))!=0 ) error("sarun error %d",e);
}


