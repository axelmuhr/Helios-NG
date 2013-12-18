#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#define bufsize		32768

char buf[bufsize];

void error(char *s, int e);

int main(argc,argv)
int argc;
char **argv;
{
	int s;
	int i;
	int e;
	struct sockaddr_in addr;
	int fdmask;
	struct timeval tv;
	int addrsize = sizeof(struct sockaddr_in);
	char *hostname = "nick";
	struct hostent *h;
	int total;
		
	if( argc > 1 ) hostname = argv[1];
	
	s = socket(AF_INET,SOCK_STREAM,0);
	if( s < 0 ) error("socket",s);

	h = gethostbyname(hostname);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1235);
	addr.sin_addr.s_addr = *(u_long *)h->h_addr;

	e = connect(s,&addr,addrsize);
	if( e < 0 ) error("connect",e);
	
	for( i = 0; i < bufsize; i++ ) buf[i] = i&0xff;
	
	e = bufsize;
	setsockopt(s,SOL_SOCKET,SO_SNDBUF,&e,4);	
Delay(1000000);
	while( (e = write(s,buf,bufsize)) > 0)
	{
		total += e;
		if( (total % (1024*1024)) == 0 ) IOdebug("TX %d",total);
	}

/*	e = write(s,buf,bufsize); IOdebug("TX %d",e); */

	if( e < 0 ) error("write",e);
	
	close(s);

}

void error(char *s, int e)
{
	printf("TX error %s: %d %d %x\n",s,e,errno,oserr);
	exit(1);
}

