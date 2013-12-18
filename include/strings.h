/* strings.h: BSD compatibility header					*/
/* SccsId: %W% %G% (C) Copyright 1990, Perihelion Software Ltd.		*/
/* $Id: strings.h,v 1.1 90/09/05 11:07:25 nick Exp $ */

#ifdef _BSD

#include <string.h>

#else
#error strings.h included without _BSD set
#endif
