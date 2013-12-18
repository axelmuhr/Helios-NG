
#define	RCVBUF		32*1024
#define	SNDBUF		32*1024

#define	DEBUG		0
#define	KEEPALIVE 	0
#define	OOBINLINE	0

#define INADDR_NONE 0xffffffff

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <netinet/in.h>

#include <arpa/telnet.h>

#include <helios.h>
#include <nonansi.h>
#include <syslib.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sgtty.h>
#include <netdb.h>
#include <syslog.h>
#include <ctype.h>

#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define MAX(a,b) ((a) < (b)) ? (b) : (a)


Semaphore sem_dump;

unsigned int count_r = 0;
unsigned int count_s = 0;
unsigned int count_old = 0;


#ifndef RCVBUF
int rcvbuf = 10000;
#else
int rcvbuf = RCVBUF;
#endif

#ifndef SNDBUF
int sndbuf = 10000;
#else
int sndbuf = SNDBUF;
#endif

#ifndef SNDBUF
char buffer[10000];
#else
char buffer[SNDBUF];
#endif

char valbuf[128];
word ttime = 0;
int val, s, ns;
	

void
initsem(void)
{
	InitSemaphore (&sem_dump,1);
}	

void
monitor_process(int s)
{
	int speed, error = 0;
	
	ttime = _cputime();
	forever
	{
		Delay(2*OneSec);
		Wait(&sem_dump);
		speed = (count_s - count_old)*100/1024;
		speed /= (_cputime()-ttime);
		ttime = _cputime();
		count_old = count_s;
		if (speed)
			fprintf(stderr, "ECHO:%u bytes, %d Kb/s speed\n", 
			   count_s, speed);
		else {
			fprintf(stderr,"*");
 			fflush(stderr);
		}
		Signal(&sem_dump);
	}
}

/*
void
terminator(void)
{
	char c;
	
	Wait(&sem_dump);   
	fprintf(stderr,"\nPress return to terminate at anytime!!!\n\n");
	Signal(&sem_dump);

	while((c = getchar())!='\n');;

	Wait(&sem_dump);
	exit(1);
}
*/

int closesock(void)
{
	close(s);
	close(ns);
	exit(0);
}

int 
main (int argc, char *argv[])
{
	int err=0;
	struct sockaddr_in sa;
	int ssa = sizeof(struct sockaddr_in);
	struct hostent *hp;

	if (argc != 2) {
	    fprintf(stderr, "usage: echod <own hostname>\n");
	    exit(1);
	}

	signal(SIGTERM, closesock);
	signal(SIGINT, closesock);
	signal(SIGKILL, closesock);
	signal(SIGSTOP, closesock);

	initsem();

/*
	Wait(&sem_dump);
	fprintf(stderr,"\nFork(terminator) == %d\n", Fork(2000,terminator,0));
	Signal(&sem_dump);
*/

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s<0)
	{
		Wait(&sem_dump);
		perror("socket");
		Signal(&sem_dump);
		goto end;
	}

 	hp = gethostbyname(argv[1]);
	if (hp != NULL)
	{
		memset((char *)&sa, sizeof(sa), 0);
		memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
		sa.sin_family = hp->h_addrtype;
	}
	else
	{
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(argv[1]);
	}
	sa.sin_port = htons(8000);
	
	if (sa.sin_addr.s_addr == INADDR_NONE) {
	    fprintf(stderr, "bad address\n");
	    exit(1);
	}
	    		
	err = bind (s, (caddr_t)&sa, ssa);
	if (err)
	{
		Wait(&sem_dump);
		perror("bind");
		Signal(&sem_dump);
		goto end;
	}
	err = listen (s, 2);
	if (err)
	{
		Wait(&sem_dump);
		perror("listen");
		Signal(&sem_dump);
		goto end;
	}

	Wait(&sem_dump);
	fprintf(stderr,"\nWaiting for accept...\n");
	Signal(&sem_dump);

	ns = accept(s, (caddr_t)&sa, &ssa);
	if (ns<0)
	{
		Wait(&sem_dump);
		perror("accept");
		Signal(&sem_dump);
		goto end;
	}

#if DEBUG
	val = 1;
	err = setsockopt (ns, SOL_SOCKET, SO_DEBUG, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_DEBUG, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	Wait(&sem_dump);
	if (*((int *)valbuf) == SO_DEBUG)
		fprintf(stderr,"socket option set to SO_DEBUG\n");
	else
		fprintf(stderr,"failed to set socket option to SO_DEBUG\n");
	Signal(&sem_dump);
#endif

#if OOBINLINE
	val = 1;
	err = setsockopt (ns, SOL_SOCKET, SO_OOBINLINE, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_OOBINLINE, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	Wait(&sem_dump);
	if (*((int *)valbuf) == SO_OOBINLINE)
		fprintf(stderr,"socket option set to SO_OOBINLINE\n");
	else
		fprintf(stderr,"failed to set socket option to SO_OOBINLINE\n");
	Signal(&sem_dump);
#endif 

#if KEEPALIVE
	val = 1;
	err = setsockopt (ns, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_KEEPALIVE, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	Wait(&sem_dump);
	if (*((int *)valbuf) == SO_KEEPALIVE)
		fprintf(stderr,"socket option set to SO_KEEPALIVE\n");
	else
		fprintf(stderr,"failed to set socket option to SO_KEEPALIVE\n");
	Signal(&sem_dump);
#endif

/*
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_RCVBUF, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	rcvbuf = MIN(rcvbuf, *((int *)valbuf));
	Wait(&sem_dump);
	fprintf(stderr,"initial SO_RCVBUF: %d\n", rcvbuf);
	Signal(&sem_dump);
*/

#ifdef RCVBUF
 	val = RCVBUF;
	err = setsockopt (ns, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
/*
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_RCVBUF, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	rcvbuf = *((int *)valbuf);
	Wait(&sem_dump);
	fprintf(stderr,"now is  SO_RCVBUF: %d\n", rcvbuf);
	Signal(&sem_dump);
*/
#endif

/*
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_SNDBUF, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	sndbuf = MIN(sndbuf, *((int *)valbuf));
	Wait(&sem_dump);
	fprintf(stderr,"initial SO_SNDBUF: %d\n", sndbuf);
	Signal(&sem_dump);
*/

#ifdef SNDBUF
 	val = SNDBUF;
	err = setsockopt (ns, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
/*
	val = 4;
	err = getsockopt (ns, SOL_SOCKET, SO_SNDBUF, valbuf, &val);
	if (err)
	{
		Wait(&sem_dump);
		perror("getsockopt");
		Signal(&sem_dump);
		goto end;
	}
	sndbuf = *((int *)valbuf);
	Wait(&sem_dump);
	fprintf(stderr,"now is  SO_SNDBUF: %d\n", sndbuf);
	Signal(&sem_dump);
*/
#endif

	Wait(&sem_dump);
	fprintf(stderr,"Fork(monitor_process) == %d\n\n", 
			Fork(2000,monitor_process, 4, ns));
	Signal(&sem_dump);

	{
		extern char *inet_ntoa();
		static struct sockaddr_in local, peer;
		int ssa, err = 0;
		
		ssa = sizeof(struct sockaddr_in);
		err = getsockname(ns, &local, &ssa);
		if (err)
		{
			Wait(&sem_dump);
			perror("getsockname");
			Signal(&sem_dump);
		}
		ssa = sizeof(struct sockaddr_in);
		err = getpeername(ns, &peer, &ssa);
		if (err)
		{
			Wait(&sem_dump);
			perror("getpeername");
			Signal(&sem_dump);
		}
		
		Wait(&sem_dump);		
		fprintf(stderr,"			addressfamily	port# 	internetaddress\n");
		fprintf(stderr,"local Socket:		%d		%d	%s\n"
			, local.sin_family, ntohs(local.sin_port)
			, inet_ntoa(local.sin_addr.s_addr));
		fprintf(stderr,"remote Socket:		%d		%d	%s\n" 
			, peer.sin_family, ntohs(peer.sin_port)
			, inet_ntoa(peer.sin_addr.s_addr));
		fprintf(stderr,"\n");
		Signal(&sem_dump);		
	}

	forever
	{		
		int off;
		int len;
r1:		err = recv(ns,buffer, rcvbuf, 0);
		if (err < 0)
		{
			if (errno = EWOULDBLOCK)
				goto r1;
			Wait(&sem_dump);
			perror("recv");
			Signal(&sem_dump);
			goto end;
		}
		count_r += err;

/*
		Wait(&sem_dump);
		fprintf(stderr,"ECHO: >%d< bytes received, Abs %u", err, count_r);
		Signal(&sem_dump);
*/
#if 1
		len = err;
		off = 0;
		
		while (len > 0)
		{	
s1:			err = send(ns,&buffer[off],len, 0);
			if (err < 0)
			{
				if (errno = EWOULDBLOCK)
					goto s1;
				Wait(&sem_dump);
				perror("send");
				Signal(&sem_dump);
				goto end;
			}

			off +=err;
			len -=err; 
			count_s += err;

			if (len > 0) {
				Wait(&sem_dump);
				fprintf(stderr,"			ECHO: ERR < LEN"); 
				Signal(&sem_dump);
			}

/*
			Wait(&sem_dump);
			fprintf(stderr,"ECHO: >%d< bytes sent, Abs %u", err, count_r);
			Signal(&sem_dump);
*/
		}
#endif
	}

end:	Wait(&sem_dump);
	close(s);
	close(ns);
	return(1);
}	

/* End of tcp_echo.c */

