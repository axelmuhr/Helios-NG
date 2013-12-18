
#define	SNDBUF		32*1024
#define	RCVBUF		32*1024

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

#define BUFFER 60000*4
int rcv_p = 0;
int snd_p = 0;
char buffer[BUFFER];
#ifndef RCVBUF
char recvbuf[10000];
#else
char recvbuf[RCVBUF];
#endif

char valbuf[128];
int val;
word ttime = 0;


void
initsem(void)
{
	InitSemaphore (&sem_dump,1);
}	

void
initbuffer(void)
{	
/*
	int c;
	int aw = _ldtimer(0);
	int *buf = (int *) buffer;

	for (c = 0; c < (BUFFER/4); c++)
		*(buf+c) = (aw += 0x1a3b4c5f);
*/
}	

int 
compare(int len)
{
/*	int a, b, res = 0;
	
	a = MIN(len, BUFFER - rcv_p);
	b = len - a;
	
	if (a)
		res = memcmp(&buffer[rcv_p], recvbuf, a);
	if (b)
		res &= memcmp(buffer, &recvbuf[a], b);

	if (res)
	{
		Wait(&sem_dump);   
		fprintf(stderr,"compare %d", res);
		Signal(&sem_dump);
	}
	return(res);
*/
	return(0);
}


void
monitor_process(int s)
{	
	int speed;

	ttime = _cputime();
	forever
	{
		Delay(2*OneSec);
		Wait(&sem_dump);
		speed = (count_r - count_old)*100/1024;
		speed /= (_cputime()-ttime);
		ttime = _cputime();
		count_old = count_r;
		if (speed)
		{
			fprintf(stderr, "CLIENT:>%d< bytes , %d Kb/s speed\n",
				count_r, speed);
			fflush(stderr);
		}
		else {
 			fprintf(stderr,"*");
 			fflush(stderr);
		}
		Signal(&sem_dump);
	}
}

void
receiver(int s)
{
	int err=0;
	
	forever
	{
r1:		err = recv(s, recvbuf, rcvbuf, 0);
		if (err < 0)
		{
			if (errno = EWOULDBLOCK)
				goto r1;
			Wait(&sem_dump);
			perror("recv");
			exit(1);
		}
/*
		if (compare(err) != 0) {
			Wait(&sem_dump);
			fprintf(stderr,"Fehlerhaftes Paket: rcv_p %d, len %d\n",
				rcv_p, err);
			Signal(&sem_dump);
		}


		rcv_p = ((rcv_p + err) < BUFFER) ? 
			 (rcv_p + err) : (rcv_p + err - BUFFER);
*/
		count_r += err;
/*
		Wait(&sem_dump);
		fprintf(stderr,"CLIENT: >%d< bytes received, Diff %d, Abs %d",
			err,
			((snd_p-rcv_p)<0)?snd_p-rcv_p+BUFFER:snd_p-rcv_p,
			count_r);
		Signal(&sem_dump);
*/
	}
}


int s;

int closesock(void)
{
	close(s);
	exit(0);
}

char *inet_ntoa();

int 
main (int argc, char *argv[])
{
	int err=0, len=0, i;
	struct sockaddr_in sa;
	int ssa = sizeof(struct sockaddr_in);
	char c;
	struct hostent *hp;

	if (argc != 3) {
	    fprintf(stderr, "usage: client <own hostname> <remote hostname>\n");
	    exit(1);
	}

	signal(SIGTERM, closesock);
	signal(SIGINT, closesock);
	signal(SIGKILL, closesock);
	signal(SIGSTOP, closesock);

	initsem();
	initbuffer();
	
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
	sa.sin_port = htons(7777);
	
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
	
#if DEBUG
	val = 1;
	err = setsockopt (s, SOL_SOCKET, SO_DEBUG, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
	val = 4;
	err = getsockopt (s, SOL_SOCKET, SO_DEBUG, valbuf, &val);
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
	err = setsockopt (s, SOL_SOCKET, SO_OOBINLINE, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
	val = 4;
	err = getsockopt (s, SOL_SOCKET, SO_OOBINLINE, valbuf, &val);
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
	err = setsockopt (s, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
}
	val = 4;
	err = getsockopt (s, SOL_SOCKET, SO_KEEPALIVE, valbuf, &val);
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
	err = getsockopt (s, SOL_SOCKET, SO_RCVBUF, valbuf, &val);
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
	err = setsockopt (s, SOL_SOCKET, SO_RCVBUF, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
/*
	val = 4;
	err = getsockopt (s, SOL_SOCKET, SO_RCVBUF, valbuf, &val);
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
	err = getsockopt (s, SOL_SOCKET, SO_SNDBUF, valbuf, &val);
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
	err = setsockopt (s, SOL_SOCKET, SO_SNDBUF, &val, sizeof(val));
	if (err)
	{
		Wait(&sem_dump);
		perror("setsockopt");
		Signal(&sem_dump);
		goto end;
	}
/*
	val = 4;
	err = getsockopt (s, SOL_SOCKET, SO_SNDBUF, valbuf, &val);
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

/*
	Wait(&sem_dump);
	fprintf(stderr,"Press return to continue ...");
	fflush(stderr);
	while((c = getchar())!='\n');
	Signal(&sem_dump);
*/

	hp = gethostbyname(argv[2]);
	if (hp != NULL)
	{
		memset((char *)&sa, sizeof(sa), 0);
		memcpy(&sa.sin_addr,hp->h_addr,hp->h_length);
		sa.sin_family = hp->h_addrtype;
	}
	else
	{
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(argv[2]);
	}
	sa.sin_port = htons(8000);
	
	if (sa.sin_addr.s_addr == INADDR_NONE) {
	    fprintf(stderr, "bad address\n");
	    exit(1);
	}

	Wait(&sem_dump);
	fprintf(stderr,"connecting...\n");
	fflush(stderr);
	Signal(&sem_dump);

	err = connect (s, (caddr_t)&sa, ssa);
	if (err)
	{
		Wait(&sem_dump);
		perror("connect");
		Signal(&sem_dump);
		goto end;
	}


	{
		int on = 1;
		ioctl(s, FIONBIO, &on);
	}

	Wait(&sem_dump);
	fprintf(stderr,"connected!!!\n");
	fflush(stderr);
	fprintf(stderr,"\nFork(monitor_process) == %d\n", 
		Fork(2000,monitor_process, 4, s));
	fprintf(stderr,"\nFork(receiver) == %d\n",
		Fork(8000,receiver, 4, s));
	Signal(&sem_dump);

	{
		extern char *inet_ntoa();
		static struct sockaddr_in local, peer;
		int ssa, err = 0;

		ssa = sizeof(struct sockaddr_in);
		err = getsockname(s, &local, &ssa);
		if (err)
		{
			Wait(&sem_dump);
			perror("getsockname");
			Signal(&sem_dump);
		}
		ssa = sizeof(struct sockaddr_in);
		err = getpeername(s, &peer, &ssa);
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
	{	int len;
	
/*
		len = MIN(BUFFER - snd_p, MAX(abs(_ldtimer(0)%sndbuf),12) );
*/
len = sndbuf/2;
/*
		err = send (s, &buffer[snd_p], len, 0);
*/
s1:		err = send (s, buffer, len, 0);
		if (err < 0)
		{
			if (errno = EWOULDBLOCK)
				goto s1;
			Wait(&sem_dump);
			perror("send");
			Signal(&sem_dump);
			goto end;
		}
		
		count_s += err;
/*
		snd_p = ((snd_p + err) < BUFFER) ? 
			 (snd_p + err) : (snd_p + err - BUFFER);
*/

  	}

end:	Wait(&sem_dump);
	return(0);
}	

/* End of tcp_client.c */

