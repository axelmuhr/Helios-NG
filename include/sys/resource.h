/* sys/resource.h: BSD compatability header				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: resource.h,v 1.1 90/09/05 11:09:20 nick Exp $ */

#ifdef _BSD

/* This is here ONLY so that we can maintain source compatibility with	*/
/* BSD programs which call wait3. 					*/

struct rusage { int dummy; };

#else
#error sys/resource.h included without _BSD set
#endif
