/* termios.h : Posix library Terminal I/O control		*/
/* %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: termios.h,v 1.1 90/09/05 11:07:32 nick Exp $ */

#ifndef _termios_h
#define _termios_h

#include <attrib.h>

#define NCCS	9

#define VEOF	0
#define VEOL	1
#define VERASE	2
#define VINTR	3
#define	VKILL	4
#define VQUIT	5
#define VSUSP	6
#define VSTART	7
#define VSTOP	8

#define	VMIN	VEOF
#define VTIME	VEOL

/* defines for tcsetattr */
#define TCSANOW		1
#define TCSADRAIN	2
#define TCSADFLUSH	3

/* defines for tcflush */
#define	TCIFLUSH	1
#define TCOFLUSH	2
#define TCIOFLUSH	3

/* defines for tcflow	*/
#define	TCOOFF		1
#define TCOON		2
#define TCIOFF		4
#define TCION		8

/* This structure correseponds EXACTLY to a Helios Attributes strucuture */
struct termios {
	unsigned long	c_iflag;
	unsigned long	c_oflag;
	unsigned long	c_cflag;
	unsigned long	c_lflag;
	unsigned short	c_min;
	unsigned short	c_time;
	unsigned char	c_cc[NCCS];	/* MUST ADD THIS TO ATTRIBUTES */
};

/* Note that the following bits corrspond exactly to those defined in	*/
/* attrib.h. These two files must be kept in step.			*/

/* bits for c_iflag field */

#define BRKINT		0x00000200
#define ICRNL		0x00100000
#define IGNBRK		0x00000100
#define IGNCR		0x00000000
#define IGNPAR		0x00000800
#define INLCR		0x00200000
#define INPCK		0x00400000
#define ISTRIP		0x00010000
#define IXOFF		0x00004000
#define IXON		0x00008000
#define PARMRK		0x00001000

/* bits for c_oflag */
#define OPOST		0x00000100

/* bits for c_cflag */
#define CLOCAL		0x00002000
#define CREAD		0x00000200
#define CSIZE		0x0003C000
#define   CS5		0x00004000
#define   CS6		0x00008000
#define   CS7		0x00010000
#define   CS8		0x00020000
#define CSTOPB		0x00000100
#define HUPCL		0x00001000
#define PARENB		0x00000400
#define PARODD		0x00000800

/* bits for c_lflag */
#define ECHO		0x00000004
#define ECHOE		0x00010000
#define ECHOK		0x00020000
#define ECHONL		0x00040000
#define ICANNON		0x00000008
#define IEXTEN		0x00080000
#define ISIG		0x00100000
#define NOFLSH		0x00200000
#define TOSTOP		0x00400000

/* baud rates */

#define B0              0
#define B50             1
#define B75             2
#define B110            3
#define B134            4
#define B150            5
#define B200            6
#define B300            7
#define B600            8
#define B1200           9
#define B1800          10
#define B2400          11
#define B4800          12
#define B9600          13
#define B19200         14
#define B38400         15

extern int cf_getospeed(struct termios *t);
extern int cf_setospeed(struct termios *t, int speed);
extern int cf_getispeed(struct termios *t);
extern int cf_setispeed(struct termios *t, int speed);
/* 1003.1-1988 changed these names */
#define cfgetospeed cf_getospeed
#define cfsetospeed cf_setospeed
#define cfgetispeed cf_getispeed
#define cfsetispeed cf_setispeed
extern int tcgetattr(int fd, struct termios *t);
extern int tcsetattr(int fd, int actions, struct termios *t);
extern int tcsendbreak(int fd, int duration);
extern int tcdrain(int fd);
extern int tcflush(int fd, int qselect);
extern int tcflow(int fd, int action);
extern int tcgetpgrp(int fd);
extern int tcsetpgrp(int fd, int pid);

#endif

/* end of termios.h */
