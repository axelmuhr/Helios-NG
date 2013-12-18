
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

main()
{
	int s;
	int e;
	int on = 1;
	struct sockaddr_in addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = 1234;
	addr.sin_addr.s_addr = 0;
	
	s = socket(AF_INET,SOCK_STREAM,0);
	printf("socket: s %d errno %d\n",s,errno);
	
	e = setsockopt(s,SOL_SOCKET,SO_REUSEADDR, &on, sizeof(on));
	printf("setsockopt: e %d errno %d\n",e,errno);
	
	e = bind(s,&addr,sizeof(addr));
	printf("bind: e %d errno %d\n",e,errno);
}
