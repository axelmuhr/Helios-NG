/* $Id: ioctl.c,v 1.5 1993/04/20 09:12:53 nickc Exp $ */
#define _IN_IOCTL_C

#include <sys/types.h>
#include <sys/ioctl.h>
#include <syslib.h>
#include <gsp.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include <sgtty.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

extern int gtty(int fd, struct sgttyb *buf);
extern int stty(int fd, struct sgttyb *buf);

#define IOC_WRITE(x)	(((x)&0xf0000000)==IOC_IN)
#define IOC_READ(x)	(((x)&0xf0000000)==IOC_OUT)
#define IOC_READWRITE(x)(((x)&0xf0000000)==IOC_INOUT)
#define IOC_SIZE(x)	(((x)>>16)&IOCPARM_MASK)

static struct tchars _tchars = { CINTR, CQUIT, CSTART, CSTOP, CEOF, CBRK };

static struct ltchars _ltchars = { CSUSP, CDSUSP, CRPRNT, CFLUSH, CWERASE, CLNEXT };

extern word ioctl(int fd, int code, caddr_t data)
{
	Stream *s;
	word e = 0;
	union
	{
		struct sgttyb s;
		struct termios t;
	}u;
		
	s = fdstream(fd);
	
	if( s == NULL ) { e = EINVAL; goto done; }
	
	/* first see if we can do it via get/setsockopt 		*/
	if( (s->Type & ~Type_Flags) == Type_Socket )
	{
		int size = IOC_SIZE(code);
		if( IOC_READ(code) || IOC_READWRITE(code) )
			e = getsockopt(fd,SOL_IOCTL,code,(char *)data,&size);
		elif( IOC_WRITE(code) )
			e = setsockopt(fd,SOL_IOCTL,code,(char *)data,size);
		if( e == 0 ) goto done;
	}
	
	switch( code )
	{
	case FIONREAD:
		*(word *)data = GetFileSize(s);
		break;
		
	case FIONBIO:
	{
		int x = fcntl(fd,F_GETFL);
		if( *(int *)data ) x |= O_NONBLOCK;
		else x &= ~O_NONBLOCK;
		fcntl(fd,F_SETFL,x);
		break;
	}

	case TIOCGETD:
		*(int *)data = NTTYDISC;
		break;

	case TIOCGETP:
		e = gtty(fd,(struct sgttyb *)data);
		break;
		
	case TIOCSETP:		
	case TIOCSETN:		
		e = stty(fd,(struct sgttyb *)data);
		break;
		
	case TIOCFLUSH:
		e = tcflush(fd,TCIOFLUSH);
		break;
		
	case TIOCGETC:
		*(struct tchars *)data = _tchars;
		break;
		
	case TIOCLSET:
		e = gtty(fd,&u.s);
		if( e != 0 ) break;
		u.s.sg_flags &= 0x0000FFFF;
		u.s.sg_flags |= *(word *)data << 16;
		e = stty(fd,&u.s);
		break;
		
	case TIOCLGET:
		e = gtty(fd,&u.s);
		if ( e == 0 ) *(word *)data = u.s.sg_flags>>16;
		break;
		
	case TIOCGPGRP:
		*(int *)data = tcgetpgrp(fd);
		break;
		
	case TIOCSPGRP:
		e = tcsetpgrp(fd,*(int *)data);
		break;
		
	case TIOCGLTC:
		*(struct ltchars *)data = _ltchars;
		break;
		
	case TIOCSTOP:
		e = tcflow(fd,TCOOFF);
		break;
		
	case TIOCSTART:
		e = tcflow(fd,TCOON);
		break;
		
	case TIOCGWINSZ:
	{
		struct winsize *w = (struct winsize *)data;
		if( (e = tcgetattr(fd, &u.t)) != 0 ) break;
		w->ws_row = u.t.c_min;
		w->ws_col = u.t.c_time;
		w->ws_xpixel = w->ws_col * 10;
		w->ws_ypixel = w->ws_row * 10;
		break;
	}
				
	case TIOCOUTQ:
		/* make up a number */
		*(int *)data = 256;
		break;
		
	/* The following ioctls do nothing and return quietly */
	case TIOCSETD:	
	case TIOCHPCL:
	case TIOCMODS:
	case TIOCMODG:
	case TIOCEXCL:		
	case TIOCNXCL:		
	case TIOCSETC:
	case TIOCLBIS:		
	case TIOCLBIC:		
	case TIOCSBRK:
	case TIOCCBRK:		
	case TIOCSDTR:		
	case TIOCCDTR:
	case TIOCSTI:		
	case TIOCNOTTY:
	case TIOCMSET:
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCSLTC:		
	case TIOCMGET:
	case TIOCREMOTE:
	case TIOCPKT:
	case TIOCSWINSZ:
	case TIOCUCNTL:
		break;			
		
	default:
		errno = EINVAL;
		e = -1;
		break;
	}
done:
	return e;
}

extern int gtty(int fd, struct sgttyb *buf)
{
	struct termios t;
	int e = tcgetattr(fd,&t);
	int flags = 0;
	if( e != 0 ) return e;

	buf->sg_ispeed = cfgetispeed(&t);
	buf->sg_ospeed = cfgetospeed(&t);

	buf->sg_erase = t.c_cc[VERASE];
	buf->sg_kill = t.c_cc[VKILL];
	
	if( t.c_iflag & IXOFF ) flags |= TANDEM;
	if( t.c_lflag & ECHO ) flags |= _ECHO;
	unless( t.c_lflag & ICANNON ) 
	{
		unless( t.c_oflag & OPOST ) flags |= RAW;
		else flags |= CBREAK;
	}
	if( t.c_cflag & PARENB )
	{
		if( t.c_cflag & PARODD ) flags |= ODDP;
		else flags |= EVENP;
	}
	
	buf->sg_flags = flags;

	return 0;	
}

extern int stty(int fd, struct sgttyb *buf)
{
	struct termios t;
	word flags = buf->sg_flags;
	
	memset(&t,0,sizeof(t));	
	cfsetispeed(&t,buf->sg_ispeed);
	cfsetospeed(&t,buf->sg_ospeed);
	
	t.c_cc[VERASE] = buf->sg_erase;
	t.c_cc[VKILL] = buf->sg_kill;
	
	unless( flags & RAW ) 
	{
		unless( flags & CBREAK ) 
			t.c_lflag |= ICANNON, t.c_oflag |= OPOST;
		else 	t.c_lflag |= OPOST;
	}
	if( flags & TANDEM ) t.c_iflag |= IXOFF;
	if( flags & _ECHO ) t.c_lflag |= ECHO;
	if( (flags & ANYP) == EVENP ) t.c_cflag |= PARENB;
	if( (flags & ANYP) == ODDP ) t.c_cflag |= PARENB|PARODD;
	
	return tcsetattr(fd,TCSANOW,&t);
}

