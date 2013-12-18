/********************************************************
*							*	
*	gt						*
*							*
*	A command to fetch time from an Internet	*
*	time service (RFC868) on a network.		*
*							*
*	Copyright Perihelion Software Ltd. 1992		*
********************************************************/

static char *rcsid = "$Header: /hsrc/tcpip/cmds/gt/RCS/gt.c,v 1.2 1992/08/28 15:24:50 chris Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <posix.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifdef helios
#include <helios.h>
#include <root.h>
#endif

int makesrvaddress( char *hostname, char *remoteservice,
					struct sockaddr_in *addr, int udp);

void usage(void)
{
#ifdef helios
	fprintf(stderr,"Usage: gt [-u|t] [-r] <hostname>\n");
#else
	fprintf(stderr,"Usage: gt [-u|t] <hostname>\n");
#endif
}

int main( int argc, char **argv )
{
	struct sockaddr_in addr;
	time_t t;
	int fd;
	int rc;
	int udp = 0;
	int updateroot = 0;
	char *s;

	argv++;

	while( --argc && (s = *argv)[0] == '-' )
	{
		argv++;

		switch( s[1] )
		{
		case 't':
			udp = 0;
			break;
		case 'u':
			udp = 1;
			break;
#ifdef helios
		case 'r':
			updateroot = 1;
			break;
#endif
		default:
			usage();
			exit(1);
		}
	}

	if( argc != 1 )
	{
		usage();
		exit(1);
	}

	if( makesrvaddress( argv[0], "time", &addr, udp) == 0 )
	{
		fprintf(stderr,"Can't find service\n");
		exit(1);
	}

	if( udp )
		fd = socket( AF_INET, SOCK_DGRAM, 0);
	else
		fd = socket( AF_INET, SOCK_STREAM, 0);

	if( fd < 0 )
	{
		perror("socket");
		exit(1);
	}
	
	if( udp )
	{
		if( (rc = sendto(fd, (char *)&t, 0, 0, (struct sockaddr *)&addr,
					sizeof(struct sockaddr_in) )) < 0)
		{
			perror("sendto");
			exit(1);
		}
		if( (rc = recv(fd, (char *)&t, 4, 0 )) < 0)
		{
			perror("recv");
			exit(1);
		}
	}
	else
	{	if( (rc = connect(fd, (struct sockaddr *)&addr,
						sizeof(struct sockaddr_in) )) < 0)
		{
			perror("connect");
			exit(1);
		}
		if( (rc = read(fd, (char *)&t, 4 )) < 0)
		{
			perror("read");
			exit(1);
		}
	}

	close(fd);

	if( rc == 4 )
	{
/* RFC868 is the source for the magic number in the next line */
		t = ntohl(t)-2208988800ul;
#ifdef helios
		if( updateroot )
			GetRoot()->Time = t;
#endif
		printf("%s", ctime((time_t *)&t));
	}
	else
		fprintf(stderr,"Insufficent data returned by time service\n");
	return 0;
}

int makesrvaddress( char *hostname, char *remoteservice,
					struct sockaddr_in *addr, int udp)
{	struct hostent *hp;
	struct servent *sp;
	char *proto;
	if ((hp = gethostbyname(hostname)) == NULL)
	{
		perror("gethostbyname");
		return 0;
	}
	proto = udp? "udp" : "tcp";
	if ((sp = getservbyname(remoteservice, proto)) == NULL)
	{
		perror("getservbyname");
		return 0;
	}

/* Now set up the address of the service */
	memset((char *)addr, sizeof(struct sockaddr_in), 0);
	memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);
	addr->sin_family = hp->h_addrtype;
	addr->sin_port = sp->s_port;
	return 1;
}
