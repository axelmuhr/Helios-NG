
/* SARUN - 20/8/89 						*/
/* Program to load and run a stand alone program		*/
/* To compile:  c sarun.c sasup.o -o sarun			*/

#include <stdio.h>
#include <stdarg.h>
#include <linkio.h>
#include <stdlib.h>
#include <sem.h>
#include <nonansi.h>

Semaphore sync;			/* Termination synchronization	*/

bool link = -1;			/* link we are using		*/

bool finished = FALSE;		/* set True on EOF		*/

static int error(char *f,...)
{
	va_list a;
	
	va_start(a,f);
	
	vfprintf(stderr,f,a);
	
	putc( '\n', stderr );

	if( link != -1 ) link_close(link);
		
	exit(1);
}

/* Input process						*/
/* Reads characters from STDIN and passes them through link	*/

void input(word link)
{
	word c;
	do {
		c = getchar();
		if( c == EOF ) break;
		link_out_byte(link,c);
	} while( !finished );
	finished = TRUE;
	Signal(&sync);
}

/* Output process						*/
/* Reads characters from link and prints them on STDOUT		*/

void output(word link)
{
	word c = 0;
	do {
		word e = link_in_byte(link,c);
		if( e < 0 ) continue;	/* timeout, just loop	*/
		putchar(c);
	} while( !finished );
	finished = TRUE;
	Signal(&sync);
}

/* Main								*/
/* Get control of link, boot program through it, fork input and	*/
/* output proceses and then wait for them to finish.		*/

int main(int argc, char **argv)
{
	int e;
	
	if( argc < 3 ) error("usage: sarun link bootfile");

	link = atoi(argv[1]);

	/* open the link for raw I/O */	
	if( !link_open(link) ) error("failed to open link %d",link);

	/* boot the program through it */
	if( (e=link_boot(link,argv[2]))!=0 ) error("link_boot error %d",e);

	/* now spool stdin into the link and	*/
	/* anything from the link to stdout	*/
	
	InitSemaphore(&sync,0);
	
	Fork(2000,input,4,link);
	
	Fork(2000,output,4,link);
	
	Wait(&sync);
	Wait(&sync);
	
	if( !link_close(link) ) error("failed to close link %d",link);
}


