/*
 * cchelios/options.h -- compiler configuration options set at compile time
 * Copyright (C) Acorn Computers Ltd. 1988
 */

#ifndef _options_LOADED
#define _options_LOADED

#define TARGET_SYSTEM     "Helios"
#define TARGET_IS_HELIOS  1

#define UNIQUE_DATASEG_NAMES		1
#define GLOBAL_SOURCE_NAME		1
#define TARGET_LINKER_OMITS_DOLLAR	1
#define NO_DEBUGGER			1
#define TARGET_CALL_USES_DESCRIPTOR	1
#define EXTENSION_VALOF			1
#define NON_CODEMIST_MIDDLE_END		1
#define NO_CONFIG			1
#define HOST_USES_CCOM_INTERFACE	1
#define TARGET_HAS_DEBUGGER		1

/* do we want? #define TARGET_HAS_DIVREM_FUNCTION 1 */

/* ACN, please check these options (-wp suppresses #include <kernel.h>) */
/* do we kill -Darm?    			                        */
#ifndef DRIVER_OPTIONS
#  define DRIVER_OPTIONS  \
     {"-wp", "-D__TRAN", "-Dtran", "-D__HELIOS", "-D__HELIOSTRAN", NULL}
#endif

#ifndef RELEASE_VSN
#  define ENABLE_ALL          1 /* -- to enable all debugging options */
#endif

#ifndef offsetof
#  define offsetof(type, member) ((char *)&(((type *)0)->member) - (char *)0)
#endif
#  define size_t int

#endif

/* end of cchelios/options.h */
