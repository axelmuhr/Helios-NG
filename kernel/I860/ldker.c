#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <sem.h>
#include <signal.h>
#include <nonansi.h>
#include <codes.h>

#include "config1.h"

#include <link.h>
#include <time.h>

#include "xplib.h"

typedef unsigned long ulong;
typedef unsigned int uint;


int freeze;

int exiting;
Semaphore exit_sem;
clock_t clockstart,clockend;
void tidyup(int st)
{
	xptidy();
	exit(st);
}

void usage()
{
	fprintf(stderr,"usage: iload [-n] <file> <address>\n");
	tidyup(1);
}

void error(char *s,...)
{	
	va_list ap;
	va_start(ap,s);

	vfprintf(stderr,"%s\n",ap);
	va_end(ap);
	exiting =1;
	tidyup(1);
}

ulong mystrtol(char *s)
{	int base = 10;
	ulong r;

	if( s[0] == '0' )
	{
		if(toupper(s[1]) == 'X' )
			base = 16;
		else
			base = 8;
	}
	
	r = strtoul(s,&s,base);
	return r;
}

#ifdef BUG19
int isldorst(uword w)
{
	switch( (w>>26) & 0x3f )
	{
	case 0x00: case 0x01: case 0x04: case 0x05:
	case 0x03: case 0x07: case 0x08: case 0x09:
	case 0x0a: case 0x0b: case 0x0f: case 0x18:
	case 0x19:
		return 1;
	default: return 0;
	}
}

int checkbug19( uword *buf, int n, uword at, int *bugvec)
{	int i = (at>>2) & 0x3ff;
	int nbugs = 0;
	if( i == 0 ) i = 0x400;
	i -= 3;
/*	printf("check at = %08lx, i= %08lx\n",at,i); */
	for( ; i < n; i += 0x400)
	{	int j;
		for( j= i<0?-i:0; j<3;j++)
		{
			if( isldorst(buf[i+j]) )
				bugvec[nbugs++] = i+j;
		}
	}
	return nbugs;
}
#endif

uword loadfile(FILE *f, ulong at)
{
	uword *buf;
	int nread;
	int i;
	int wordstosend;
	uword head[3];
	ulong base = at - 32;

	if( fread(head,sizeof(uword),3,f) != 3 )
		error("Can't read file header\n");
	if( head[0] != 0x12345678 )
		error("Invalid magic no. in file\n");

	if( (buf = (uword *)malloc(head[2])) == NULL )
	{
		error("Unable to allocate %d bytes for file\n",head[2]);
	}

	printf("ldker: header size %x \n",head[2]);
	dbwrword(base,head[2]+32);
	dbwrword(base+4,0);
	dbwrword(base+8,0);
	dbwrword(base+12,0);
	
	nread = fread(buf, 1,head[2], f);
	if(nread != head[2])
	{
		free(buf);
		error("Unable to read %d bytes from file\n",head[2]);
	}
	wordstosend = (head[2]+3)/4;
	
#ifdef BUG19
	{	int bugvec[100];	/* enough for 33*4k = 132k */
		int nbugs;
		uword offset;
		uword bestoffset = 0;
		uword bestnbugs =100000;
		for( offset = 0; offset < 0x1000; offset +=32)
		{
			nbugs = checkbug19(buf,wordstosend,at+offset,bugvec);
			if( freeze ) break;
			if( nbugs < bestnbugs )
			{	bestnbugs = nbugs;
				bestoffset = offset;
			}
			printf("%d bug19 faults at offset %04lx\n",nbugs,offset);
			if( nbugs == 0 ) break;
		}
#ifdef never
		{	int nn;
			printf("Be warned - bug 19 possibility at\n");
			for( nn=0; nn<nbugs; nn++ )
				printf("%08lx -> %08lx\n",at+bugvec[nn]*4,
							buf[bugvec[nn]]);
		}
#endif
		if( bestnbugs != 0 ) offset = bestoffset;
		if( offset != 0 )
			printf("File being loaded with offset of %04lx\n",offset);
		at += offset;
	}
#endif
	{	uword addr=at;
		for(i=0; i < wordstosend; i++)
		{
			dbwrword(addr,buf[i]);
			addr+=4;
		}
	}
	free(buf);
	return at;
}

void exit_handler(int sig)
{
	exiting = 1;
}

int mode = 0;
static void send_test_vec(void);
static void send_config_vec(void);

void stdout_fork()
{
/*	setvbuf(stdout,NULL,_IONBF,0); */

	while(!exiting)
	{	word e;
		char b = 0;

		e = LinkIn(1,linkno,&b,2*OneSec);
		if( e < Err_Null ) continue;
		if( isprint(b) || (b == '\n') || (b == 7))
		{	if (!mode) putchar(b);
			else  printf("(\\0x%02x)",b);  
		}
		else
		{	switch( b )
			{

			case 2:	printf("ldker: 2 received \n"); 
				/* send_config_vec(); */
				break;
			case 3:	printf("ldker: 3 received \n");
				/* send_test_vec(); */
				break;
			case 0: mode = 0; break;
			case 1: mode = 1; break;
					
			default:
				printf("\\0x%02x",b); break;
			}
		}

		fflush(stdout);
	}
	fprintf(stderr,"stdout exiting\n");
	Signal(&exit_sem);
}


void stdin_fork()
{	int i = 0;
	while(!exiting)
	{
		char ch = getchar();
		word e;
		switch (ch)
		{
			case  'C': send_config_vec(); break;
			case  'T': send_test_vec(); break;
		}
#if 0
		while(!exiting)
		{
			e = LinkOut(1,linkno,&ch,2*OneSec);
			if( e == Err_Null ) break;
		}
#endif
	}
	fprintf(stderr,"stdin exiting\n");
	Signal(&exit_sem);
}

static void send_test_vec(void)
{ 
  static char buf[8] = {0x12,0x34,0x56,0x78,0x87,0x65,0x43,0x21};
  WORD bootlink    = 0;

  LinkOut(8,linkno,buf,2*OneSec);
  printf("ldker: buf sent \n");
}

/**
*** The configuration vector
**/
#define swap(a) a 

static void send_config_vec(void)
{ 
  Config config;
  WORD memory_size = 0x800000;
  WORD bootlink    = 0;
  char c;
  
  config.PortTabSize = swap(1024L);
  config.Incarnation = swap(1L);
  config.LoadBase    = (word *)swap(0xf000a000L);
  config.ImageSize   = swap(0L);
  config.FirstProg   = swap(6L);
  config.MemSize     = swap(memory_size);
  config.NLinks      = swap(1L);
  config.LinkConf[0] = swap(0x00060000L);  /* by default the links are dead */
  config.LinkConf[(int) bootlink] &= swap(0x0300FF00L); 
  config.LinkConf[(int) bootlink] |= swap(0x00030270L);
  
#if 1
  bootlink = sizeof(Config);
  LinkOut(4,linkno,&bootlink,2*OneSec);
#else
  xpwrword(sizeof(Config));	
#endif
  printf("ldker: config size %d sent\n",sizeof(Config));

#if 1
  LinkOut(sizeof(Config),linkno,(byte *)&config,2*OneSec);
#else
  xpwrdata((UBYTE *)&config,sizeof(Config));
#endif

  printf("ldker: config sent\n"); 
}


#define DEFAULT_ADDRESS 0xf000a000
#define START_OFFSET 56	/* sizeof(Module) */

int main(int argc, char *argv[])
{	FILE *f;
	ulong at = DEFAULT_ADDRESS+32;
	int norun=0;
	word a, b;
		 
	signal(SIGINT,exit_handler);
	xpinit(1,0);

	if( argc < 2 )
		usage();
		
	while( argv[1][0] == '-' )
	{
		switch( argv[1][1] )
		{
		case 'n': norun = 1; break;
		case 'f': freeze = 1; break;
		default:
			usage();
		}
		argv++; argc--;
	}

#if 0
	if( argc != 3 ) usage();
#endif
	f = fopen(argv[1],"rb");
	if( f == NULL )
		usage();
	
	if ( argc == 3) at = mystrtol(argv[2]);

	if( (at = loadfile(f,at)) == 0 )
		error("Unable to load file %s to processor\n",argv[1]);
	if( norun )
	{	printf("File loaded\n");
		tidyup(0);
	}

	printf("ldker: jump to start address = %x \n\n",at+START_OFFSET);
	fflush(stdout);

	dbwrint(0,at+START_OFFSET);
	
	
	InitSemaphore(&exit_sem,0);
	 Fork(1000, stdin_fork, 0);  
	 Fork(1000, stdout_fork, 0);

	Wait(&exit_sem); 
	Wait(&exit_sem);

	tidyup(0);
}
