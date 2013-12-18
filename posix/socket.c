/* $Id: socket.c,v 1.13 1993/07/12 10:25:14 nickc Exp $ */

#include "pposix.h"
#include <sys/time.h>
#include <sys/socket.h>
#include <codes.h>
#include <gsp.h>
#include <string.h>

#define bpw (sizeof(int)*8)

#define getmaskbit_(i,m) (m?((m)[(i)/bpw]&(1<<((i)%bpw))):0)
#define setmaskbit_(i,m) (m?((m)[(i)/bpw]|=(1<<((i)%bpw))):0)

int select(
	   int 	 nfds,
	   int * readfds,
	   int * writefds,
	   int * exceptfds,
	   struct timeval * tv )
{
	word timeout = (tv==0)?-1:(tv->tv_sec*OneSec+tv->tv_usec);
	Stream **streams = NULL;
	word *flags = NULL;
	word maskwords = ((word)nfds + (word)bpw - 1)/bpw;
	int i;
	int s = 0;
	int nfound = -1;
		
	CHECKSIGS();
	streams = (Stream **)Malloc((word)nfds * sizeof(Stream *));
	flags   = (word *)Malloc((word)nfds * sizeof(word));
	
	if( flags == 0 || streams == 0 )
	{ errno = ENOMEM; goto fail; }

	/* setup stream and flag vectors	*/
	for( i = 0; i < nfds; i++ )
	{
		if( 	getmaskbit_(i,readfds) || 
			getmaskbit_(i,writefds) ||
			getmaskbit_(i,exceptfds) )
		{
			int f = 0;
			fdentry *fde;
			
			if((fde = checkfd(i)) == NULL ) goto fail;

			streams[s] = fde->pstream->stream;
			if( getmaskbit_(i,readfds) ) f |= O_ReadOnly;
			if( getmaskbit_(i,writefds) ) f |= O_WriteOnly;
			if( getmaskbit_(i,exceptfds) ) f |= O_Exception;
			f |= (i<<16);
			flags[s] = f;

			s++;
		}
	}
	
	for( i = 0; i < maskwords ; i++ ) 
	{
		if( readfds ) readfds[i] = 0;
		if( writefds ) writefds[i] = 0;
		if( exceptfds ) exceptfds[i] = 0;
	}

	nfound = (int)SelectStream(s,streams,flags,timeout);

	if( nfound <= 0 ) goto fail;
		
	for( i = 0; i < s ; i++ )
	{
		int f = (int)flags[i];
		if( f & O_Selected )
		{
			int fd = f>>16;
			if( f & O_ReadOnly ) setmaskbit_(fd,readfds);
			if( f & O_WriteOnly ) setmaskbit_(fd,writefds);
			if( f & O_Exception ) setmaskbit_(fd,exceptfds);
		}
	}
fail:
	if( streams != NULL ) Free( streams );
	if( flags != NULL ) Free( flags );

	if( nfound < -1 ) 
	{
		if( nfound == EK_Timeout ) { nfound = 0; }
		else { errno = posix_error(nfound); nfound = -1; }
	}
	CHECKSIGS();
	return nfound;
}

int socket(int domain, int type, int protocol)
{
	Stream *s = NULL;
	Pstream *p = NULL;
	word e = 0;
	int fd;
	extern char nodename[];
	string dname = nodename;
	fdentry *f;
			
	CHECKSIGS();
	if( (fd = findfd(0)) == -1 ) goto done;

	if( (p = New(Pstream)) == NULL ) { e = EMFILE; goto done; }

	switch( domain )
	{
	case AF_HELIOS:	dname = "/.socket"; break;
	case AF_INET:	dname = "/internet"; break;
	default:
		if(opendb("/helios/etc/socket.conf",0)!=0 ||
		   scandb("!d?d?d%s",domain,type,protocol,dname)!=0)
		{ e = EPROTOTYPE; goto done; }
		closedb(0);
	}
	
	if( (s = Socket(dname,type,protocol)) == NULL )
	{ e = Result2(cdobj()); goto done; }

	p->type = Type_Socket;
	p->refs = 1;
	p->stream = s;

	f = checkfd(fd);
	f->pstream = p;

done:
	if( e < 0 )
	{
		errno = posix_error(e);
		if( s != NULL ) Close(s);
		if( p != NULL ) Free(p);
		freefd(fd);
		fd = -1;
	}

	CHECKSIGS();
	return fd;
}

int bind(int fd, struct sockaddr *addr, int len)
{
	Stream *s;
	word e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	e = Bind(s,(byte *)addr,len);
	
	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return (int)e;
}

int listen(int fd, int len)
{
	Stream *s;
	word e;
		
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	e = Listen(s,len);
	
	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return (int)e;
}

int accept(int fd, struct sockaddr *addr, int *len)
{
	Stream *s, *s1 = NULL;
	Pstream *p = NULL;
	int fd1, e = 0;	
	fdentry *f;
	fdentry *f1;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	if( (fd1 = findfd(0)) == -1 ) goto done;

	if( (p = New(Pstream)) == NULL ) { e = EMFILE; goto done; }

	s = f->pstream->stream;
	
	if( (s1 = Accept(s,(byte *)addr,(word *)len)) == NULL )
	{ e = (int)Result2(s); goto done; }

	p->type = Type_Socket;
	p->refs = 1;
	p->stream = s1;

	f1 = checkfd(fd1);
	f1->pstream = p;

done:	

	if( e < 0 )
	{
		errno = posix_error(e);
		if( s1 != NULL ) Close(s1);
		if( p != NULL ) Free(p);
		freefd(fd1);
		fd1 = -1;
	}

	CHECKSIGS();
	return fd1;
}

int connect(int fd, struct sockaddr *addr, int len)
{
	Stream *s;
	word e;
		
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	e = Connect(s,(byte *)addr,len);
	
	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return (int)e;
}

extern int socketpair(int domain, int type, int protocol, int *sv)
{
	int e;

	if( domain != AF_HELIOS || type != SOCK_STREAM || protocol != 0 )
	{ errno = -1; return -1; }
	
	e = pipe(sv);

	if( e == 0 )
	{
		checkfd(sv[0])->pstream->stream->Flags |= O_ReadWrite;
		checkfd(sv[1])->pstream->stream->Flags |= O_ReadWrite;
	}

	return e;
}

extern int recv(int fd, char *buf, int len, int flags)
{
	Stream *s;
	struct msghdr msg;
	struct iovec iov[1];
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	memset(&msg,0,sizeof(msg));
	
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	s->Timeout = setuptimeout();
	e = (int)RecvMessage(s,flags,&msg);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	}
	resettimeout();
	CHECKSIGS();
	return e;
}

extern int recvfrom(int fd, char *buf, int len, int flags,
				struct sockaddr *from, int *fromlen)
{
	Stream *s;
	struct msghdr msg;
	struct iovec iov[1];
	int e;
	fdentry *f;

	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	memset(&msg,0,sizeof(msg));
	
	msg.msg_name = (char *)from;
	msg.msg_namelen = *fromlen;
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	s->Timeout = setuptimeout();

	e = (int)RecvMessage(s,flags,&msg);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	}
	*fromlen = msg.msg_namelen;

	resettimeout();
	CHECKSIGS();
	return e;
}

extern int recvmsg(int fd, struct msghdr *msg, int flags)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	s->Timeout = setuptimeout();
	e = (int)RecvMessage(s,flags,msg);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	}
	resettimeout();
	CHECKSIGS();
	return e;
}

extern int send(int fd, char *buf, int len, int flags)
{
	Stream *s;
	struct msghdr msg;
	struct iovec iov[1];
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	memset(&msg,0,sizeof(msg));
	
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	
	s->Timeout = setuptimeout();
	e = (int)SendMessage(s,flags,&msg);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	}
	resettimeout();
	CHECKSIGS();
	return e;
}

extern int sendto(int fd, char *buf, int len, int flags,
				struct sockaddr *to, int tolen)
{
	Stream *s;
	struct msghdr msg;
	struct iovec iov[1];
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	memset(&msg,0,sizeof(msg));
	
	msg.msg_name = (char *)to;
	msg.msg_namelen = tolen;
	iov[0].iov_base = buf;
	iov[0].iov_len = len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	s->Timeout = setuptimeout();
	e = (int)SendMessage(s,flags,&msg);
	
	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e =  -1;
	}
	resettimeout();
	CHECKSIGS();

	return e;
}

extern int sendmsg(int fd, struct msghdr *msg, int flags)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	s->Timeout = setuptimeout();
	e = (int)SendMessage(s,flags,msg);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	}
	resettimeout();
	CHECKSIGS();
	return e;
}


extern int gethostid(void)
{
	int id = -1;
	word size = sizeof(int);
	Stream *s;
	int e = 0;

	CHECKSIGS();
	s = Socket("/internet",SOCK_DGRAM,0);

	if( s == NULL ) e = ENOMEM;
	
	if( s != NULL ) e = (int)Bind(s,0,0);
	
	if( e >= 0 ) e = (int)GetSocketInfo(s,SOL_SYSTEM,SO_HOSTID,(void *)&id,(word *)&size);

	if( s != NULL ) Close(s);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		id = -1;
	}
	CHECKSIGS();
	return id;
}

extern int gethostname(char *name, int namelen)
{
	Stream *s;
	int e = 0;

	CHECKSIGS();
	s = Socket("/internet",SOCK_DGRAM,0);

	if( s == NULL ) e = ENOMEM;
	
	if( s != NULL ) e = (int)Bind(s,0,0);
	
	if( e >= 0 ) e = (int)GetSocketInfo(s,SOL_SYSTEM,SO_HOSTNAME,(void *)name,(word *)&namelen);

	if( s != NULL ) Close(s);
	
	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return e;
}

extern int getpeername(int fd, struct sockaddr *name, int *namelen)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;

	if( s->Type == Type_Pipe )
	{
		name->sa_family = AF_HELIOS;
		*namelen = 2;
		return 0;
	}
	
	e = (int)GetSocketInfo(s,SOL_SOCKET,SO_PEERNAME,(void *)name,(word *)namelen);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return e;
}

extern int getsockname(int fd, struct sockaddr *name, int *namelen)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	if( s->Type == Type_Pipe )
	{
		name->sa_family = AF_HELIOS;
		*namelen = 2;
		return 0;
	}
	
	e = (int)GetSocketInfo(s,SOL_SOCKET,SO_SOCKNAME,(void *)name,(word *)namelen);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return e;
}

extern int getsockopt(int fd, int level, int optname, char *optval, int *optlen)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	e = (int)GetSocketInfo(s,level,optname,(void *)optval,(word *)optlen);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return e;
}

extern int setsockopt(int fd, int level, int optname, char *optval, int optlen)
{
	Stream *s;
	int e;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL) return -1;
	
	s = f->pstream->stream;
	
	e = (int)SetSocketInfo(s,level,optname,optval,optlen);

	if( e < 0 ) 
	{
		errno = posix_error(Result2(s));
		e = -1;
	} else e = 0;
	CHECKSIGS();
	return e;
}

extern int shutdown(int fd, int how)
{
	fd = fd; how = how;
	CHECKSIGS();
	return -1;
}

extern unsigned long swap_long(unsigned long a,unsigned long b)
{	
	((char *)&b)[0] = ((char *)&a)[3];
	((char *)&b)[1] = ((char *)&a)[2];
	((char *)&b)[2] = ((char *)&a)[1];
	((char *)&b)[3] = ((char *)&a)[0];
	return b;
}

extern unsigned short swap_short(unsigned short a,unsigned long b)
{	
	b = 0;
	((char *)&b)[0] = ((char *)&a)[1];
	((char *)&b)[1] = ((char *)&a)[0];
	return (unsigned short)b;
}

