/*
 * Do all necessary includes here, so that we don't have to worry about
 * overlapping includes in the files in missing.d.
 */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#ifndef VAXC
#include <fcntl.h>
#include <sys/types.h>
#else	/*VAXC (VMS)*/
#include <file.h>
#include <types.h>
#endif
#include <varargs.h>

#include "config.h"

#ifdef TZSET_MISSING
#include <sys/time.h>
#else
#include <time.h>
#endif

#ifdef atarist
/*
 * this will work with gcc compiler - for other compilers you may
 * have to replace path separators in this file into backslashes
 */
#include "atari/stack.c"
#include "atari/tmpnam.c"
#include "atari/textrd.c"	/* gnulib bug fix */
#endif /* atarist */

#ifdef SYSTEM_MISSING
#ifdef atarist
#include "atari/system.c"
#else
#include "missing/system.c"
#endif
#endif /* SYSTEM_MISSING */

#ifdef GETOPT_MISSING
#include "missing/getopt.c"
#endif	/* GETOPT_MISSING */

#ifdef MEMCMP_MISSING
#include "missing/memcmp.c"
#endif	/* MEMCMP_MISSING */

#ifdef MEMCPY_MISSING
#include "missing/memcpy.c"
#endif	/* MEMCPY_MISSING */

#ifdef MEMSET_MISSING
#include "missing/memset.c"
#endif	/* MEMSET_MISSING */

#ifdef RANDOM_MISSING
#include "missing/random.c"
#endif	/* RANDOM_MISSING */

#ifdef STRCASE_MISSING
#include "missing/strcase.c"
#endif	/* STRCASE_MISSING */

#ifdef STRCHR_MISSING
#include "missing/strchr.c"
#endif	/* STRCHR_MISSING */

#ifdef STRERROR_MISSING
#include "missing/strerror.c"
#endif	/* STRERROR_MISSING */

#ifdef STRFTIME_MISSING
#include "missing/strftime.c"
#endif	/* STRFTIME_MISSING */

#ifdef STRTOD_MISSING
#include "missing/strtod.c"
#endif	/* STRTOD_MISSING */

#ifdef STRTOL_MISSING
#include "missing/strtol.c"
#endif	/* STRTOL_MISSING */

#if defined(VPRINTF_MISSING) && defined(BSDSTDIO)
#include "missing/vprintf.c"
#endif	/* VPRINTF_MISSING && BSDSTDIO */

#ifdef TZSET_MISSING
#include "missing/tzset.c"
#endif /* TZSET_MISSING */
