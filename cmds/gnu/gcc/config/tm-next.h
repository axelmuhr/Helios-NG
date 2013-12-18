/* Definitions for Next as target machine for GNU C compiler.  */

#include "tm-sun3.h"

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-Dmc68000 -DNeXT -Dunix -D__MACH__"

/* Assumes no need to run special floating-point initialization code.  */
#undef STARTFILE_SPEC
#define STARTFILE_SPEC "%{pg:gcrt0.o%s}%{!pg:%{p:mcrt0.o%s}%{!p:crt0.o%s}}"

#define CPLUSPLUS
#define DOLLARS_IN_IDENTIFIERS 1
