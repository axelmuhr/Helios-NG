
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main()
{
	struct sockaddr_in addr;
	int s1, s2;
	

	s1 = socket(AF_INET,SOCK_STREAM,0);
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(700);
	addr.sin_addr.s_addr = 0;

	bind(s1,&addr,sizeof(addr));

	s2 = socket(AF_INET,SOCK_STREAM,0);
	
	bind(s2,&addr,sizeof(addr));

	addr.sin_port = htons(701);
	
	bind(s2,&addr,sizeof(addr));
}
