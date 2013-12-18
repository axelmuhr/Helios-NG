
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

#include <sys/ioctl.h>

char buf[1000];

char *message = "Hello Server";

int main(int argc, char **argv)
{
	int i;
	int s;
	int e;
	struct sockaddr_in addr;
	struct hostent *h;
	char *hostname = "nick";
	
	if( argc > 1 ) hostname = argv[1];
	
	s = socket(AF_INET,SOCK_STREAM,0);
	printf("TX socket: %d %d %x\n",s,errno,oserr);
		
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	addr.sin_addr.s_addr = INADDR_ANY;
#if 0
	e = bind(s,&addr,sizeof(addr));	
	printf("TX bind: %d %d %x\n",e,errno,oserr);

#endif	
	h = gethostbyname(hostname);
	
	addr.sin_port = htons(1235);
	addr.sin_addr.s_addr = *(u_long *)h->h_addr;

	printf("host id %x\n",addr.sin_addr.s_addr);

	e = connect(s,&addr,sizeof(addr));
	printf("TX connect: %d %d %x\n",e,errno,oserr);

	e = 1;
	ioctl(s,FIONBIO,&e);
	
	for( i = 0; i < 20; i++ )
	{
		sprintf(buf,"Hello Server %d",i);
		e = write(s,buf,1000);
		printf("TX write: %d %d %x\n",e,errno,oserr);
	}

#if 0
	e = 1;
	setsockopt(s,SOL_SOCKET,SO_DEBUG,&e,sizeof(e));
#endif

	close(s);
}
