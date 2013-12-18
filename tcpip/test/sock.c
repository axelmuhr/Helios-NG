/* $Id$
 *
 * This is a socket example program. It establishes a connection between
 * a client and a server and sends a simple message from one to the other.
 * This program can act as both client and server...
 *
 * The server is invoked by:
 * 	% sock
 *
 * The client is invoked by:
 *	% sock <server host>
 *
 * This program should compile both under Helios and under any Unix system.
 * The default is to use UDP sockets, if TCP is defined then a TCP stream
 * is used.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef TCP
#define SOCK_TYPE	SOCK_STREAM
#else
#define SOCK_TYPE	SOCK_DGRAM
#endif

/* Magic port number - change if necessary */
#define	PORT		4567

static char buf[128];

static void fatal(msg)
char *msg;
{
	perror(msg);
	exit(1);
}

int main(argc,argv)
int argc;
char **argv;
{
	int s;
	int server = 0;
	struct hostent *h;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);

	/* client takes remote host name as argument, server takes no	*/
	/* arguments.							*/
		
	if( argc == 1 ) server = 1;

	/* create the socket						*/
	if( (s = socket(AF_INET, SOCK_TYPE, 0)) < 0 ) fatal("socket");

	/* Initialize the address					*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	if( server )
	{
		int f = s;
		int got;

		/* bind socket to PORT					*/
		if( bind(s,(struct sockaddr *)&addr,addrlen) < 0 ) 
								fatal("bind");
		
#ifdef TCP
		/* make socket listen for connects			*/
		if( listen(s,5) < 0 ) fatal("listen");
		
		/* Accept/wait for a connection 			*/
		if( (f = accept(s,(struct sockaddr *)&addr,&addrlen)) < 0 ) 
								fatal("accept");
		
		/* find out who our peer is				*/
		if( getpeername(f,(struct sockaddr *)&addr,&addrlen) < 0 )
							fatal("getpeername");
				
		/* read some data from him				*/
		got = read(f,buf,sizeof(buf));
		if( got < 0 ) fatal("read");
#else
		/* get a message from socket, if you dont want the	*/
		/* peer address read() will also work.			*/
		got = recvfrom(f,buf,sizeof(buf),0,
				(struct sockaddr *)&addr,&addrlen);
		if( got < 0 ) fatal("recvfrom");
#endif		
		/* translate peer address into a host name		*/
		h = gethostbyaddr(&addr.sin_addr,
					sizeof(struct in_addr),AF_INET);
		if( h == NULL ) fatal("gethostbyaddr");
				
		printf("server got: %s : from %s\n",buf,h->h_name);
	}	
	else /* client */
	{
		/* lookup remote host name in etc/hosts database	*/
		h = gethostbyname(argv[1]);
		if( h == NULL ) fatal("getservbyname");
		
		addr.sin_addr = *((struct in_addr *)h->h_addr);
		
		/* connect socket to remote PORT			*/
		if( connect(s,(struct sockaddr *)&addr,addrlen) < 0 ) fatal("connect");
		
		strcpy(buf,"Hello ");
		strcat(buf,h->h_name);

		/* write the message out to the server			*/
		write(s,buf,strlen(buf)+1);
	}
	
	return 0;
}
