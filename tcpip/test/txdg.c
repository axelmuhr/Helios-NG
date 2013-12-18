
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

char buf[100];

int main(argc,argv)
int argc; char **argv;
{
	int s;
	int e;
	struct sockaddr_in addr;
	struct hostent *h;
	char *hostname = "nick";
	
	if( argc > 1 ) hostname = argv[1];
	
	s = socket(AF_INET,SOCK_DGRAM,0);
	printf("TX socket: %d %d %x\n",s,errno,oserr);
		
	addr.sin_family = AF_INET;
	addr.sin_port = 1234;
	addr.sin_addr.s_addr = 0;

	e = bind(s,&addr,sizeof(addr));	
	printf("TX bind: %d %d %x\n",e,errno,oserr);

	h = gethostbyname(hostname);
	
	addr.sin_port = 1235;
	addr.sin_addr.s_addr = *(u_long *)h->h_addr;

	printf("dest id %x\n",addr.sin_addr.s_addr);
	
	e = connect(s,&addr,sizeof(addr));
	printf("TX connect: %d %d %x\n",e,errno,oserr);
		
	e = send(s,buf,100,0);
	printf("TX send: %d %d %x\n",e,errno,oserr);
	
}
