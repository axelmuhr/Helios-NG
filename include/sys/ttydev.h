/* sys/ttydev.h: BSD compatibility header				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: ttydev.h,v 1.1 90/09/05 11:09:29 nick Exp $ */

#ifdef _BSD

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

#else
#error sys/ttydev.h included without _BSD set
#endif

