/*
 * options.h -- compiler configuration options set at compile time
 * Copyright (C) Acorn Computers Ltd. 1988
 * Copyright (C) Codemist Ltd. 1988
 */

#ifndef _options_LOADED
#define _options_LOADED

#define TARGET_SYSTEM   "68K"

/* #define TARGET_IS_68020 */

#define HOSTS_WANTS_NO_BANNER 1

#define DRIVER_OPTIONS { "-D__M68K", "-fw", "-D__HELIOS", NULL }

#ifndef RELEASE_VSN
#  define ENABLE_ALL          1 /* -- to enable all debugging options */
#endif

#define DRIVER_ENV { 0, ( KEY_UNIX | KEY_LINK ), 0,	\
		        "/hsrc/include",		\
			"/hsrc/include",		\
			"/hsrc/include",		\
			"/hprod/C40/lib",		\
			"/hsrc/include",		\
			"/hsrc/include",		\
			"lst",				\
			"as",				\
		        "ldc40",			\
			"a.out",			\
			"",				\
			"/hprod/C40/lib/c0.o",		\
			"/hprod/C40/lib/c0.o",		\
			"/hprod/C40/lib/c0.o",		\
			"/hprod/C40/lib/c.lib /hprod/C40/lib/helios.lib", \
			"",				\
			"-lp",				\
			"",				\
			"-lpc"				\
    }


#endif

/* end of m68k/options.h */
