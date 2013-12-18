
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>


char buf[1000];

int main()
{
	int s;
	int n = -1;
	int e;
	struct sockaddr_in addr;
	int fdmask;
	struct timeval tv;
	int addrsize = sizeof(struct sockaddr_in);
	int opt, optsize;
		
	s = socket(AF_INET,SOCK_STREAM,0);
	printf("RX socket: %d %d %x\n",s,errno,oserr);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(1235);
	addr.sin_addr.s_addr = INADDR_ANY;

	e = bind(s,&addr,sizeof(addr));	
	printf("RX bind: %d %d %x\n",e,errno,oserr);

	e = listen(s,5);
	printf("RX listen: %d %d %x\n",e,errno,oserr);

/*	printf("host id %x\n",gethostid(s));	*/

#if 1
	n = accept(s,&addr,&addrsize);
	printf("RX accept: %d %d %x\n",n,errno,oserr);
	printf("RX addr: %d %d %x\n",addr.sin_family,addr.sin_port,
					addr.sin_addr.s_addr);

#if 0	
	opt = 32768;
	optsize = sizeof(opt);
	e = setsockopt(n,SOL_SOCKET,SO_RCVBUF,&opt,optsize);
	printf("RX: setsockopt: %d %d %x\n",n,errno,oserr);
	e = getsockopt(n,SOL_SOCKET,SO_RCVBUF,&opt,&optsize);
	printf("RX: getsockopt: %d %d %x\n",n,errno,oserr);
	printf("RX: rcvbuf: %d size %d",opt,optsize);
#endif

	do
	{
		e = read(n,buf,1000);
		printf("RX read: %d %d %x\n",e,errno,oserr);	
		printf("RX message: %s\n",buf);
		Delay(1000000);
	} while( e > 0 );
#if 0	
	e = 1;
	setsockopt(n,SOL_SOCKET,SO_DEBUG,&e,sizeof(e));
#endif	
	close(n);
	close(s);

#else
	for(;;)
	{
		fdmask = 1<<s;
		if( n >= 0 ) fdmask |= 1<<n;
		
		tv.tv_sec = 10;
		tv.tv_usec = 0;

		e = select(32,&fdmask,0,0,&tv);

		if( e == 0 ) continue;
		
		if( fdmask & (1<<s) )
		{
			n = accept(s,&addr,&addrsize);
			printf("RX accept: %d %d %x\n",n,errno,oserr);
			printf("RX addr: %d %d %x\n",addr.sin_family,addr.sin_port,
							addr.sin_addr.s_addr);
		}

		if( n >= 0 && (fdmask & (1<<n)) )
		{
			e = read(n,buf,1000); 
/*			e = recv(n,buf,1000,0); */
			printf("RX read: %d %d %x\n",e,errno,oserr);	
			printf("RX message: %s\n",buf);
			close(n);
			n = -1;
		}
	}
#endif
}
