#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>

#define bufsize		32768

char buf[bufsize];

void error(char *s, int e);

int main()
{
	int s;
	int n = -1;
	int e;
	struct sockaddr_in addr;
	int fdmask;
	struct timeval tv;
	int addrsize = sizeof(struct sockaddr_in);
	int total;
		
	s = socket(AF_INET,SOCK_STREAM,0);
	if( s < 0 ) error("socket",s);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1235);
	addr.sin_addr.s_addr = INADDR_ANY;

	e = bind(s,&addr,sizeof(addr));	
	if( e < 0 ) error("bind",e);

	e = listen(s,5);
	if( e < 0 ) error("listen",e);

	n = accept(s,&addr,&addrsize);
	if( e < 0 ) error("accept",e);
	
	printf("RX addr: %d %d %x\n",addr.sin_family,ntohs(addr.sin_port),
					addr.sin_addr.s_addr);
	
	e = bufsize;
	setsockopt(s,SOL_SOCKET,SO_RCVBUF,&e,4);	

Delay(2000000);
	while( (e = read(n,buf,bufsize)) > 0)
	{
		int i;
		total += e;
if( (total % (1024*1024))==0 ) IOdebug("RX %d",total);
/*
		for( i = 0; i < bufsize; i++ )
			if( buf[i] != (i&0xff)) error("cmp",(errno=i,buf[i]));
*/
	}
	if( e < 0 ) error("read",e);
	
	close(n);
	close(s);

IOdebug("total %d",total);
}

void error(char *s, int e)
{
	printf("RX error %s: %d %d %x\n",s,e,errno,oserr);
	exit(1);
}

