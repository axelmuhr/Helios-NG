/*------------------------------------------------------------------------
--                                                                      --
--                     P O S I X    L I B R A R Y			--
--                     --------------------------                       --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- termios.c								--
--                                                                      --
--	Terminal IO control.						--
--                                                                      --
--	Author:  NHG 8/5/88						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId:	 %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.*/
/* $Id: termios.c,v 1.4 1993/04/20 12:59:03 nickc Exp $ */


#include <helios.h>	/* standard header */

#define __in_termios 1	/* flag that we are in this module */


#include "pposix.h"
#include <termios.h>
#include <attrib.h>

#define	CTRL(c)	(c&037)

extern int cf_getospeed(struct termios *p)
{ 
	CHECKSIGS();
	return (int) GetOutputSpeed((Attributes *)p); 
}

extern int cf_setospeed(struct termios *p, int speed)
{
	CHECKSIGS();
	SetOutputSpeed((Attributes *)p,speed); return 0; 
}

extern int cf_getispeed(struct termios *p)
{
	CHECKSIGS();
	return (int) GetInputSpeed((Attributes *)p);
}

extern int cf_setispeed(struct termios *p, int speed)
{
	CHECKSIGS();
	SetInputSpeed((Attributes *)p,speed); return 0;
}


extern int tcgetattr(int fd, struct termios *p)
{
	Stream *s;
	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return -1;
	
	s = f->pstream->stream;

	if( GetAttributes(s,(Attributes *)p) < 0 )
	{ errno = posix_error(Result2(s)); return -1; }

	/* These bits are really ConsoleRawInput and ConsoleRawOutput */
	/* so we must invert them to mean what POSIX thinks.	      */
	
	p->c_lflag ^= ICANNON;
	p->c_oflag ^= OPOST;

	/* now setup c_cc vector */
	p->c_cc[VINTR] = CTRL('c');
	p->c_cc[VQUIT] = 034;
	p->c_cc[VSUSP] = CTRL('z');
	p->c_cc[VSTART] = CTRL('q');
	p->c_cc[VSTOP] = CTRL('s');

	if( p->c_lflag & ICANNON )
	{
		p->c_cc[VEOF] = CTRL('d');
		p->c_cc[VEOL] = '\n';
		p->c_cc[VERASE] = 0177;
		p->c_cc[VKILL] = CTRL('u');		
	}
	else
	{
		p->c_cc[VMIN] = p->c_min;
		p->c_cc[VTIME] = p->c_time;
	}
	
	CHECKSIGS();
	return 0;
}

extern int tcsetattr(int fd, int actions, struct termios *p)
{
	Stream *s;

	fdentry *f;
	
	CHECKSIGS();
	if((f = checkfd(fd)) == NULL ) return -1;

	actions = actions;
	
	s = f->pstream->stream;

	p->c_lflag ^= ICANNON;
	p->c_oflag ^= OPOST;

	p->c_min = p->c_cc[VMIN];
	p->c_time = p->c_cc[VTIME];
	
	if( SetAttributes(s,(Attributes *)p) < 0 )
	{ errno = posix_error(Result2(s)); return -1; }

	CHECKSIGS();
	return 0;
}

extern int tcsendbreak(int fd, int duration)
{
	fd = fd; duration = duration;
	/* @@@ tcsendbreak */
	CHECKSIGS();
	return 0; 
}

extern int tcdrain(int fd)
{
	fd = fd;
	/* @@@ tcdrain */
	CHECKSIGS();
	return 0;
}

extern int tcflush(int fd, int qselect)
{
	fd = fd; qselect = qselect;
	/* @@@ tcflush */
	CHECKSIGS();
	return 0;
}

extern int tcflow(int fd, int action)
{
	fd = fd; action = action;
	/* @@@ tcflow */
	CHECKSIGS();
	errno = EINVAL; return -1;
}

/* end of termios.c */
