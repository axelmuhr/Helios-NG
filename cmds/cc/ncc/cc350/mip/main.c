
/* main.c:  C compiler driver file.  Copyright (C) Codemist Ltd. 1988 */
/* version 2a */

/* Compiling this file will builds a C compiler flat (by concatenating all  */
/* modules by #include).                                                    */
/* The target machine is as directed by the target.h file and a sensible    */
/* directory-like system together with a compiler option (-i in Norcroft)   */
/* for controlling search lists for #include "xxx.h" files will enable      */
/* multiple target compilers to be built with little pain.                  */
/* The file cc.c (q.v) just includes main.c as a basis for building (e.g)   */
/* multiple object formats.                                                 */

/*
 * See jopcode.h, which must have DEFINE_JOPTABLE defined once when it gets
 * loaded - when compiling everything as separate modules this is achieved by
 * leting just one module (it happens to be regalloc) set said flag.  Here
 * that will not work since jopcode.h gets included several times, and it
 * must have DEFINE_JOPTABLE set on the first of these (on subsequent scans
 * it disables itself using _jopcode_LOADED).  What a mess.
 * Similarly errors.h must be included first from misc.c to get the table
 * of error strings set up properly.
 */
  
#define DEFINE_JOPTABLE 1
#include "misc.c"

#include "version.c"
#include "pp.c"
#include "lex.c"
#include "syn.c"
#include "bind.c"
#include "sem.c"
#include "simplify.c"

#ifdef TARGET_HAS_IEEE
#  include "IEEEflt.c"
#elif defined TARGET_HAS_370_FP
#  include "s370flt.c"
#else
#  error No floating point mode defined - assume IEEE
#  include "IEEEflt.c"
#endif

#include "builtin.c"
#include "aetree.c"
#include "vargen.c"
#include "store.c"
#include "jopprint.c"
#include "codebuf.c"
#include "flowgraf.c"
#include "cg.c"
#include "cse.c"
#include "csescan.c"
#include "regsets.c"
#include "regalloc.c"   /* still a bit machine dependent */

/* machine dependent modules compiled last */

#include "mcdep.c"

#include "gen.c"        /* all systems have one of these */

#ifndef TARGET_IS_NULL
#  ifndef objfilename
#    ifdef TARGET_HAS_AOUT
#      define objfilename "aoutobj.c"
#    elif defined TARGET_HAS_COFF
#      define objfilename "coffobj.c"
#    elif defined TARGET_IS_HELIOS
#      define objfilename "heliobj.c"
#    endif
#  endif
/* Note that the next two lines are OK under ANSI C but will not work */
/* if your compiler fails to expand the files names, and earlier C    */
/* dialects may suffer from that.                                     */
/* In that case edit this file to include the files you realy want.   */
#  ifdef asmfilename
#    include asmfilename
#  else
#    include "asm.c"
#  endif
#  ifdef objfilename
#    include objfilename
#  else
#    include "obj.c"
#  endif
#endif

#ifdef TARGET_HAS_DEBUGGER
#  ifndef dbgfilename
#     define dbgfilename "dbg.c"
#  endif
/* Same as above note re #include [macroname that expands to "filename"] */
#  include dbgfilename
#endif

#include "compiler.c"
#include "fname.c"

#ifndef HOST_USES_CCOM_INTERFACE
#  include "driver.c"
#endif

/* End of compiler */
