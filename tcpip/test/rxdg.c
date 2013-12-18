
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>

char buf[100];

int main()
{
	int s;
	int e;
	struct sockaddr_in addr;
	int fdmask;
	struct timeval tv;
	int addrsize = sizeof(struct sockaddr_in);
	
	s = socket(AF_INET,SOCK_DGRAM,0);
	printf("RX socket: %d %d %x\n",s,errno,oserr);

	addr.sin_family = AF_INET;
	addr.sin_port = 1235;
	addr.sin_addr.s_addr = INADDR_ANY;

	e = bind(s,&addr,sizeof(addr));	
	printf("RX bind: %d %d %x\n",e,errno,oserr);

	fdmask = 1<<s;
	tv.tv_sec = 10;
	tv.tv_usec = 0;

#if 0
	e = select(32,&fdmask,0,0,&tv);
	printf("RX select: %d %d %x\n",e,errno,oserr);
#endif		
	e = recvfrom(s,buf,100,0,&addr,&addrsize);
	printf("RX recv: %d %d %x\n",e,errno,oserr);

	printf("RX addr: %d %d %x\n",addr.sin_family,addr.sin_port,
		addr.sin_addr.s_addr);
	
	
}
