

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/in.h>
#include <netdb.h>

int main(int argc, char **argv)
{
	int e,i;
	char buf[100];
	char *host;
	struct hostent *he;
	int s;
	struct sockaddr_in addr;

	if( argc != 2 )
	{
		printf("usage: %s hostname\n",argv[0]);
		exit(1);
	}
	
	host = argv[1];
	
	he = gethostbyname(host);

IOdebug("he %s %x %d %d %x",*he);
	addr.sin_family = he->h_addrtype;	
	addr.sin_addr.s_addr = *(u_long *)(he->h_addr);
	addr.sin_port = 1230;
IOdebug("addr %x %x",addr);

	s = socket(AF_INET,SOCK_DGRAM,0);
IOdebug("s = %d",s);

	for( i = 0; i < 100; i++ ) buf[i] = 0;

	e = sendto(s,buf,100,0,&addr,sizeof(addr));
IOdebug("sendto: %d",e);

	close(s);
}
