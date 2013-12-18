/* sys/errno.h: BSD compatibility header				*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* RcsId: $Id: errno.h,v 1.2 1991/02/13 12:25:33 nick Exp $ */

#ifdef _BSD

#ifndef __errno_h


#ifndef __CROSSCOMP
#  include "/helios/include/errno.h"
#else
#  ifndef __sys_errno_h
#    define __sys_errno_h
#    include <errno.h>
#  else
#    include <../errno.h>
#  endif
#endif

#endif

#else
#error sys/errno.h included without _BSD set
#endif

