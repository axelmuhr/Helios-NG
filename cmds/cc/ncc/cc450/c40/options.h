/*
 * options.h -- compiler configuration options set at compile time
 * Copyright (C) 1991 Perihelion Software Ltd. 
 *  All rights reserved.
 * Copyright (C) Codemist Ltd. 1988
 */

#ifndef _options_LOADED
#define _options_LOADED

#define TARGET_SYSTEM   	"Helios"
#define TARGET_IS_HELIOS	1

#define DRIVER_OPTIONS 		{ "-D__C40", "-D__HELIOS", "-D__HELIOSC40", "-D__SMT", NULL }

#ifndef RELEASE_VSN
#  define ENABLE_ALL          	1 /* -- to enable all debugging options */
#endif

#ifdef FORTRAN
#define DRIVER_ENV { 0, ( KEY_UNIX | KEY_LINK ), 		   	\
		     (EXT_DOUBLECOMPLEX | EXT_HINTEGER | EXT_CASEFOLD |    	\
		      EXT_LCKEYWORDS | EXT_FREEFORMAT | EXT_IMPUNDEFINED | 	\
		      EXT_RECURSION | EXT_AUTO | EXT_HOLLERITH |           	\
		      EXT_TOPEXPRESS | EXT_F66 | EXT_MIXEDCOMM |	   	\
		      EXT_VMSCHARS | EXT_VMSCASTS | EXT_VMSIO | 	   	\
		      EXT_VMSTYPES | OPT_STATICLOCALS | OPT_NOARGALIAS),   	\
		       "/helios/include",		   			\
		       "/helios/include",		   			\
		       "/helios/include/iso", "/", "", "", "lst",		\
		       "as",						 	\
		       "asm", "a.out", "",	   				\
		       "/helios/lib/f0.o",                 			\
		       "/helios/lib/f0.o",                 			\
		       "/helios/lib/f0.o",                                 	\
		       "/helios/lib/libf.a /helios/lib/helios.lib", "",    	\
		       "/helios/lib/libf.a /helios/lib/helios.lib", "",		\
		       "-lpc"	                                           	\
    }
#else /* ! FORTRAN */
#ifdef __HELIOSC40	/* compiling native */
#define DRIVER_ENV {   0,	     			/* host_flags        */	\
		       ( KEY_UNIX | KEY_LINK ),		/* initial_flags     */	\
		       0,				/* initial_pragmax   */	\
		       "/helios/include",		/* default_ansi_path */ \
		       "/helios/include/pcc",		/* default_pcc_path  */	\
		       "/helios/include/pas",		/* default_pas_path  */	\
		       "/helios/lib",			/* lib directory     */	\
		       "/helios",			/* lib_root          */	\
		       "/helios",			/* pas_lib_root      */	\
		       "lst",				/* list_extension    */	\
		       "asm",				/* assembler_command */	\
		       "ld",				/* linker_command    */	\
		       "a.out",				/* output_file       */	\
		       "",				/* trailer           */	\
		       "/helios/lib/c0.o",		/* link_startup      */	\
		       "/helios/lib/c0.o",		/* profile_startup   */	\
		       "/helios/lib/c0.o",		/* profile_g_startup */	\
		       "/helios/lib/c.lib /helios/lib/helios.lib",	/* default_library   */	\
		       "", 				/* host_library	     */	\
		       "-lp",				/* profile_library   */	\
		       "-lf",				/* fortran_library   */	\
		       "-lpc"				/* pas_library       */	\
    }
#else			/* cross compiling */
#define DRIVER_ENV {   0,		   		/* host_flags        */	\
		       ( KEY_UNIX | KEY_LINK ),		/* initial_flags     */	\
		       0,				/* initial_pragmax   */	\
		       "/hsrc/include",			/* default_ansi_path */ \
		       "/hsrc/include/pcc",		/* default_pcc_path  */	\
		       "/hsrc/include/pas",		/* default_pas_path  */	\
		       "/hprod/C40/lib",		/* lib directory     */	\
		       "/hprod/C40",			/* lib_root          */	\
		       "/hprod/C40",			/* pas_lib_root      */	\
		       "lst",				/* list_extension    */	\
		       "asc40",				/* assembler_command */	\
		       "ldc40",				/* linker_command    */	\
		       "a.out",				/* output_file       */	\
		       "",				/* trailer           */	\
		       "/hprod/C40/lib/c0.o",		/* link_startup      */	\
		       "/hprod/C40/lib/c0.o",		/* profile_startup   */	\
		       "/hprod/C40/lib/c0.o",		/* profile_g_startup */	\
		       "/hprod/C40/lib/c.lib /hprod/C40/lib/helios.lib",	/* default_library   */	\
		       "",				/* host_library	     */	\
		       "-lp",				/* profile_library   */	\
		       "-lf",				/* fortran_library   */	\
		       "-lpc"				/* pas_library       */	\
    }
#endif


#endif /* FORTRAN */

#endif /* _options_LOADED */

/* end of ccc40/options.h */
